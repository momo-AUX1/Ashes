#include "Descriptor/XBoxDescriptorPool.hpp"

#include "Descriptor/XBoxDescriptorSet.hpp"
#include "Descriptor/XBoxDescriptorSetLayout.hpp"
#include "Core/XBoxDevice.hpp"

#include "ashesxbox_api.hpp"

namespace ashes::xbox
{
	DescriptorPool::DescriptorPool( VkDevice device
		, VkDescriptorPoolCreateInfo createInfos )
		: m_device{ device }
		, m_poolSizes{ makeVector( createInfos.pPoolSizes, createInfos.poolSizeCount ) }
		, m_createInfos{ std::move( createInfos ) }
	{
		m_createInfos.pPoolSizes = m_poolSizes.data();
	}

	DescriptorPool::~DescriptorPool()noexcept
	{
		for ( auto & set : m_sets )
		{
			deallocateNA( set );
		}
	}

	void DescriptorPool::registerSet( VkDescriptorSet set )
	{
		if ( checkFlag( m_createInfos.flags, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ) )
		{
			m_sets.push_back( set );
		}

		m_allSets.push_back( set );
	}

	VkResult DescriptorPool::reset()
	{
		for ( auto & set : m_allSets )
		{
			deallocateNA( set );
		}

		m_allSets.clear();
		m_sets.clear();
		return VK_SUCCESS;
	}

	VkResult DescriptorPool::freeDescriptors( ArrayView< VkDescriptorSet const > sets )
	{
		if ( checkFlag( m_createInfos.flags, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ) )
		{
			for ( auto set : sets )
			{
				auto it = std::find( m_sets.begin()
					, m_sets.end()
					, set );

				if ( it != m_sets.end() )
				{
					deallocateNA( *it );
					m_sets.erase( it );
				}
			}
		}

		return VK_SUCCESS;
	}
}
