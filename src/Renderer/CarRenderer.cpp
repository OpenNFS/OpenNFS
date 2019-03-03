//
// Created by Amrik Sadhra on 21/08/2018.
//

#include "CarRenderer.h"

CarRenderer::CarRenderer(shared_ptr<Car> &activeCar) : carShader(activeCar) {
    car = activeCar;
    if(!car->isMultitextured()){
        // TODO: Move this out of car shader and into Car constructor
        carShader.load_tga_texture();
    }
}

void CarRenderer::render(const Camera &mainCamera, const std::vector<Light> &contributingLights) {
    carShader.use();

    // This shader state doesnt change during a car renderpass
    carShader.loadProjectionViewMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix);
    carShader.setPolyFlagged(car->hasPolyFlags());
    carShader.loadCarColor(glm::vec3(1, 1, 1));
    carShader.loadLights(contributingLights);
    carShader.loadEnvironmentMapTexture();
    // Check if we're texturing the car from multiple textures, if we are, let the shader know with a uniform and bind texture array
    carShader.setMultiTextured(car->isMultitextured());
    if(car->isMultitextured()){
        carShader.bindTextureArray(car->textureArrayID);
    } else {
        carShader.loadCarTexture();
    }

    // Render the Car models
    for (auto &misc_model : car->miscModels) {
        carShader.loadTransformationMatrix(misc_model.ModelMatrix);
        carShader.loadSpecular(misc_model.specularDamper, 0, 0);
        misc_model.render();
    }

    carShader.loadTransformationMatrix(car->leftFrontWheelModel.ModelMatrix);
    carShader.loadSpecular(car->leftFrontWheelModel.specularDamper, 0, 0);
    car->leftFrontWheelModel.render();

    carShader.loadTransformationMatrix(car->leftRearWheelModel.ModelMatrix);
    carShader.loadSpecular(car->leftRearWheelModel.specularDamper, 0, 0);
    car->leftRearWheelModel.render();

    carShader.loadTransformationMatrix(car->rightFrontWheelModel.ModelMatrix);
    carShader.loadSpecular(car->rightFrontWheelModel.specularDamper, 0, 0);
    car->rightFrontWheelModel.render();

    carShader.loadTransformationMatrix(car->rightRearWheelModel.ModelMatrix);
    carShader.loadSpecular(car->rightRearWheelModel.specularDamper, 0, 0);
    car->rightRearWheelModel.render();

    carShader.loadTransformationMatrix(car->carBodyModel.ModelMatrix);
    carShader.loadSpecular(car->carBodyModel.specularDamper, car->carBodyModel.specularReflectivity, car->carBodyModel.envReflectivity);
    car->carBodyModel.render();

    carShader.unbind();
}

CarRenderer::~CarRenderer() {
    // Cleanup VBOs and shaders
    carShader.cleanup();
}

