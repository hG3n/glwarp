cmake_minimum_required(VERSION 2.6)
project(GlWarp)

# Version Number
set(GLWARP_VERISION_MAJOR 0)
set(GLWARP_VERISION_MINOR 1)

# compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-W -Wall")

# find packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)

# find glew somewhere else when on apple
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
endif ()

# use pkg-config to find further libs
pkg_check_modules(GLFW REQUIRED glfw3)
#pkg_check_modules(GLM REQUIRED glm)


# libraries
set(ALL_LIBS
        ${OPENGL_LIBRARIES}
        ${GLFW_STATIC_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${X11_LIBRARIES})

# specify executable
add_executable(glwarp main.cpp)

# get include directories
include_directories(${GLFW_INCLUDE_DIRS} ${GLEW_INLCUDE_DIRS} ${GLM_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})

target_link_libraries(glwarp ${ALL_LIBS})

