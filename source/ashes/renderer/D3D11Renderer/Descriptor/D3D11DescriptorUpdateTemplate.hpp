/**
*\file
*	D3D11DescriptorUpdateTemplate.hpp
*\author
*	Momo-AUX1
*/
#ifndef ___D3D11Renderer_DescriptorUpdateTemplate_HPP___
#define ___D3D11Renderer_DescriptorUpdateTemplate_HPP___
#pragma once

#include "renderer/D3D11Renderer/D3D11RendererPrerequisites.hpp"

#include <vector>

namespace ashes::D3D11_NAMESPACE
{
	class DescriptorUpdateTemplate
		: public NonCopyable
	{
	public:
		DescriptorUpdateTemplate( VkDevice device
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
