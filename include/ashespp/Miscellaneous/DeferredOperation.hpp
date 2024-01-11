/*
See LICENSE file in root folder
*/
#ifndef ___AshesPP_DeferredOperation_H___
#define ___AshesPP_DeferredOperation_H___

#include "ashespp/AshesPPPrerequisites.hpp"

namespace ashes
{
	/**
	*\brief
	*	Deferred operation implementation.
	*/
	class DeferredOperation
		: public VkObject
	{
	public:
		/**
		*\brief
		*	Constructor.
		*\param[in] device
		*	The logical device.
		*/
		explicit DeferredOperation( Device const & device );
		/**
		*\brief
		*	Constructor.
		*\param[in] device
		*	The logical device.
		*\param[in] debugName
		*	The debug name.
		*/
		DeferredOperation( Device const & device
			, std::string const & debugName );
		/**
		*\brief
		*	Destructor.
		*/
		~DeferredOperation()noexcept;

		uint32_t getMaxConcurrency()const;
		VkResult getResult()const;
		void join()const;
		/**
		*\brief
		*	VkQueryPool implicit cast operator.
		*/
		operator VkDeferredOperationKHR const & ()const noexcept
		{
			return m_internal;
		}

	private:
		Device const & m_device;
		VkDeferredOperationKHR m_internal{};
	};
}

#endif
