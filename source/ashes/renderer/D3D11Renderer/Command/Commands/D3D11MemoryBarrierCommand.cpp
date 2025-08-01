/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#include "Command/Commands/D3D11MemoryBarrierCommand.hpp"

#include "ashesd3d11_api.hpp"

namespace ashes::D3D11_NAMESPACE
{
	MemoryBarrierCommand::MemoryBarrierCommand( VkDevice device
		, ArrayView< VkBufferMemoryBarrier const > const & bufferBarriers )
		: CommandBase{ device }
	{
		for ( auto & barrier : bufferBarriers )
		{
			if ( get( barrier.buffer )->isMapped() )
			{
				if ( checkFlag( barrier.srcAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT )
					|| checkFlag( barrier.dstAccessMask, VK_ACCESS_HOST_READ_BIT )
					|| checkFlag( barrier.dstAccessMask, VK_ACCESS_MEMORY_READ_BIT ) )
				{
					m_downloadBuffers.push_back(
						{
							barrier.offset,
							barrier.size,
							barrier.buffer,
						} );
				}
				else if ( checkFlag( barrier.dstAccessMask, VK_ACCESS_TRANSFER_READ_BIT )
					|| checkFlag( barrier.srcAccessMask, VK_ACCESS_MEMORY_WRITE_BIT )
					|| checkFlag( barrier.srcAccessMask, VK_ACCESS_HOST_WRITE_BIT ) )
				{
					m_uploadBuffers.push_back(
						{
							barrier.offset,
							barrier.size,
							barrier.buffer,
						} );
				}
			}
		}
	}

	void MemoryBarrierCommand::apply( Context const & context )const
	{
		for ( auto & buffer : m_uploadBuffers )
		{
			get( get( buffer.buffer )->getMemory() )->updateUpload( context.context
				, buffer.offset
				, buffer.size
				, 0u );
		}

		for ( auto & buffer : m_downloadBuffers )
		{
			get( get( buffer.buffer )->getMemory() )->updateDownload( context.context
				, buffer.offset
				, buffer.size
				, 0u );
		}
	}

	CommandPtr MemoryBarrierCommand::clone()const
	{
		return std::make_unique< MemoryBarrierCommand >( *this );
	}
}
