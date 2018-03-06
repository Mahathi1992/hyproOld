
# Add all targets to the build-tree export set
export(TARGETS ${HYPRO_TARGETS} FILE "${PROJECT_BINARY_DIR}/hyproTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
if(EXPORT_TO_CMAKE)
	message(STATUS "Exporting to cmake")
	if(STATICLIB_SWITCH)
		export(PACKAGE ${PROJECT_NAME}_static)
	else()
		export(PACKAGE ${PROJECT_NAME})
	endif()
else()
	message(STATUS "Not exporting to cmake")
endif()

if(STATICLIB_SWITCH)
	export_target(DEPENDENT_TARGETS lib_carl_static)
	export_target(DEPENDENT_TARGETS GLPK_STATIC)
	if (HYPRO_USE_Z3)
		export_target(DEPENDENT_TARGETS z3_STATIC)
	endif ()
	if(HYPRO_USE_SMTRAT)
	    export_target(DEPENDENT_TARGETS lib_smtrat_static)
	endif()
else()
	#set(DEPENDENT_TARGETS "${DEPENDENT_TARGETS}find_package(carl)
#include_directories(${carl_INCLUDE_DIR})")

	export_target(DEPENDENT_TARGETS lib_carl)
	export_target(DEPENDENT_TARGETS GLPK_STATIC)
	if (HYPRO_USE_Z3)
		export_target(DEPENDENT_TARGETS z3_SHARED)
	endif ()
	if(HYPRO_USE_SMTRAT)
	    export_target(DEPENDENT_TARGETS lib_smtrat)
	endif()
endif()

export_target(DEPENDENT_TARGETS Boost_SHARED)
export_target(DEPENDENT_TARGETS Boost_STATIC)
export_target(DEPENDENT_TARGETS Boost_program_options_SHARED)
export_target(DEPENDENT_TARGETS Boost_program_options_STATIC)
export_target(DEPENDENT_TARGETS EIGEN3)
export_target(DEPENDENT_TARGETS GMP_SHARED)
export_target(DEPENDENT_TARGETS GMP_STATIC)
export_target(DEPENDENT_TARGETS GMPXX_SHARED)
export_target(DEPENDENT_TARGETS GMPXX_STATIC GMP_STATIC)
export_target(DEPENDENT_TARGETS ANTLR4_STATIC)


include(CMakePackageConfigHelpers)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/hyproConfigVersion.cmake
					VERSION ${hypro_VERSION}
					COMPATIBILITY SameMajorVersion )

set(TARGET ${PROJECT_NAME})

# Create the hyproConfig.cmake and hyproConfigVersion files
# ... for the build tree
set(CONF_INCLUDES ${INCLUDES})

set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src") # TODO looks not correct, this would be hypro_INCLUDE_DIR
if(STATICLIB_SWITCH)
	set(CONF_LINKER_LIBS ${hypro_STATIC_LIBRARIES})
else ()
	set(CONF_LINKER_LIBS ${hypro_DYNAMIC_LIBRARIES})
endif()

configure_package_config_file(cmake/hyproConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/hyproConfig.cmake
							  INSTALL_DESTINATION ${PROJECT_BINARY_DIR}
							  PATH_VARS PROJECT_BINARY_DIR #SYSCONFIG_INSTALL_DIR
							 )

file(RELATIVE_PATH REL_INCLUDE_DIR "${CMAKE_INSTALL_DIR}" "${INCLUDE_INSTALL_DIR}")
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${hypro_CMAKE_DIR}/${REL_INCLUDE_DIR}")

configure_package_config_file(cmake/hyproConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/installedConfig/hyproConfig.cmake
							  INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
							  PATH_VARS INCLUDE_INSTALL_DIR #SYSCONFIG_INSTALL_DIR
			 )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/installedConfig/hyproConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/hyproConfigVersion.cmake
		DESTINATION ${CMAKE_INSTALL_DIR} )
install(EXPORT ${PROJECT_NAME} FILE "${PROJECT_NAME}Targets.cmake" DESTINATION ${CMAKE_INSTALL_DIR})

# Install the export set for use with the install-tree
#install(EXPORT ${HYPRO_TARGETS} DESTINATION ${PROJECT_BINARY_DIR}) # COMPONENT dev)
