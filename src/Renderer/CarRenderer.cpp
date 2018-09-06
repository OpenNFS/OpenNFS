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
    carShader.use();

    // This shader state doesnt change during a car renderpass
    carShader.loadProjectionViewMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix);
    carShader.setPolyFlagged(car->hasPolyFlags());
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLight(cameraLight);
    carShader.loadEnvironmentMapTexture();
    // Check if we're texturing the car from multiple textures, if we are, let the shader know with a uniform and bind texture array
    carShader.setMultiTextured(car->isMultitextured());
    if(car->isMultitextured()){
        carShader.bindTextureArray(car->textureArrayID);
    } else {
        carShader.loadCarTexture();
    }

    // Render the Car models
    for (auto &misc_model : car->misc_models) {
        carShader.loadTransformationMatrix(misc_model.ModelMatrix);
        carShader.loadSpecular(misc_model.specularDamper, 0, 0);
        misc_model.render();
    }

    carShader.loadTransformationMatrix(car->left_front_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->left_front_wheel_model.specularDamper, 0, 0);
    car->left_front_wheel_model.render();

    carShader.loadTransformationMatrix(car->left_rear_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->left_rear_wheel_model.specularDamper, 0, 0);
    car->left_rear_wheel_model.render();

    carShader.loadTransformationMatrix(car->right_front_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->right_front_wheel_model.specularDamper, 0, 0);
    car->right_front_wheel_model.render();

    carShader.loadTransformationMatrix(car->right_rear_wheel_model.ModelMatrix);
    carShader.loadSpecular(car->right_rear_wheel_model.specularDamper, 0, 0);
    car->right_rear_wheel_model.render();

    carShader.loadTransformationMatrix(car->car_body_model.ModelMatrix);
    carShader.loadSpecular(car->car_body_model.specularDamper, car->car_body_model.specularReflectivity, car->car_body_model.envReflectivity);
    car->car_body_model.render();

    carShader.unbind();
}

CarRenderer::~CarRenderer() {
    // Cleanup VBOs and shaders
    carShader.cleanup();
}

