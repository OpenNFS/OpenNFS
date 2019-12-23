#include "CarRenderer.h"


void CarRenderer::render(shared_ptr<Car> &car, const Camera &mainCamera, const std::vector<Light> &contributingLights) {
    carShader.use();

    // This shader state doesnt change during a car renderpass
    carShader.loadProjectionViewMatrices(mainCamera.projectionMatrix, mainCamera.viewMatrix);
    carShader.setPolyFlagged(car->hasPolyFlags());
    carShader.loadCarColor(glm::vec3(1,1,1));
    carShader.loadLights(contributingLights);
    carShader.loadEnvironmentMapTexture();
    // Check if we're texturing the car from multiple textures, if we are, let the shader know with a uniform and bind texture array
    carShader.setMultiTextured(car->isMultitextured());
    if(car->isMultitextured()){
        carShader.bindTextureArray(car->textureArrayID);
    } else {
        carShader.loadCarTexture(car->textureID);
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
    carShader.loadCarColor(car->colour); // The colour should only apply to the car body
    car->carBodyModel.render();

    carShader.unbind();
}

CarRenderer::~CarRenderer() {
    // Cleanup VBOs and shaders
    carShader.cleanup();
}

