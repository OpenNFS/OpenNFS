#include "Frustum.h"

void Frustum::Update(const glm::mat4 &projectionViewMatrix)
{
    this->_ExtractPlanes(projectionViewMatrix);
    this->_CalculatePlaneIntersections();
}

bool Frustum::CheckIntersection(const AABB &other) const
{
    // Loop through each frustum plane
    for (uint8_t planeIdx = 0; planeIdx < FrustumPlanes::Length; ++planeIdx)
    {
        auto planeNormal = glm::vec3(m_planes[planeIdx]);
        float planeConstant = m_planes[planeIdx].w;

        // Check each axis (x,y,z) to get the AABB vertex furthest away from the direction the plane is facing (plane normal)
        glm::vec3 axisVert;

        // X-axis
        if (m_planes[planeIdx].x < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the x axis
            axisVert.x = other.min.x + other.position.x; // min x plus entity position x
        else
            axisVert.x = other.max.x + other.position.x; // max x plus entity position x

        // Y-axis
        if (m_planes[planeIdx].y < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the y axis
            axisVert.y = other.min.y + other.position.y; // min y plus entity position y
        else
            axisVert.y = other.max.y + other.position.y; // max y plus entity position y

        // Z-axis
        if (m_planes[planeIdx].z < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the z axis
            axisVert.z = other.min.z + other.position.z; // min z plus entity position z
        else
            axisVert.z = other.max.z + other.position.z; // max z plus entity position z

        // Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
        // and if the signed distance is negative, then the entire bounding box is behind the frustum plane, which means
        // that it should be culled
        if((glm::dot(planeNormal, axisVert) + planeConstant) < 0.0f)
        {
            return false;
        }
    }
    return true;
}

template<FrustumPlanes a, FrustumPlanes b, FrustumPlanes c>
inline glm::vec3 Frustum::GetPlaneIntersection(const glm::vec3* crosses) const
{
    float D = glm::dot(glm::vec3(m_planes[a]), crosses[ij2k<b, c>::k]);
    glm::vec3 res = glm::mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
                    glm::vec3(m_planes[a].w, m_planes[b].w, m_planes[c].w);
    return res * (-1.0f / D);
}

void Frustum::_ExtractPlanes(const glm::mat4 &projectionViewMatrix)
{
    // Transpose for Column-Major ordering, cleans up indexing
    glm::mat4 projViewMatrixT = glm::transpose(projectionViewMatrix);

    // Left Frustum Plane: Add first column of the matrix to the fourth column
    m_planes[FrustumPlanes::LEFT]   = projViewMatrixT[3] + projViewMatrixT[0];
    // Right Frustum Plane: Subtract first column of matrix from the fourth column
    m_planes[FrustumPlanes::RIGHT]  = projViewMatrixT[3] - projViewMatrixT[0];
    // Top Frustum Plane: Subtract second column of matrix from the fourth column
    m_planes[FrustumPlanes::TOP]    = projViewMatrixT[3] - projViewMatrixT[1];
    // Bottom Frustum Plane: Add second column of the matrix to the fourth column
    m_planes[FrustumPlanes::BOTTOM] = projViewMatrixT[3] + projViewMatrixT[1];
    // Near Frustum Plane: Add third column of the matrix to the fourth column
    m_planes[FrustumPlanes::NEAR_P] = projViewMatrixT[3] + projViewMatrixT[2];
    // Far Frustum Plane: Subtract third column of matrix from the fourth column
    m_planes[FrustumPlanes::FAR_P]  = projViewMatrixT[3] - projViewMatrixT[2];

    // Normalise the planes
    for (uint8_t planeIdx = 0; planeIdx < FrustumPlanes::Length; ++planeIdx) {
        float length = glm::length(glm::vec3(m_planes[planeIdx]));
        m_planes[planeIdx] /= length;
    }

}

void Frustum::_CalculatePlaneIntersections()
{
    glm::vec3 crosses[Combinations] = {
            glm::cross(glm::vec3(m_planes[LEFT]),   glm::vec3(m_planes[RIGHT])),
            glm::cross(glm::vec3(m_planes[LEFT]),   glm::vec3(m_planes[BOTTOM])),
            glm::cross(glm::vec3(m_planes[LEFT]),   glm::vec3(m_planes[TOP])),
            glm::cross(glm::vec3(m_planes[LEFT]),   glm::vec3(m_planes[NEAR_P])),
            glm::cross(glm::vec3(m_planes[LEFT]),   glm::vec3(m_planes[FAR_P])),
            glm::cross(glm::vec3(m_planes[RIGHT]),  glm::vec3(m_planes[BOTTOM])),
            glm::cross(glm::vec3(m_planes[RIGHT]),  glm::vec3(m_planes[TOP])),
            glm::cross(glm::vec3(m_planes[RIGHT]),  glm::vec3(m_planes[NEAR_P])),
            glm::cross(glm::vec3(m_planes[RIGHT]),  glm::vec3(m_planes[FAR_P])),
            glm::cross(glm::vec3(m_planes[BOTTOM]), glm::vec3(m_planes[TOP])),
            glm::cross(glm::vec3(m_planes[BOTTOM]), glm::vec3(m_planes[NEAR_P])),
            glm::cross(glm::vec3(m_planes[BOTTOM]), glm::vec3(m_planes[FAR_P])),
            glm::cross(glm::vec3(m_planes[TOP]),    glm::vec3(m_planes[NEAR_P])),
            glm::cross(glm::vec3(m_planes[TOP]),    glm::vec3(m_planes[FAR_P])),
            glm::cross(glm::vec3(m_planes[NEAR_P]), glm::vec3(m_planes[FAR_P]))
    };

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
