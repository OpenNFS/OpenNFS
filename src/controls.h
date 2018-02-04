#pragma once
extern GLFWwindow* window;

void computeMatricesFromInputs(bool &window_active, ImGuiIO& io);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
