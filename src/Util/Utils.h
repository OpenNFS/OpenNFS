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
#include <cassert>
#include <cstdio>

#define BI_RGB 0x0000

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef long LONG;

#pragma pack(push,2)
typedef struct tagBITMAPFILEHEADER {
    WORD	bfType;
    DWORD	bfSize;
    WORD	bfReserved1;
    WORD	bfReserved2;
    DWORD	bfOffBits;
} BITMAPFILEHEADER,*LPBITMAPFILEHEADER,*PBITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, *PBITMAPINFO;

#define MAKEWORD(a,b)	((WORD)(((BYTE)(a))|(((WORD)((BYTE)(b)))<<8)))

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
    glm::vec3 genDimensions(std::vector<glm::vec3> model_vertices);
};


#endif //OPENNFS3_UTILS_H
