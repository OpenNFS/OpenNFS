//
// Created by Amrik on 25/10/2017.
//

#include "Model.h"

#include <utility>

Model::Model(std::string name, int model_id, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms,
             std::vector<unsigned int> indices, bool removeVertexIndexing) {
    m_name = std::move(name);
    id =  model_id;
    m_uvs = std::move(uvs);
    m_vertex_indices = std::move(indices);
    m_normals = std::move(norms);

    if (removeVertexIndexing) {
        for (unsigned int m_vertex_index : m_vertex_indices) {
            m_vertices.push_back(verts[m_vertex_index]);
        }
    } else {
        m_vertices = std::move(verts);
    }

    position = glm::vec3(0, 0, 0);
    orientation_vec = glm::vec3(0,0,0);
    orientation = glm::normalize(glm::quat(orientation_vec));
    //Generate Physics collision data
    motionstate = new btDefaultMotionState(btTransform(
            btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w),
            btVector3(position.x, position.y, position.z)
    ));
    rigidBodyCI = btRigidBody::btRigidBodyConstructionInfo(
            0,                  // mass, in kg. 0 -> Static object, will never move.
            motionstate,
            //genCollisionBox(m_vertices),  // collision shape of body
            new btBoxShape(btVector3(5.0f, 5.0f, 5.0f)),
            btVector3(0, 0, 0)    // local inertia
    );
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
}

void Model::enable() {
    enabled = true;
}