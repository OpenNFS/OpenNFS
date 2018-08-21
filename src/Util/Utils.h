//
// Created by Amrik Sadhra on 11/03/2018.
//

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btMatrix3x3.h>
#include <tiny_obj_loader.h>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <boost/filesystem/path.hpp>
#include <boost/preprocessor.hpp>
#include <boost/filesystem/operations.hpp>
#include <GL/glew.h>
#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "../Scene/CarModel.h"

extern "C" {
    #include "../../tools/fshtool.h"
}

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


#define SAFE_READ(file, structure, size)  if((file).read((char *) (structure), (size)).gcount() != (size)) return false
#define MAKEWORD(a,b)	((WORD)(((BYTE)(a))|(((WORD)((BYTE)(b)))<<8)))
#define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)

#define nyop "nop"

#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)    \
    case elem : return BOOST_PP_STRINGIZE(elem);

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)                \
    enum name {                                                               \
        BOOST_PP_SEQ_ENUM(enumerators)                                        \
    };                                                                        \
                                                                              \
    inline const char* ToString(name v)                                       \
    {                                                                         \
        switch (v)                                                            \
        {                                                                     \
            BOOST_PP_SEQ_FOR_EACH(                                            \
                X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,          \
                name,                                                         \
                enumerators                                                   \
            )                                                                 \
            default: return "[Unknown " BOOST_PP_STRINGIZE(name) "]";         \
        }                                                                     \
    }

namespace Utils {
    glm::vec3 bulletToGlm(const btVector3 &v);

    btVector3 glmToBullet(const glm::vec3 &v);

    // btTransform does not contain a full 4x4 matrix, so this transform is lossy.
    // Affine transformations are OK but perspective transformations are not.
    glm::quat bulletToGlm(const btQuaternion &q);

    btQuaternion glmToBullet(const glm::quat &q);

    btMatrix3x3 glmToBullet(const glm::mat3 &m);

    btBoxShape *genCollisionBox(std::vector<glm::vec3> model_vertices);

    glm::vec3 genDimensions(std::vector<glm::vec3> model_vertices);

    unsigned int endian_swap(unsigned int x);

    unsigned int readInt32(FILE *file, bool littleEndian);

    // TODO: Move to resource handling class
    std::vector<CarModel> LoadOBJ(std::string obj_path);

    bool ExtractVIV(const std::string &viv_path, const std::string &output_dir);

    bool SaveImage(const char *szPathName, void *lpBits, uint16_t w, uint16_t h);

    bool ExtractQFS(const std::string &qfs_input, const std::string &output_dir);

    bool ExtractPSH(const std::string &psh_path, const std::string &output_path);

    bool LoadBmpCustomAlpha(const char *fname, GLubyte **bits, GLsizei *width_, GLsizei *height_, uint8_t alphaColour);

    bool LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei width, GLsizei height);
}
