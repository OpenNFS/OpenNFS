//
// Created by Amrik on 25/10/2017.
//
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <cstdlib>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "physics.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <LinearMath/btDefaultMotionState.h>

// TODO: Rename this to something reasonable
class Buffers {
public:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint elementbuffer;
    GLuint normalbuffer;
};

class Model {
public:
    Model(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms,
                 std::vector<unsigned int> indices, bool removeIndexing);

    std::string getName(void);

    std::vector<glm::vec2> getUVs(void);

    std::vector<glm::vec3> getVertices(void);

    std::vector<glm::vec3> getNormals(void);

    std::vector<unsigned int> getIndices(void);

    bool genBuffers();

    void setShaderID(GLuint shaderID);

    void enable();

    void update();

    void destroy();

    void render();

    /*--------- Model State --------*/
    //UI
    bool enabled = false;
    bool indexed = false;
    bool track = false; // Sign I need to superclass Model and provide Track and Car mesh implementations...
    GLuint shader_id = 0;
    std::vector<short> texture_ids;
    //Rendering
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 RotationMatrix;
    glm::mat4 TranslationMatrix;
    glm::vec3 position;
    glm::vec3 orientation_vec;
    glm::quat orientation;
    // Physics
    btRigidBody *rigidBody;
    btDefaultMotionState* motionstate;
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(0, nullptr, nullptr);

    /* Iterators to allow for ranged for loops with class*/
    class iterator {
    public:
        explicit iterator(Model *ptr) : ptr(ptr) { }

        iterator operator++() {
            ++ptr;
            return *this;
        }

        bool operator!=(const iterator &other) { return ptr != other.ptr; }

        const Model &operator*() const { return *ptr; }

    private:
        Model *ptr;
    };

    iterator begin() const { return iterator(val); }

    iterator end() const { return iterator(val + len); }

    Model *val;
private:
    std::string m_name;
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs;
    std::vector<unsigned int> m_vertex_indices;
    /* Iterator vars */
    unsigned len;
    Buffers gl_buffers;
};