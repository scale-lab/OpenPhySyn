cmake_minimum_required(VERSION 3.1...3.15)

project(OpenPhySyn VERSION 1.4.1 LANGUAGES CXX)


option(OPENPHYSYN_WARNINGS_SETTINGS_ENABLED "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters." ON)
option(OPENPHYSYN_LTO_ENABLED "Enable link time optimization" ON)
option(OPENPHYSYN_UNIT_TESTS_ENABLED "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled." ON)
option(OPENPHYSYN_STANDARD_TRANSFORMS_ENABLED "Build OpenPhySyn Standard Transforms" ON)
option(OPENPHYSYN_TRANSFORM_HELLO_TRANSFORM_ENABLED "Build demo hello transform" ON)
option(OPENPHYSYN_TRANSFORM_BUFFER_FANOUT_ENABLED "Build Buffer Fan-Out transform" ON)
option(OPENPHYSYN_TRANSFORM_GATE_CLONE_ENABLED "Build Gate Clone transform" ON)
option(OPENPHYSYN_TRANSFORM_PIN_SWAP_ENABLED "Build Pin Swap transform" ON)
option(OPENPHYSYN_TRANSFORM_CONSTANT_PROPAGATION_ENABLED "Build Constant Propagation transform" ON)
option(OPENPHYSYN_TRANSFORM_TIMING_BUFFER_ENABLED "Build Buffer Resize transform" ON)
option(OPENPHYSYN_READLINE_ENABLED "Enable Tcl Readline" ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wno-error -Wno-deprecated-copy")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wl,-export_dynamic -Wno-unused-command-line-argument")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(${OPENPHYSYN_READLINE_ENABLED})
add_definitions(-DOPENPHYSYN_READLINE_ENABLED)
endif()

# Tool build configuration
option(OPENROAD_BUILD "Indicates if the tool is being built as a module in the top level application" OFF)
option(OPENPHYSYN_TF_ENABLED "Build and use cpp-taskflow" OFF)
set(OPENPHYSYN_DATABASE_HANDLER "OpenSTA" CACHE STRING "Specify the database handler type, can be OpenDB or OpenSTA")

if (${OPENPHYSYN_DATABASE_HANDLER} STREQUAL "OpenDB")
add_definitions(-DUSE_OPENDB_DB_HANDLER)
else()
add_definitions(-DUSE_OPENSTA_DB_HANDLER)
endif()


set(SWIG_FLAGS "-w467,72,503,509")
set(SWIG_PSN_LIB_FLAGS "")



set(PSN_HOME
  ${PROJECT_SOURCE_DIR}
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PSN_HOME}/cmake")


# Default to bulding optimnized/release executable.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(CheckSymbolExists)
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
if (NOT DEFINED TCL_LIB)
set(TCL_LIB ${TCL_LIBRARY})
endif()

#Add SWIG Package
cmake_policy(SET CMP0078 NEW)
find_package(SWIG REQUIRED)
find_package(ZLIB REQUIRED)
INCLUDE(${SWIG_USE_FILE})



# Build OpenDB
set(OPENDB_HOME
${PROJECT_SOURCE_DIR}/external/OpenDB
)
set(BUILD_PYTHON OFF)

# Build OpenSTA
set(OPENSTA_HOME
${PROJECT_SOURCE_DIR}/external/OpenSTA
)

# Build Flute
set(FLUTE_HOME
${PROJECT_SOURCE_DIR}/external/flute
)

# Build spdlok
set(SPD_LOG_HOME
  ${PROJECT_SOURCE_DIR}/external/spdlog
)

# Build cxxopts
set(CXXOPTS_HOME
  ${PROJECT_SOURCE_DIR}/external/cxxopts
)

# Add Flute
add_subdirectory(external/flute)

# Add OpenDB
add_subdirectory(external/OpenDB)

# Add OpenSTA
add_subdirectory(external/OpenSTA)

# Add spdlog
add_subdirectory(external/spdlog)
# Add cxxopts
add_subdirectory(external/cxxopts)

#CXXOPTS regex causes issues on older compilers..
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9.2)
add_definitions(-DOPENPHYSYN_CXX_OPTS_DISABLED)
endif()

#Add cpp-taskflow
if (${OPENPHYSYN_TF_ENABLED})

add_definitions(-DTF_ENABLED)
set(TF_BUILD_EXAMPLES OFF)
set(TF_BUILD_TESTS OFF)
set(TF_BUILD_BENCHMARKS OFF)

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9.2)
set(TF_HOME ${PROJECT_SOURCE_DIR}/external/cpp-taskflow)
add_subdirectory(external/cpp-taskflow)

else()

set(TF_HOME ${PROJECT_SOURCE_DIR}/external/cpp-taskflow/cpp-taskflow-cpp14)
add_subdirectory(external/cpp-taskflow)
set(OPENPHYSYN_TF_14 ON)

endif()

endif()


message(STATUS "OpenPhySyn version: ${PROJECT_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${OPENSTA_HOME}/cmake")
include(GetGitRevisionDescription) # Utility available from OpenSTA
get_git_head_revision(GIT_REFSPEC OPENPHYSYN_GIT_SHA1)
message(STATUS "OpenPhySyn Git SHA: ${OPENPHYSYN_GIT_SHA1}")




set(PSN_WRAP  ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Psn_wrap.cpp)
set(PSN_LIBRARY_WRAP  ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Psn_lib_wrap.cpp)
set(SWIG_SOURCE_COMPILE_FLAGS "-Wno-cast-qual -Wno-deprecated-copy -Wno-unused-variable -Wno-unused-parameter -Wno-deprecated-copy")
set_source_files_properties(${PSN_WRAP} PROPERTIES COMPILE_FLAGS ${SWIG_SOURCE_COMPILE_FLAGS})
set_source_files_properties(${PSN_LIBRARY_WRAP} PROPERTIES COMPILE_FLAGS ${SWIG_SOURCE_COMPILE_FLAGS})

set(PSN_MAIN ${PSN_HOME}/app/PsnMain.cpp)
set(PSN_SOURCES_NO_MAIN_NO_WRAP
    ${PSN_HOME}/src/Psn/Psn.cpp
    ${PSN_HOME}/src/Psn/Exports.cpp
    ${PSN_HOME}/src/Psn/ProgramOptions.cpp
    ${PSN_HOME}/src/Psn/DesignSettings.cpp
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
    ${PSN_HOME}/src/Utils/FilesystemLegacyHelpers.cpp
    ${PSN_HOME}/src/Utils/StringUtils.cpp
    ${PSN_HOME}/src/Utils/PsnGlobal.cpp
    ${PSN_HOME}/src/PsnException/Error.cpp
    ${PSN_HOME}/src/PsnException/FileException.cpp
    ${PSN_HOME}/src/PsnException/FluteInitException.cpp
    ${PSN_HOME}/src/PsnException/NoTechException.cpp
    ${PSN_HOME}/src/PsnException/SteinerException.cpp
    ${PSN_HOME}/src/PsnException/ParseLibertyException.cpp
    ${PSN_HOME}/src/PsnException/PsnException.cpp
    ${PSN_HOME}/src/PsnException/ProgramOptionsException.cpp
    ${PSN_HOME}/src/PsnException/LoadTransformException.cpp
    ${PSN_HOME}/src/PsnException/TransformNotFoundException.cpp
    ${PSN_HOME}/src/Sta/DatabaseSta.cpp
    ${PSN_HOME}/src/Sta/PathPoint.cpp
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

set(PSN_TESTFILES        # All .cpp files in tests/
    ${PROJECT_SOURCE_DIR}/tests/SteinerTree.cpp
    ${PROJECT_SOURCE_DIR}/tests/HelloTransform.cpp
    ${PROJECT_SOURCE_DIR}/tests/ReadLefDef.cpp
    ${PROJECT_SOURCE_DIR}/tests/WriteDef.cpp
    ${PROJECT_SOURCE_DIR}/tests/ReadLiberty.cpp
    ${PROJECT_SOURCE_DIR}/tests/Sta.cpp
    ${PROJECT_SOURCE_DIR}/tests/TestMain.cpp
)
if (${OPENPHYSYN_TRANSFORM_HELLO_TRANSFORM_ENABLED})
set(PSN_TESTFILES
  ${PSN_TESTFILES}
  ${PROJECT_SOURCE_DIR}/tests/HelloTransform.cpp
)
endif()
if (${OPENPHYSYN_TRANSFORM_BUFFER_FANOUT_ENABLED})
set(PSN_TESTFILES
  ${PSN_TESTFILES}
  ${PROJECT_SOURCE_DIR}/tests/BufferFanout.cpp
  ${PROJECT_SOURCE_DIR}/tests/BufferTopPort.cpp
)
endif()
if (${OPENPHYSYN_TRANSFORM_GATE_CLONE_ENABLED})
set(PSN_TESTFILES
  ${PSN_TESTFILES}
  ${PROJECT_SOURCE_DIR}/tests/GateClone.cpp
)
endif()
if (${OPENPHYSYN_TRANSFORM_PIN_SWAP_ENABLED})
set(PSN_TESTFILES
  ${PSN_TESTFILES}
  ${PROJECT_SOURCE_DIR}/tests/PinSwap.cpp
  ${PROJECT_SOURCE_DIR}/tests/PinSwapPower.cpp
)
endif()

if (${OPENPHYSYN_TRANSFORM_CONSTANT_PROPAGATION_ENABLED})
set(PSN_TESTFILES
  ${PSN_TESTFILES}
  ${PROJECT_SOURCE_DIR}/tests/SimpleConstantPropagation.cpp
  ${PROJECT_SOURCE_DIR}/tests/SimpleConstantPropagationInversion.cpp
  ${PROJECT_SOURCE_DIR}/tests/ComplexConstantPropagation.cpp
  )
  endif()

if (${OPENPHYSYN_TRANSFORM_TIMING_BUFFER_ENABLED})
  set(PSN_TESTFILES
  ${PSN_TESTFILES}
  ${PROJECT_SOURCE_DIR}/tests/TimingBuffer.cpp
)
endif()


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

set(PUBLIC_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/OpenPhySyn
  ${PROJECT_SOURCE_DIR}/src
)
set(PRIVATE_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/external/doctest
  ${PROJECT_SOURCE_DIR}/external/spdlog
  ${PROJECT_SOURCE_DIR}/external/cxxopts/include
  ${PROJECT_SOURCE_DIR}/external/StandardTransforms
  ${PROJECT_SOURCE_DIR}/external/flute
  ${TCL_INCLUDE_PATH}
  ${SPD_LOG_HOME}/include
  ${FLUTE_HOME}/include
  ${OPENDB_HOME}/include
  ${OPENDB_HOME}/src
  ${OPENDB_HOME}/src/db
  ${OPENDB_HOME}/src/lefin
  ${OPENDB_HOME}/src/defin
  ${OPENDB_HOME}/src/defout
)

if (${OPENPHYSYN_TF_ENABLED})
set(PRIVATE_INCLUDE_DIRS
  ${TF_HOME}
  ${PRIVATE_INCLUDE_DIRS}
)
endif()

set(PUBLIC_LIBRARIES
${ZLIB_LIBRARIES}
${TCL_LIBRARY}
dl
)


CHECK_SYMBOL_EXISTS(std::filesystem::path::preferred_separator filesystem cxx17fs)

if(cxx17fs AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9.2)
set(PUBLIC_LIBRARIES
  ${PUBLIC_LIBRARIES}
  stdc++fs
)
endif()

# Stand-alone executable specific libraries
set(PUBLIC_EXEC_LIBRARIES
)

if (${OPENPHYSYN_TF_ENABLED})
set(PUBLIC_LIBRARIES
  ${PUBLIC_LIBRARIES}
  Cpp-Taskflow
)
endif()


set(PRIVATE_INCLUDE_DIRS
  ${PRIVATE_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/external
)

set(PUBLIC_LIBRARIES
opendb
OpenSTA
sta_swig
flute
${PUBLIC_LIBRARIES}
)


# Compile all sources into a library.
add_library(${LIBRARY_NAME} ${PSN_SOURCES_NO_MAIN_NO_WRAP} ${PSN_LIBRARY_WRAP})
add_executable(Psn ${PSN_SOURCES})  # Name of exec. and location of file.

target_include_directories(${LIBRARY_NAME} PUBLIC ${PUBLIC_INCLUDE_DIRS} PRIVATE ${PRIVATE_INCLUDE_DIRS})
target_link_libraries(${LIBRARY_NAME} PUBLIC ${PUBLIC_LIBRARIES}) 
target_include_directories(Psn PUBLIC ${PUBLIC_INCLUDE_DIRS} PRIVATE ${PRIVATE_INCLUDE_DIRS})
target_link_libraries(Psn PUBLIC ${PUBLIC_EXEC_LIBRARIES} ${PUBLIC_LIBRARIES})



include_directories (
  PUBLIC
  ${PUBLIC_INCLUDE_DIRS}
  PRIVATE
  ${PRIVATE_INCLUDE_DIRS}
)

set_target_properties(
    ${LIBRARY_NAME} Psn
      PROPERTIES 
        CXX_STANDARD 14
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
add_custom_command(OUTPUT ${PSN_LIBRARY_WRAP}
  COMMAND ${SWIG_EXECUTABLE} ${SWIG_PSN_LIB_FLAGS} ${SWIG_FLAGS} -tcl8 -c++ -namespace -prefix psn -I${PROJECT_SOURCE_DIR}/src -o ${PSN_LIBRARY_WRAP} ${PSN_SWIG_FILES}
  WORKING_DIRECTORY ${PSN_HOME}
  DEPENDS ${PSN_SWIG_FILES}
)

install(TARGETS Psn DESTINATION bin)

# Set up tests (see tests/CMakeLists.txt).
if (${OPENPHYSYN_UNIT_TESTS_ENABLED})
add_subdirectory(tests)
endif()

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES 
  ${PSN_WRAP}
  ${PSN_LIBRARY_WRAP}
)

if(OPENPHYSYN_STANDARD_TRANSFORMS_ENABLED)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PSN_HOME}/external/StandardTransforms")
    include(Transforms)
endif()
