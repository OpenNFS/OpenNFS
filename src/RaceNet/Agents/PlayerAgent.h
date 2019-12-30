#pragma once

#include "CarAgent.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class PlayerAgent : public CarAgent {
public:
    PlayerAgent(GLFWwindow *pWindow, const std::shared_ptr<Car> &car, const std::shared_ptr<ONFSTrack> &raceTrack);
    void Simulate() override;

private:
    GLFWwindow *m_pWindow;
};
