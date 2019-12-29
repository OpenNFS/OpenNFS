cmake_minimum_required(VERSION 3.4)
project(OpenNFS)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(AddCompilerFlag)
include(OptimizeForArchitecture)
include(CompileGLSLToSPIRV)

#[[OptimizeForArchitecture()]
MESSAGE(STATUS "Compiler flags are: " ${CMAKE_CXX_FLAGS})

#[[Static linking broken under Mac OSX]]
if (NOT (APPLE OR UNIX))
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
endif ()

set(OpenGL_GL_PREFERENCE GLVND)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)

file(GLOB CRP_LIB_SOURCES
        include/CrpLib/*.h
        include/CrpLib/*.cpp
        )

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
        #[[include/ImGuiColorTextEdit/TextEditor.cpp
        include/ImGuiColorTextEdit/TextEditor.h]]
        resources/asset/icon.rc
        src/main.cpp
        src/Camera/Camera.cpp
        src/Scene/Model.cpp
        src/Physics/PhysicsEngine.cpp
        lib/glew-cmake/src/glew.c
        src/Scene/TrackBlock.cpp
        src/Scene/TrackBlock.h
        src/nfs_data.h
        src/Scene/Light.cpp
        src/Scene/Light.h
        src/Camera/Camera.cpp
        src/Camera/Camera.h
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
        src/Loaders/MusicLoader.cpp
        src/Loaders/MusicLoader.h
        src/Config.cpp
        src/Config.h
        src/Loaders/NFS2Loader.cpp
        src/Loaders/NFS2Loader.h
        src/Loaders/NFS3Loader.cpp
        src/Loaders/NFS3Loader.h
        src/Loaders/TrackLoader.cpp
        src/Loaders/TrackLoader.h
        src/Loaders/NFS2Loader.cpp
        src/Loaders/NFS4Loader.cpp
        src/Loaders/NFS4Loader.h
        src/Renderer/Renderer.cpp
        src/Renderer/Renderer.h
        src/Loaders/TrackUtils.cpp
        src/Loaders/TrackUtils.h
        src/Loaders/CarLoader.cpp
        src/Loaders/CarLoader.h
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
        src/Renderer/ShadowMapRenderer.h
        src/Shaders/ShaderSet.cpp
        src/Shaders/ShaderSet.h
        shaders/ShaderPreamble.h
        src/RaceNet/RaceNEAT.cpp
        src/RaceNet/RaceNEAT.h
        src/RaceNet/CarAgent.cpp
        src/RaceNet/CarAgent.h
        src/Race/RaceSession.cpp
        src/Race/RaceSession.h
        src/Scene/Spotlight.cpp
        src/Scene/Spotlight.h
        #[[src/Renderer/MenuRenderer.cpp
        src/Renderer/MenuRenderer.h]]
        src/Shaders/FontShader.cpp
        src/Shaders/FontShader.h
        src/Shaders/MenuShader.cpp
        src/Shaders/MenuShader.h
        src/Util/ImageLoader.cpp
        src/Util/ImageLoader.cpp
        src/Util/ImageLoader.h
        src/Loaders/NFS5Loader.cpp
        src/Loaders/NFS5Loader.h
        src/Loaders/NFS4PS1Loader.cpp
        src/Loaders/NFS4PS1Loader.h
        src/Loaders/SerializedGroupOps.cpp
        src/Loaders/SerializedGroupOps.h
        src/Scene/GlobalLight.cpp
        src/Scene/GlobalLight.h
        src/Physics/AABB.cpp
        src/Physics/AABB.h
        src/Physics/AABBTree.h
        src/Physics/AABBTree.cpp
        src/Physics/IAABB.h
        src/Physics/Frustum.cpp
        src/Physics/Frustum.h
        src/Camera/HermiteCamera.cpp
        src/Camera/HermiteCamera.h
        src/Camera/CarCamera.cpp
        src/Camera/CarCamera.h
        src/Camera/FreeCamera.cpp
        src/Camera/FreeCamera.h
        src/Renderer/BulletDebugDrawer.cpp
        src/Renderer/BulletDebugDrawer.h
        src/Shaders/BulletShader.cpp
        src/Shaders/BulletShader.h
        src/Renderer/DebugRenderer.cpp
        src/Renderer/DebugRenderer.h)

add_executable(OpenNFS ${SOURCE_FILES} ${CRP_LIB_SOURCES})


#[[JSON]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/json")

#[[STB]]
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/stb")

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
set(ENABLE_VECTORED_EXCEPTIONHANDLING ON CACHE BOOL "Turn off to handoff exception to system debugger")
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
#[[Avoid Vulkan on Mac, until I add MoltenVK support. Avoid Windows too until I add Vulkan SDK to VSTS container]]
if (NOT (APPLE OR WIN32 OR UNIX))
    find_package(Vulkan REQUIRED)
    message("VULKAN FOUND")
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries(OpenNFS ${Vulkan_LIBRARIES})
    CompileGLSLToSpirV(OpenNFS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vk" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vk")
    set(VULKAN_SOURCE_FILES src/Renderer/vkRenderer.cpp src/Renderer/vkRenderer.h)
    target_sources(OpenNFS PUBLIC ${VULKAN_SOURCE_FILES})
    add_definitions(-DVULKAN_BUILD)
endif ()

#[[GLFW Configuration]]
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw/ "${CMAKE_CURRENT_BINARY_DIR}/glfw")
target_link_libraries(OpenNFS glfw)

#[[Google Test Framework Configuration]]
#[[
add_subdirectory(lib/googletest)
# Setup testing
enable_testing()
include_directories(lib/googletest/googletest/include)
list(REMOVE_ITEM SOURCE_FILES "src/main.cpp")
# Add test cpp files
file(GLOB TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
# From list of files we'll create tests test_name.cpp -> test_name
foreach(_test_file ${TEST_SRC_FILES})
    get_filename_component(_test_name ${_test_file} NAME_WE)
    add_executable(${_test_name} ${_test_file} ${SOURCE_FILES} ${NEAT_SOURCE_FILES})
    target_link_libraries(${_test_name} gtest gtest_main freetype Boost::program_options Boost::filesystem Boost::system Boost::boost g3logger BulletDynamics BulletCollision LinearMath Bullet3Common ${OPENGL_LIBRARIES} glfw ${CMAKE_THREAD_LIBS_INIT})
    add_test(${_test_name} ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES TIMEOUT 10)
endforeach()]]
