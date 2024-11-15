cmake_minimum_required(VERSION 3.15)

project(LibOpenNFS VERSION 0.0.1 LANGUAGES CXX)

set(LIB_OPENNFS_SOURCES
        LibOpenNFS.h
        Common/Utils.cpp
        Common/TextureUtils.cpp
        lib/fshtool/fshtool.c
        Entities/BaseLight.cpp
        Entities/Car.cpp
        Entities/CarGeometry.cpp
        Entities/Geometry.cpp
        Entities/TrackSound.cpp
        Entities/Track.cpp
        Entities/TrackBlock.cpp
        Entities/TrackEntity.cpp
        Entities/TrackGeometry.cpp
        Entities/TrackLight.cpp
        Entities/TrackTexture.cpp
        Entities/TrackVRoad.cpp
        #[[NFS2/NFS2Loader.cpp]]
        NFS2/COL/ColFile.cpp
        NFS2/GEO/GeoFile.cpp
        NFS2/PSH/PshFile.cpp
        NFS2/TRK/ExtraObjectBlock.cpp
        NFS2/TRK/StructureBlock.cpp
        NFS2/TRK/StructureRefBlock.cpp
        NFS2/TRK/SuperBlock.cpp
        NFS2/TRK/TrackBlock.cpp
        NFS2/TRK/TrkFile.cpp
        NFS3/NFS3Loader.cpp
        NFS3/COL/ColFile.cpp
        NFS3/FCE/FceFile.cpp
        NFS3/FEDATA/FedataFile.cpp
        NFS3/FFN/FfnFile.cpp
        NFS3/FRD/ExtraObjectBlock.cpp
        NFS3/FRD/FrdFile.cpp
        NFS3/FRD/PolyBlock.cpp
        NFS3/FRD/TexBlock.cpp
        NFS3/FRD/TrkBlock.cpp
        NFS3/SPEEDS/SpeedsFile.cpp
        Shared/CanFile.cpp
        Shared/HrzFile.cpp
        Shared/VivFile.cpp)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${LIB_OPENNFS_SOURCES})

#[[GLM Configuration]]
add_subdirectory(lib/glm)
target_link_libraries(${PROJECT_NAME} glm)

# sets the search paths for the include files after installation
# as well as during when building the library (as these may differ)
# this allows the library itself and users to #include the library headers
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# this command will append "d" to the name of the debug version of
# the library - this is very helpful when installing as it ensures
# the debug and release version of library can be installed to the
# same location and will not conflict (overwrite each other)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

