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
#include "../Loaders/trk_loader.h"
#include "../Util/Logger.h"
#include "../Config.h"

class RaceNetRenderer {
private:
    GLFWwindow *window;
    std::shared_ptr<Logger> logger;
public:
    explicit RaceNetRenderer(GLFWwindow *gl_window, std::shared_ptr<Logger> &onfs_logger);

    void Render(std::vector <Car> car_list, shared_ptr <ONFSTrack> track_to_render);
};
