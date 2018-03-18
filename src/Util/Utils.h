//
// Created by Amrik on 11/03/2018.
//

#ifndef OPENNFS3_UTILS_H
#define OPENNFS3_UTILS_H

#define GLM_ENABLE_EXPERIMENTAL
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btMatrix3x3.h>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <GL/glew.h>
#include <vector>
#include <windef.h>
#include <afxres.h>
#include <cassert>
#include <cstdio>

#define ASSERT(condition,...) assert( \
    condition|| \
    (fprintf(stdout,__VA_ARGS__)&&fprintf(stdout," at %s:%d\n",__FILE__,__LINE__)) \
);


namespace Utils {
    bool LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei width, GLsizei height);

    glm::vec3 bulletToGlm(const btVector3& v);

    btVector3 glmToBullet(const glm::vec3& v);

    glm::quat bulletToGlm(const btQuaternion& q);

    btQuaternion glmToBullet(const glm::quat& q);

    btMatrix3x3 glmToBullet(const glm::mat3& m);

// btTransform does not contain a full 4x4 matrix, so this transform is lossy.
// Affine transformations are OK but perspective transformations are not.

    btBoxShape* genCollisionBox(std::vector<glm::vec3> model_vertices);
};


#endif //OPENNFS3_UTILS_H
