cmake_minimum_required(VERSION 3.25)
project(SR1)
set(EXECUTABLE_NAME "SR1")

set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED)
find_package(TBB REQUIRED)
# Incluye los directorios de cabecera de GLM
include_directories("/usr/include/glm")

add_executable(${EXECUTABLE_NAME} main.cpp render.h color.h framebuffer.h vertexGML.h objSettings.h uniform.h print.h FastNoiseLite.h)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL2 tbb)

target_sources(${EXECUTABLE_NAME} PRIVATE render.cpp color.cpp framebuffer.cpp vertexGML.cpp objSettings.cpp uniform.cpp)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${TBB_INCLUDE_DIRS})
