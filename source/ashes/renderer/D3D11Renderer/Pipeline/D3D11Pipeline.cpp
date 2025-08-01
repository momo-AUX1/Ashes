#include "Pipeline/D3D11Pipeline.hpp"

#include "Core/D3D11Device.hpp"
#include "Pipeline/D3D11VertexInputState.hpp"
#include "Shader/D3D11ShaderModule.hpp"

#include "ashesd3d11_api.hpp"

namespace ashes::D3D11_NAMESPACE
{
	namespace
	{
		template<typename T>
		void doHashCombine( size_t & seed, T const & v )
		{
			const uint64_t kMul = 0x9ddfea08eb382d69ULL;

			std::hash< T > hasher;
			uint64_t a = ( hasher( v ) ^ seed ) * kMul;
			a ^= ( a >> 47 );

			uint64_t b = ( seed ^ a ) * kMul;
			b ^= ( b >> 47 );

			seed = std::size_t( b * kMul );
		}

		size_t doHash( VkVertexInputAttributeDescription const & desc )
		{
			size_t result = 0u;
			doHashCombine( result, desc.binding );
			doHashCombine( result, desc.format );
			doHashCombine( result, desc.location );
			doHashCombine( result, desc.offset );
			return result;
		}

		size_t doHash( VkVertexInputBindingDescription const & desc )
		{
			size_t result = 0u;
			doHashCombine( result, desc.binding );
			doHashCombine( result, desc.inputRate );
			doHashCombine( result, desc.stride );
			return result;
		}

		size_t doHash( VkPipelineVertexInputStateCreateInfo const & state )
		{
			size_t result = 0u;

			for ( auto & desc : makeArrayView( state.pVertexAttributeDescriptions, state.vertexAttributeDescriptionCount ) )
			{
				doHashCombine( result, doHash( desc ) );
			}

			for ( auto & desc : makeArrayView( state.pVertexBindingDescriptions, state.vertexBindingDescriptionCount ) )
			{
				doHashCombine( result, doHash( desc ) );
			}

			return result;
		}
	}

	Pipeline::Pipeline( VkDevice device
		, VkGraphicsPipelineCreateInfo createInfo )
		: m_device{ device }
		, m_layout{ createInfo.layout }
		, m_vertexInputState{ ( createInfo.pVertexInputState
			? deepCopy( *createInfo.pVertexInputState, m_vertexBindingDescriptions, m_vertexAttributeDescriptions )
			: VkPipelineVertexInputStateCreateInfo{} ) }
		, m_inputAssemblyState{ ( createInfo.pInputAssemblyState
			? deepCopy( *createInfo.pInputAssemblyState )
			: VkPipelineInputAssemblyStateCreateInfo{} ) }
		, m_viewportState{ ( createInfo.pViewportState
			? deepCopy( *createInfo.pViewportState, m_stateViewports, m_stateScissors )
			: VkPipelineViewportStateCreateInfo{} ) }
		, m_rasterizationState{ ( createInfo.pRasterizationState
			? deepCopy( *createInfo.pRasterizationState )
			: VkPipelineRasterizationStateCreateInfo{} ) }
		, m_multisampleState{ ( createInfo.pMultisampleState
			? deepCopy( *createInfo.pMultisampleState )
			: VkPipelineMultisampleStateCreateInfo{} ) }
		, m_depthStencilState{ ( createInfo.pDepthStencilState
			? Optional< VkPipelineDepthStencilStateCreateInfo >( deepCopy( *createInfo.pDepthStencilState ) )
			: ashes::nullopt ) }
		, m_colorBlendState{ ( createInfo.pColorBlendState
			? deepCopy( *createInfo.pColorBlendState, m_colorBlendStateAttachments )
			: VkPipelineColorBlendStateCreateInfo{} ) }
		, m_dynamicStates{ createInfo.pDynamicState }
		, m_scissors{ makeScissors( m_stateScissors.begin(), m_stateScissors.end() ) }
		, m_viewports{ makeViewports( m_stateViewports.begin(), m_stateViewports.end() ) }
		, m_vertexInputStateHash{ doHash( m_vertexInputState ) }
	{
		doCreateBlendState( device );

		if ( !hasDynamicStateEnable( VK_DYNAMIC_STATE_DEPTH_BIAS ) )
		{
			doCreateRasterizerState( device );
		}

		if ( !hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK )
			&& !hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_WRITE_MASK )
			&& !hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_REFERENCE ) )
		{
			doCreateDepthStencilState( device );
		}

		doCompileProgram( device, { createInfo.pStages, createInfo.pStages + createInfo.stageCount }, createInfo.flags );
		doCreateInputLayout( device );

		get( m_layout )->addPipeline( get( this ) );
	}

	Pipeline::Pipeline( VkDevice device
		, VkComputePipelineCreateInfo createInfo )
		: m_device{ device }
		, m_layout{ createInfo.layout }
		, m_dynamicStates{ nullptr }
	{
		doCompileProgram( device, { createInfo.stage }, createInfo.flags );

		get( m_layout )->addPipeline( get( this ) );
	}

	Pipeline::~Pipeline()noexcept
	{
		if ( m_layout )
		{
			get( m_layout )->removePipeline( get( this ) );
		}

		for ( auto & pcb : m_constantsPcbs )
		{
			deallocate( pcb.memory, get( m_device )->getAllocationCallbacks() );
			deallocate( pcb.ubo, get( m_device )->getAllocationCallbacks() );
		}

		safeRelease( m_bdState );
		safeRelease( m_rsState );
		safeRelease( m_iaState );
		safeRelease( m_dsState );
	}

	PushConstantsBuffer Pipeline::findPushConstantBuffer( PushConstantsDesc const & pushConstants )const
	{
		// Try to find a PCB that has the same flags, and the same size as the push constants.
		auto it = std::find_if( m_constantsPcbs.begin()
			, m_constantsPcbs.end()
			, [&pushConstants]( PushConstantsBuffer const & lookup )
			{
				return lookup.data.stageFlags == pushConstants.stageFlags
					&& lookup.data.size == pushConstants.offset + pushConstants.size;
			} );

		if ( it == m_constantsPcbs.end() )
		{
			// Try a PCB that has the same flags, but is larger than the push constants.
			it = std::find_if( m_constantsPcbs.begin()
				, m_constantsPcbs.end()
				, [&pushConstants]( PushConstantsBuffer const & lookup )
				{
					return lookup.data.stageFlags == pushConstants.stageFlags
						&& lookup.data.size > pushConstants.offset + pushConstants.size;
				} );
		}

		if ( it == m_constantsPcbs.end() )
		{
			// Try a PCB that contains the flags of the push constants.
			it = std::find_if( m_constantsPcbs.begin()
				, m_constantsPcbs.end()
				, [&pushConstants]( PushConstantsBuffer const & lookup )
				{
					return checkFlag( lookup.data.stageFlags, pushConstants.stageFlags )
						&& lookup.data.size == pushConstants.offset + pushConstants.size;
				} );
		}

		if ( it == m_constantsPcbs.end() )
		{
			// Try a PCB that contains the flags of the push constants, and is larger than them.
			it = std::find_if( m_constantsPcbs.begin()
				, m_constantsPcbs.end()
				, [&pushConstants]( PushConstantsBuffer const & lookup )
				{
					return checkFlag( lookup.data.stageFlags, pushConstants.stageFlags )
						&& lookup.data.size > pushConstants.offset + pushConstants.size;
				} );
		}

		if ( it != m_constantsPcbs.end() )
		{
			return PushConstantsBuffer
			{
				it->ubo,
				it->location,
				pushConstants
			};
		}

		static PushConstantsBuffer const dummy{};
		return dummy;
	}

	VkDescriptorSetLayoutArray const & Pipeline::getDescriptorsLayouts()const
	{
		return get( m_layout )->getDescriptorsLayouts();
	}

	void Pipeline::update()
	{
		if ( hasDynamicStateEnable( VK_DYNAMIC_STATE_DEPTH_BIAS )
			&& ( !m_rsState || m_dynamicStates.isDirty( VK_DYNAMIC_STATE_DEPTH_BIAS ) ) )
		{
			auto & depthBias = m_dynamicStates.getDepthBias();
			m_rasterizationState.depthBiasConstantFactor = depthBias.constantFactor;
			m_rasterizationState.depthBiasClamp = depthBias.clamp;
			m_rasterizationState.depthBiasSlopeFactor = depthBias.slopeFactor;
			m_rasterizationState.depthBiasEnable = VK_TRUE;
			doCreateRasterizerState( m_device );
		}

		if ( ( hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK )
				|| hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_WRITE_MASK )
				|| hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_REFERENCE ) )
			&& ( !m_dsState
				|| m_dynamicStates.isDirty( VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK )
				|| m_dynamicStates.isDirty( VK_DYNAMIC_STATE_STENCIL_WRITE_MASK )
				|| m_dynamicStates.isDirty( VK_DYNAMIC_STATE_STENCIL_REFERENCE ) ) )
		{
			if ( m_depthStencilState.has_value() )
			{
				if ( hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK ) )
				{
					m_depthStencilState.value().front.compareMask = m_dynamicStates.getFrontStencilCompareMask();
					m_depthStencilState.value().back.compareMask = m_dynamicStates.getBackStencilCompareMask();
				}

				if ( hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_WRITE_MASK ) )
				{
					m_depthStencilState.value().front.writeMask = m_dynamicStates.getFrontStencilWriteMask();
					m_depthStencilState.value().back.writeMask = m_dynamicStates.getBackStencilWriteMask();
				}

				if ( hasDynamicStateEnable( VK_DYNAMIC_STATE_STENCIL_WRITE_MASK ) )
				{
					m_depthStencilState.value().front.reference = m_dynamicStates.getFrontStencilReference();
					m_depthStencilState.value().back.reference = m_dynamicStates.getBackStencilReference();
				}

				doCreateDepthStencilState( m_device );
			}
			else
			{
				reportError( get( this )
					, VK_ERROR_INITIALIZATION_FAILED
					, "Initialisation failed"
					, "VkPipeline doesn't have a depth stencil state" );
			}
		}
	}

	void Pipeline::doCreateBlendState( VkDevice device )
	{
		auto d3ddevice = get( device )->getDevice();
		auto blendDesc = convert( m_colorBlendState );

		if ( HRESULT hr = d3ddevice->CreateBlendState( &blendDesc, &m_bdState );
			!checkError( device, hr, "CreateBlendState" ) )
		{
			get( device )->onReportMessage( VK_DEBUG_REPORT_ERROR_BIT_EXT
				, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT
				, uint64_t( get( this ) )
				, 0u
				, VK_ERROR_INCOMPATIBLE_DRIVER
				, "Direct3D11"
				, "CreateBlendState() failed" );
			return;
		}

		dxDebugName( m_bdState, PipelineBlendState );
	}

	void Pipeline::doCreateRasterizerState( VkDevice device )
	{
		auto d3ddevice = get( device )->getDevice();
		auto rasterizerDesc = convert( m_rasterizationState
			, m_multisampleState );

		if ( auto hr = d3ddevice->CreateRasterizerState( &rasterizerDesc, &m_rsState );
			!checkError( device, hr, "CreateRasterizerState" ) )
		{
			get( device )->onReportMessage( VK_DEBUG_REPORT_ERROR_BIT_EXT
				, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT
				, uint64_t( get( this ) )
				, 0u
				, VK_ERROR_INCOMPATIBLE_DRIVER
				, "Direct3D11"
				, "CreateRasterizerState() failed" );
			return;
		}

		dxDebugName( m_rsState, PipelineRasterizerState );
	}

	void Pipeline::doCreateDepthStencilState( VkDevice device )
	{
		if ( m_depthStencilState )
		{
			auto d3ddevice = get( device )->getDevice();
			auto depthStencilDesc = convert( *m_depthStencilState );

			if ( auto hr = d3ddevice->CreateDepthStencilState( &depthStencilDesc, &m_dsState );
				!checkError( device, hr, "CreateDepthStencilState" ) )
			{
				get( device )->onReportMessage( VK_DEBUG_REPORT_ERROR_BIT_EXT
					, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT
					, uint64_t( get( this ) )
					, 0u
					, VK_ERROR_INCOMPATIBLE_DRIVER
					, "Direct3D11"
					, "CreateDepthStencilState() failed" );
				return;
			}

			dxDebugName( m_dsState, PipelineDepthStencilState );
		}
	}

	void Pipeline::doCompileProgram( VkDevice device
		, VkPipelineShaderStageCreateInfoArray const & stages
		, VkPipelineCreateFlags createFlags )
	{
		for ( auto const & state : stages )
		{
			auto shaderModule = get( state.module );
			m_programModules.emplace_back( shaderModule->compile( state, m_layout, createFlags ) );
			m_programLayout.try_emplace( state.stage, m_programModules.back().getLayout() );
		}

		for ( auto const & [stage, desc] : m_programLayout )
		{
			for ( auto const & blockLayout : desc.interfaceBlockLayout )
			{
				PushConstantsBuffer pcb
				{
					nullptr,
					blockLayout.binding,
					{
						VkShaderStageFlags( stage ),
						0u,
						blockLayout.size
					},
					nullptr,
				};
				allocate( pcb.ubo
					, get( device )->getAllocationCallbacks()
					, device
					, VkBufferCreateInfo
					{
						VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
						nullptr,
						0u,
						blockLayout.size,
						( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
							| VK_BUFFER_USAGE_TRANSFER_DST_BIT
							| VK_BUFFER_USAGE_TRANSFER_SRC_BIT ),
						VK_SHARING_MODE_EXCLUSIVE,
						0u,
						nullptr,
					} );
				auto requirements = get( pcb.ubo )->getMemoryRequirements();
				auto deduced = deduceMemoryType( requirements.memoryTypeBits
					, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
				allocate( pcb.memory
					, get( device )->getAllocationCallbacks()
					, m_device
					, VkMemoryAllocateInfo
					{
						VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
						nullptr,
						requirements.size,
						deduced
					} );
				get( pcb.ubo )->bindMemory( pcb.memory, 0u );
				m_constantsPcbs.push_back( std::move( pcb ) );
			}
		}
	}

	void Pipeline::doCreateInputLayout( VkDevice device )
	{
		auto it = m_programLayout.find( VK_SHADER_STAGE_VERTEX_BIT );

		if ( it != m_programLayout.end() )
		{
			auto compiled = it->second.shaderModule->getCompiled();
			auto const & inputLayout = it->second.inputLayout;
			auto d3ddevice = get( device )->getDevice();
			auto inputDesc = convert( m_vertexInputState, inputLayout );

			if ( !inputDesc.empty() )
			{
				if ( auto hr = d3ddevice->CreateInputLayout( inputDesc.data()
						, UINT( inputDesc.size() )
						, compiled->GetBufferPointer()
						, compiled->GetBufferSize()
						, &m_iaState );
					!checkError( device, hr, "CreateInputLayout" ) )
				{
					get( device )->onReportMessage( VK_DEBUG_REPORT_ERROR_BIT_EXT
						, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT
						, uint64_t( get( this ) )
						, 0u
						, VK_ERROR_INCOMPATIBLE_DRIVER
						, "Direct3D11"
						, "CreateInputLayout() failed" );
					return;
				}

				dxDebugName( m_iaState, PipelineInputLayout );
			}
		}
	}
}
