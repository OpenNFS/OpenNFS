#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

namespace OpenNFS {

class InputManager {
  struct Inputs {
    bool accelerate{};
    bool reverse{};
    bool left{};
    bool right{};
}
public:
  InputManager(const std::shared_ptr<GLFWwindow> &window);
  void Scan();

  private:
    std::shared_ptr<GLFWwindow> m_window;
};

} // OpenNFS
