cmake_minimum_required(VERSION 3.5)
project(OpenNFS3)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-declarations")
#[[set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})]]

set(SOURCE_FILES
        src/nfs_loader.cpp
        src/main.cpp
        src/Scene/Camera.cpp
        src/Scene/Model.cpp
        include/TGALoader/TGALoader.cpp
        src/trk_loader.cpp
        src/Physics/Physics.cpp
        lib/glew-cmake/src/glew.c
        src/Scene/TrackBlock.cpp
        src/Scene/TrackBlock.h
        src/nfs_data.h
        src/Scene/Light.cpp
        src/Scene/Light.h
        src/Scene/Camera.cpp
        src/Scene/Camera.h
        src/Shaders/TrackShader.cpp
        src/Shaders/TrackShader.h
        src/Shaders/CarShader.cpp
        src/Shaders/CarShader.h
        src/Shaders/BaseShader.cpp
        src/Shaders/BaseShader.h
        src/Util/Assert.h
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
        include/imgui/examples/opengl3_example/imgui_impl_glfw_gl3.cpp
        )

add_executable(OpenNFS3 ${SOURCE_FILES})

#[[IMGUI]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/imgui")
#[[TINYOBJLOADER]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/tinyobjloader")
#[[BOOST]]
add_subdirectory(lib/boost-cmake/)
target_link_libraries(OpenNFS3 Boost::filesystem Boost::boost)
#[[Bullet Configuration]]
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/bullet3/ "${CMAKE_CURRENT_BINARY_DIR}/bullet3")
TARGET_LINK_LIBRARIES(OpenNFS3 BulletDynamics BulletCollision LinearMath Bullet3Common)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/bullet3/src")
#[[GLEW Configuration]]
add_definitions(-DGLEW_STATIC  -D__NO_INLINE__)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/glew-cmake/include")
#[[GLM Configuration]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/glm")
#[[OpenGL Configuration]]
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS})
target_link_libraries(OpenNFS3 ${OPENGL_LIBRARIES})
#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw/ "${CMAKE_CURRENT_BINARY_DIR}/glfw")
target_link_libraries(OpenNFS3 glfw)
