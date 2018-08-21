//
// Created by Amrik Sadhra on 21/08/2018.
//

#include "CarRenderer.h"

CarRenderer::CarRenderer(const shared_ptr<Car> &activeCar) : carShader(activeCar) {
    car = activeCar;
    // Check if we're texturing the car from multiple textures.
    if(!car->isMultitextured()){
        carShader.load_tga_texture();
    }
}

void CarRenderer::render(Camera mainCamera, Light cameraLight) {
    // Render the Car
    carShader.use();
    // Clean this up
    if(car->isMultitextured()){
        carShader.setMultiTextured();
        carShader.bindCarTextures(car->car_body_model, car->car_texture_gl_mappings);
    } else {
        carShader.loadCarTexture();
    }
    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->car_body_model.ModelMatrix);
    carShader.loadSpecular(car->car_body_model.specularDamper, car->car_body_model.specularReflectivity, car->car_body_model.envReflectivity);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->car_body_model.render();

    if(car->left_front_wheel_model.texture_ids.size()){
        carShader.setMultiTextured();
        carShader.bindCarTextures(car->left_front_wheel_model, car->car_texture_gl_mappings);
    } else {
        carShader.loadCarTexture();
    }
    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->left_front_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->left_front_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->left_front_wheel_model.render();

    if(car->left_rear_wheel_model.texture_ids.size()){
        carShader.setMultiTextured();
        carShader.bindCarTextures(car->left_rear_wheel_model, car->car_texture_gl_mappings);
    } else {
        carShader.loadCarTexture();
    }
    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->left_rear_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->left_rear_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->left_rear_wheel_model.render();

    if(car->right_front_wheel_model.texture_ids.size()){
        carShader.setMultiTextured();
        carShader.bindCarTextures(car->right_front_wheel_model, car->car_texture_gl_mappings);
    } else {
        carShader.loadCarTexture();
    }
    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->right_front_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->right_front_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->right_front_wheel_model.render();

    if(car->right_rear_wheel_model.texture_ids.size()){
        carShader.setMultiTextured();
        carShader.bindCarTextures(car->right_rear_wheel_model, car->car_texture_gl_mappings);
    } else {
        carShader.loadCarTexture();
    }
    carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, car->right_rear_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->right_rear_wheel_model.specularDamper, 0, 0);
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    car->right_rear_wheel_model.render();

    for (auto &misc_model : car->misc_models) {
        // Clean this up
        if(misc_model.texture_ids.size()){
            carShader.setMultiTextured();
            carShader.bindCarTextures(misc_model, car->car_texture_gl_mappings);
        } else {
            carShader.loadCarTexture();
        }
        carShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, misc_model.ModelMatrix);
        carShader.loadSpecular(misc_model.specularDamper, 0, 0);
        carShader.loadCarColor(glm::vec3(1, 1, 1));
        carShader.loadLight(cameraLight);
        misc_model.render();
    }
    carShader.unbind();
}

CarRenderer::~CarRenderer() {
    // Cleanup VBOs and shaders
    carShader.cleanup();
}

