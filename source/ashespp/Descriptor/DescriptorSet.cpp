/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#include "ashespp/Descriptor/DescriptorSet.hpp"

#include "ashespp/Buffer/Buffer.hpp"
#include "ashespp/Buffer/BufferView.hpp"
#include "ashespp/Buffer/UniformBuffer.hpp"
#include "ashespp/Core/Device.hpp"
#include "ashespp/Descriptor/DescriptorPool.hpp"
#include "ashespp/Descriptor/DescriptorSetLayout.hpp"
#include "ashespp/Image/ImageView.hpp"
#include "ashespp/Image/Sampler.hpp"

namespace ashes
{
	DescriptorSet::DescriptorSet( Device const & device
		, DescriptorPool const & pool
		, DescriptorSetLayout const & layout
		, uint32_t bindingPoint
		, void const * pNext )
		: DescriptorSet{ device, "DescriptorSet", pool, layout, bindingPoint, pNext }
	{
	}

	DescriptorSet::DescriptorSet( Device const & device
		, std::string const & debugName
		, DescriptorPool const & pool
		, DescriptorSetLayout const & layout
		, uint32_t bindingPoint
		, void const * pNext )
		: VkObject{ debugName }
		, m_device{ device }
		, m_pool{ pool }
		, m_layout{ layout }
		, m_bindingPoint{ bindingPoint }
	{
		VkDescriptorSetLayout vkLayout = m_layout;
		VkDescriptorSetAllocateInfo allocateInfo
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			pNext,
			pool,
			1u,
			&vkLayout
		};
		DEBUG_DUMP( allocateInfo );
		auto res = m_device.vkAllocateDescriptorSets( m_device
			, &allocateInfo
			, &m_internal );
		checkError( res, "DescriptorSet allocation" );
		registerObject( m_device, debugName, *this );
	}

	DescriptorSet::~DescriptorSet()noexcept
	{
		unregisterObject( m_device, *this );

		if ( !m_pool.hasAutomaticFree() )
		{
			m_device.vkFreeDescriptorSets( m_device
				, m_pool
				, 1u
				, &m_internal );
		}
	}

	void DescriptorSet::update()const noexcept
	{
		for ( auto const & write : m_writes )
		{
			write.update( m_internal );
		}

		updateBindings( m_writes );
	}

	void DescriptorSet::setBindings( WriteDescriptorSetArray bindings )
	{
		m_writes = std::move( bindings );
	}

	void DescriptorSet::setBindings( VkWriteDescriptorSetArray const & bindings )
	{
		m_writes.clear();

		for ( auto binding : bindings )
		{
			WriteDescriptorSet write{ binding.dstBinding
				, binding.dstArrayElement
				, binding.descriptorCount
				, binding.descriptorType };

			if ( binding.pImageInfo )
			{
				for ( uint32_t i = 0; i < binding.descriptorCount; ++i )
				{
					write.imageInfo.emplace_back( *binding.pImageInfo );
					++binding.pImageInfo;
				}
			}

			if ( binding.pBufferInfo )
			{
				for ( uint32_t i = 0; i < binding.descriptorCount; ++i )
				{
					write.bufferInfo.emplace_back( *binding.pBufferInfo );
					++binding.pBufferInfo;
				}
			}

			if ( binding.pTexelBufferView )
			{
				for ( uint32_t i = 0; i < binding.descriptorCount; ++i )
				{
					write.texelBufferView.emplace_back( *binding.pTexelBufferView );
					++binding.pTexelBufferView;
				}
			}

			m_writes.emplace_back( write );
		}
	}

	void DescriptorSet::updateBindings( WriteDescriptorSetArray const & bindings )const noexcept
	{
		for ( auto & write : bindings )
		{
			write.update( *this );
		}

		updateBindings( ashes::makeVkArray< VkWriteDescriptorSet >( bindings ) );
	}

	void DescriptorSet::updateBindings( VkWriteDescriptorSetArray const & bindings )const noexcept
	{
		DEBUG_DUMP( vkwrites );
		m_device.vkUpdateDescriptorSets( m_device
			, static_cast< uint32_t >( bindings.size() )
			, bindings.data()
			, 0
			, nullptr );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, VkImageView view
		, VkSampler sampler
		, VkImageLayout layout
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ sampler, view, layout } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, ImageView const & view
		, Sampler const & sampler
		, VkImageLayout layout
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ sampler, view, layout } );
	}

	void DescriptorSet::createSamplerBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, VkSampler sampler
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_SAMPLER );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ sampler, VkImageView{}, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } );
	}

	void DescriptorSet::createSamplerBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, Sampler const & sampler
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_SAMPLER );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ sampler, VkImageView{}, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, VkImageView view
		, VkImageLayout layout
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ VkSampler{}, view, layout } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, ImageView const & view
		, VkImageLayout layout
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ VkSampler{}, view, layout } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, VkImageView view
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ VkSampler{}, view, VK_IMAGE_LAYOUT_GENERAL } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, ImageView const & view
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE );
		m_writes.back().imageInfo.push_back( VkDescriptorImageInfo{ VkSampler{}, view, VK_IMAGE_LAYOUT_GENERAL } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, UniformBuffer const & uniformBuffer
		, uint32_t offset
		, uint32_t range
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo
			{
				uniformBuffer.getBuffer(),
				offset * uniformBuffer.getAlignedSize( uniformBuffer.getElementSize() ),
				range * uniformBuffer.getAlignedSize( uniformBuffer.getElementSize() )
			} );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, VkBuffer buffer
		, uint32_t offset
		, uint32_t range
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, layoutBinding.descriptorType );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo{ buffer, offset, range } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, BufferBase const & buffer
		, uint32_t offset
		, uint32_t range
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, ( ( buffer.getUsage() & VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT )
				? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
				: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ) );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo{ buffer, offset, range } );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, VkBuffer buffer
		, VkBufferView view
		, uint32_t offset
		, uint32_t range
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, layoutBinding.descriptorType );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo{ buffer, offset, range } );
		m_writes.back().texelBufferView.push_back( view );
	}

	void DescriptorSet::createBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, BufferBase const & buffer
		, BufferView const & view
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, ( ( buffer.getUsage() & VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT )
				? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
				: VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ) );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo{ buffer, view.getOffset(), view.getRange() } );
		m_writes.back().texelBufferView.push_back( view );
	}

	void DescriptorSet::createDynamicBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, UniformBuffer const & uniformBuffer
		, uint32_t offset
		, uint32_t range
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo
			{
				uniformBuffer.getBuffer(),
				offset * uniformBuffer.getAlignedSize( uniformBuffer.getElementSize() ),
				range * uniformBuffer.getAlignedSize( uniformBuffer.getElementSize() )
			} );
	}

	void DescriptorSet::createDynamicBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, VkBuffer buffer
		, uint32_t offset
		, uint32_t range
		, uint32_t index )
	{

		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, layoutBinding.descriptorType );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo{ buffer, offset, range } );
	}

	void DescriptorSet::createDynamicBinding( VkDescriptorSetLayoutBinding const & layoutBinding
		, BufferBase const & buffer
		, uint32_t offset
		, uint32_t range
		, uint32_t index )
	{
		m_writes.emplace_back( layoutBinding.binding
			, index
			, 1u
			, ( ( buffer.getUsage() & VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT )
				? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
				: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ) );
		m_writes.back().bufferInfo.push_back( VkDescriptorBufferInfo{ buffer, offset, range } );
	}
}
