cmake_minimum_required(VERSION 4.4.0)
project(MiniTP VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

add_library(MiniTP)

target_sources(
    MiniTP
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ThreadPool.cpp
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MiniTP/ThreadPool.hpp
)
