//
// Created by Amrik.Sadhra on 13/06/2018.
//

#pragma once

#include <imgui.h>
#include "Enums.h"

// TODO: Change these to consts
#define ASSET_PATH "./assets/"
#define CAR_PATH ASSET_PATH "car/"
#define TRACK_PATH ASSET_PATH "tracks/"
#define RESOURCE_PATH "../resources/"

#define NFS_2_TRACK_PATH  "/GAMEDATA/TRACKS/PC/"
#define NFS_2_CAR_PATH  "/GAMEDATA/CARMODEL/PC/"

#define NFS_2_SE_TRACK_PATH "/GAMEDATA/TRACKS/SE/"
#define NFS_2_SE_CAR_PATH "/GAMEDATA/CARMODEL/PC/"

#define NFS_3_TRACK_PATH "/gamedata/tracks/"
#define NFS_3_CAR_PATH  "/gamedata/carmodel/"

#define NFS_4_TRACK_PATH "/DATA/TRACKS/"
#define NFS_4_CAR_PATH "/DATA/CARS/"

#define MAX_TEXTURE_ARRAY_SIZE 512

struct ParamData {
    ImVec4 clear_color = ImVec4(119 / 255.0f, 197 / 255.0f, 252 / 255.0f, 1.0f);
    ImVec4 nfs2_rotate = ImVec4(0.f, 0.f, 0.f, 0.f);
    float trackSpecReflectivity = 1;

    float trackSpecDamper = 10;
    int blockDrawDistance = 15;
    bool window_active = true;
    bool physics_debug_view = false;
    bool use_classic_graphics = true;
    bool attach_cam_to_hermite = false;
    bool use_nb_data = true;
    bool attach_cam_to_car = true;
    bool frustum_cull = false;
    bool draw_vroad = false;
    bool draw_can = true;

    bool draw_raycast = true;
    bool simulate_car = false;
};

struct AssetData {
    NFSVer carTag;
    std::string car;
    NFSVer trackTag;
    std::string track;
};

struct NeedForSpeed {
    NFSVer tag;
    std::vector<std::string> tracks;
    std::vector<std::string> cars;
};