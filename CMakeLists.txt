cmake_minimum_required(VERSION 3.7)
project(CaveGen)

set(CMAKE_CXX_STANDARD 14)

find_package(SDL2 REQUIRED system window graphics)

include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp Cave.cpp ConfigReader.cpp ConfigReader.h)
target_link_libraries(${PROJECT_NAME} SDL2)