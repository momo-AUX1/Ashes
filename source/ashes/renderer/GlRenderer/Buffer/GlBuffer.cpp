#include "Buffer/GlBuffer.hpp"

#include "Core/GlContextLock.hpp"
#include "Core/GlDevice.hpp"
#include "Miscellaneous/GlDeviceMemory.hpp"
#include "Miscellaneous/GlDeviceMemoryBinding.hpp"

#include "ashesgl_api.hpp"

namespace ashes::gl
{
	Buffer::Buffer( [[maybe_unused]] VkAllocationCallbacks const * allocInfo
		, VkDevice device
		, VkBufferCreateInfo createInfo )
		: m_device{ device }
		, m_queueFamilyIndices{ makeVector( createInfo.pQueueFamilyIndices, createInfo.queueFamilyIndexCount ) }
		, m_createInfo{ std::move( createInfo ) }
		, m_target{ getTargetFromUsageFlags( m_createInfo.usage ) }
	{
		if ( m_createInfo.size > get( get( device )->getPhysicalDevice() )->getMemoryProperties().memoryHeaps[0].size )
		{
			throw ashes::Exception{ VK_ERROR_OUT_OF_DEVICE_MEMORY, "Buffer size is too large" };
		}
		m_createInfo.pQueueFamilyIndices = m_queueFamilyIndices.data();
		registerObject( m_device, *this );
	}

	Buffer::~Buffer()noexcept
	{
		unregisterObject( m_device, *this );

		if ( m_binding )
		{
			get( m_binding->getParent() )->unbindBuffer( get( this ) );
		}

		m_target = GlBufferTarget( 0u );
		m_internal = GL_INVALID_INDEX;
		m_queueFamilyIndices.clear();
	}

	VkMemoryRequirements Buffer::getMemoryRequirements()const noexcept
	{
		auto physicalDevice = get( get( m_device )->getPhysicalDevice() );
		VkMemoryRequirements result{};
		result.size = m_createInfo.size;
		result.memoryTypeBits = physicalDevice->getMemoryTypeBits( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT );
		result.memoryTypeBits = ~0u;
		result.alignment = get( m_device )->getLimits().nonCoherentAtomSize;

		if ( checkFlag( m_createInfo.usage, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ) )
		{
			result.alignment = get( m_device )->getLimits().minUniformBufferOffsetAlignment;
		}

		result.size = ashes::getAlignedSize( m_createInfo.size, result.alignment );
		return result;
	}

	bool Buffer::isMapped()const noexcept
	{
		assert( m_binding != nullptr );
		return m_binding->isMapped();
	}

	VkDeviceSize Buffer::getOffset()const noexcept
	{
		assert( m_binding != nullptr );
		return m_binding->getOffset();
	}
}
