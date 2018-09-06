//
// Created by Amrik Sadhra on 21/08/2018.
//

#include "CarRenderer.h"

CarRenderer::CarRenderer(const shared_ptr<Car> &activeCar) : carShader(activeCar) {
    car = activeCar;
    if(!car->isMultitextured()){
        // TODO: Move this out of car shader and into Car constructor
        carShader.load_tga_texture();
    }
}

void CarRenderer::render(const Camera &mainCamera, const Light &cameraLight) {
    // Render the Car
    carShader.use();
    carShader.setPolyFlagged(car->hasPolyFlags());

    // Check if we're texturing the car from multiple textures, if we are, let the shader know with a uniform and bind texture array
    if(car->isMultitextured()){
        carShader.setMultiTextured(true);
        carShader.bindTextureArray(car->texture_array);
    } else {
        carShader.setMultiTextured(false);
        carShader.loadCarTexture();
    }

    for (auto &misc_model : car->misc_models) {
        carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, misc_model.ModelMatrix);
        carShader.loadSpecular(misc_model.specularDamper, 0, 0);
        carShader.loadCarColor(glm::vec3(1, 1, 1));
        carShader.loadLight(cameraLight);
        misc_model.render();
    }

    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->left_front_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->left_front_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->left_front_wheel_model.render();

    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->left_rear_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->left_rear_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->left_rear_wheel_model.render();

    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->right_front_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->right_front_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->right_front_wheel_model.render();

    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->right_rear_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->right_rear_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->right_rear_wheel_model.render();

    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->car_body_model.ModelMatrix);
    carShader.loadSpecular(car->car_body_model.specularDamper, car->car_body_model.specularReflectivity, car->car_body_model.envReflectivity);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->car_body_model.render();

    carShader.unbind();
}

CarRenderer::~CarRenderer() {
    // Cleanup VBOs and shaders
    carShader.cleanup();
}

