cmake_minimum_required(VERSION 3.13.4)

project(OpenPhySyn VERSION 1.0.0 LANGUAGES CXX)


option(ENABLE_WARNINGS_SETTINGS "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters." ON)
option(ENABLE_LTO "Enable link time optimization" ON)
option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled." ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -ltcl -lz -lm -ldl -Wno-error -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Tool build configuration
option(OPENROAD_BUILD "Indicates if the tool is being built as a module in the top level application" OFF)
set(DATABASE_HANDLER "OpenSTA" CACHE STRING "Specify the database handler type, can be OpenDB or OpenSTA")

if (${DATABASE_HANDLER} STREQUAL "OpenDB")
  add_definitions(-DUSE_OPENDB_DB_HANDLER)
else()
  add_definitions(-DUSE_OPENSTA_DB_HANDLER)
endif()
  
if (${OPENROAD_BUILD})
  add_definitions(-DOPENROAD_BUILD)
endif()

set(SWIG_FLAGS "")
if (OPENROAD_BUILD)
set(SWIG_FLAGS "-DOPENROAD_BUILD")
endif()

# message("Database handler: ${DATABASE_HANDLER}")

set(PSN_HOME
  ${PROJECT_SOURCE_DIR}
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PSN_HOME}/cmake")

# Include stuff.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)
include(CTest)
include(Doctest)
include(Documentation)
include(LTO)
include(Misc)
include(Warnings)
include(ClangFormat)
include(CppCheck)


# Check for LTO support.
find_lto(CXX)

#Add TCL Package
find_package(TCL REQUIRED)

# Add Boost Package
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS log program_options filesystem REQUIRED)

#Add SWIG Package
cmake_policy(SET CMP0078 NEW)
find_package(SWIG REQUIRED)
find_package(ZLIB REQUIRED)
INCLUDE(${SWIG_USE_FILE})



set(OPENDB_HOME
  ${PROJECT_SOURCE_DIR}/external/OpenDB
)
set(OPENSTA_HOME
  ${PROJECT_SOURCE_DIR}/external/OpenSTA
)
set(FLUTE_HOME
  ${PROJECT_SOURCE_DIR}/external/flute
)
set(SPD_LOG_HOME
  ${PROJECT_SOURCE_DIR}/external/spdlog
)

# Add Flute
if (NOT ${OPENROAD_BUILD})
add_subdirectory(external/flute)
endif()

# Add OpenDB
if (NOT ${OPENROAD_BUILD})
add_subdirectory(external/OpenDB)
endif()

# Add OpenSTA
if (NOT ${OPENROAD_BUILD})
add_subdirectory(external/OpenSTA)
endif()

# Add spdlog
add_subdirectory(external/spdlog)



set(PSN_WRAP  ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Psn_wrap.cc)

set(PSN_MAIN ${PSN_HOME}/app/PsnMain.cpp)
set(PSN_SOURCES_NO_MAIN_NO_WRAP
    ${PSN_HOME}/src/Psn/Psn.cpp
    ${PSN_HOME}/src/Psn/Exports.cpp
    ${PSN_HOME}/src/Psn/ProgramOptions.cpp
    ${PSN_HOME}/src/Psn/DesignSettings.cpp
    ${PSN_HOME}/src/Analyzer/Analyzer.cpp
    ${PSN_HOME}/src/SteinerTree/SteinerTree.cpp
    ${PSN_HOME}/src/PsnLogger/PsnLogger.cpp
    ${PSN_HOME}/src/Database/OpenDBHandler.cpp
    ${PSN_HOME}/src/Database/OpenStaHandler.cpp
    ${PSN_HOME}/src/DefReader/DefReader.cpp
    ${PSN_HOME}/src/DefWriter/DefWriter.cpp
    ${PSN_HOME}/src/LefReader/LefReader.cpp
    ${PSN_HOME}/src/LibertyReader/LibertyReader.cpp
    ${PSN_HOME}/src/Transform/PsnTransform.cpp
    ${PSN_HOME}/src/Transform/TransformHandler.cpp
    ${PSN_HOME}/src/Transform/TransformInfo.cpp
    ${PSN_HOME}/src/Utils/FileUtils.cpp
    ${PSN_HOME}/src/Utils/PsnGlobal.cpp
    ${PSN_HOME}/src/PsnException/Error.cpp
    ${PSN_HOME}/src/PsnException/FileException.cpp
    ${PSN_HOME}/src/PsnException/NoTechException.cpp
    ${PSN_HOME}/src/PsnException/SteinerException.cpp
    ${PSN_HOME}/src/PsnException/ParseLibertyException.cpp
    ${PSN_HOME}/src/PsnException/PsnException.cpp
    ${PSN_HOME}/src/PsnException/ProgramOptionsException.cpp
    ${PSN_HOME}/src/PsnException/TransformNotFoundException.cpp

    ${PSN_HOME}/src/Sta/DatabaseSta.cpp
    ${PSN_HOME}/src/Sta/DatabaseSdcNetwork.cpp
    ${PSN_HOME}/src/Sta/DatabaseStaNetwork.cpp
)



set(PSN_SOURCES_NO_MAIN
    ${PSN_SOURCES_NO_MAIN_NO_WRAP}
    ${PSN_WRAP}
)
set(PSN_SOURCES 
    ${PSN_SOURCES_NO_MAIN}
    ${PSN_MAIN}
)

if (${OPENROAD_BUILD})
# temporary bad fix..
set(OPENSTA_UTIL_TCL_INIT ${CMAKE_CURRENT_BINARY_DIR}/OpenStaUtilTclInitVar.cc)

add_custom_command(OUTPUT ${OPENSTA_UTIL_TCL_INIT}
  COMMAND ${OPENSTA_HOME}/etc/TclEncode.tcl ${OPENSTA_UTIL_TCL_INIT} opensta_util_tcl_inits ${OPENSTA_HOME}/tcl/Util.tcl
  DEPENDS ${OPENSTA_HOME}/tcl/Util.tcl ${OPENSTA_HOME}/etc/TclEncode.tcl
)

set(PSN_SOURCES
    ${PSN_SOURCES}
    ${OPENSTA_UTIL_TCL_INIT}
)
endif()

set(PSN_TESTFILES        # All .cpp files in tests/
    ${PROJECT_SOURCE_DIR}/tests/ReadLefDef.cpp
    ${PROJECT_SOURCE_DIR}/tests/WriteDef.cpp
    ${PROJECT_SOURCE_DIR}/tests/ReadLiberty.cpp
    ${PROJECT_SOURCE_DIR}/tests/BufferFanout.cpp
    ${PROJECT_SOURCE_DIR}/tests/SteinerTree.cpp
    ${PROJECT_SOURCE_DIR}/tests/GateClone.cpp
    ${PROJECT_SOURCE_DIR}/tests/HelloTransform.cpp
    ${PROJECT_SOURCE_DIR}/tests/TestMain.cpp
)

set(PSN_SWIG_FILES
  ${PSN_HOME}/app/Psn.i
)
set(PSN_DEP_FILES
  ${PSN_SWIG_FILES}
  ${PSN_HOME}/src/Psn/Exports.cpp
  ${PSN_HOME}/src/Psn/Psn.cpp
  ${PSN_HOME}/src/Psn/Exports.hpp
  ${PSN_HOME}/src/Psn/Psn.hpp
)
set(LIBRARY_NAME ${PROJECT_NAME})  # Default name for the library built from src/*.cpp (change if you wish)
# string(TOLOWER ${PROJECT_NAME} LIBRARY_NAME)

set(INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/OpenPhySyn
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/external
  ${TCL_INCLUDE_PATH}
  ${Boost_INCLUDE_DIR}
  ${SPD_LOG_HOME}/include
  ${FLUTE_HOME}/include
  ${OPENDB_HOME}/include
  ${OPENDB_HOME}/src
  ${OPENDB_HOME}/src/db
  ${OPENDB_HOME}/src/lefin
  ${OPENDB_HOME}/src/defin
  ${OPENDB_HOME}/src/defout
)

set(PUBLIC_LIBRARIES
  flute
  opendb
  OpenSTA
  ${ZLIB_LIBRARIES}
  ${TCL_LIBRARY}
  ${Boost_LIBRARIES}
  dl
  doctest
)
if (${OPENROAD_BUILD})
set(INCLUDE_DIRS
  ${INCLUDE_DIRS}
  ${OPENROAD_HOME}/include
  PRIVATE
  ${OPENROAD_HOME}/src
  ${OPENDB_HOME}/include
  flute3
)
set(PUBLIC_LIBRARIES
dbSta
${PUBLIC_LIBRARIES}
)
endif()
# Compile all sources into a library.
add_library(${LIBRARY_NAME} STATIC ${PSN_SOURCES})
add_executable(Psn ${PSN_SOURCES})  # Name of exec. and location of file.

target_include_directories(${LIBRARY_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${LIBRARY_NAME} PUBLIC ${PUBLIC_LIBRARIES}) 
target_include_directories(Psn PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Psn PUBLIC ${PUBLIC_LIBRARIES})



include_directories (
  ${INCLUDE_DIRS}
)

set_target_properties(
    ${LIBRARY_NAME} Psn
      PROPERTIES 
        CXX_STANDARD 17 
        CXX_STANDARD_REQUIRED YES 
        CXX_EXTENSIONS NO
)
set_property(TARGET ${LIBRARY_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET Psn PROPERTY POSITION_INDEPENDENT_CODE ON)



add_custom_command(OUTPUT ${PSN_WRAP}
  COMMAND ${SWIG_EXECUTABLE} ${SWIG_FLAGS} -tcl8 -c++ -namespace -prefix psn -I${PROJECT_SOURCE_DIR}/src -o ${PSN_WRAP} ${PSN_SWIG_FILES}
  WORKING_DIRECTORY ${PSN_HOME}
  DEPENDS ${PSN_SWIG_FILES}
)

install(TARGETS Psn DESTINATION bin)
# Set up tests (see tests/CMakeLists.txt).
if (NOT ${OPENROAD_BUILD})
add_subdirectory(tests)
endif()
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES 
  ${PSN_WRAP}
)

option(PSN_STANDARD_TRANSFORMS "Build OpenPhySyn Standard Transforms" ON)
if(PSN_STANDARD_TRANSFORMS)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PSN_HOME}/external/StandardTransforms")
    include(Transforms)
endif()