
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/resources/src/coinUtils/include/coin/)

ExternalProject_Add(
    coinUtils
    LOG_CONFIGURE 1
    LOG_DOWNLOAD 1
    LOG_BUILD 1
    GIT_REPOSITORY https://github.com/coin-or/CoinUtils.git
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./configure --enable-static
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND
	)

ExternalProject_Get_Property(coinUtils source_dir)
ExternalProject_Get_Property(coinUtils binary_dir)

message(STATUS "CoinUtils source dir: ${source_dir}")
message(STATUS "CoinUtils binary dir: ${binary_dir}")

set( coinUtils_INCLUDE_DIR "${source_dir}/include/coin" )

add_imported_library(coinUtils STATIC "${binary_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}CoinUtils${CMAKE_STATIC_LIBRARY_SUFFIX}" "${coinUtils_INCLUDE_DIR}")
add_imported_library(coinUtils SHARED "${binary_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}CoinUtils${CMAKE_SHARED_LIBRARY_SUFFIX}" "${coinUtils_INCLUDE_DIR}")

add_dependencies(coinUtils_STATIC coinUtils_SHARED clp)
add_dependencies(resources coinUtils_STATIC coinUtils_SHARED)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/resources/src/clp/include/coin/)

ExternalProject_Add(
    clp
    LOG_CONFIGURE 1
    LOG_DOWNLOAD 1
    LOG_BUILD 1
    DEPENDS coinUtils
    GIT_REPOSITORY https://github.com/coin-or/Clp.git
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./configure --with-coinutils-lib=${binary_dir}/lib/libCoinUtils.la --with-coinutils-incdir=${source_dir}/include/coin --enable-static
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND
    TEST_COMMAND ${CMAKE_MAKE_PROGRAM} test
	)


ExternalProject_Get_Property(clp source_dir)
ExternalProject_Get_Property(clp binary_dir)

message(STATUS "Clp source dir: ${source_dir}")
message(STATUS "Clp binary dir: ${binary_dir}")

set( clp_INCLUDE_DIR "${source_dir}/include/coin" )

add_imported_library(clp STATIC "${binary_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}Clp${CMAKE_STATIC_LIBRARY_SUFFIX}" ${clp_INCLUDE_DIR})
add_imported_library(clp SHARED "${binary_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}Clp${CMAKE_SHARED_LIBRARY_SUFFIX}" ${clp_INCLUDE_DIR})

add_dependencies(clp_STATIC clp_SHARED clp)
add_dependencies(resources clp_STATIC clp_SHARED)
