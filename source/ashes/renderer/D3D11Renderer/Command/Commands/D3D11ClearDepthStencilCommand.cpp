/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#include "Command/Commands/D3D11ClearDepthStencilCommand.hpp"

#include "Image/D3D11ImageView.hpp"

#include "ashesd3d11_api.hpp"

namespace ashes::D3D11_NAMESPACE
{
	namespace
	{
		VkImageViewArray createViews( VkDevice device
			, VkImage image
			, VkImageSubresourceRangeArray ranges )
		{
			VkImageViewArray results;
			results.resize( ranges.size() );
			uint32_t index = 0u;

			for ( auto & result : results )
			{
				allocate( result
					, get( device )->getAllocationCallbacks()
					, device
					, VkImageViewCreateInfo
					{
						VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						nullptr,
						0u,
						image,
						VK_IMAGE_VIEW_TYPE_2D,
						get( image )->getFormat(),
						VkComponentMapping{},
						std::move( ranges[index] ),
					} );
				++index;
			}

			return results;
		}
	}

	ClearDepthStencilCommand::ClearDepthStencilCommand( VkDevice device
		, VkImage image
		, ArrayView< VkImageSubresourceRange const > ranges
		, VkClearDepthStencilValue value )
		: CommandBase{ device }
		, m_image{ image }
		, m_ranges{ ranges.begin(), ranges.end() }
		, m_value{ value }
		, m_flags{ ( isDepthFormat( get( image )->getFormat() )
				? D3D11_CLEAR_DEPTH
				: 0u )
			| ( isStencilFormat( get( image )->getFormat() )
				? D3D11_CLEAR_STENCIL
				: 0u ) }
		, m_views{ createViews( getDevice(), m_image, m_ranges ) }
	{
	}

	ClearDepthStencilCommand::ClearDepthStencilCommand( ClearDepthStencilCommand const & rhs )
		: CommandBase{ rhs.getDevice() }
		, m_image{ rhs.m_image }
		, m_ranges{ rhs.m_ranges }
		, m_value{ rhs.m_value }
		, m_flags{ rhs.m_flags }
		, m_views{ createViews( getDevice(), m_image, m_ranges ) }
	{
	}

	ClearDepthStencilCommand & ClearDepthStencilCommand::operator=( ClearDepthStencilCommand const & rhs )
	{
		for ( auto const & view : m_views )
		{
			deallocate( view, get( getDevice() )->getAllocationCallbacks() );
		}

		CommandBase::operator=( rhs );
		m_image = rhs.m_image;
		m_ranges = rhs.m_ranges;
		m_value = rhs.m_value;
		m_flags = rhs.m_flags;
		m_views = createViews( getDevice(), m_image, m_ranges );

		return *this;
	}

	ClearDepthStencilCommand::~ClearDepthStencilCommand()noexcept
	{
		for ( auto const & view : m_views )
		{
			deallocate( view, get( getDevice() )->getAllocationCallbacks() );
		}
	}

	void ClearDepthStencilCommand::apply( Context const & context )const
	{
		for ( auto const & view : m_views )
		{
			context.context->ClearDepthStencilView( get( view )->getDepthStencilView()
				, m_flags
				, m_value.depth
				, UINT8( m_value.stencil ) );
		}
	}

	CommandPtr ClearDepthStencilCommand::clone()const
	{
		return std::make_unique< ClearDepthStencilCommand >( *this );
	}
}
