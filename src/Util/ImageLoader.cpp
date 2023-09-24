#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint ImageLoader::LoadImage(const std::string &imagePath, int *width, int *height, GLint wrapParam, GLint sampleParam) {
    GLuint textureID;
    int nChannels;

    unsigned char *image = stbi_load(imagePath.c_str(), width, height, &nChannels, STBI_rgb_alpha);
    ASSERT(image != nullptr, "Failed to load texture " << imagePath);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampleParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampleParam);
    // No need to change this dependent on nChannels, we've always requested an RGBA load from stb
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

    return textureID;
}

// lpBits stand for long pointer bits
// szPathName : Specifies the pathname        -> the file path to save the image
// lpBits    : Specifies the bitmap bits      -> the buffer (content of the) image
// w    : Specifies the image width
// h    : Specifies the image height
bool ImageLoader::SaveImage(const char *szPathName, void *lpBits, uint16_t w, uint16_t h) {
    // Create a new file for writing
    FILE *pFile = fopen(szPathName, "wb"); // wb -> w: writable b: binary, open as writable and binary
    if (pFile == NULL) {
        return false;
    }

    CP_BITMAPINFOHEADER BMIH; // BMP header
    BMIH.biSize      = sizeof(CP_BITMAPINFOHEADER);
    BMIH.biSizeImage = w * h * 4;
    // Create the bitmap for this OpenGL context
    BMIH.biSize        = sizeof(CP_BITMAPINFOHEADER);
    BMIH.biWidth       = w;
    BMIH.biHeight      = h;
    BMIH.biPlanes      = 1;
    BMIH.biBitCount    = 32;
    BMIH.biCompression = CP_BI_RGB;

    CP_BITMAPFILEHEADER bmfh; // Other BMP header
    int nBitsOffset    = sizeof(CP_BITMAPFILEHEADER) + BMIH.biSize;
    int32_t lImageSize = BMIH.biSizeImage;
    int32_t lFileSize  = nBitsOffset + lImageSize;
    bmfh.bfType        = 'B' + ('M' << 8);
    bmfh.bfOffBits     = nBitsOffset;
    bmfh.bfSize        = lFileSize;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

    // Write the bitmap file header               // Saving the first header to file
    size_t nWrittenFileHeaderSize = fwrite(&bmfh, 1, sizeof(CP_BITMAPFILEHEADER), pFile);

    // And then the bitmap info header            // Saving the second header to file
    size_t nWrittenInfoHeaderSize = fwrite(&BMIH, 1, sizeof(CP_BITMAPINFOHEADER), pFile);

    // Finally, write the image data itself
    //-- the data represents our drawing          // Saving the file content in lpBits to file
    size_t nWrittenDIBDataSize = fwrite(lpBits, 1, lImageSize, pFile);
    fclose(pFile); // closing the file.

    return true;
}



// TODO: Integrate into LoadBmpCustomAlpha as a master bitmap loader
bool ImageLoader::LoadBmpCustomAlpha(const char *fname, GLubyte **bits, GLsizei *width_, GLsizei *height_, uint8_t alphaColour) {
    GLsizei width, height;
    bool retval = false;
    // load file and check if it looks reasonable
    FILE *fp = fopen(fname, "rb");
    if (fp) {
        fseek(fp, 0L, 2);
        long size = ftell(fp);
        if (size > (long) sizeof(CP_BITMAPFILEHEADER)) {
            unsigned char *data = new unsigned char[size];
            if (data) {
                fseek(fp, 0L, 0);
                if (fread(data, size, 1, fp) == 1) {
                    CP_BITMAPFILEHEADER *file_header = (CP_BITMAPFILEHEADER *) data;
                    if (file_header->bfType == MAKEuint16_t('B', 'M')) {
                        if (file_header->bfSize == (uint32_t) size) {
                            CP_BITMAPINFO *info = (CP_BITMAPINFO *) (data + sizeof(CP_BITMAPFILEHEADER));
                            // we only handle uncompressed bitmaps
                            if (info->bmiHeader.biCompression == CP_BI_RGB) {
                                width   = info->bmiHeader.biWidth;
                                *width_ = width;
                                if (width > 0) {
                                    height   = info->bmiHeader.biHeight;
                                    *height_ = height;
                                    if (height) {
                                        if (height < 0)
                                            height = (-height);
                                        // we want RGBA. let's alloc enough space
                                        *bits = new GLubyte[width * height * 4L];
                                        if (*bits) {
                                            retval                = true;
                                            GLubyte *current_bits = *bits;
                                            GLubyte *pixel        = data + file_header->bfOffBits;
                                            GLsizei h = height, w = width;
                                            long padding;
                                            switch (info->bmiHeader.biBitCount) {
                                            // 8-bit palette bitmaps
                                            case 8:
                                                padding = w % 2;
                                                CP_RGBQUAD rgba;
                                                for (; h > 0; h--) {
                                                    for (w = width; w > 0; w--) {
                                                        rgba = info->bmiColors[*pixel];
                                                        pixel++;
                                                        *current_bits++ = rgba.rgbRed;
                                                        *current_bits++ = rgba.rgbGreen;
                                                        *current_bits++ = rgba.rgbBlue;
                                                        if (rgba.rgbRed == 0 && rgba.rgbGreen == alphaColour && rgba.rgbBlue == 0) {
                                                            *current_bits++ = 0;
                                                        } else {
                                                            *current_bits++ = 255;
                                                        }
                                                    }
                                                    pixel += padding;
                                                }
                                                break;
                                                // 24-bit bitmaps
                                            case 24:
                                                padding = (w * 3) % 2;
                                                for (; h > 0; h--) {
                                                    for (w = width; w > 0; w--) {
                                                        *current_bits++ = pixel[2];
                                                        *current_bits++ = pixel[1];
                                                        *current_bits++ = pixel[0];
                                                        if (pixel[2] == 0 && pixel[1] == alphaColour && pixel[0] == 0) {
                                                            *current_bits++ = 0;
                                                        } else {
                                                            *current_bits++ = 255;
                                                        }
                                                        pixel += 3;
                                                    }
                                                    pixel += padding;
                                                }
                                                break;
                                            case 32:
                                                // 32-bit bitmaps
                                                // never seen it, but Win32 SDK claims the existance
                                                // of that value. 4th byte is assumed to be alpha-channel.
                                                for (; h > 0; h--) {
                                                    for (w = width; w > 0; w--) {
                                                        *current_bits++ = pixel[2];
                                                        *current_bits++ = pixel[1];
                                                        *current_bits++ = pixel[0];
                                                        if (pixel[2] == 0 && pixel[1] == alphaColour && pixel[0] == 0) {
                                                            *current_bits++ = 0;
                                                        } else {
                                                            *current_bits++ = pixel[3];
                                                        }
                                                        pixel += 4;
                                                    }
                                                }
                                                break; // I don't like 1,4 and 16 bit.
                                            default:
                                                delete[] * bits;
                                                retval = false;
                                                break;
                                            }
                                            if (retval) {
                                                if (info->bmiHeader.biHeight < 0) {
                                                    long *data_q = (long *) *bits;
                                                    long wt      = width * 4L;
                                                    long *dest_q = (long *) (*bits + (height - 1) * wt);
                                                    long tmp;
                                                    while (data_q < dest_q) {
                                                        for (w = width; w > 0; w--) {
                                                            tmp       = *data_q;
                                                            *data_q++ = *dest_q;
                                                            *dest_q++ = tmp;
                                                        }
                                                        dest_q -= (wt + wt);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                delete[] data;
            }
        }
        fclose(fp);
    }
    return retval;
}

bool ImageLoader::LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei *width_, GLsizei *height_) {
    GLsizei width, height;
    bool retval = false;
    // load file and check if it looks reasonable
    FILE *fp   = fopen(fname, "rb");
    FILE *fp_a = fopen(afname, "rb");
    if (fp && fp_a) {
        fseek(fp, 0L, 2);
        fseek(fp_a, 0L, 2);
        long size             = ftell(fp);
        long size_a           = ftell(fp_a);
        unsigned char *data   = new unsigned char[size];
        unsigned char *data_a = new unsigned char[size_a];
        if (data && data_a) {
            fseek(fp, 0L, 0);
            fseek(fp_a, 0L, 0);
            if ((fread(data, size, 1, fp) == 1) && (fread(data_a, size_a, 1, fp_a) == 1)) {
                CP_BITMAPFILEHEADER *file_header   = (CP_BITMAPFILEHEADER *) data;
                CP_BITMAPFILEHEADER *file_header_a = (CP_BITMAPFILEHEADER *) data_a;
                if (file_header->bfType == MAKEuint16_t('B', 'M')) {
                    if (file_header->bfSize == (uint32_t) size) {
                        CP_BITMAPINFO *info   = (CP_BITMAPINFO *) (data + sizeof(CP_BITMAPFILEHEADER));   // we only handle uncompressed bitmaps
                        CP_BITMAPINFO *info_a = (CP_BITMAPINFO *) (data_a + sizeof(CP_BITMAPFILEHEADER)); // we only handle uncompressed bitmaps
                        if (info->bmiHeader.biCompression == CP_BI_RGB) {
                            width   = info->bmiHeader.biWidth;
                            *width_ = width;
                            if (width > 0) {
                                height   = info->bmiHeader.biHeight;
                                *height_ = height;
                                if (height) {
                                    if (height < 0)
                                        height = (-height); // we want RGBA. let's alloc enough space
                                    *bits = new GLubyte[width * height * 4L];
                                    if (*bits) {
                                        retval                = true;
                                        GLubyte *current_bits = *bits;
                                        GLubyte *pixel        = data + file_header->bfOffBits;
                                        GLubyte *pixel_a      = data_a + file_header_a->bfOffBits;
                                        GLsizei h = height, w = width;
                                        long padding, padding_a;
                                        switch (info->bmiHeader.biBitCount) { // 24-bit bitmaps
                                        case 8:
                                            padding_a = w % 2;
                                            padding   = w % 2;
                                            CP_RGBQUAD rgba;
                                            for (; h > 0; h--) {
                                                for (w = width; w > 0; w--) {
                                                    rgba = info->bmiColors[*pixel];
                                                    pixel++;
                                                    pixel_a++;
                                                    *current_bits++ = rgba.rgbRed;
                                                    *current_bits++ = rgba.rgbGreen;
                                                    *current_bits++ = rgba.rgbBlue;
                                                    *current_bits++ = rgba.rgbRed;
                                                }
                                                pixel += padding;
                                                pixel_a += padding_a;
                                            }
                                            break;
                                        case 24:
                                            // Read the 8 Bit bitmap alpha data
                                            padding_a = w % 2;
                                            padding   = (w * 3) % 2;
                                            for (; h > 0; h--) {
                                                for (w = width; w > 0; w--) {
                                                    rgba = info_a->bmiColors[*pixel_a];
                                                    pixel_a++;
                                                    *current_bits++ = pixel[2];
                                                    *current_bits++ = pixel[1];
                                                    *current_bits++ = pixel[0];
                                                    *current_bits++ = rgba.rgbRed;
                                                    pixel += 3;
                                                }
                                                pixel += padding;
                                                pixel_a += padding_a;
                                            }
                                            break;
                                        case 32:
                                            // 32-bit bitmaps
                                            // never seen it, but Win32 SDK claims the existance
                                            // of that value. 4th byte is assumed to be alpha-channel.
                                            for (; h > 0; h--) {
                                                for (w = width; w > 0; w--) {
                                                    *current_bits++ = pixel[2];
                                                    *current_bits++ = pixel[1];
                                                    *current_bits++ = pixel[0];
                                                    *current_bits++ = pixel[3];
                                                    pixel += 4;
                                                }
                                            }
                                            break; // I don't like 1,4 and 16 bit.
                                        default:
                                            delete[] * bits;
                                            retval = false;
                                            break;
                                        }
                                        if (retval) { // mirror image if neccessary (never tested)
                                            if (info->bmiHeader.biHeight < 0) {
                                                long *data_q = (long *) *bits;
                                                long wt      = width * 4L;
                                                long *dest_q = (long *) (*bits + (height - 1) * wt);
                                                long tmp;
                                                while (data_q < dest_q) {
                                                    for (w = width; w > 0; w--) {
                                                        tmp       = *data_q;
                                                        *data_q++ = *dest_q;
                                                        *dest_q++ = tmp;
                                                    }
                                                    dest_q -= (wt + wt);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            delete[] data;
            delete[] data_a;
        }
        fclose(fp);
        fclose(fp_a);
    }
    return retval;
}
