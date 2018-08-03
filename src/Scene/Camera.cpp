#include "Camera.h"

#include <glm/gtx/quaternion.hpp>
Camera::Camera(glm::vec3 initial_position, float FoV, float horizontal_angle, float vertical_angle, GLFWwindow *gl_window) {
    window = gl_window;
    // Initial position : on +Z
    position = initial_position;
    initialPosition = initial_position;
    // Initial Field of View
    initialFoV = FoV;
    horizontalAngle = horizontal_angle;
    verticalAngle = vertical_angle;
    // Projection matrix : 45deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 1000.0f);
}

void Camera::resetView() {
    position = initialPosition;
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

void Camera::generateSpline(std::vector<TrackBlock> trackBlock) {
    std::vector<glm::vec3> cameraPoints;
    for (auto &track_block : trackBlock) {
        cameraPoints.emplace_back(
                glm::vec3(track_block.center.x / 10, track_block.center.y / 10, track_block.center.z / 10));
    }
    cameraSpline = HermiteCurve(cameraPoints, 0.5, 0.0f);
    loopTime = cameraSpline.points.size() * 100;
    hasSpline = true;
}

void Camera::useSpline() {
    ASSERT(hasSpline, "Attempted to use Camera spline without generating one first with \'generateSpline\'");
    totalTime += deltaTime;
    float tmod = (totalTime) / (loopTime / 200);
    position = cameraSpline.getPointAt(tmod);
}

void Camera::calculateCameraPosition(const shared_ptr<Car> &target_car, float horizDistance, float vertDistance) {
    float theta =  (target_car->getRotY() +  angleAroundCar) - 180;
    float offsetX = horizDistance * sin(theta * (SIMD_PI / 180.0f));
    float offsetZ = horizDistance * cos(theta * (SIMD_PI / 180.0f));
    position.x = target_car->car_models[0].position.x - offsetX;
    position.z = target_car->car_models[0].position.z - offsetZ;
    position.y = target_car->car_models[0].position.y + vertDistance;
}

void Camera::calculateZoom() {
    float zoomLevel = ImGui::GetIO().MouseWheel * 0.1f;
    distanceFromCar -= zoomLevel;
}

void Camera::calculatePitch() {
    if (ImGui::GetIO().MouseDown[1]) {
        float pitchChange = ImGui::GetIO().MouseDelta.y * 0.1f;
        pitch -= pitchChange;
    }
}

void Camera::calculateAngleAroundCar() {
    if (ImGui::GetIO().MouseDown[0]) {
        float angleChange = ImGui::GetIO().MouseDelta.x * 0.3f;
        angleAroundCar -= angleChange;
    }
}

float Camera::calculateVerticalDistance() {
    return distanceFromCar * sin(pitch * (SIMD_PI / 180.0f));
}

float Camera::calculateHorizontalDistance() {
    return distanceFromCar * cos(pitch * (SIMD_PI / 180));
}

void Camera::followCar(const shared_ptr<Car> &target_car, bool &window_active, ImGuiIO &io){
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

    // Blessed be ThinMatrix
    calculateZoom();
    calculatePitch();
    calculateAngleAroundCar();
    float horizontalDistance = calculateHorizontalDistance();
    float verticalDistance = calculateVerticalDistance();
    calculateCameraPosition(target_car, horizontalDistance, verticalDistance);
    yaw = 180 - ((target_car->getRotY() +  angleAroundCar)-180);

    ViewMatrix = glm::mat4(1.0f);
    ViewMatrix = glm::rotate(ViewMatrix, pitch * SIMD_PI/180, glm::vec3(1,0,0));
    ViewMatrix = glm::rotate(ViewMatrix, yaw * SIMD_PI/180, glm::vec3(0,1,0));
    glm::vec3 negativeCameraPos(-position);
    ViewMatrix = glm::translate(ViewMatrix, negativeCameraPos);

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}

void Camera::computeMatricesFromInputs(bool &window_active, ImGuiIO &io) {
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

    if (ImGui::GetIO().MouseDown[1]) {
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
