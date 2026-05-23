cmake_minimum_required(VERSION 3.24)
project(cgvc_project)

set(CMAKE_CXX_STANDARD 17)

# Local libraries
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Project includes
include_directories(
        ${CMAKE_SOURCE_DIR}/includes
)

# OpenGL
find_package(OpenGL REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(assimp REQUIRED MODULE)
find_package(PugiXML REQUIRED)
find_package(polyclipping CONFIG REQUIRED)
find_package(poly2tri CONFIG REQUIRED)
find_package(minizip CONFIG REQUIRED)

# Source files
set(SOURCES
        main.cpp
        Camera.cpp
        Bezier.cpp
        CameraController.cpp
        FollowCam.cpp
        FreeCam.cpp
        object.cpp
)


# Executable
add_executable(cgvc_project ${SOURCES}
        includes/glad/glad.c)

# Link everything
target_link_libraries(cgvc_project
        glfw
        GL
        dl
        assimp::assimp
        ${CMAKE_SOURCE_DIR}/lib/libGLEW.a
        z
        polyclipping::polyclipping
        poly2tri::poly2tri
        pugixml
        minizip::minizip
)