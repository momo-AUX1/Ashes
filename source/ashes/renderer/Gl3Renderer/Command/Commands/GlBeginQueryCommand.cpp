/*
This file belongs to Ashes.
See LICENSE file in root folder.
*/
#include "Command/Commands/GlBeginQueryCommand.hpp"

#include "Miscellaneous/GlQueryPool.hpp"

#include "ashesgl3_api.hpp"

namespace ashes::gl3
{
	void apply( ContextLock const & context
		, CmdBeginQuery const & cmd )
	{
		glLogCommand( "BeginQueryCommand" );
		glLogCall( context
			, glBeginQuery
			, GlQueryType( cmd.target )
			, GLuint( cmd.query ) );
	}

	void buildBeginQueryCommand( VkQueryPool pool
		, uint32_t query
		, CmdList & list )
	{
		list.push_back( makeCmd< OpType::eBeginQuery >( convert( get( pool )->getType() )
			, *( get( pool )->begin() + query ) ) );
	}
}
