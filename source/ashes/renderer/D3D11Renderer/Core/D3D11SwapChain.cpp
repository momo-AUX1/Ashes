#include "Core/D3D11SwapChain.hpp"

#include "Core/D3D11Device.hpp"
#include "Core/D3D11Instance.hpp"
#include "Core/D3D11PhysicalDevice.hpp"
#include "Core/D3D11Surface.hpp"
#include "Image/D3D11Image.hpp"
#include "RenderPass/D3D11FrameBuffer.hpp"
#include "RenderPass/D3D11RenderPass.hpp"
#include "Sync/D3D11Semaphore.hpp"

#include "ashesd3d11_api.hpp"

#pragma warning( push )
#pragma warning( disable: 5262 )
#include <sstream>
#if defined( Ashes_D3D11_XBox )
#include <winrt/windows.ui.core.h>
#endif
#pragma warning( pop )

namespace ashes::D3D11_NAMESPACE
{
	namespace
	{
		VkImage createImage( VkDevice device
			, VkFormat format
			, VkExtent2D dimensions
			, VkDeviceMemory & deviceMemory )
		{
			VkImage result;
			allocate( result
				, get( device )->getAllocationCallbacks()
				, device
				, format
				, std::move( dimensions )
				, nullptr );
			auto requirements = get( result )->getMemoryRequirements();
			uint32_t deduced = deduceMemoryType( requirements.memoryTypeBits
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
			allocate( deviceMemory
				, get( device )->getAllocationCallbacks()
				, device
				, VkMemoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, requirements.size, deduced } );
			get( result )->bindMemory( deviceMemory, 0u );
			return result;
		}

		VkImageView createImageView( VkDevice device
			, VkImage image
			, VkFormat format )
		{
			VkImageView result;
			allocate( result
				, get( device )->getAllocationCallbacks()
				, device
				, VkImageViewCreateInfo
				{
					VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					nullptr,
					0,
					image,
					VK_IMAGE_VIEW_TYPE_2D,
					format,
					VkComponentMapping{},
					VkImageSubresourceRange{ VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u },
				} );
			return result;
		}
	}

	SwapchainKHR::SwapchainKHR( VkDevice device
		, VkSwapchainCreateInfoKHR createInfo )
		: m_device{ device }
		, m_createInfo{ std::move( createInfo ) }
	{
		try
		{
			doInitPresentParameters();
			auto factory = get( get( m_device )->getInstance() )->getDXGIFactory();
			auto d3ddevice = get( m_device )->getDevice();
#if defined( Ashes_D3D11_XBox )

			auto window = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread();
			HRESULT hr = static_cast< IDXGIFactory2 * >( factory )->CreateSwapChainForCoreWindow( d3ddevice
				, static_cast< IUnknown * >( winrt::get_abi( window ) )
				, &m_presentDesc
				, nullptr
				, reinterpret_cast< IDXGISwapChain1 ** >( &m_swapChain ) );

			if ( !checkError( m_device, hr, "CreateSwapChain" )
				|| !m_swapChain )
			{
				throw ashes::BaseException{ "Could not create the swapchain" };
			}

#else

			HRESULT hr = factory->CreateSwapChain( d3ddevice
				, &m_presentDesc
				, &m_swapChain );

			if ( !checkError( m_device, hr, "CreateSwapChain" )
				|| !m_swapChain )
			{
				throw ashes::BaseException{ "Could not create the swapchain" };
			}

			if ( get( m_createInfo.surface )->isDisplay() )
			{
				hr = m_swapChain->SetFullscreenState( TRUE, get( get( device )->getPhysicalDevice() )->getOutput() );

				if ( !checkError( m_device, hr, "SetFullscreenState" ) )
				{
					throw ashes::BaseException{ "Could not set the swapchain to fullscreen mode" };
				}

				hr = m_swapChain->ResizeTarget( &m_displayMode );

				if ( !checkError( m_device, hr, "ResizeTarget" ) )
				{
					m_swapChain->SetFullscreenState( FALSE, get( get( device )->getPhysicalDevice() )->getOutput() );
					throw ashes::BaseException{ "Could not resize the swapchain" };
				}
			}

#endif
			dxDebugName( m_swapChain, SwapChain );
			ID3D11Texture2D * rtTex = nullptr;
			hr = m_swapChain->GetBuffer( 0
				, __uuidof( ID3D11Texture2D )
				, reinterpret_cast< void ** >( &rtTex ) );

			if ( !checkError( m_device, hr, "SwapChain::GetBuffer" ) )
			{
				throw ashes::BaseException( "GetBuffer() failed" );
			}

			m_swapchainExtent = VkExtent2D{ m_displayMode.Width, m_displayMode.Height };
			allocate( m_swapChainImage
				, get( device )->getAllocationCallbacks()
				, m_device
				, m_createInfo.imageFormat
				, m_swapchainExtent
				, rtTex );
			dxDebugName( get( m_swapChainImage )->getResource(), SwapChainImage );

			m_windowExtent = m_createInfo.imageExtent;
			m_image = createImage( device
				, m_createInfo.imageFormat
				, m_windowExtent
				, m_deviceMemory );
			dxDebugName( get( m_image )->getResource(), SwapChainFakeImage );
			m_view = createImageView( device
				, m_image
				, m_createInfo.imageFormat );
			dxDebugName( get( m_view )->getRenderTargetView(), SwapChainFakeImageView );
		}
		catch ( std::exception & exc )
		{
			deallocate( m_view, get( m_device )->getAllocationCallbacks() );
			deallocate( m_image, get( m_device )->getAllocationCallbacks() );
			deallocate( m_swapChainImage, get( m_device )->getAllocationCallbacks() );
			safeRelease( m_swapChain );

			std::stringstream stream;
			stream << "Swapchain creation failed: " << exc.what() << std::endl;
			get( device )->onReportMessage( VK_DEBUG_REPORT_ERROR_BIT_EXT
				, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT
				, uint64_t( device )
				, 0u
				, VK_ERROR_INCOMPATIBLE_DRIVER
				, "Direct3D11"
				, stream.str().c_str() );
		}
	}

	SwapchainKHR::~SwapchainKHR()noexcept
	{
		deallocate( m_view, get( m_device )->getAllocationCallbacks() );
		deallocate( m_image, get( m_device )->getAllocationCallbacks() );
		deallocate( m_swapChainImage, get( m_device )->getAllocationCallbacks() );
		safeRelease( m_swapChain );
	}

	uint32_t SwapchainKHR::getImageCount()const
	{
		return 1u;
	}

	VkImageArray SwapchainKHR::getImages()const
	{
		VkImageArray result;
		result.emplace_back( m_image );
		return result;
	}

	VkResult SwapchainKHR::present()const
	{
		auto context{ get( m_device )->getImmediateContext() };
		D3D11_BOX srcBox{};
		srcBox.right = m_windowExtent.width;
		srcBox.bottom = m_windowExtent.height;
		srcBox.back = 1u;
		context->CopySubresourceRegion( get( m_swapChainImage )->getResource()
			, 0u
			, 0u
			, 0u
			, 0u
			, get( m_image )->getResource()
			, 0u
			, &srcBox );
		return checkError( m_device, getSwapChain()->Present( 0u, 0u ), "Presentation" )
			? VK_SUCCESS
			: VK_ERROR_SURFACE_LOST_KHR;
	}

	VkResult SwapchainKHR::acquireNextImage( uint32_t & imageIndex )const
	{
		imageIndex = 0u;
		return VK_SUCCESS;
	}

	void SwapchainKHR::doInitPresentParameters()
	{
		auto caps = get( m_createInfo.surface )->getCapabilities( get( m_device )->getPhysicalDevice() );
		m_windowExtent = caps.maxImageExtent;
		m_displayMode = get( m_createInfo.surface )->getMatchingDesc( m_createInfo.imageFormat );
#if defined( Ashes_D3D11_XBox )

		// Initialize the swap chain description.
		auto result = DXGI_SWAP_CHAIN_DESC1{};

		// Use triple buffering.
		result.BufferCount = 3;

		// Set the back buffer format and size.
		result.Width = m_createInfo.imageExtent.width;
		result.Height = m_createInfo.imageExtent.height;
		result.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		result.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		// Turn multisampling off.
		result.SampleDesc.Count = 1;
		result.SampleDesc.Quality = 0;

		result.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		result.Scaling = DXGI_SCALING_STRETCH;
		result.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		result.Flags = 0;

#else

		auto hWnd = get( m_createInfo.surface )->getHwnd();

		// Initialize the swap chain description.
		auto result = DXGI_SWAP_CHAIN_DESC{};

		// Set to a single back buffer.
		result.BufferCount = 1;

		// Set the back buffer desc to the surface matching one.
		result.BufferDesc = m_displayMode;
		// Except for the dimensions.
		result.BufferDesc.Width = m_createInfo.imageExtent.width;
		result.BufferDesc.Height = m_createInfo.imageExtent.height;

		// Set the usage of the back buffer.
		result.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		// Set the handle for the window to render to.
		result.OutputWindow = hWnd;

		// Turn multisampling off.
		result.SampleDesc.Count = 1;
		result.SampleDesc.Quality = 0;

		// Set to windowed mode.
		result.Windowed = TRUE;

		// Discard the back buffer contents after presenting.
		result.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		// Set the advanced flags, if surface is a display surface.
		result.Flags = UINT( get( m_createInfo.surface )->isDisplay()
			? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
			: 0 );

#endif

		m_presentDesc = result;
	}
}
