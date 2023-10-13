cmake_minimum_required(VERSION 3.25)
project(lab5)

set(SDL_MAIN_HANDLED ON)
find_package(SDL2 REQUIRED)
# Incluye los directorios de cabecera de GLM
include_directories("/usr/include/glm")



set(EXECUTABLE_NAME "lab5")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(${EXECUTABLE_NAME}   main.cpp render.h color.h framebuffer.h vertexGML.h objSettings.h uniform.h print.h FastNoiseLite.h)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL2::SDL2main)
target_include_directories(${EXECUTABLE_NAME}  PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE_NAME}  PRIVATE ${SDL2_LIBRARIES})
target_sources(${EXECUTABLE_NAME} PRIVATE render.cpp color.cpp framebuffer.cpp vertexGML.cpp objSettings.cpp uniform.cpp)
