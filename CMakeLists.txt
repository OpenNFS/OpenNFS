cmake_minimum_required(VERSION 3.4)
project(OpenNFS)

set(CMAKE_CXX_STANDARD 14)

set(DEP_ROOT_DIR ${CMAKE_SOURCE_DIR}/lib)

include(cmake/CheckCXXCompilerFlag.cmake)
include(cmake/CheckCCompilerFlag.cmake)
include(cmake/AddCompilerFlag.cmake)
include(cmake/OptimizeForArchitecture.cmake)
include(cmake/CompileGLSLToSPIRV.cmake)

#[[OptimizeForArchitecture()]
MESSAGE(STATUS "Compiler flags are: " ${CMAKE_CXX_FLAGS})

#[[Static linking broken under Mac OSX]]
if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
endif ()

set(OpenGL_GL_PREFERENCE LEGACY)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)

set(SOURCE_FILES
        include/bmpread/bmpread.c
        include/bmpread/bmpread.h
        include/imgui/imgui.cpp
        include/imgui/imgui.h
        include/imgui/imgui_demo.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/imgui_internal.h
        include/imgui/imconfig.h
        include/imgui/imstb_rectpack.h
        include/imgui/imstb_textedit.h
        include/imgui/imstb_truetype.h
        include/imgui/examples/imgui_impl_glfw.cpp
        include/imgui/examples/imgui_impl_glfw.h
        include/imgui/examples/imgui_impl_opengl3.cpp
        include/imgui/examples/imgui_impl_opengl3.h
        resources/asset/icon.rc
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
        src/Scene/CarModel.cpp
        src/Scene/CarModel.h
        src/Scene/Track.cpp
        src/Scene/Track.h
        src/Shaders/BillboardShader.cpp
        src/Shaders/BillboardShader.h
        src/Physics/Car.cpp
        src/Physics/Car.h
        src/Loaders/music_loader.cpp
        src/Loaders/music_loader.h
        src/Config.cpp
        src/Config.h
        src/Loaders/nfs2_loader.cpp
        src/Loaders/nfs2_loader.h
        src/Loaders/nfs3_loader.cpp
        src/Loaders/nfs3_loader.h
        src/Loaders/trk_loader.cpp
        src/Loaders/trk_loader.h
        src/Loaders/nfs2_loader.cpp
        src/Loaders/nfs4_loader.cpp
        src/Loaders/nfs4_loader.h
        src/Renderer/Renderer.cpp
        src/Renderer/Renderer.h
        src/Loaders/track_utils.cpp
        src/Loaders/track_utils.h
        src/Loaders/car_loader.cpp
        src/Loaders/car_loader.h
        src/Renderer/HermiteCurve.cpp
        src/Renderer/HermiteCurve.h
        src/Scene/Sound.cpp
        src/Scene/Sound.h
        src/Scene/Entity.cpp
        src/Scene/Entity.h
        src/Enums.h
        src/Renderer/CarRenderer.cpp
        src/Renderer/CarRenderer.h
        src/Renderer/TrackRenderer.cpp
        src/Renderer/TrackRenderer.h
        src/Shaders/SkydomeShader.cpp
        src/Shaders/SkydomeShader.h
        src/Renderer/SkyRenderer.cpp
        src/Renderer/SkyRenderer.h
        src/Shaders/DepthShader.cpp
        src/Shaders/DepthShader.h
        src/Util/Logger.cpp
        src/Util/Logger.h
        src/RaceNet/Network.cpp
        src/RaceNet/Network.h
        src/RaceNet/Matrix.h
        src/RaceNet/RaceNet.cpp
        src/RaceNet/RaceNet.h
        src/RaceNet/TrainingGround.cpp
        src/RaceNet/TrainingGround.h
        src/Renderer/RaceNetRenderer.cpp
        src/Renderer/RaceNetRenderer.h
        src/Shaders/RaceNetShader.cpp
        src/Shaders/RaceNetShader.h
        src/Scene/Quad.cpp
        src/Scene/Quad.h
        src/Renderer/ShadowMapRenderer.cpp
        src/Renderer/ShadowMapRenderer.h)

#[[Avoid Vulkan on Mac, until I add MoltenVK support. Avoid Windows too until I add Vulkan SDK to VSTS container]]
if (NOT (APPLE OR WIN32))
    set(SOURCE_FILES ${SOURCE_FILES} src/Renderer/vkRenderer.cpp src/Renderer/vkRenderer.h)
    add_definitions(-DVULKAN_BUILD)
endif ()

add_executable(OpenNFS ${SOURCE_FILES})

#[[IMGUI]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/imgui")
#[[TINYOBJLOADER]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/tinyobjloader")
#[[BMPREAD]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/bmpread")
#[[BOOST]]
add_subdirectory(lib/boost-cmake/ EXCLUDE_FROM_ALL)
target_link_libraries(OpenNFS Boost::program_options Boost::filesystem Boost::system Boost::boost)
#[[G3Log (Because Boost-cmake logging won't build]]
set(G3_SHARED_LIB OFF CACHE BOOL "Compile g3log as static library")
set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Don't bother compiling invalid code in g3log")
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/g3log")
include_directories(${DEP_ROOT_DIR}/${G3LOG_NAME}/src)
target_include_directories(OpenNFS INTERFACE g3logger)
target_link_libraries(OpenNFS g3logger)
#[[Bullet Configuration]]
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/bullet3/ "${CMAKE_CURRENT_BINARY_DIR}/bullet3" EXCLUDE_FROM_ALL)
target_link_libraries(OpenNFS BulletDynamics BulletCollision LinearMath Bullet3Common)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/bullet3/src")
#[[GLEW Configuration]]
add_definitions(-DGLEW_STATIC -D__NO_INLINE__)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/glew-cmake/include")
#[[GLM Configuration]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/glm")
#[[OpenGL Configuration]]
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(OpenNFS ${OPENGL_LIBRARIES})
#[[Vulkan Configuration]]
if (NOT (APPLE OR WIN32))
    find_package(Vulkan REQUIRED)
    message("VULKAN FOUND")
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries(OpenNFS ${Vulkan_LIBRARIES})
    CompileGLSLToSpirV(OpenNFS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vk" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vk")
endif ()
#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw/ "${CMAKE_CURRENT_BINARY_DIR}/glfw")
target_link_libraries(OpenNFS glfw)
