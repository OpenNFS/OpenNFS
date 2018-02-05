//
// Created by Amrik on 25/10/2017.
//
#pragma once

#include <vector>
#include <cstdlib>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

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
                  std::vector<unsigned int> indices);

    class iterator {
    public:
        explicit iterator(Model *ptr): ptr(ptr){}
        iterator operator++() {++ptr; return *this;}
        bool operator!=(const iterator & other) { return ptr != other.ptr; }
        const Model& operator*() const { return *ptr; }
    private:
        Model* ptr;
    };
    Model();
    explicit Model(std::string name);
    std::string getName(void);
    std::vector<glm::vec2> getUVs(void);
    std::vector<glm::vec3> getVertices(void);
    std::vector<glm::vec3> getNormals(void);
    bool genBuffers();
    std::vector<unsigned int> getIndices(void);
    void setUVs(std::vector<glm::vec2> uvs);
    void setVertices(std::vector<glm::vec3> verts);
    void setNormals(std::vector<glm::vec3> norms);
    void setIndices(std::vector<unsigned int> indices);
    void enable();
    void update();
    bool enabled = false;
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    /* Iterators to allow for ranged for loops with class*/
    iterator begin() const { return iterator(val); }
    iterator end() const { return iterator(val + len); }
    void render();
    void destroy();
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