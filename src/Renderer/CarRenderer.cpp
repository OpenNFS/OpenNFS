#include "CarRenderer.h"

void CarRenderer::Render(const shared_ptr<Car> &car, const BaseCamera &camera, const std::vector<std::shared_ptr<BaseLight>> &lights) {
    m_carShader.use();

    // This shader state doesnt change during a car renderpass
    m_carShader.loadProjectionViewMatrices(camera.projectionMatrix, camera.viewMatrix);
    m_carShader.setPolyFlagged(car->carBodyModel.hasPolyFlags);
    m_carShader.loadCarColor(glm::vec3(1, 1, 1));
    m_carShader.loadLights(lights);
    m_carShader.loadEnvironmentMapTexture();
    // Check if we're texturing the car from multiple textures, if we are, let the shader know with a uniform and bind texture array
    m_carShader.setMultiTextured(car->renderInfo.isMultitexturedModel);
    if (car->renderInfo.isMultitexturedModel) {
        m_carShader.bindTextureArray(car->renderInfo.textureArrayID);
    } else {
        m_carShader.loadCarTexture(car->renderInfo.textureID);
    }

    // Render the Car models
    for (auto &misc_model : car->miscModels) {
        m_carShader.loadTransformationMatrix(misc_model.ModelMatrix);
        m_carShader.loadSpecular(misc_model.specularDamper, 0, 0);
        misc_model.render();
    }

    m_carShader.loadTransformationMatrix(car->leftFrontWheelModel.ModelMatrix);
    m_carShader.loadSpecular(car->leftFrontWheelModel.specularDamper, 0, 0);
    car->leftFrontWheelModel.render();

    m_carShader.loadTransformationMatrix(car->leftRearWheelModel.ModelMatrix);
    m_carShader.loadSpecular(car->leftRearWheelModel.specularDamper, 0, 0);
    car->leftRearWheelModel.render();

    m_carShader.loadTransformationMatrix(car->rightFrontWheelModel.ModelMatrix);
    m_carShader.loadSpecular(car->rightFrontWheelModel.specularDamper, 0, 0);
    car->rightFrontWheelModel.render();

    m_carShader.loadTransformationMatrix(car->rightRearWheelModel.ModelMatrix);
    m_carShader.loadSpecular(car->rightRearWheelModel.specularDamper, 0, 0);
    car->rightRearWheelModel.render();

    m_carShader.loadTransformationMatrix(car->carBodyModel.ModelMatrix);
    m_carShader.loadSpecular(car->carBodyModel.specularDamper, car->carBodyModel.specularReflectivity, car->carBodyModel.envReflectivity);
    m_carShader.loadCarColor(car->vehicleProperties.colour); // The colour should only apply to the car body
    car->carBodyModel.render();

    m_carShader.unbind();
}

CarRenderer::~CarRenderer() {
    // Cleanup VBOs and shaders
    m_carShader.cleanup();
}
