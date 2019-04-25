#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <utility>
#include <vector>
#include <cstdlib>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <LinearMath/btDefaultMotionState.h>

class Model {
public:
    Model(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, bool removeVertexIndexing, glm::vec3 center_position);
    std::string m_name;
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs;
    std::vector<unsigned int> m_vertex_indices;

    void enable();

    virtual bool genBuffers()= 0;
    virtual void update()= 0;
    virtual void destroy()= 0;
    virtual void render()= 0;

    /*--------- Model State --------*/
    //UI
    bool enabled = false;
    //Rendering
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 RotationMatrix;
    glm::mat4 TranslationMatrix;
    glm::vec3 position;
    glm::vec3 initialPosition;
    glm::vec3 orientation_vec;
    glm::quat orientation;
protected:
    GLuint VertexArrayID;
};