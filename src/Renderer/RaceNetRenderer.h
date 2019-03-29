//
// Created by Amrik on 28/10/2018.
//

#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "../Scene/Entity.h"
#include "../Loaders/TrackLoader.h"
#include "../Util/Logger.h"
#include "../Config.h"
#include "../Shaders/RaceNetShader.h"
#include "../RaceNet/CarAgent.h"

class RaceNetRenderer {
private:
    GLFWwindow *window;
    std::shared_ptr<Logger> logger;
    glm::mat4 projectionMatrix;
    RaceNetShader raceNetShader;
    float minX = -250.f, maxX = 250.f, minY = 140.625f, maxY = -140.625f; // Default area to display for start of training

    std::vector<int> GetVisibleTrackBlocks(shared_ptr <ONFSTrack> &track_to_render);
    void RescaleUI();
public:
    explicit RaceNetRenderer(GLFWwindow *gl_window, std::shared_ptr<Logger> &onfs_logger);
    ~RaceNetRenderer();
    void Render(uint32_t tick, std::vector <CarAgent> &carList, std::shared_ptr <ONFSTrack> &trackToRender);
};
