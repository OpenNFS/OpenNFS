#pragma once

#include "CarAgent.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class PlayerAgent : public CarAgent {
public:
    PlayerAgent(GLFWwindow *pWindow, std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> raceTrack);
    void Simulate();

private:
    GLFWwindow *m_pWindow;
};
