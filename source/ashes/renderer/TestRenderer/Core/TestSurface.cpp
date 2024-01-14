/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#include "Core/TestSurface.hpp"

#include "Core/TestPhysicalDevice.hpp"
#include "Core/TestInstance.hpp"

#include "ashestest_api.hpp"

namespace ashes::test
{
	SurfaceKHR::SurfaceKHR()
	{
		m_presentModes.emplace_back( VK_PRESENT_MODE_FIFO_KHR );
		m_surfaceFormats.push_back( { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR } );

		m_surfaceCapabilities.minImageCount = 1u;
		m_surfaceCapabilities.maxImageCount = 1u;
		m_surfaceCapabilities.currentExtent.width = ~0u;
		m_surfaceCapabilities.currentExtent.height = ~0u;
		m_surfaceCapabilities.minImageExtent = m_surfaceCapabilities.currentExtent;
		m_surfaceCapabilities.maxImageExtent = m_surfaceCapabilities.currentExtent;
		m_surfaceCapabilities.maxImageArrayLayers = 1u;
		m_surfaceCapabilities.supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		m_surfaceCapabilities.currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		m_surfaceCapabilities.supportedCompositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		m_surfaceCapabilities.supportedUsageFlags = 0u;
	}

#if _WIN32

	SurfaceKHR::SurfaceKHR( VkInstance
		, VkWin32SurfaceCreateInfoKHR const & )
		: SurfaceKHR{}
	{
	}

#elif __linux__

	SurfaceKHR::SurfaceKHR( VkInstance
		, VkXlibSurfaceCreateInfoKHR const & )
		: SurfaceKHR{}
	{
	}

	SurfaceKHR::SurfaceKHR( VkInstance
		, VkXcbSurfaceCreateInfoKHR const & )
		: SurfaceKHR{}
	{
	}

	SurfaceKHR::SurfaceKHR( VkInstance
		, VkWaylandSurfaceCreateInfoKHR const & )
		: SurfaceKHR{}
	{
	}

#elif __APPLE__

	SurfaceKHR::SurfaceKHR( VkInstance
		, VkMacOSSurfaceCreateInfoMVK const & )
		: SurfaceKHR{}
	{
	}

#endif
#if VK_KHR_display

	SurfaceKHR::SurfaceKHR( VkInstance
		, VkDisplaySurfaceCreateInfoKHR const & )
		: SurfaceKHR{}
	{
	}

#endif

	bool SurfaceKHR::getSupport( uint32_t )const
	{
		return true;
	}
}
