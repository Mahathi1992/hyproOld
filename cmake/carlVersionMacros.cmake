if(${carl_VERSION} VERSION_LESS_EQUAL "18.10" )
	message(STATUS "Using carl less than 18.10")
	set(CARL_OLD_INTERVALS "True")
	set(CARL_OLD_STRUCTURE "True")
endif()
if(NOT EXISTS "${carl_INCLUDE_DIR}/carl/core/polynomialfunctions/Substitution.h")
message(STATUS "Using member substitute function of CArL")
	set(CARL_POLY_MEMBER_SUBSTITUTE "True")
endif()


    set(CARL_VARIABLE_VARIANTS "True")



    set(CARL_LOGGING_VX "True")
