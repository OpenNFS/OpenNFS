#pragma once
// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>


extern GLFWwindow* window;

void resetView();
void computeMatricesFromInputs(bool &window_active, ImGuiIO& io);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
