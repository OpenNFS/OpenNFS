#include "Camera.h"

using namespace glm;

Camera::Camera(glm::vec3 initial_position, float FoV, float horizontal_angle, float vertical_angle, GLFWwindow *gl_window){
    window = gl_window;
    // Initial position : on +Z
    position = initial_position;
    // Initial Field of View
    initialFoV = FoV;
    horizontalAngle = horizontal_angle;
    verticalAngle = vertical_angle;
}

void Camera::resetView(){
    position = glm::vec3(0, 0, 5);
    horizontalAngle = 3.14f;
    verticalAngle = 0.0f;
    glm::vec3 direction(
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle)
    );
    glm::vec3 right = glm::vec3(
            sin(horizontalAngle - 3.14f / 2.0f),
            0,
            cos(horizontalAngle - 3.14f / 2.0f)
    );
    // Camera matrix
    ViewMatrix = glm::lookAt(
            position,
            position + direction,
            glm::cross(right, direction)
    );
}

void Camera::computeMatricesFromInputs(bool &window_active, ImGuiIO& io) {
    if (!window_active)
        return;
    // Bail on the window active status if we hit the escape key
    window_active = (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
    io.MouseDrawCursor = true;

    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    deltaTime = float(currentTime - lastTime);

    // Get mouse position
    double xpos, ypos;

    // Get mouse position and compute new orientation with it
    horizontalAngle += mouseSpeed * (1024 / 2 - io.MousePos.x);
    verticalAngle += mouseSpeed * (768 / 2 - io.MousePos.y);

    // Reset mouse position for next frame
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 direction(
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle)
    );

    // Right vector
    glm::vec3 right = glm::vec3(
            sin(horizontalAngle - 3.14f / 2.0f),
            0,
            cos(horizontalAngle - 3.14f / 2.0f)
    );

    // Up vector
    glm::vec3 up = glm::cross(right, direction);

    // Speed boost
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        speed = 100.0f;
    } else {
        speed = 3.0f;
    }

    if(ImGui::GetIO().MouseDown[1]){
    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += direction * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= direction * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += right * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= right * deltaTime * speed;
    }
    }

    float FoV = initialFoV;

    // Projection matrix : 45deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 1000.0f);
    // Camera matrix
    ViewMatrix = glm::lookAt(
            position,           // Camera is here
            position + direction, // and looks here : at the same position, plus "direction"
            up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}

Camera::Camera() {}
