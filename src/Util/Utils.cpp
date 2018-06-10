//
// Created by Amrik on 11/03/2018.
//

#include "Utils.h"

namespace Utils {
    // TODO: Integrate into LoadBmpCustomAlpha as a master bitmap loader
    bool LoadBmpCustomAlpha(const char *fname, GLubyte **bits, GLsizei *width_, GLsizei *height_, int alphaColour) {
        GLsizei width, height;
        bool retval = false;
        // load file and check if it looks reasonable
        FILE *fp = fopen(fname, "rb");
        if (fp) {
            fseek(fp, 0L, 2);
            long size = ftell(fp);
            if (size > (long) sizeof(BITMAPFILEHEADER)) {
                unsigned char *data = new unsigned char[size];
                if (data) {
                    fseek(fp, 0L, 0);
                    if (fread(data, size, 1, fp) == 1) {
                        BITMAPFILEHEADER *file_header = (BITMAPFILEHEADER *) data;
                        if (file_header->bfType == MAKEWORD('B', 'M')) {
                            if (file_header->bfSize == (DWORD) size) {
                                BITMAPINFO *info = (BITMAPINFO *) (data + sizeof(BITMAPFILEHEADER));
                                // we only handle uncompressed bitmaps
                                if (info->bmiHeader.biCompression == BI_RGB) {
                                    width = info->bmiHeader.biWidth;
                                    *width_ = width;
                                    if (width > 0) {
                                        height = info->bmiHeader.biHeight;
                                        *height_ = height;
                                        if (height) {
                                            if (height < 0) height = (-height);
                                            // we want RGBA. let's alloc enough space
                                            *bits = new GLubyte[width * height * 4L];
                                            if (*bits) {
                                                retval = true;
                                                GLubyte *current_bits = *bits;
                                                GLubyte *pixel = data + file_header->bfOffBits;
                                                GLsizei h = height, w = width;
                                                long padding;
                                                switch (info->bmiHeader.biBitCount) {
                                                    // 8-bit palette bitmaps
                                                    case 8:
                                                        padding = w % 2;
                                                        RGBQUAD rgba;
                                                        for (; h > 0; h--) {
                                                            for (w = width; w > 0; w--) {
                                                                rgba = info->bmiColors[*pixel];
                                                                pixel++;
                                                                *current_bits++ = rgba.rgbRed;
                                                                *current_bits++ = rgba.rgbGreen;
                                                                *current_bits++ = rgba.rgbBlue;
                                                                *current_bits++ = 255;
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
                                                                *current_bits++ = 255;
                                                                pixel += 3;
                                                            }
                                                            pixel += padding;
                                                        }
                                                        break;
                                                    default:
                                                        delete[] *bits;
                                                        retval = false;
                                                        break;
                                                }
                                                if (retval) {
                                                    if (info->bmiHeader.biHeight < 0) {
                                                        long *data_q = (long *) *bits;
                                                        long wt = width * 4L;
                                                        long *dest_q = (long *) (*bits + (height - 1) * wt);
                                                        long tmp;
                                                        while (data_q < dest_q) {
                                                            for (w = width; w > 0; w--) {
                                                                tmp = *data_q;
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

    bool LoadBmpWithAlpha(const char *fname, const char *afname, GLubyte **bits, GLsizei width, GLsizei height) {
        bool retval = false;
        // load file and check if it looks reasonable
        FILE *fp = fopen(fname, "rb");
        FILE *fp_a = fopen(afname, "rb");
        if (fp && fp_a) {
            fseek(fp, 0L, 2);
            fseek(fp_a, 0L, 2);
            long size = ftell(fp);
            long size_a = ftell(fp_a);
            unsigned char *data = new unsigned char[size];
            unsigned char *data_a = new unsigned char[size_a];
            if (data && data_a) {
                fseek(fp, 0L, 0);
                fseek(fp_a, 0L, 0);
                if ((fread(data, size, 1, fp) == 1) && (fread(data_a, size_a, 1, fp_a) == 1)) {
                    BITMAPFILEHEADER *file_header = (BITMAPFILEHEADER *) data;
                    BITMAPFILEHEADER *file_header_a = (BITMAPFILEHEADER *) data_a;
                    if (file_header->bfType == MAKEWORD('B', 'M')) {
                        if (file_header->bfSize == (DWORD) size) {
                            BITMAPINFO *info = (BITMAPINFO *) (data +
                                                               sizeof(BITMAPFILEHEADER));// we only handle uncompressed bitmaps
                            BITMAPINFO *info_a = (BITMAPINFO *) (data_a +
                                                                 sizeof(BITMAPFILEHEADER));// we only handle uncompressed bitmaps
                            if (info->bmiHeader.biCompression == BI_RGB) {
                                width = info->bmiHeader.biWidth;
                                if (width > 0) {
                                    height = info->bmiHeader.biHeight;
                                    if (height) {
                                        if (height < 0)
                                            height = (-height);// we want RGBA. let's alloc enough space
                                        *
                                                bits = new GLubyte[width * height * 4L];
                                        if (*bits) {
                                            retval = true;
                                            GLubyte *current_bits = *bits;
                                            GLubyte *pixel = data + file_header->bfOffBits;
                                            GLubyte *pixel_a = data_a + file_header_a->bfOffBits;
                                            GLsizei h = height, w = width;
                                            long padding, padding_a;
                                            switch (info->bmiHeader.biBitCount) {// 24-bit bitmaps
                                                case 24:
                                                    // Read the 8 Bit bitmap alpha data
                                                    padding_a = w % 2;
                                                    RGBQUAD rgba;
                                                    padding = (w * 3) % 2;
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
                                                    break;// I don't like 1,4 and 16 bit.
                                                default:
                                                    delete[] *bits;
                                                    retval = false;
                                                    break;
                                            }
                                            if (retval) {// mirror image if neccessary (never tested)
                                                if (info->bmiHeader.biHeight < 0) {
                                                    long *data_q = (long *) *bits;
                                                    long wt = width * 4L;
                                                    long *dest_q = (long *) (*bits + (height - 1) * wt);
                                                    long tmp;
                                                    while (data_q < dest_q) {
                                                        for (w = width; w > 0; w--) {
                                                            tmp = *data_q;
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

    glm::vec3 bulletToGlm(const btVector3 &v) { return glm::vec3(v.getX(), v.getY(), v.getZ()); }

    btVector3 glmToBullet(const glm::vec3 &v) { return btVector3(v.x, v.y, v.z); }

    glm::quat bulletToGlm(const btQuaternion &q) {
        return glm::quat(q.getW(), q.getX(), q.getY(), q.getZ());
    }

    btQuaternion glmToBullet(const glm::quat &q) { return btQuaternion(q.x, q.y, q.z, q.w); }

    btMatrix3x3 glmToBullet(const glm::mat3 &m) {
        return btMatrix3x3(m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]);
    }

    btBoxShape *genCollisionBox(std::vector<glm::vec3> model_vertices) {
        glm::vec3 bottom_left = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);
        glm::vec3 top_right = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);


        for (auto &vertex : model_vertices) {
            if (vertex.x < bottom_left.x) {
                bottom_left.x = vertex.x;
            }
            if (vertex.y < bottom_left.y) {
                bottom_left.y = vertex.y;
            }
            if (vertex.z < bottom_left.z) {
                bottom_left.z = vertex.z;
            }
            if (vertex.x > top_right.x) {
                top_right.x = vertex.x;
            }
            if (vertex.y > top_right.y) {
                top_right.y = vertex.y;
            }
            if (vertex.z > top_right.z) {
                top_right.z = vertex.z;
            }
        }

        return new btBoxShape(
                btVector3((top_right.x - bottom_left.x) / 2, (top_right.y - bottom_left.y) / 2,
                          (top_right.z - bottom_left.z) / 2));
    }

    glm::vec3 genDimensions(std::vector<glm::vec3> model_vertices) {
        glm::vec3 bottom_left = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);
        glm::vec3 top_right = glm::vec3(model_vertices[0].x, model_vertices[0].y, model_vertices[0].z);


        for (auto &vertex : model_vertices) {
            if (vertex.x < bottom_left.x) {
                bottom_left.x = vertex.x;
            }
            if (vertex.y < bottom_left.y) {
                bottom_left.y = vertex.y;
            }
            if (vertex.z < bottom_left.z) {
                bottom_left.z = vertex.z;
            }
            if (vertex.x > top_right.x) {
                top_right.x = vertex.x;
            }
            if (vertex.y > top_right.y) {
                top_right.y = vertex.y;
            }
            if (vertex.z > top_right.z) {
                top_right.z = vertex.z;
            }
        }

        return glm::vec3((top_right.x - bottom_left.x) / 2, (top_right.y - bottom_left.y) / 2,
                         (top_right.z - bottom_left.z) / 2);
    }
}


