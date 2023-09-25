cmake_minimum_required(VERSION 3.4)
project(OpenNFS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(AddCompilerFlag)
include(OptimizeForArchitecture)
include(CompileGLSLToSPIRV)

#[[Parameter Override]]
if(APPLE OR WIN32)
    set(USE_VULKAN OFF) #[[Avoid Vulkan on Mac until I add MoltenVK support. Windows too until Vulkan SDK on VSTS container]]
    set(STATIC_BUILD OFF) #[[Static linking broken under Mac OSX (and UNIX too? + MSVC doesnt support this flag)]]
endif()

if(${DEV})
    if(APPLE)
        # Silence Fshtool warnings til I deal with them
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-pointer-sign -Wno-return-type")
    endif()
    OptimizeForArchitecture()
    MESSAGE(STATUS "CXX Compiler flags are: " ${CMAKE_CXX_FLAGS})
    MESSAGE(STATUS "C Compiler flags are: " ${CMAKE_C_FLAGS})
endif()

if (${STATIC_BUILD})
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
endif ()

set(OpenGL_GL_PREFERENCE GLVND)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)

file(GLOB CRP_LIB_SOURCES
        lib/LibOpenNFS/lib/CrpLib/*.h
        lib/LibOpenNFS/lib/CrpLib/*.cpp)



set(IMGUI_SOURCES
        include/imgui/imgui.cpp
        include/imgui/imgui_demo.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_tables.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/backends/imgui_impl_glfw.cpp
        include/imgui/backends/imgui_impl_opengl3.cpp
        #[[include/ImGuiColorTextEdit/TextEditor.cpp]])

set(SOURCE_FILES
        ${IMGUI_SOURCES}
        ${CRP_LIB_SOURCES}
        include/bmpread/bmpread.c
        lib/glew-cmake/src/glew.c
        resources/asset/icon.rc
        src/Camera/BaseCamera.cpp
        src/Camera/BaseCamera.cpp
        src/Camera/CarCamera.cpp
        src/Camera/FreeCamera.cpp
        src/Camera/HermiteCamera.cpp
        src/Config.cpp
        src/Loaders/CarLoader.cpp
        src/Loaders/MusicLoader.cpp
        lib/LibOpenNFS/NFS2/NFS2Loader.cpp
        lib/LibOpenNFS/NFS3/NFS3Loader.cpp
        src/Loaders/TrackLoader.cpp
        src/Physics/AABB.cpp
        src/Physics/AABBTree.cpp
        src/Physics/Car.cpp
        src/Physics/Frustum.cpp
        src/Physics/PhysicsEngine.cpp
        src/Race/OrbitalManager.cpp
        src/Race/RaceSession.cpp
        src/Race/RacerManager.cpp
        src/RaceNet/Agents/CarAgent.cpp
        src/RaceNet/Agents/PlayerAgent.cpp
        src/RaceNet/Agents/RacerAgent.cpp
        src/Renderer/BulletDebugDrawer.cpp
        src/Renderer/CarRenderer.cpp
        src/Renderer/DebugRenderer.cpp
        src/Renderer/HermiteCurve.cpp
        src/Renderer/MenuRenderer.cpp
        src/Renderer/Renderer.cpp
        src/Renderer/ShadowMapRenderer.cpp
        src/Renderer/SkyRenderer.cpp
        src/Renderer/GLTexture.cpp
        src/Renderer/TrackRenderer.cpp
        src/Scene/Entity.cpp
        src/Scene/Models/CarModel.cpp
        src/Scene/Models/Model.cpp
        src/Scene/Models/Quad.cpp
        src/Scene/Models/TrackModel.cpp
        src/Scene/Models/LightModel.cpp
        src/Scene/Track.cpp
        src/Scene/GlobalLight.cpp
        src/Scene/Spotlight.cpp
        src/Shaders/BaseShader.cpp
        src/Shaders/BillboardShader.cpp
        src/Shaders/BulletShader.cpp
        src/Shaders/CarShader.cpp
        src/Shaders/DepthShader.cpp
        src/Shaders/FontShader.cpp
        src/Shaders/MenuShader.cpp
        src/Shaders/ShaderSet.cpp
        src/Shaders/SkydomeShader.cpp
        src/Shaders/TrackShader.cpp
        src/Util/ImageLoader.cpp
        src/Util/ImageLoader.cpp
        src/Util/Logger.cpp
        src/Util/Utils.cpp
        src/main.cpp
        #[[src/Renderer/RaceNetRenderer.cpp
        src/Shaders/RaceNetShader.cpp
        src/Util/Raytracer.cpp
        src/Loaders/NFS4/PC/NFS4Loader.cpp
        src/Loaders/NFS4/PS1/NFS4PS1Loader.cpp
        src/Loaders/NFS4/PS1/SerializedGroupOps.cpp
        src/Loaders/NFS5/NFS5Loader.cpp]])

add_executable(OpenNFS ${SOURCE_FILES})

#[[JSON]]
include_directories(include/json)

#[[STB]]
include_directories(include/stb)

#[[IMGUI]]
include_directories(include/imgui)

#[[TINYOBJLOADER]]
include_directories(include/tinyobjloader)

#[[BMPREAD]]
include_directories(include/bmpread)

#[[FREETYPE]]
add_subdirectory(lib/freetype2)
include_directories(lib/freetype2/include)
list(APPEND ONFS_LIBRARIES freetype)

#[[LibOpenNFS]]
add_subdirectory(lib/LibOpenNFS)
include_directories(lib/LibOpenNFS/include)
list(APPEND ONFS_LIBRARIES LibOpenNFS)

#[[G3Log]]
set(G3_SHARED_LIB OFF CACHE BOOL "Compile g3log as static library")
set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Don't bother compiling invalid code in g3log")
add_subdirectory(lib/g3log)
list(APPEND ONFS_LIBRARIES g3log)

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
add_subdirectory(lib/bullet3)
include_directories(lib/bullet3/src)
list(APPEND ONFS_LIBRARIES BulletDynamics BulletCollision LinearMath Bullet3Common)

#[[GLEW Configuration]]
add_subdirectory(lib/glew-cmake)
list(APPEND ONFS_LIBRARIES libglew_static)

#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)
list(APPEND ONFS_LIBRARIES glfw)

#[[OpenGL Configuration]]
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
list(APPEND ONFS_LIBRARIES ${OPENGL_LIBRARIES})

#[[Vulkan Configuration]]
if (${USE_VULKAN})
    find_package(Vulkan REQUIRED)
    message("VULKAN FOUND")
    include_directories(${Vulkan_INCLUDE_DIRS})
    list(APPEND ONFS_LIBRARIES ${Vulkan_LIBRARIES})
    CompileGLSLToSpirV(OpenNFS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vk" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vk")
    set(VULKAN_SOURCE_FILES src/Renderer/vkRenderer.cpp src/Renderer/vkRenderer.h)
    target_sources(OpenNFS PUBLIC ${VULKAN_SOURCE_FILES})
    add_definitions(-DVULKAN_BUILD)
endif ()

#[[Google Test Framework Configuration]]
add_subdirectory(lib/googletest)
enable_testing()
include_directories(lib/googletest/googletest/include)
# Add test cpp files
file(GLOB TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
list(REMOVE_ITEM SOURCE_FILES "src/main.cpp")
# From list of files we'll create tests test_name.cpp -> test_name
foreach (_test_file ${TEST_SRC_FILES})
    get_filename_component(_test_name ${_test_file} NAME_WE)
    add_executable(${_test_name} ${_test_file} ${SOURCE_FILES})
    target_link_libraries(${_test_name} gtest gtest_main ${ONFS_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_test(${_test_name} ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES TIMEOUT 10)
endforeach ()

#[[Get linky widdit]]
target_link_libraries(OpenNFS ${ONFS_LIBRARIES})