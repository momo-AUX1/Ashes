/**
*\file
*	GlDescriptorUpdateTemplate.hpp
*\author
*	Momo-AUX1
*/
#ifndef ___GlRenderer_DescriptorUpdateTemplate_HPP___
#define ___GlRenderer_DescriptorUpdateTemplate_HPP___
#pragma once

#include "renderer/GlRenderer/GlRendererPrerequisites.hpp"

#include <vector>

namespace ashes::gl
{
	class DescriptorUpdateTemplate
		: public AutoIdIcdObject< DescriptorUpdateTemplate >
	{
	public:
		DescriptorUpdateTemplate( VkAllocationCallbacks const * allocInfo
			, VkDevice device
			, VkDescriptorUpdateTemplateCreateInfo const & createInfo );
		~DescriptorUpdateTemplate()noexcept;

		void updateDescriptorSet( VkDescriptorSet descriptorSet
			, void const * pData );

		VkDevice getDevice()const
		{
			return m_device;
		}

	private:
		VkDevice m_device;
		VkDescriptorUpdateTemplateType m_templateType;
		VkPipelineBindPoint m_pipelineBindPoint;
		VkPipelineLayout m_pipelineLayout;
		uint32_t m_set;
		std::vector< VkDescriptorUpdateTemplateEntry > m_entries;
	};
}

#endif
