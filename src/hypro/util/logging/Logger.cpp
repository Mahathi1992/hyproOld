#include "hypro/util/logging/Logger.h"

#ifdef HYPRO_LOGGING
namespace hypro {

int initializeLogging( int& i ) {
	carl::logging::logger().configure( "logfile_hypro", "hypro.log" );
	carl::logging::logger().filter( "logfile_hypro" )( "hypro", carl::logging::LogLevel::LVL_INFO )( "hypro.settings", carl::logging::LogLevel::LVL_INFO )( "hypro.quickhull", carl::logging::LogLevel::LVL_TRACE )( "hypro.reachability", carl::logging::LogLevel::LVL_INFO )( "hypro.representations", carl::logging::LogLevel::LVL_INFO )( "hypro.representations.supportFunction", carl::logging::LogLevel::LVL_INFO )( "hypro.representations.HPolytope", carl::logging::LogLevel::LVL_INFO )( "hypro.datastructures", carl::logging::LogLevel::LVL_INFO )( "hypro.reacher", carl::logging::LogLevel::LVL_INFO )( "hypro.reacher.preprocessing", carl::logging::LogLevel::LVL_INFO );

	carl::logging::logger().configure( "pathfile_hypro", "unfinished_paths.log" );
	carl::logging::logger().filter( "pathfile_hypro" )( "benchmark.paths", carl::logging::LogLevel::LVL_TRACE );

	carl::logging::logger().configure( "stdout", std::cout );
	carl::logging::logger().filter( "stdout" )( "hypro", carl::logging::LogLevel::LVL_OFF )
		  //("hypro.representations", carl::logging::LogLevel::LVL_TRACE)
		  //("hypro.datastructures.timing", carl::logging::LogLevel::LVL_TRACE)
		  //("hypro.datastructures.hiv", carl::logging::LogLevel::LVL_TRACE)
		  ;

	carl::logging::logger().resetFormatter();

	// avoid warning of unused variable by casting it to void (thus it is used at least once).
	(void)i;

	return 0;
}

}  // namespace hypro

#endif
