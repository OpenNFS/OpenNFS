#pragma once

#include <GL/glew.h>
#include <string>
#include <filesystem>

#include "Logger.h"



#define MAKEuint16_t(a, b) ((uint16_t) (((uint8_t) (a)) | (((uint16_t) ((uint8_t) (b))) << 8)))

// Define Windows Bitmap structs and macros with CP (CrossPlatform prefix) to avoid redef when conditionally including Windows.h for logging
// colour handles
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

// Temporary, until LibOpenNFS PSH support
struct PSH {
    struct HEADER {
        char header[4];        //  "SHPP"
        uint32_t length;       // Inclusive Length of the PSH file
        uint32_t nDirectories; // Number of directory entries
        char chk[4];           // "GIMX"
    };

    struct DIR_ENTRY {
        char imageName[4];
        uint32_t imageOffset; // Offset to start of the image, len implied by difference between offsets to next
    };

    struct IMAGE_HEADER {
        uint8_t imageType; // Image type: Observed values are 0x40, 0x42, 0x43, and 0xC0 The bottom 2 bits of the image type byte specify
        // the bit depth of the image: 0 - 4-bit indexed colour 2 - 16-bit direct colour 3 - 24-bit direct colour
        uint8_t unknown[3];
        uint16_t width;
        uint16_t height;
        uint16_t unknown2[4];
    };

    struct PALETTE_HEADER {
        uint32_t unknown;
        uint16_t paletteWidth;    // Always 16
        uint16_t paletteHeight;   // Always 1
        uint16_t nPaletteEntries; // Always 16
        uint16_t unknown2[3];     // [0] always 0 [1] always 0 [2] often 240, sometimes 0
    };
};

class ImageLoader {
private:
public:
    explicit ImageLoader();
    ~ImageLoader();
    static GLuint LoadImage(const std::string &imagePath, int *width, int *height, GLint wrapParam, GLint sampleParam);
    static bool SaveImage(const char *szPathName, void *lpBits, uint16_t w, uint16_t h);
    static bool LoadBmpCustomAlpha(const char *fname, GLubyte **bits, GLsizei *width_, GLsizei *height_, uint8_t alphaColour);
    static bool LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei *width_, GLsizei *height_);
};
