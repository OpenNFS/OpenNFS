//
// Created by Amrik on 28/10/2018.
//

#include "RaceNetRenderer.h"

RaceNetRenderer::RaceNetRenderer(GLFWwindow *gl_window, std::shared_ptr<Logger> &onfs_logger) : window(gl_window), logger(onfs_logger) {

}

void RaceNetRenderer::Render(std::vector<Car> car_list, shared_ptr<ONFSTrack> track_to_render) {
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glfwPollEvents();
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}