//
// Created by Amrik on 25/10/2017.
//
#pragma once

#include <vector>
#include <cstdlib>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

class Buffers {
public:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint elementbuffer;
    GLuint normalbuffer;
};

class NFS3_Mesh {
public:
    NFS3_Mesh(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms,
                  std::vector<unsigned int> indices);

    class iterator {
    public:
        explicit iterator(NFS3_Mesh *ptr): ptr(ptr){}
        iterator operator++() {++ptr; return *this;}
        bool operator!=(const iterator & other) { return ptr != other.ptr; }
        const NFS3_Mesh& operator*() const { return *ptr; }
    private:
        NFS3_Mesh* ptr;
    };
    NFS3_Mesh();
    explicit NFS3_Mesh(std::string name);
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
    /* Iterators to allow for ranged for loops with class*/
    iterator begin() const { return iterator(val); }
    iterator end() const { return iterator(val + len); }
    void render();
    void destroy();

    NFS3_Mesh *val;
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