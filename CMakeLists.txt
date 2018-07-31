cmake_minimum_required(VERSION 3.4)
project(OpenNFS)

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")


set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
#[[set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})]]

set(SOURCE_FILES
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
        resources/asset/icon.rc
        # Source
        src/main.cpp
        src/Scene/Camera.cpp
        src/Scene/Model.cpp
        include/TGALoader/TGALoader.cpp
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
        src/Util/Utils.cpp
        src/Util/Utils.h
        src/Util/Raytracer.cpp
        src/Util/Raytracer.h
        tools/fshtool.c
        tools/fshtool.h
        src/Scene/CarModel.cpp src/Scene/CarModel.h src/Scene/Track.cpp src/Scene/Track.h src/Shaders/BillboardShader.cpp src/Shaders/BillboardShader.h src/Physics/Car.cpp src/Physics/Car.h
        src/Loaders/music_loader.cpp src/Loaders/music_loader.h src/Config.h src/Loaders/nfs2_loader.cpp src/Loaders/nfs2_loader.h src/Loaders/nfs3_loader.cpp src/Loaders/nfs3_loader.h src/Loaders/trk_loader.cpp src/Loaders/trk_loader.h src/Loaders/nfs2_loader.cpp src/Loaders/nfs4_loader.cpp src/Loaders/nfs4_loader.h src/Renderer/Renderer.cpp src/Renderer/Renderer.h src/Loaders/track_utils.cpp src/Loaders/track_utils.h src/Loaders/car_loader.cpp src/Loaders/car_loader.h src/Renderer/HermiteCurve.cpp src/Renderer/HermiteCurve.h src/Scene/Sound.cpp src/Scene/Sound.h src/Scene/Entity.cpp src/Scene/Entity.h)

add_executable(OpenNFS ${SOURCE_FILES})

#[[IMGUI]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/imgui")
#[[TINYOBJLOADER]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/tinyobjloader")
#[[BOOST]]
add_subdirectory(lib/boost-cmake/)
target_link_libraries(OpenNFS Boost::filesystem Boost::boost)
#[[Bullet Configuration]]
#[[set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)]]
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/bullet3/ "${CMAKE_CURRENT_BINARY_DIR}/bullet3")
TARGET_LINK_LIBRARIES(OpenNFS BulletDynamics BulletCollision LinearMath Bullet3Common)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/bullet3/src")
#[[SDL Configuration]]
# Support both 32 and 64 bit builds
#[[set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl")
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(OpenNFS ${SDL2_LIBRARIES})]]
#[[GLEW Configuration]]
add_definitions(-DGLEW_STATIC  -D__NO_INLINE__)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/glew-cmake/include")
#[[GLM Configuration]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/glm")
#[[OpenGL Configuration]]
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS})
target_link_libraries(OpenNFS ${OPENGL_LIBRARIES})
#[[VULKAN]]
#[[set(VULKAN_INCLUDE_DIR "C:/VulkanSDK/1.1.70.1/Include")
include_directories(${VULKAN_INCLUDE_DIR})
target_link_libraries(OpenNFS C:/VulkanSDK/1.1.70.1/Lib32/vulkan-1.lib)]]
#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw/ "${CMAKE_CURRENT_BINARY_DIR}/glfw")
target_link_libraries(OpenNFS glfw)
