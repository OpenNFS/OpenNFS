#include "Frustum.h"

Frustum::Frustum(const glm::mat4 &viewProjectionMatrix) {
    this->Update(viewProjectionMatrix);
}

void Frustum::Update(const glm::mat4 &viewProjectionMatrix) {
    // Left Frustum Plane: Add first column of the matrix to the fourth column
    frustumPlanes[FrustumPlane::LEFT] = viewProjectionMatrix[0] + viewProjectionMatrix[3];
    // Right Frustum Plane: Subtract first column of matrix from the fourth column
    frustumPlanes[FrustumPlane::RIGHT] = viewProjectionMatrix[3] - viewProjectionMatrix[0];
    // Top Frustum Plane: Subtract second column of matrix from the fourth column
    frustumPlanes[FrustumPlane::TOP] = viewProjectionMatrix[3] - viewProjectionMatrix[1];
    // Bottom Frustum Plane: Add second column of the matrix to the fourth column
    frustumPlanes[FrustumPlane::BOTTOM] = viewProjectionMatrix[1] + viewProjectionMatrix[3];
    // Near Frustum Plane: Third column is the near plane
    frustumPlanes[FrustumPlane::NEAR_P] = viewProjectionMatrix[2];
    // Far Frustum Plane: Subtract third column of matrix from the fourth column
    frustumPlanes[FrustumPlane::FAR_P] = viewProjectionMatrix[3] - viewProjectionMatrix[2];

    // Normalise the planes
    for(uint8_t planeIdx = 0; planeIdx < FrustumPlane::Length; ++planeIdx)
    {
        //frustumPlanes[planeIdx] = glm::normalize(frustumPlanes[planeIdx]);
        // Ignore W component for length calculation
        float length = glm::length(glm::vec3(frustumPlanes[planeIdx]));
        frustumPlanes[planeIdx].x /= length;
        frustumPlanes[planeIdx].y /= length;
        frustumPlanes[planeIdx].z /= length;
        frustumPlanes[planeIdx].w /= length;
    }
}

bool Frustum::CheckIntersection(const AABB &other) const
{
    return true;
    // Loop through each frustum plane
    for (uint8_t planeIdx = 0; planeIdx < FrustumPlane::Length; ++planeIdx)
    {
        auto planeNormal = glm::vec3(frustumPlanes[planeIdx]);
        float planeConstant = frustumPlanes[planeIdx].w;

        // Check each axis (x,y,z) to get the AABB vertex furthest away from the direction the plane is facing (plane normal)
        glm::vec3 axisVert;

        // X-axis
        if (frustumPlanes[planeIdx].x < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the x axis
            axisVert.x = other.minX + other.position.x; // min x plus entity position x
        else
            axisVert.x = other.maxX + other.position.x; // max x plus entity position x

        // Y-axis
        if (frustumPlanes[planeIdx].y < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the y axis
            axisVert.y = other.minY + other.position.y; // min y plus entity position y
        else
            axisVert.y = other.maxY + other.position.y; // max y plus entity position y

        // Z-axis
        if (frustumPlanes[planeIdx].z < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the z axis
            axisVert.z = other.minZ + other.position.z; // min z plus entity position z
        else
            axisVert.z = other.maxZ + other.position.z; // max z plus entity position z

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

