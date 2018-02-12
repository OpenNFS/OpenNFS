cmake_minimum_required(VERSION 3.5)
project(OpenNFS3)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-declarations -O0")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/bullet3/src")

set(SOURCE_FILES
        src/nfs_loader.cpp
        src/nfs_loader.h
        include/tinyobjloader/tiny_obj_loader.h
        src/main.cpp
        external/glew-2.1.0/src/glew.c
        src/controls.cpp
        src/shader.cpp
        src/Model.cpp
        src/TGALoader.cpp
        src/trk_loader.cpp
        #[[IMGUI]]
        include/imgui/imgui.cpp
        include/imgui/imgui.h
        include/imgui/imgui_demo.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_internal.h
        include/imgui/imconfig.h
        include/imgui/stb_rect_pack.h
        include/imgui/stb_textedit.h
        include/imgui/stb_truetype.h
        include/imgui/imgui_impl_glfw_gl3.cpp
        src/physics.cpp src/physics.h include/tinyobjloader/tiny_obj_loader.h)

add_executable(OpenNFS3 ${SOURCE_FILES})

#[[Bullet Configuration]]
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bullet3/ "${CMAKE_CURRENT_BINARY_DIR}/bullet3")
TARGET_LINK_LIBRARIES(OpenNFS3 BulletDynamics BulletCollision LinearMath Bullet3Common)
#[[GLEW Configuration]]
add_definitions(-DGLEW_STATIC  -D__NO_INLINE__)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/glew-2.1.0/include")
#[[GLM Configuration]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/glm-0.9.9-a2")
#[[OpenGL Configuration]]
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS})
target_link_libraries(OpenNFS3 ${OPENGL_LIBRARIES})
#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw-3.2.1/ "${CMAKE_CURRENT_BINARY_DIR}/glfw")
target_link_libraries(OpenNFS3 glfw)
