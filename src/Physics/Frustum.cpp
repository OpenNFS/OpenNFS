#include "Frustum.h"

void Frustum::Update(const glm::mat4 &projectionViewMatrix) {
    this->_ExtractPlanes(projectionViewMatrix);
    this->_CalculatePlaneIntersections();
}

bool Frustum::CheckIntersection(const AABB &other) const {
    // Loop through each frustum plane, checking box is outside of frustum for early rejection
    for (uint8_t planeIdx = 0; planeIdx < FrustumPlanes::Length; ++planeIdx) {
        // Check each axis (x,y,z) to get the AABB vertex furthest away from the direction the plane is facing (plane normal)
        float planeConstant = m_planes[planeIdx].w;
        if (((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.min.x, other.min.y, other.min.z)) + planeConstant) < 0.f) &&
            ((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.max.x, other.min.y, other.min.z)) + planeConstant) < 0.f) &&
            ((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.min.x, other.max.y, other.min.z)) + planeConstant) < 0.f) &&
            ((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.max.x, other.max.y, other.min.z)) + planeConstant) < 0.f) &&
            ((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.min.x, other.min.y, other.max.z)) + planeConstant) < 0.f) &&
            ((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.max.x, other.min.y, other.max.z)) + planeConstant) < 0.f) &&
            ((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.min.x, other.max.y, other.max.z)) + planeConstant) < 0.f) &&
            ((glm::dot(glm::vec3(m_planes[planeIdx]), other.position + glm::vec3(other.max.x, other.max.y, other.max.z)) + planeConstant) < 0.f)) {
            return false;
        }
    }

    // Now check at the vertex level whether we go through a frustum plane
    uint8_t out;
    out = 0;
    for (uint8_t vertIdx = 0; vertIdx < 8; vertIdx++)
        out += ((points[vertIdx].x > other.position.x + other.max.x) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (uint8_t vertIdx = 0; vertIdx < 8; vertIdx++)
        out += ((points[vertIdx].x < other.position.x + other.min.x) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (uint8_t vertIdx = 0; vertIdx < 8; vertIdx++)
        out += ((points[vertIdx].y > other.position.y + other.max.y) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (uint8_t vertIdx = 0; vertIdx < 8; vertIdx++)
        out += ((points[vertIdx].y < other.position.y + other.min.y) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (uint8_t vertIdx = 0; vertIdx < 8; vertIdx++)
        out += ((points[vertIdx].z > other.position.z + other.max.z) ? 1 : 0);
    if (out == 8)
        return false;
    out = 0;
    for (uint8_t vertIdx = 0; vertIdx < 8; vertIdx++)
        out += ((points[vertIdx].z < other.position.z + other.min.z) ? 1 : 0);
    if (out == 8)
        return false;

    return true;
}

template <FrustumPlanes a, FrustumPlanes b, FrustumPlanes c>
inline glm::vec3 Frustum::GetPlaneIntersection(const glm::vec3 *crosses) const {
    float D       = glm::dot(glm::vec3(m_planes[a]), crosses[ij2k<b, c>::k]);
    glm::vec3 res = glm::mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) * glm::vec3(m_planes[a].w, m_planes[b].w, m_planes[c].w);
    return res * (-1.0f / D);
}

void Frustum::_ExtractPlanes(const glm::mat4 &projectionViewMatrix) {
    // Transpose for Column-Major ordering, cleans up indexing
    glm::mat4 projViewMatrixT = glm::transpose(projectionViewMatrix);

    // Left Frustum Plane: Add first column of the matrix to the fourth column
    m_planes[FrustumPlanes::LEFT] = projViewMatrixT[3] + projViewMatrixT[0];
    // Right Frustum Plane: Subtract first column of matrix from the fourth column
    m_planes[FrustumPlanes::RIGHT] = projViewMatrixT[3] - projViewMatrixT[0];
    // Top Frustum Plane: Subtract second column of matrix from the fourth column
    m_planes[FrustumPlanes::TOP] = projViewMatrixT[3] - projViewMatrixT[1];
    // Bottom Frustum Plane: Add second column of the matrix to the fourth column
    m_planes[FrustumPlanes::BOTTOM] = projViewMatrixT[3] + projViewMatrixT[1];
    // Near Frustum Plane: Add third column of the matrix to the fourth column
    m_planes[FrustumPlanes::NEAR_P] = projViewMatrixT[3] + projViewMatrixT[2];
    // Far Frustum Plane: Subtract third column of matrix from the fourth column
    m_planes[FrustumPlanes::FAR_P] = projViewMatrixT[3] - projViewMatrixT[2];

    // Normalise the planes
    for (uint8_t planeIdx = 0; planeIdx < FrustumPlanes::Length; ++planeIdx) {
        float length = glm::length(glm::vec3(m_planes[planeIdx]));
        m_planes[planeIdx] /= length;
    }
}

void Frustum::_CalculatePlaneIntersections() {
    glm::vec3 crosses[FrustumPlanes::Combinations] = {
      glm::cross(glm::vec3(m_planes[LEFT]), glm::vec3(m_planes[RIGHT])),    glm::cross(glm::vec3(m_planes[LEFT]), glm::vec3(m_planes[BOTTOM])),
      glm::cross(glm::vec3(m_planes[LEFT]), glm::vec3(m_planes[TOP])),      glm::cross(glm::vec3(m_planes[LEFT]), glm::vec3(m_planes[NEAR_P])),
      glm::cross(glm::vec3(m_planes[LEFT]), glm::vec3(m_planes[FAR_P])),    glm::cross(glm::vec3(m_planes[RIGHT]), glm::vec3(m_planes[BOTTOM])),
      glm::cross(glm::vec3(m_planes[RIGHT]), glm::vec3(m_planes[TOP])),     glm::cross(glm::vec3(m_planes[RIGHT]), glm::vec3(m_planes[NEAR_P])),
      glm::cross(glm::vec3(m_planes[RIGHT]), glm::vec3(m_planes[FAR_P])),   glm::cross(glm::vec3(m_planes[BOTTOM]), glm::vec3(m_planes[TOP])),
      glm::cross(glm::vec3(m_planes[BOTTOM]), glm::vec3(m_planes[NEAR_P])), glm::cross(glm::vec3(m_planes[BOTTOM]), glm::vec3(m_planes[FAR_P])),
      glm::cross(glm::vec3(m_planes[TOP]), glm::vec3(m_planes[NEAR_P])),    glm::cross(glm::vec3(m_planes[TOP]), glm::vec3(m_planes[FAR_P])),
      glm::cross(glm::vec3(m_planes[NEAR_P]), glm::vec3(m_planes[FAR_P]))};

    // Calculate intersection point across 6 frustum planes for viz
    points[0] = GetPlaneIntersection<LEFT, TOP, FAR_P>(crosses);
    points[1] = GetPlaneIntersection<RIGHT, TOP, FAR_P>(crosses);
    points[2] = GetPlaneIntersection<RIGHT, BOTTOM, FAR_P>(crosses);
    points[3] = GetPlaneIntersection<LEFT, BOTTOM, FAR_P>(crosses);
    points[4] = GetPlaneIntersection<LEFT, TOP, NEAR_P>(crosses);
    points[5] = GetPlaneIntersection<RIGHT, TOP, NEAR_P>(crosses);
    points[6] = GetPlaneIntersection<RIGHT, BOTTOM, NEAR_P>(crosses);
    points[7] = GetPlaneIntersection<LEFT, BOTTOM, NEAR_P>(crosses);
}
