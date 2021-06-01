if(${carl_VERSION} VERSION_LESS_EQUAL "18.10" )
	message(STATUS "Using carl less than 18.10")
	set(CARL_OLD_INTERVALS "True" )
	set(CARL_OLD_STRUCTURE "True" )
endif()
if(NOT EXISTS "${carl_INCLUDE_DIR}/carl/core/polynomialfunctions/Substitution.h")
message(STATUS "Using member substitute function of CArL")
	set(CARL_POLY_MEMBER_SUBSTITUTE "True" )
endif()

file(STRINGS "${carl_INCLUDE_DIR}/carl/formula/Constraint.h" CARL_VARIABLE_VARIANTS REGEX ".*carlVariables mVariables;.*")
#message(STATUS "Have read: ${CARL_VARIABLE_VARIANTS}")
#if(${carl_VERSION} VERSION_LESS_EQUAL "19.10" )
#	set(CARL_VARIABLE_VARIANTS "True")
#endif()

if(NOT EXISTS "${carl_INCLUDE_DIR}/carl/core/carlLogging.h")
	set(CARL_LOGGING_POST_1910 "True")
	message(STATUS "Carl logging post 19.10")
else()
	message(STATUS "Carl logging pre 19.10")
endif()