cmake_minimum_required(VERSION 3.5)
project(FCE_to_OBJ)

#[[set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")]]
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        src/nfs_loader.cpp
        src/nfs_loader.h
        src/main.cpp
        external/glew-2.1.0/src/glew.c
        src/obj_loader.cpp
        src/controls.cpp
        src/shader.cpp
        src/NFS3_Mesh.cpp
        src/TGALoader.cpp)

add_executable(FCE_to_OBJ ${SOURCE_FILES})

#[[GLEW Configuration]]
add_definitions(-DGLEW_STATIC  -D__NO_INLINE__)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/glew-2.1.0/include")

find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS})
target_link_libraries(FCE_to_OBJ ${OPENGL_LIBRARIES})

#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw-3.2.1/ "${CMAKE_CURRENT_BINARY_DIR}/glfw")
target_link_libraries(FCE_to_OBJ glfw)

#[[GLM Configuration]]
add_subdirectory(external/glm-0.9.9-a2/ "${CMAKE_CURRENT_BINARY_DIR}/glm")
target_link_libraries(FCE_to_OBJ glm)
