cmake_minimum_required(VERSION 3.24)
project(cgvc_project)

set(CMAKE_CXX_STANDARD 17)

link_directories(${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(OpenGL REQUIRED)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")


set(LIBS glfw OpenGL::GL dl assimp)

include_directories(${CMAKE_SOURCE_DIR}/includes)
file(GLOB SOURCES "*.cpp")
add_executable(cgvc_project ${SOURCES})
target_link_libraries(cgvc_project
        ${LIBS}
        ${CMAKE_SOURCE_DIR}/lib/libGLEW.a
        ${CMAKE_SOURCE_DIR}/lib/libassimp.a
        z
)