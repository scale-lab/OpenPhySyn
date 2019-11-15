cmake_minimum_required(VERSION 3.13.4)

project(PhyKnight VERSION 1.0.0 LANGUAGES CXX)


option(ENABLE_WARNINGS_SETTINGS "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters." ON)
option(ENABLE_LTO "Enable link time optimization" ON)
option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled." ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltcl -lz -lm -ldl -Wno-error -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Tool build configuration
option(OPENROAD_BUILD "Indicates if the tool is being built as a module in the top level application" OFF)
set(DATABASE_HANDLER "OpenDB" CACHE STRING "Specify the database handler type, can be OpenDB or OpenSTA")

if (${DATABASE_HANDLER} STREQUAL "OpenDB")
  add_definitions(-DUSE_OPENDB_DB_HANDLER)
else()
  add_definitions(-DUSE_OPENSTA_DB_HANDLER)
endif()
  
if (${OPENROAD_BUILD})
  add_definitions(-DOPENROAD_BUILD)
endif()
message("Database handler: ${DATABASE_HANDLER}")


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


set(PHY_HOME
  ${PROJECT_SOURCE_DIR}
)
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
add_subdirectory(external/flute)

# Add OpenDB
add_subdirectory(external/OpenDB)

# Add OpenSTA
add_subdirectory(external/OpenSTA)

# Add spdlog
add_subdirectory(external/spdlog)



set(PHY_WRAP  ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Phy_wrap.cc)

set(PHY_MAIN ${PHY_HOME}/app/PhyMain.cpp)
set(PHY_SOURCES_NO_MAIN_NO_WRAP
    ${PHY_HOME}/src/Phy/Phy.cpp
    ${PHY_HOME}/src/Phy/Exports.cpp
    ${PHY_HOME}/src/Phy/ProgramOptions.cpp
    ${PHY_HOME}/src/Analyzer/Analyzer.cpp
    ${PHY_HOME}/src/SteinerTree/SteinerTree.cpp
    ${PHY_HOME}/src/PhyLogger/PhyLogger.cpp
    ${PHY_HOME}/src/Database/OpenDBHandler.cpp
    ${PHY_HOME}/src/Database/OpenStaHandler.cpp
    ${PHY_HOME}/src/DefReader/DefReader.cpp
    ${PHY_HOME}/src/DefWriter/DefWriter.cpp
    ${PHY_HOME}/src/LefReader/LefReader.cpp
    ${PHY_HOME}/src/LibertyReader/LibertyReader.cpp
    ${PHY_HOME}/src/Transform/PhyTransform.cpp
    ${PHY_HOME}/src/Transform/TransformHandler.cpp
    ${PHY_HOME}/src/Utils/FileUtils.cpp
    ${PHY_HOME}/src/PhyException/Error.cpp
    ${PHY_HOME}/src/PhyException/FileException.cpp
    ${PHY_HOME}/src/PhyException/NoTechException.cpp
    ${PHY_HOME}/src/PhyException/ParseLibertyException.cpp
    ${PHY_HOME}/src/PhyException/PhyException.cpp
    ${PHY_HOME}/src/PhyException/ProgramOptionsException.cpp
    ${PHY_HOME}/src/PhyException/TransformNotFoundException.cpp

    ${PHY_HOME}/src/Sta/DatabaseSta.cpp
    ${PHY_HOME}/src/Sta/DatabaseSdcNetwork.cpp
    ${PHY_HOME}/src/Sta/DatabaseStaNetwork.cpp
)
set(PHY_SOURCES_NO_MAIN
    ${PHY_SOURCES_NO_MAIN_NO_WRAP}
    ${PHY_WRAP}
)
set(PHY_SOURCES 
    ${PHY_SOURCES_NO_MAIN}
    ${PHY_MAIN}
)

set(PHY_TESTFILES        # All .cpp files in tests/
    ${PROJECT_SOURCE_DIR}/tests/ReadLefDef.cpp
    ${PROJECT_SOURCE_DIR}/tests/WriteDef.cpp
    ${PROJECT_SOURCE_DIR}/tests/ReadLiberty.cpp
    ${PROJECT_SOURCE_DIR}/tests/BufferFanout.cpp
    ${PROJECT_SOURCE_DIR}/tests/TestMain.cpp
)

set(PHY_SWIG_FILES
  app/Phy.i
)
set(PHY_DEP_FILES
  ${PHY_SWIG_FILES}
  ${PHY_HOME}/src/Phy/Exports.cpp
)
set(LIBRARY_NAME ${PROJECT_NAME})  # Default name for the library built from src/*.cpp (change if you wish)
# string(TOLOWER ${PROJECT_NAME} LIBRARY_NAME)

set(INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/PhyKnight
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

# Compile all sources into a library.
add_library(${LIBRARY_NAME} STATIC ${PHY_SOURCES})
add_executable(Phy ${PHY_SOURCES})  # Name of exec. and location of file.

target_include_directories(${LIBRARY_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${LIBRARY_NAME} PUBLIC ${PUBLIC_LIBRARIES}) 
target_include_directories(Phy PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Phy PUBLIC ${PUBLIC_LIBRARIES}) 

include_directories (
  ${INCLUDE_DIRS}
)

set_target_properties(
    ${LIBRARY_NAME} Phy
      PROPERTIES 
        CXX_STANDARD 17 
        CXX_STANDARD_REQUIRED YES 
        CXX_EXTENSIONS NO
)
set_property(TARGET ${LIBRARY_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET Phy PROPERTY POSITION_INDEPENDENT_CODE ON)



add_custom_command(OUTPUT ${PHY_WRAP}
  COMMAND ${SWIG_EXECUTABLE} -tcl8 -c++ -namespace -prefix phy -I${PROJECT_SOURCE_DIR}/src -o ${PHY_WRAP} ${PHY_SWIG_FILES}
  WORKING_DIRECTORY ${PHY_HOME}
  DEPENDS ${PHY_SWIG_FILES}
)

install(TARGETS Phy DESTINATION bin)
# Set up tests (see tests/CMakeLists.txt).
add_subdirectory(tests)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES 
  ${PHY_WRAP}
)

option(PHY_STANDARD_TRANSFORMS "Build PhyKnight Standard Transforms" ON)
if(PHY_STANDARD_TRANSFORMS)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/external/StandardTransforms")
    include(Transforms)
endif()