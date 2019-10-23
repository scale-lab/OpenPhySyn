cmake_minimum_required(VERSION 3.8.2)

# List all files containing tests. (Change as needed)
set(PHY_TESTFILES        # All .cpp files in tests/
    TestMain.cpp
    PhyDemo.cpp
)

set(TEST_MAIN unit_tests)   # Default name for test executable (change if you wish).
set(TEST_RUNNER_PARAMS "")  # Any arguemnts to feed the test runner (change as needed).

# --------------------------------------------------------------------------------
#                         Make Tests (no change needed).
# --------------------------------------------------------------------------------
add_executable(${TEST_MAIN} ${PHY_TESTFILES} ${PHY_SOURCES_NO_MAIN})
target_link_libraries(${TEST_MAIN} PRIVATE doctest ${PUBLIC_LIBRARIES})
set_target_properties(${TEST_MAIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
# target_set_warnings(${TEST_MAIN} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).

set_target_properties(${TEST_MAIN} PROPERTIES 
    CXX_STANDARD 17 
    CXX_STANDARD_REQUIRED YES 
    CXX_EXTENSIONS NO
)

add_test(
    # Use some per-module/project prefix so that it is easier to run only tests for this module
    NAME ${LIBRARY_NAME}.${TEST_MAIN}
    COMMAND ${TEST_MAIN} ${TEST_RUNNER_PARAMS})

# Adds a 'coverage' target.
include(CodeCoverage)

