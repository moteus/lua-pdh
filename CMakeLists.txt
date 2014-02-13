cmake_minimum_required ( VERSION 2.8 )

project ( pdh C )
include ( cmake/dist.cmake )
include ( lua )

include_directories ( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set ( PDH_SOURCE src/l52util.c src/lpdh.c )

set ( PDH_LIBRARIES pdh psapi advapi32 )

set ( PDH_DEFINITIONS WINVER=0x0502 _WIN32_WINNT=0x0502 )

install_lua_module ( pdh.core ${PDH_SOURCE} LINK ${PDH_LIBRARIES} )

set_target_properties ( pdh_core PROPERTIES COMPILE_DEFINITIONS "${PDH_DEFINITIONS}" )

install_lua_module ( pdh         lua/pdh.lua       )

install_lua_module ( pdh.psapi   lua/pdh/psapi.lua )

install_data ( README.md LICENCE.txt )

install_example( examples/ )
