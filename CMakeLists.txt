cmake_minimum_required(VERSION 2.6)
project(GlWarp)

# Version Number
set(GLWARP_VERISION_MAJOR 0)
set(GLWARP_VERISION_MINOR 1)

# compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-W -Wall -Wno-unused-parameter -Werror")

# find packages
find_package(OpenGL REQUIRED)
find_package(glew REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLFW REQUIRED glfw3)

# libraries
set(ALL_LIBS
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARY}
        ${GLFW_STATIC_LIBRARIES}
        ${GLEW_LIBRARY})

# specify executable
add_executable(glwarp main.cpp)

# get include directories
include_directories(${GLFW_INCLUDE_DIRS})

target_link_libraries(glwarp ${ALL_LIBS})

