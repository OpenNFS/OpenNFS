//
// Created by Amrik on 16/01/2018.
//


#ifndef FCE_TO_OBJ_TRK_LOADER_H
#define FCE_TO_OBJ_TRK_LOADER_H

#include <vector>
#include <sstream>
#include <set>
#include <iomanip>
#include <glm/vec3.hpp>
#include "boost/filesystem.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <map>
#include "Scene/Model.h"
#include "Scene/TrackBlock.h"
#include "nfs_data.h"
#include <boost/concept_check.hpp>
#include "Scene/TrackBlock.h"
#include "Scene/Light.h"
#define EPSILON 0.000001

class Texture {
public:
    unsigned int texture_id, width, height;
    GLubyte *texture_data;

    Texture() = default;

    explicit Texture(unsigned int id, GLubyte *data, unsigned int w, unsigned int h) {
        texture_id = id;
        texture_data = data;
        width = w;
        height = h;
    }
};

class trk_loader {
public:
    // !!! for arrays : structures are aligned to their largest member
    // !!! structure members are aligned on their own size (up to the /Zp parameter)
    // Attributes
    bool bEmpty;
    bool bHSMode;
    char header[28]; /* file header */
    long nBlocks;
    struct TRKBLOCK *trk;
    struct POLYGONBLOCK *poly;
    struct XOBJBLOCK *xobj; // xobj[4*blk+chunk]; global=xobj[4*nblocks]
    long hs_morexobjlen;
    char *hs_morexobj;  // 4N & 4N+1 in HS format (xobj[4N] left empty)
    long nTextures;
    struct TEXTUREBLOCK *texture;
    struct COLFILE col;

    // Implementation
    explicit trk_loader(const std::string &frd_path);

    virtual ~trk_loader();

    bool LoadFRD(std::string frd_path);

    std::vector<TrackBlock> getTrackBlocks();

    std::vector<Track> getCOLModels();

    std::map<short, GLuint> getTextureGLMap();

    std::map<short, Texture> getTextures();

protected:
    std::map<short, GLuint> texture_gl_mappings;
    std::map<short, Texture> textures;
    std::vector<Track> col_models;
    std::vector<TrackBlock> track_blocks;

    bool LoadCOL(std::string col_path);

    std::map<short, GLuint> GenTrackTextures(std::map<short, Texture> textures);

    std::vector<Track> ParseCOLModels();

    void ParseTRKModels();

    std::vector<short> RemapTextureIDs(const std::set<short> &minimal_texture_ids_set,
                                       std::vector<unsigned int> &texture_indices);

    void BlockShadingFixer(int blk, float theta, float rho);

    void ObjectShadingFixer(int blk, float theta, float rho);

    FLOATPT VertexNormal(int blk, int VertexIndex);

    FLOATPT VectorNormalize(FLOATPT nc);

    FLOATPT SumVector(FLOATPT Vect1, FLOATPT Vect2);

    FLOATPT QuadNormalVectorCalc(FLOATPT a, FLOATPT b, FLOATPT c, FLOATPT d);

    FLOATPT NormalVectorCalc(FLOATPT a, FLOATPT b, FLOATPT c);

    void raytrace(float rho, float theta);
};

#endif //FCE_TO_OBJ_TRK_LOADER_H

