# File: macros.cmake
# Authors: Igor N. Bongartz
# Erstellt: 2015-10-29
# Version: 2015-10-29
#
# This file contains several macros which are used in this project. Notice that several are copied straight from web ressources.

macro(add_imported_library_interface name include)
    add_library(${name} INTERFACE IMPORTED)
    #message("Adding ${include} as interface directories to ${name}")
    set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${include}")
endmacro(add_imported_library_interface)

macro(add_imported_library name type lib include)
    # Workaround from https://cmake.org/Bug/view.php?id=15052
    #file(MAKE_DIRECTORY "${include}")
    #message("Name is: " ${name} ", type is " ${type} ", lib is " ${lib} ", include is " ${include})

    if("${lib}" STREQUAL "")
        if("${type}" STREQUAL "SHARED")
        	#message(STATUS "Add imported library 1:  ${name} ${type} IMPORTED" )
            #add_library(${name} INTERFACE IMPORTED)
            add_library(${name} SHARED IMPORTED GLOBAL)
            set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${include}")
        else()
        	#message(STATUS "Add imported library 2:  ${name} ${type} IMPORTED" )
        	#add_library(${name} INTERFACE IMPORTED)
        	add_library(${name} STATIC IMPORTED GLOBAL)
            set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${include}")
        endif()
    else()
    	#message(STATUS "Add imported library 3:  ${name}_${type} ${type} IMPORTED" )
        add_library(${name}_${type} ${type} IMPORTED GLOBAL)
        set_target_properties(${name}_${type} PROPERTIES IMPORTED_LOCATION "${lib}")
        set_target_properties(${name}_${type} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${include}")
    endif()

endmacro(add_imported_library)

macro(export_target output TARGET)
    get_target_property(TYPE ${TARGET} TYPE)
    if(TYPE STREQUAL "SHARED_LIBRARY")
        get_target_property(LOCATION ${TARGET} IMPORTED_LOCATION)
        if(${LOCATION} STREQUAL "LOCATION-NOTFOUND")
            get_target_property(LOCATION ${TARGET} IMPORTED_LOCATION_RELEASE)
        endif()
        if(${LOCATION} STREQUAL "LOCATION-NOTFOUND")
            get_target_property(LOCATION ${TARGET} IMPORTED_LOCATION_DEBUG)
        endif()
        #message(STATUS "shared library, location is: ${LOCATION}")

        get_target_property(INCLUDE ${TARGET} INTERFACE_INCLUDE_DIRECTORIES)
        set(${output} "${${output}}
if(NOT TARGET ${TARGET})
  add_library(${TARGET} SHARED IMPORTED)
  set_target_properties(${TARGET} PROPERTIES IMPORTED_LOCATION \"${LOCATION}\")
  set_target_properties(${TARGET} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"${INCLUDE}\")
endif()
")
    elseif(TYPE STREQUAL "STATIC_LIBRARY")
        get_target_property(LOCATION ${TARGET} IMPORTED_LOCATION)
        if(${LOCATION} STREQUAL "LOCATION-NOTFOUND")
        	get_target_property(LOCATION ${TARGET} IMPORTED_LOCATION_RELEASE)
        endif()
        get_target_property(INCLUDE ${TARGET} INTERFACE_INCLUDE_DIRECTORIES)
        set(${output} "${${output}}
if(NOT TARGET ${TARGET})
  add_library(${TARGET} STATIC IMPORTED)
  set_target_properties(${TARGET} PROPERTIES IMPORTED_LOCATION \"${LOCATION}\")
  set_target_properties(${TARGET} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"${INCLUDE}\")
endif()
")
        if(NOT "${ARGN}" STREQUAL "")
            set(${output} "${${output}}set_target_properties(${TARGET} PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES \"${ARGN}\")
")
        endif()
    elseif(TYPE STREQUAL "INTERFACE_LIBRARY")
        get_target_property(INCLUDE ${TARGET} INTERFACE_INCLUDE_DIRECTORIES)
        set(${output} "${${output}}
if(NOT TARGET ${TARGET})
  add_library(${TARGET} INTERFACE IMPORTED)
  set_target_properties(${TARGET} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"${INCLUDE}\")
endif()
")
    else()
        message(STATUS "Unknown type ${TYPE}")
    endif()
endmacro(export_target)

macro(export_dependency output TARGET DEP)
    set(${output} "${${output}}set_target_properties(${TARGET} PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES \"${DEP}\")
")
endmacro(export_dependency)

macro(load_library group name version)
    string(TOUPPER ${name} LIBNAME)
    set(Boost_USE_STATIC_LIBS OFF)
    if(${version} GREATER 0 )
        #message(STATUS "name: ${name}, version: ${version}, argn: ${ARGN}")
        find_package(${name} ${version} ${ARGN} QUIET)
    else()
        find_package(${name} ${ARGN} QUIET)
    endif()
    if(${name}_FOUND OR ${LIBNAME}_FOUND)
        if (${name}_FOUND)
            set(LIBNAME ${name})
        endif()

        message(STATUS "Load library ${group} ${name} ${version}")
        message(STATUS "Found Library ${name}.")
        message(STATUS "Libraries: ${${LIBNAME}_LIBRARIES}")
        message(STATUS "Includes file: ${${LIBNAME}_INCLUDE_DIR}")
        message(STATUS "libname is: ${LIBNAME}")
        message(STATUS "Library_dirs: ${${LIBNAME}_LIBRARY_DIRS}")

        if(STATICLIB_SWITCH)
        		list(APPEND ${group}_LIBRARIES_STATIC ${${LIBNAME}_LIBRARIES})
        		add_imported_library(${LIBNAME} STATIC "${${LIBNAME}_LIBRARIES}" "${${LIBNAME}_INCLUDE_DIR}")
        else()
                list(APPEND ${group}_LIBRARIES_DYNAMIC ${${LIBNAME}_LIBRARIES})
                add_imported_library(${LIBNAME} SHARED "${${LIBNAME}_LIBRARIES}" "${${LIBNAME}_INCLUDE_DIR}")
        endif()

        list(APPEND ${group}_INCLUDE_DIRS ${${LIBNAME}_INCLUDE_DIR})

        #unset(${LIBNAME}_FOUND CACHE)
        #unset(${LIBNAME}_INCLUDE_DIR CACHE)
        #unset(${LIBNAME}_LIBRARIES CACHE)

        #unset(${LIBNAME}_LIBRARY CACHE)
    else()
        message(STATUS "Library ${name} was not found.")
    endif()
endmacro(load_library)
