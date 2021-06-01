
macro(add_imported_library_interface name include)
    add_library(${name} INTERFACE IMPORTED)
    #message("Adding ${include} as interface directories to ${name}")
    set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${include}")
endmacro(add_imported_library_interface)

macro(add_imported_library name type lib include)
    #message(STATUS "add_imported_library: ${name} ${lib} ${include}")
    if("${lib}" STREQUAL "")
        #message(STATUS "Lib name empty")
        if("${type}" STREQUAL "SHARED")
            #add_library(${name} INTERFACE IMPORTED)
            add_library(${name} SHARED IMPORTED GLOBAL)
            set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${include}")
        else()
        	#add_library(${name} INTERFACE IMPORTED)
        	add_library(${name} STATIC IMPORTED GLOBAL)
            set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${include}")
        endif()
    else()
        #message(STATUS "Type ${type}")
        #message(STATUS "Name ${name}_${type}")
        add_library(${name}_${type} ${type} IMPORTED GLOBAL)
        #add_library(${name}_${type} INTERFACE IMPORTED GLOBAL)
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
        find_package(${name} ${version} ${ARGN} EXACT)
    else()
        find_package(${name} ${ARGN} QUIET)
    endif()
    if(${name}_FOUND OR ${LIBNAME}_FOUND)
        if (${name}_FOUND)
            set(LIBNAME ${name})
        endif()

        #message(STATUS "Load library ${group} ${name} ${version}")
        #message(STATUS "Found Library ${name}.")
        #message(STATUS "Libraries: ${${LIBNAME}_LIBRARIES}")
        #message(STATUS "Includes file: ${${LIBNAME}_INCLUDE_DIR}")
        #message(STATUS "Includes file: ${${LIBNAME}_INCLUDE_DIRS}")
        #message(STATUS "libname is: ${LIBNAME}")
        #message(STATUS "Library_dirs: ${${LIBNAME}_LIBRARY_DIRS}")

        if("${CMAKE_FIND_LIBRARY_SUFFIXES}" STREQUAL ".a")
            add_imported_library(${LIBNAME} STATIC "${${LIBNAME}_LIBRARIES}" "${${LIBNAME}_INCLUDE_DIR}")
        else()
            add_imported_library(${LIBNAME} SHARED "${${LIBNAME}_LIBRARIES}" "${${LIBNAME}_INCLUDE_DIR}")
        endif()

        list(APPEND ${group}_INCLUDE_DIRS ${${LIBNAME}_INCLUDE_DIR})
    else()
        message(STATUS "Library ${name} was not found.")
    endif()
endmacro(load_library)

macro(load_library_static_shared group name version)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    load_library(${group} ${name} ${version})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.dylib")
    load_library(${group} ${name} ${version})
endmacro(load_library_static_shared)

macro(build_target_include_dirs_from target sys type)
	set(args "${ARGN}")
	foreach(arg IN LISTS args)
		if(TARGET ${arg})
			get_target_property(tmp ${arg} INTERFACE_INCLUDE_DIRECTORIES)
			if(tmp)
				target_include_directories(${target} ${sys} ${type} "$<BUILD_INTERFACE:${tmp}>")
			endif()
			get_target_property(tmp ${arg} INTERFACE_LINK_LIBRARIES)
			if(tmp)
				build_target_include_dirs_from(${target} SYSTEM ${type} ${tmp})
			endif()
		endif()
	endforeach()
endmacro(build_target_include_dirs_from)
