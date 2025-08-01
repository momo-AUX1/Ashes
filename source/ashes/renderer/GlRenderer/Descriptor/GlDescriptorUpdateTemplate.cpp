/**
*\file
*	GlDescriptorUpdateTemplate.cpp
*\author
*	Momo-AUX1
*/
#include "Descriptor/GlDescriptorUpdateTemplate.hpp"

#include "Core/GlDevice.hpp"
#include "Descriptor/GlDescriptorSet.hpp"

#include "ashesgl_api.hpp"

namespace ashes::gl
{
	DescriptorUpdateTemplate::DescriptorUpdateTemplate( VkAllocationCallbacks const * allocInfo
		, VkDevice device
		, VkDescriptorUpdateTemplateCreateInfo const & createInfo )
		: m_device{ device }
		, m_templateType{ createInfo.templateType }
		, m_pipelineBindPoint{ createInfo.pipelineBindPoint }
		, m_pipelineLayout{ createInfo.pipelineLayout }
		, m_set{ createInfo.set }
	{
		m_entries.reserve( createInfo.descriptorUpdateEntryCount );

		for ( uint32_t i = 0; i < createInfo.descriptorUpdateEntryCount; ++i )
		{
			m_entries.push_back( createInfo.pDescriptorUpdateEntries[i] );
		}
	}

	DescriptorUpdateTemplate::~DescriptorUpdateTemplate()noexcept
	{
	}

	void DescriptorUpdateTemplate::updateDescriptorSet( VkDescriptorSet descriptorSet
		, void const * pData )
	{
		auto const * dataBytes = reinterpret_cast< uint8_t const * >( pData );

		for ( auto const & entry : m_entries )
		{
			for ( uint32_t i = 0; i < entry.descriptorCount; ++i )
			{
				VkWriteDescriptorSet writeSet{};
				writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSet.pNext = nullptr;
				writeSet.dstSet = descriptorSet;
				writeSet.dstBinding = entry.dstBinding;
				writeSet.dstArrayElement = entry.dstArrayElement + i;
				writeSet.descriptorCount = 1;
				writeSet.descriptorType = entry.descriptorType;

				auto const * entryData = dataBytes + entry.offset + (i * entry.stride);

				switch ( entry.descriptorType )
				{
				case VK_DESCRIPTOR_TYPE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
					writeSet.pImageInfo = reinterpret_cast< VkDescriptorImageInfo const * >( entryData );
					break;

				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
					writeSet.pBufferInfo = reinterpret_cast< VkDescriptorBufferInfo const * >( entryData );
					break;

				case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
					writeSet.pTexelBufferView = reinterpret_cast< VkBufferView const * >( entryData );
					break;

				default:
					continue;
				}

				get( descriptorSet )->update( writeSet );
			}
		}
	}
}
