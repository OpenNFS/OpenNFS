//
// Created by Amrik.Sadhra on 13/06/2018.
//

#pragma once

#include <imgui.h>

#define ASSET_PATH "./assets/"
#define CAR_PATH ASSET_PATH "car/"
#define TRACK_PATH ASSET_PATH "tracks/"

struct ParamData {
    ImVec4 clear_color = ImVec4(119 / 255.0f, 197 / 255.0f, 252 / 255.0f, 1.0f);
    ImVec4 test_light_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float trackSpecReflectivity = 1;

    float trackSpecDamper = 10;
    int blockDrawDistance = 15;
    bool window_active = true;
    bool physics_debug_view = false;
    bool use_classic_graphics = true;
    bool attach_cam_to_hermite = false;
    bool use_nb_data = true;
    bool attach_cam_to_car = true;
};