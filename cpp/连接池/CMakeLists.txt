cmake_minimum_required(VERSION 3.16)
project(connection_pool_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(connection_pool_demo
    main.cpp
    connection_pool.cpp
)
