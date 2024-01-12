/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#pragma once

#include "renderer/GlRenderer/GlRendererPrerequisites.hpp"

namespace ashes::gl
{
	class RenderPass
		: public AutoIdIcdObject< RenderPass >
	{
	public:
		RenderPass( VkAllocationCallbacks const * allocInfo
			, VkDevice device
			, VkRenderPassCreateInfo const & createInfo );
		~RenderPass()noexcept;

		uint32_t getAttachmentIndex( VkAttachmentDescription const & attach )const;

		VkAttachmentDescription const * findAttachment( uint32_t referenceIndex )const;
		VkAttachmentDescription const & getAttachment( VkAttachmentReference const & reference )const;
		bool isFboAttachable( VkAttachmentReference const & reference )const;

		VkSubpassDescriptionArray const & getSubpasses()const noexcept
		{
			return m_subpasses;
		}

		AttachmentDescriptionArray const & getColourAttaches()const noexcept
		{
			return m_colourAttaches;
		}

		bool hasDepthAttach()const noexcept
		{
			return m_hasDepthAttach;
		}

		VkExtent2D getRenderAreaGranularity()const noexcept
		{
			return VkExtent2D{ 1u, 1u };
		}

		VkAttachmentReferenceArray const & getFboAttachable()const noexcept
		{
			return m_referencedAttachments;
		}

		VkAttachmentReferenceArray & getFboAttachable()noexcept
		{
			return m_referencedAttachments;
		}

		uint32_t getMaxLoadClearIndex()const noexcept
		{
			return m_maxLoadClearIndex;
		}

		VkAttachmentReferenceArray const & getResolveAttachments()const noexcept
		{
			return m_resolveAttachments;
		}

		VkDevice getDevice()const noexcept
		{
			return m_device;
		}

	public:
		struct SubpassDescriptionData
		{
			VkAttachmentReferenceArray inputAttachments;
			VkAttachmentReferenceArray colorAttachments;
			VkAttachmentReferenceArray resolveAttachments;
			Optional< VkAttachmentReference > depthStencilAttachment;
			UInt32Array reserveAttachments;
		};
		using SubpassDescriptionDataPtr = std::unique_ptr< SubpassDescriptionData >;
		using SubpassDescriptionDataPtrMap = std::map< VkSubpassDescription const *, SubpassDescriptionDataPtr >;

	private:
		void referenceAttach( VkAttachmentReference const & value
			, VkAttachmentReferenceArray & attachments )const;
		void referenceAttach( Optional< VkAttachmentReference > const & value
			, VkAttachmentReferenceArray & attachments )const;
		void referenceAttaches( VkAttachmentReferenceArray const & value
			, VkAttachmentReferenceArray & attachments )const;

	private:
		VkDevice m_device;
		VkAttachmentDescriptionArray m_attachments;
		VkAttachmentReferenceArray m_referencedAttachments;
		VkAttachmentReferenceArray m_resolveAttachments;
		VkSubpassDescriptionArray m_subpasses;
		SubpassDescriptionDataPtrMap m_subpassesDatas;
		VkSubpassDependencyArray m_dependencies;
		bool m_hasDepthAttach{ false };
		VkAttachmentDescriptionArray m_depthAttaches;
		AttachmentDescriptionArray m_colourAttaches;
		uint32_t m_maxLoadClearIndex{};
	};
}
