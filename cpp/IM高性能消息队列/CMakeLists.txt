cmake_minimum_required(VERSION 3.20)
project(im_high_perf_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(folly REQUIRED)

add_executable(im_mq_demo
    src/main.cpp
)

target_include_directories(im_mq_demo PRIVATE include)
target_link_libraries(im_mq_demo PRIVATE Folly::folly Threads::Threads)
