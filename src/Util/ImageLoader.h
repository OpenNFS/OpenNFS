#pragma once

#include <GL/glew.h>
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "Logger.h"

extern "C"
{
#include "../../tools/fshtool.h"
}

#define MAKEuint16_t(a, b) ((uint16_t)(((uint8_t)(a)) | (((uint16_t)((uint8_t)(b))) << 8)))

// Define Windows Bitmap structs and macros with CP (CrossPlatform prefix) to avoid redef when conditionally including Windows.h for logging colour handles
#define CP_BI_RGB 0x0000

#pragma pack(push, 2)
typedef struct tagCP_BITMAPFILEHEADER
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} CP_BITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagCP_BITMAPINFOHEADER
{
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

typedef struct tagCP_RGBQUAD
{
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} CP_RGBQUAD;

typedef struct tagCP_BITMAPINFO
{
    CP_BITMAPINFOHEADER bmiHeader;
    CP_RGBQUAD bmiColors[1];
} CP_BITMAPINFO;

class ImageLoader
{
private:
public:
    explicit ImageLoader();
    ~ImageLoader();
    static GLuint LoadImage(const std::string &imagePath, int *width, int *height, GLint wrapParam, GLint sampleParam);
    static bool SaveImage(const char *szPathName, void *lpBits, uint16_t w, uint16_t h);
    static uint32_t abgr1555ToARGB8888(uint16_t abgr1555);
    static bool ExtractQFS(const std::string &qfs_input, const std::string &output_dir);
    static bool ExtractPSH(const std::string &psh_path, const std::string &output_path);
    static bool LoadBmpCustomAlpha(const char *fname, GLubyte **bits, GLsizei *width_, GLsizei *height_, uint8_t alphaColour);
    static bool LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei *width_, GLsizei *height_);
};
