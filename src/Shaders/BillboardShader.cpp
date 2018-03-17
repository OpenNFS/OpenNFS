//
// Created by Amrik on 11/03/2018.
//

#include <afxres.h>
#include "BillboardShader.h"

const std::string vertexSrc = "../shaders/BillboardVertexShader.vertexshader";
const std::string fragSrc = "../shaders/BillboardFragmentShader.fragmentshader";

BillboardShader::BillboardShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
    load_bmp_texture();
}

void BillboardShader::bindAttributes() {
    bindAttribute(0 ,"vertexPosition_modelspace");
    bindAttribute(1 ,"vertexUV");
    bindAttribute(2 ,"normal");
}

void BillboardShader::getAllUniformLocations() {
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation = getUniformLocation("projectionMatrix");
    viewMatrixLocation = getUniformLocation("viewMatrix");
    boardTextureLocation = getUniformLocation("boardTextureSampler");
    lightColourLocation = getUniformLocation("lightColour");
    billboardPosLocation = getUniformLocation("billboardPos");
}

void BillboardShader::loadBillboardTexture(){
    loadSampler2D(boardTextureLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void BillboardShader::loadLight(Light board_light) {
    loadVec3(lightColourLocation, board_light.colour);
    loadVec3(billboardPosLocation, glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * board_light.position);
    loadBillboardTexture();
}

void BillboardShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}


void BillboardShader::customCleanup() {
    glDeleteTextures(1, &textureID);
}

bool LoadBmpWithAlphaHack(const char *fname, const char *afname, GLubyte **bits, GLsizei width, GLsizei height) {
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


void BillboardShader::load_bmp_texture() {
    GLubyte *data;
    GLsizei width = 64;
    GLsizei height = 64;

    LoadBmpWithAlphaHack("../resources/sfx/0004.bmp", "../resources/sfx/0004-a.bmp", &data, width, height);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                      (const GLvoid *) data);
}
