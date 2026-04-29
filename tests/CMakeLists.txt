# Find GTest package (assuming vcpkg is used)
find_package(GTest CONFIG REQUIRED)

add_executable(ML_tests ML_tests.cpp)
target_link_libraries(ML_tests PRIVATE CoreLib GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_executable(Scheduler_tests Scheduler_tests.cpp)
target_link_libraries(Scheduler_tests PRIVATE CoreLib GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(ML_tests)
gtest_discover_tests(Scheduler_tests)
