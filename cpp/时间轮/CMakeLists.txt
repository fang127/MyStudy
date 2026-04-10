cmake_minimum_required(VERSION 3.16)
project(high_perf_time_wheel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(time_wheel_demo
    src/main.cpp
)
target_include_directories(time_wheel_demo PRIVATE include)

add_executable(time_wheel_unit_tests
    tests/unit_tests.cpp
)
target_include_directories(time_wheel_unit_tests PRIVATE include)
add_test(NAME time_wheel_unit_tests COMMAND time_wheel_unit_tests)

add_executable(time_wheel_perf_test
    tests/perf_test.cpp
)
target_include_directories(time_wheel_perf_test PRIVATE include)
add_test(NAME time_wheel_perf_test COMMAND time_wheel_perf_test)
