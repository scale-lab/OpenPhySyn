cmake_minimum_required(VERSION 3.13.4)

project(PhyKnightBufferFanoutTransform VERSION 1.0.0 LANGUAGES CXX)

set(TRANSFORM_NAME buffer_fanout)

set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS_INIT} -std=c++11 -fPIC")

find_package(TCL REQUIRED)
set(Boost_USE_STATIC_LIBS ON)



if(DEFINED ENV{PHY_HOME_PATH})
set(PHY_HOME_PATH $ENV{PHY_HOME_PATH})
else()
set(PHY_HOME_PATH "/usr/include/phyknight")
endif()

IF(NOT EXISTS ${PHY_HOME_PATH})
message( FATAL_ERROR "Cannot find PhyKnight headers, set PHY_HOME_PATH variable." )
endif()

if(DEFINED ENV{OPENDB_HOME_PATH})
set(OPEDNDB_HOME $ENV{OPENDB_HOME_PATH})
else()
set(OPEDNDB_HOME ${PHY_HOME_PATH}/external/OpenDB)
endif()

if(DEFINED ENV{OPENSTA_HOME_PATH})
set(OPENSTA_HOME $ENV{OPENSTA_HOME_PATH})
else()
set(OPENSTA_HOME ${PHY_HOME_PATH}/external/OpenSTA)
endif()

IF(NOT EXISTS ${OPEDNDB_HOME})
message( FATAL_ERROR "Cannot find OpenDB headers, set OPENDB_HOME_PATH variable." )
endif()
IF(NOT EXISTS ${OPENSTA_HOME})
message( FATAL_ERROR "Cannot find OpenSTA headers, set OPENSTA_HOME_PATH variable." )
endif()

if(DEFINED ENV{PHY_LIB_PATH})
set(PHY_LIB_PATH $ENV{PHY_LIB_PATH})
link_directories(${PHY_LIB_PATH})
endif()

if(DEFINED ENV{OPENDB_LIB_PATH})
set(OPENDB_LIB_PATH $ENV{OPENDB_LIB_PATH})
link_directories(${OPENDB_LIB_PATH})
link_directories(${OPENDB_LIB_PATH}/src)
link_directories(${OPENDB_LIB_PATH}/src/db)
link_directories(${OPENDB_LIB_PATH}/src/defin)
link_directories(${OPENDB_LIB_PATH}/src/defout)
link_directories(${OPENDB_LIB_PATH}/src/lefin)
link_directories(${OPENDB_LIB_PATH}/src/lefout)
link_directories(${OPENDB_LIB_PATH}/src/zlib)
link_directories(${OPENDB_LIB_PATH}/src/zutil)
link_directories(${OPENDB_LIB_PATH}/src/tm)
endif()

include_directories(
  src/
  ${PHY_HOME_PATH}/include/
  ${PHY_HOME_PATH}/external/spdlog/include
  ${OPEDNDB_HOME}/include
  ${OPEDNDB_HOME}/src
  ${OPEDNDB_HOME}/src/db
  ${OPENSTA_HOME}/..
)

add_library(${TRANSFORM_NAME} SHARED src/BufferFanoutTransform.cpp)
target_link_libraries(${TRANSFORM_NAME}
PUBLIC
opendb
defin
defout
# lefin
lefout
# lef
zlib
zutil
tm
# OpenSTA
# ${ZLIB_LIBRARIES}
${TCL_LIBRARY}
# ${Boost_LIBRARIES}
dl
PhyKnight
)
target_compile_features(${TRANSFORM_NAME} PRIVATE cxx_range_for cxx_auto_type)
target_compile_options(${TRANSFORM_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)
set_property(TARGET ${TRANSFORM_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)


if((defined ENV{PHY_TRANSFORM_PATH}) AND NOT (ENV{PHY_TRANSFORM_PATH} STREQUAL ""))
set(PHY_TRANSFORM_PATH $ENV{PHY_TRANSFORM_PATH})
else()
set(PHY_TRANSFORM_PATH "$ENV{HOME}/.phyknight/transforms")
endif()

install(
  TARGETS ${TRANSFORM_NAME}
  DESTINATION ${PHY_TRANSFORM_PATH}
)



