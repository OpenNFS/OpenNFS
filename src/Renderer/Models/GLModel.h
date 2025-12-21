#pragma once

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class GLModel {
  public:
    explicit GLModel() = default;
    virtual ~GLModel() = default;
    void Enable() {
        enabled = true;
    }

    virtual bool GenBuffers() = 0;
    virtual void UpdateMatrices() = 0;
    virtual void Render() = 0;

    /*--------- Model State --------*/
    bool enabled{};
    bool buffersGenerated{};

    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 RotationMatrix{};
    glm::mat4 TranslationMatrix{};

  protected:
    GLuint VertexArrayID;
};