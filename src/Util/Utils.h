//
// Created by Amrik Sadhra on 11/03/2018.
//

#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <chrono>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btMatrix3x3.h>
#include <tiny_obj_loader.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <boost/filesystem/path.hpp>
#include <boost/preprocessor.hpp>
#include <boost/filesystem/operations.hpp>
#include <GL/glew.h>

#include "Logger.h"
#include "../Scene/CarModel.h"
#include "../Enums.h"

extern "C" {
#include "../../tools/fshtool.h"
}

// Define Windows Bitmap structs and macros with CP (CrossPlatform prefix) to avoid redef when conditionally including Windows.h for logging colour handles
#define CP_BI_RGB 0x0000

#pragma pack(push, 2)
typedef struct tagCP_BITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} CP_BITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagCP_BITMAPINFOHEADER {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} CP_BITMAPINFOHEADER, *PCP_BITMAPINFOHEADER;

typedef struct tagCP_RGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} CP_RGBQUAD;

typedef struct tagCP_BITMAPINFO {
    CP_BITMAPINFOHEADER bmiHeader;
    CP_RGBQUAD bmiColors[1];
} CP_BITMAPINFO;


#define SAFE_READ(file, structure, size)  if((file).read((char *) (structure), (size)).gcount() != (size)) return false
#define MAKEuint16_t(a, b)    ((uint16_t)(((uint8_t)(a))|(((uint16_t)((uint8_t)(b)))<<8)))

#define nyop "nop"

namespace Utils {
    class Timer {
    public:
        Timer() : m_beg(clock_::now()) {
        }
        void reset() {
            m_beg = clock_::now();
        }

        double elapsed() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                    clock_::now() - m_beg).count();
        }

    private:
        typedef std::chrono::high_resolution_clock clock_;
        typedef std::chrono::duration<double, std::ratio<1> > second_;
        std::chrono::time_point<clock_> m_beg;
    };


    float RandomFloat(float min, float max);

    glm::vec3 bulletToGlm(const btVector3 &v);

    btVector3 glmToBullet(const glm::vec3 &v);

    // btTransform does not contain a full 4x4 matrix, so this transform is lossy.
    // Affine transformations are OK but perspective transformations are not.
    glm::quat bulletToGlm(const btQuaternion &q);

    btQuaternion glmToBullet(const glm::quat &q);

    btMatrix3x3 glmToBullet(const glm::mat3 &m);

    btTransform glmToBullet(const glm::mat4& m);

    glm::mat4 bulletToGlm(const btTransform& t);

    btBoxShape *genCollisionBox(std::vector<glm::vec3> model_vertices);

    glm::vec3 genDimensions(std::vector<glm::vec3> model_vertices);

    unsigned int endian_swap(unsigned int x);

    unsigned int readInt32(FILE *file, bool littleEndian);

    // TODO: Move to resource handling class
    std::vector<CarModel> LoadOBJ(std::string obj_path);

    bool ExtractVIV(const std::string &viv_path, const std::string &output_dir);

    uint32_t abgr1555ToARGB8888(uint16_t abgr1555);

    bool SaveImage(const char *szPathName, void *lpBits, uint16_t w, uint16_t h);

    bool ExtractQFS(const std::string &qfs_input, const std::string &output_dir);

    bool ExtractPSH(const std::string &psh_path, const std::string &output_path);

    bool LoadBmpCustomAlpha(const char *fname, GLubyte **bits, GLsizei *width_, GLsizei *height_, uint8_t alphaColour);

    bool LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei *width_, GLsizei *height_);

    float HueToRGB(float v1, float v2, float vH);

    glm::vec3 HSLToRGB(glm::vec4 hsl);
}
