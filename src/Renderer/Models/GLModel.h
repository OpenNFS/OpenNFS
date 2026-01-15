#pragma once

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <utility>

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

    glm::mat4 ModelMatrix{1.0f};
    glm::mat4 RotationMatrix{1.0f};
    glm::mat4 TranslationMatrix{1.0f};

  protected:
    GLuint VertexArrayID{0};
};