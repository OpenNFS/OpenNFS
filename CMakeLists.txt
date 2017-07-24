cmake_minimum_required(VERSION 3.5)
project(FCE_to_OBJ)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/Functions.c
        src/Functions.h
        src/main.c)

add_executable(FCE_to_OBJ ${SOURCE_FILES})

#[[GLEW Configuration]]
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES})
endif()

#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(../glfw-3.2.1/ "${CMAKE_CURRENT_BINARY_DIR}/glfw")
target_link_libraries(FCE_to_OBJ glfw)

#[[Nuklear UI Library]]
include_directories(./include/nuklear-master)

