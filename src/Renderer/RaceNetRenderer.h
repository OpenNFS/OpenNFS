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
#include "../Scene/Track.h"
#include "../Util/Logger.h"
#include "../Config.h"
#include "../Shaders/RaceNetShader.h"

class RaceNetRenderer {
public:
    explicit RaceNetRenderer(const std::shared_ptr<GLFWwindow> &window, const std::shared_ptr<Logger> &onfs_logger);
    ~RaceNetRenderer();
    void Render(uint32_t tick, std::vector<TrainingAgent> &carList, std::shared_ptr<Track> &trackToRender);

private:
    std::shared_ptr<GLFWwindow> m_window;
    std::shared_ptr<Logger> logger;
    glm::mat4 projectionMatrix;
    RaceNetShader raceNetShader;
    float minX = -250.f, maxX = 250.f, minY = 140.625f, maxY = -140.625f; // Default area to display for start of training

    std::vector<int> GetVisibleTrackBlocks(shared_ptr<Track> &track_to_render);
    void RescaleUI();
};
