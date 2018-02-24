//
// Created by Amrik on 16/01/2018.
//

#include <boost/concept_check.hpp>
#include "trk_loader.h"

using namespace std;

Texture LoadTexture(TEXTUREBLOCK track_texture) {
    int width, height;
    unsigned char *data;
    FILE *file;
    std::stringstream filename;
    filename << "../resources/TRK006/textures/" << setfill('0') << setw(4) << track_texture.texture << ".BMP";
    file = fopen(filename.str().c_str(), "rb");
    if (file == nullptr) {
        std::cout << "Couldn't open " << filename.str() << std::endl;
        assert(file == nullptr);
    }

    width = track_texture.width;
    height = track_texture.height;

    data = (unsigned char *) malloc(width * height * 3);
    fseek(file, 54, SEEK_SET);
    fread(data, width * height * 3, 1, file);
    fclose(file);

    for (int i = 0; i < width * height; ++i) {
        int index = i * 3;
        unsigned char B, R;
        B = data[index];
        R = data[index + 2];
        data[index] = R;
        data[index + 2] = B;
    }

    return Texture((unsigned int) track_texture.texture, data, track_texture.width, track_texture.height);
}

bool trk_loader::LoadFRD(std::string frd_path) {
    // TODO: Wrap each fread with if(STATEMENT) != numElementsToRead) return false; MACRO?
    ifstream ar(frd_path, ios::in | ios::binary);

    int i, j, k, l;
    struct TRKBLOCK *b;
    struct POLYGONBLOCK *p;
    struct XOBJDATA *x;
    struct OBJPOLYBLOCK *o;

    if (ar.read(header, 28).gcount() != 28) return false; // header & numblocks
    if (ar.read((char *) &nBlocks, 4).gcount() < 4) return false;
    nBlocks++;
    if ((nBlocks < 1) || (nBlocks > 500)) return false; // 1st sanity check

    trk = (struct TRKBLOCK *) malloc(nBlocks * sizeof(struct TRKBLOCK));
    if (trk == nullptr) return false;
    memset(trk, 0, nBlocks * sizeof(struct TRKBLOCK));
    poly = (struct POLYGONBLOCK *) malloc(nBlocks * sizeof(struct POLYGONBLOCK));
    if (poly == nullptr) return false;
    memset(poly, 0, nBlocks * sizeof(struct POLYGONBLOCK));
    xobj = (struct XOBJBLOCK *) malloc((4 * nBlocks + 1) * sizeof(struct XOBJBLOCK));
    if (xobj == nullptr) return false;
    memset(xobj, 0, (4 * nBlocks + 1) * sizeof(struct XOBJBLOCK));

    if (ar.read((char *) &l, 4).gcount() < 4) return false; // choose between NFS3 & NFSHS
    if ((l < 0) || (l > 5000)) bHSMode = false;
    else if (((l + 7) / 8) == nBlocks) bHSMode = true;
    else return false; // unknown file type

    memcpy(trk, &l, 4);
    if (ar.read(((char *) trk) + 4, 80).gcount() != 80) return false;

    // TRKBLOCKs
    for (i = 0; i < nBlocks; i++) {
        b = &(trk[i]);
        // ptCentre, ptBounding, 6 nVertices == 84 bytes
        if (i != 0) { if (ar.read((char *) b, 84).gcount() != 84) return false; }
        if ((b->nVertices < 0)) return false;
        b->vert = (struct FLOATPT *) malloc(b->nVertices * sizeof(struct FLOATPT));
        if (b->vert == nullptr) return false;
        if ((long) ar.read((char *) b->vert, 12 * b->nVertices).gcount() != 12 * b->nVertices) return false;
        b->unknVertices = (long *) malloc(b->nVertices * sizeof(long));
        if (b->unknVertices == nullptr) return false;
        if ((long) ar.read((char *) b->unknVertices, 4 * b->nVertices).gcount() != 4 * b->nVertices) return false;
        if (ar.read((char *) b->nbdData, 4 * 0x12C).gcount() != 4 * 0x12C) return false;

        // nStartPos & various blk sizes == 32 bytes
        if (ar.read((char *) &(b->nStartPos), 32).gcount() != 32) return false;
        if (i > 0) if (b->nStartPos != trk[i - 1].nStartPos + trk[i - 1].nPositions) return false;
        b->posData = (struct POSITIONDATA *) malloc(b->nPositions * sizeof(struct POSITIONDATA));
        if (b->posData == nullptr) return false;
        if ((long) ar.read((char *) b->posData, 8 * b->nPositions).gcount() != 8 * b->nPositions) return false;
        b->polyData = (struct POLYVROADDATA *) malloc(b->nPolygons * sizeof(struct POLYVROADDATA));
        if (b->polyData == nullptr) return false;
        memset(b->polyData, 0, b->nPolygons * sizeof(struct POLYVROADDATA));
        for (j = 0; j < b->nPolygons; j++)
            if (ar.read((char *) b->polyData + j, 8).gcount() != 8) return false;
        b->vroadData = (struct VROADDATA *) malloc(b->nVRoad * sizeof(struct VROADDATA));
        if (b->vroadData == nullptr) return false;
        if ((long) ar.read((char *) b->vroadData, 12 * b->nVRoad).gcount() != 12 * b->nVRoad) return false;
        if (b->nXobj > 0) {
            b->xobj = (struct REFXOBJ *) malloc(b->nXobj * sizeof(struct REFXOBJ));
            if (b->xobj == nullptr) return false;
            if ((long) ar.read((char *) b->xobj, 20 * b->nXobj).gcount() != 20 * b->nXobj) return false;
        }
        if (b->nPolyobj > 0) {
            auto *buffer = (char *) malloc(static_cast<size_t>(b->nPolyobj * 20));
            if ((long) ar.read(buffer, 20 * b->nPolyobj).gcount() != 20 * b->nPolyobj) return false;
            free(buffer);
        }
        b->nPolyobj = 0;
        if (b->nSoundsrc > 0) {
            b->soundsrc = (struct SOUNDSRC *) malloc(b->nSoundsrc * sizeof(struct SOUNDSRC));
            if (b->soundsrc == nullptr) return false;
            if ((long) ar.read((char *) b->soundsrc, 16 * b->nSoundsrc).gcount() != 16 * b->nSoundsrc) return false;
        }
        if (b->nLightsrc > 0) {
            b->lightsrc = (struct LIGHTSRC *) malloc(b->nLightsrc * sizeof(struct LIGHTSRC));
            if (b->lightsrc == nullptr) return false;
            if ((long) ar.read((char *) b->lightsrc, 16 * b->nLightsrc).gcount() != 16 * b->nLightsrc) return false;
        }
    }

    // TODO: Identify in what cases stream reads garbage
    // This workaround is emblematic of a larger problem, why does the file stream 'ar' not read into these structs
    auto pos = static_cast<int>(ar.tellg());
    FILE *trk_file = fopen(frd_path.c_str(), "rb");
    fseek(trk_file, pos, SEEK_SET);

    // POLYGONBLOCKs
    for (i = 0; i < nBlocks; i++) {
        p = &(poly[i]);
        for (j = 0; j < 7; j++) {
            fread(&(p->sz[j]), 0x4, 1, trk_file);
            if (p->sz[j] != 0) {
                fread(&(p->szdup[j]), 0x4, 1, trk_file);
                if (p->szdup[j] != p->sz[j]) return false;
                p->poly[j] = (LPPOLYGONDATA) malloc(p->sz[j] * sizeof(struct POLYGONDATA));
                if (p->poly[j] == nullptr) return false;
                fread(p->poly[j], static_cast<size_t>(14 * p->sz[j]), 1, trk_file);
            }
        }
        if (p->sz[4] != trk[i].nPolygons) return false; // sanity check
        for (j = 0; j < 4; j++) {
            o = &(p->obj[j]);
            fread(&(o->n1), 0x4, 1, trk_file);
            if (o->n1 > 0) {
                fread(&(o->n2), 0x4, 1, trk_file);
                o->types = (long *) calloc(static_cast<size_t>(o->n2), sizeof(long));
                if (o->types == nullptr) return false;
                o->numpoly = (long *) malloc(o->n2 * sizeof(long));
                if (o->numpoly == nullptr) return false;
                o->poly = (LPPOLYGONDATA *) calloc(static_cast<size_t>(o->n2), sizeof(LPPOLYGONDATA));
                if (o->poly == nullptr) return false;
                o->nobj = 0;
                l = 0;
                for (k = 0; k < o->n2; k++) {
                    fread(o->types + k, 0x4, 1, trk_file);
                    if (o->types[k] == 1) {
                        fread(o->numpoly + o->nobj, 0x4, 1, trk_file);
                        o->poly[o->nobj] = (LPPOLYGONDATA) malloc(o->numpoly[o->nobj] * sizeof(struct POLYGONDATA));
                        if (o->poly[o->nobj] == nullptr) return false;
                        fread(o->poly[o->nobj], static_cast<size_t>(14 * o->numpoly[o->nobj]), 1, trk_file);
                        l += o->numpoly[o->nobj];
                        o->nobj++;
                    }
                }
                if (l != o->n1) return false; // n1 == total nb polygons
            }
        }
    }

    ar.seekg(ftell(trk_file), ar.beg);
    fclose(trk_file);

    // XOBJBLOCKs
    for (i = 0; i <= 4 * nBlocks; i++) {
        if (ar.read((char *) &(xobj[i].nobj), 4).gcount() != 4) return false;
        if (xobj[i].nobj > 0) {
            xobj[i].obj = (struct XOBJDATA *) malloc(xobj[i].nobj * sizeof(struct XOBJDATA));
            if (xobj[i].obj == nullptr) return false;
            memset(xobj[i].obj, 0, xobj[i].nobj * sizeof(struct XOBJDATA));
        }
        for (j = 0; j < xobj[i].nobj; j++) {
            x = &(xobj[i].obj[j]);
            // 3 headers == 12 bytes
            if (ar.read((char *) x, 12).gcount() != 12) return false;
            if (x->crosstype == 4) { // basic objects
                if (ar.read((char *) &(x->ptRef), 12).gcount() != 12) return false;
                if (ar.read((char *) &(x->unknown2), 4).gcount() != 4) return false;
            } else if (x->crosstype == 3) { // animated objects
                // unkn3, type3, objno, nAnimLength, unkn4 == 24 bytes
                if (ar.read((char *) x->unknown3, 24).gcount() != 24) return false;
                if (x->type3 != 3) return false;
                x->animData = (struct ANIMDATA *) malloc(static_cast<size_t>(20 * x->nAnimLength));
                if (x->animData == nullptr) return false;
                if ((long) ar.read((char *) x->animData, 20 * x->nAnimLength).gcount() != 20 * x->nAnimLength)
                    return false;
                // make a ref point from first anim position
                x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
            } else return false; // unknown object type
            // common part : vertices & polygons
            if (ar.read((char *) &(x->nVertices), 4).gcount() != 4) return false;
            x->vert = (struct FLOATPT *) malloc(static_cast<size_t>(12 * x->nVertices));
            if (x->vert == nullptr) return false;
            if ((long) ar.read((char *) x->vert, 12 * x->nVertices).gcount() != 12 * x->nVertices) return false;
            x->unknVertices = (long *) malloc(static_cast<size_t>(4 * x->nVertices));
            if (x->unknVertices == nullptr) return false;
            if ((long) ar.read((char *) x->unknVertices, 4 * x->nVertices).gcount() != 4 * x->nVertices) return false;
            if (ar.read((char *) &(x->nPolygons), 4).gcount() != 4) return false;
            x->polyData = (struct POLYGONDATA *) malloc(static_cast<size_t>(x->nPolygons * 14));
            if (x->polyData == nullptr) return false;
            if ((long) ar.read((char *) x->polyData, 14 * x->nPolygons).gcount() != 14 * x->nPolygons) return false;
        }
    }

    // TEXTUREBLOCKs
    if (ar.read((char *) &nTextures, 4).gcount() != 4) return false;
    texture = (struct TEXTUREBLOCK *) malloc(nTextures * sizeof(struct TEXTUREBLOCK));
    for (i = 0; i < nTextures; i++) {
        if (ar.read((char *) &(texture[i]), 47).gcount() != 47) return false;
        auto p = std::make_pair(texture[i].texture, LoadTexture(texture[i]));
        textures.insert(p);
    }

    return ar.read((char *) &i, 4).gcount() == 0; // we ought to be at EOF now
}

bool trk_loader::LoadCOL(std::string col_path) {
    // TODO: Wrap each fread with if(STATEMENT) != numElementsToRead) return false; MACRO?
    ifstream coll(col_path, ios::in | ios::binary);

    struct COLSTRUCT3D *s;
    struct COLOBJECT *o;
    int i, delta, dummy;

    col.hs_extra = NULL;
    if (coll.read((char *) &col, 16).gcount() != 16) return false;
    if ((col.collID[0] != 'C') || (col.collID[1] != 'O') ||
        (col.collID[2] != 'L') || (col.collID[3] != 'L'))
        return false;
    if (col.version != 11) return false;
    if ((col.nBlocks != 2) && (col.nBlocks != 4) && (col.nBlocks != 5)) return false;
    if ((long) coll.read((char *) col.xbTable, 4 * col.nBlocks).gcount() != 4 * col.nBlocks) return false;

    // texture XB
    if (coll.read((char *) &col.textureHead, 8).gcount() != 8) return false;
    if (col.textureHead.xbid != XBID_TEXTUREINFO) return false;
    if (col.textureHead.size != 8 + 8 * col.textureHead.nrec) return false;
    col.texture = (struct COLTEXTUREINFO *)
            malloc(col.textureHead.nrec * sizeof(struct COLTEXTUREINFO));
    if (col.texture == NULL) return false;
    if ((long) coll.read((char *) col.texture, 8 * col.textureHead.nrec).gcount() != 8 * col.textureHead.nrec)
        return false;

    // struct3D XB
    if (col.nBlocks >= 4) {
        if (coll.read((char *) &col.struct3DHead, 8).gcount() != 8) return false;
        if (col.struct3DHead.xbid != XBID_STRUCT3D) return false;
        s = col.struct3D = (struct COLSTRUCT3D *)
                malloc(col.struct3DHead.nrec * sizeof(struct COLSTRUCT3D));
        if (s == NULL) return false;
        memset(s, 0, col.struct3DHead.nrec * sizeof(struct COLSTRUCT3D));
        for (i = 0; i < col.struct3DHead.nrec; i++, s++) {
            if (coll.read((char *) s, 8).gcount() != 8) return false;
            delta = (8 + 16 * s->nVert + 6 * s->nPoly) % 4;
            delta = (4 - delta) % 4;
            if (s->size != 8 + 16 * s->nVert + 6 * s->nPoly + delta) return false;
            s->vertex = (struct COLVERTEX *) malloc(16 * s->nVert);
            if (s->vertex == NULL) return false;
            if ((long) coll.read((char *) s->vertex, 16 * s->nVert).gcount() != 16 * s->nVert) return false;
            s->polygon = (struct COLPOLYGON *) malloc(6 * s->nPoly);
            if (s->polygon == NULL) return false;
            if ((long) coll.read((char *) s->polygon, 6 * s->nPoly).gcount() != 6 * s->nPoly) return false;
            if (delta > 0) if ((int) coll.read((char *) &dummy, delta).gcount() != delta) return false;
        }

        // object XB
        if (coll.read((char *) &col.objectHead, 8).gcount() != 8) return false;
        if ((col.objectHead.xbid != XBID_OBJECT) && (col.objectHead.xbid != XBID_OBJECT2)) return false;
        o = col.object = (struct COLOBJECT *)
                malloc(col.objectHead.nrec * sizeof(struct COLOBJECT));
        if (o == NULL) return false;
        memset(o, 0, col.objectHead.nrec * sizeof(struct COLOBJECT));
        for (i = 0; i < col.objectHead.nrec; i++, o++) {
            if (coll.read((char *) o, 4).gcount() != 4) return false;
            if (o->type == 1) {
                if (o->size != 16) return false;
                if (coll.read((char *) &(o->ptRef), 12).gcount() != 12) return false;
            } else if (o->type == 3) {
                if (coll.read((char *) &(o->animLength), 4).gcount() != 4) return false;
                if (o->size != 8 + 20 * o->animLength) return false;
                o->animData = (struct ANIMDATA *) malloc(20 * o->animLength);
                if (o->animData == nullptr) return false;
                if ((long) coll.read((char *) o->animData, 20 * o->animLength).gcount() != 20 * o->animLength)
                    return false;
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            } else return false; // unknown object type
        }
    }

    // object2 XB
    if (col.nBlocks == 5) {
        if (coll.read((char *) &col.object2Head, 8).gcount() != 8) return false;
        if ((col.object2Head.xbid != XBID_OBJECT) && (col.object2Head.xbid != XBID_OBJECT2)) return false;
        o = col.object2 = (struct COLOBJECT *)
                malloc(col.object2Head.nrec * sizeof(struct COLOBJECT));
        if (o == NULL) return false;
        memset(o, 0, col.object2Head.nrec * sizeof(struct COLOBJECT));
        for (i = 0; i < col.object2Head.nrec; i++, o++) {
            if (coll.read((char *) o, 4).gcount() != 4) return false;
            if (o->type == 1) {
                if (o->size != 16) return false;
                if (coll.read((char *) &(o->ptRef), 12).gcount() != 12) return false;
            } else if (o->type == 3) {
                if (coll.read((char *) &(o->animLength), 4).gcount() != 4) return false;
                if (o->size != 8 + 20 * o->animLength) return false;
                o->animData = (struct ANIMDATA *) malloc(20 * o->animLength);
                if (o->animData == NULL) return false;
                if ((long) coll.read((char *) o->animData, 20 * o->animLength).gcount() != 20 * o->animLength)
                    return false;
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            } else return false; // unknown object type
        }
    }

    // vroad XB
    if (coll.read((char *) &col.vroadHead, 8).gcount() != 8) return false;
    if (col.vroadHead.xbid != XBID_VROAD) return false;
    if (col.vroadHead.size != 8 + 36 * col.vroadHead.nrec) return false;
    //ASSERT(col.vroadHead.nrec==trk[nBlocks-1].nStartPos+trk[nBlocks-1].nPositions);
    col.vroad = (struct COLVROAD *) malloc(col.vroadHead.nrec * sizeof(struct COLVROAD));
    if (col.vroad == NULL) return false;
    if ((long) coll.read((char *) col.vroad, 36 * col.vroadHead.nrec).gcount() != 36 * col.vroadHead.nrec) return false;

    return coll.read((char *) &i, 4).gcount() == 0; // we ought to be at EOF now
}

std::map<short, GLuint> trk_loader::GenTrackTextures(std::map<short, Texture> textures) {
    std::map<short, GLuint> gl_id_map;

    for (auto it = textures.begin(); it != textures.end(); ++it) {
        Texture texture = it->second;
        GLuint textureID;
        glGenTextures(1, &textureID);
        auto p = std::make_pair(it->first, textureID);
        gl_id_map.insert(p);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width, texture.height, GL_RGB, GL_UNSIGNED_BYTE,
                          texture.texture_data);
    }

    return gl_id_map;
}

trk_loader::~trk_loader() = default;

std::vector<short> trk_loader::RemapNormals(const std::set<short> &minimal_texture_ids_set, std::vector<glm::vec3> &normals){
    // Get ordered list of unique texture id's present in block
    std::vector<short> texture_ids;
    texture_ids.assign(minimal_texture_ids_set.begin(), minimal_texture_ids_set.end());
    // Remap Normals to correspond to ordered texture ID's
    std::map<int, int> ordered_mapping;
    for (int t = 0; t < texture_ids.size(); ++t) {
        auto p = std::make_pair((int) texture_ids[t], t);
        ordered_mapping.insert(p);
    }
    for (auto &normal : normals) {
        normal.x = ordered_mapping.find((int) normal.x)->second;
    }
    return texture_ids;
}

std::vector<Model> trk_loader::GetCOLModels() {
    std::vector<Model> col_models;
    /* COL DATA - TODO: Come back for VROAD AI/Collision data */
    COLSTRUCT3D *s = col.struct3D;
    for (int i = 0; i < col.struct3DHead.nrec; i++, s++) {
        // Keep track of unique textures in trackblock for later OpenGL bind
        std::set<short> minimal_texture_ids_set;
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> verts;
        for (int j = 0; j < s->nVert; j++, s->vertex++) {
            verts.emplace_back(glm::vec3(s->vertex->pt.x / 10,
                                         s->vertex->pt.y / 10,
                                         s->vertex->pt.z / 10));
        }
        for (int k = 0; k < s->nPoly; k++, s->polygon++) {
            // Remap the COL TextureID's using the COL texture block (XBID2)
            COLTEXTUREINFO col_texture = col.texture[s->polygon->texture];
            TEXTUREBLOCK texture_for_block = texture[col_texture.texture];
            minimal_texture_ids_set.insert(texture_for_block.texture); //TODO: Different QFS file being referenced
            indices.emplace_back(s->polygon->v[0]);
            indices.emplace_back(s->polygon->v[1]);
            indices.emplace_back(s->polygon->v[2]);
            indices.emplace_back(s->polygon->v[0]);
            indices.emplace_back(s->polygon->v[2]);
            indices.emplace_back(s->polygon->v[3]);
            uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
            uvs.emplace_back(texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
            uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
            uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
            uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
            uvs.emplace_back(texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
            normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
            normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
            normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
            normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
            normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
            normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
        }
        // Get ordered list of unique texture id's present in block
        std::vector<short> texture_ids = RemapNormals(minimal_texture_ids_set, normals);
        Model col_model = Model("ColBlock" + std::to_string(i), verts, uvs, normals, indices, true, texture_ids);
        col_model.enable();
        col_model.track = true;
        col_models.emplace_back(col_model);
    }
    return col_models;
}

std::vector<Model> trk_loader::GetXOBJModels() {
    std::vector<Model> xobj_models;

    /* XOBJS - EXTRA OBJECTS */
    for (int i = 0; i <= 4 * nBlocks; i++) {
        for (int j = 0; j < xobj[i].nobj; j++) {
            XOBJDATA *x = &(xobj[i].obj[j]);
            if (x->crosstype == 4) { // basic objects
            } else if (x->crosstype == 3) { // animated objects
            }
            // common part : vertices & polygons
            std::vector<glm::vec3> verts;
            for (int k = 0; k < x->nVertices; k++, x->vert++) {
                verts.emplace_back(glm::vec3(x->ptRef.x / 10 + x->vert->x / 10,
                                             x->ptRef.y / 10 + x->vert->y / 10,
                                             x->ptRef.z / 10 + x->vert->z / 10));
            }
            // TODO: There are also these extras: x->unknVertices, sz: 4*x->nVertices
            std::set<short> minimal_texture_ids_set;
            std::vector<unsigned int> indices;
            std::vector<glm::vec2> uvs;
            std::vector<glm::vec3> normals;
            for (int k = 0; k < x->nPolygons; k++, x->polyData++) {
                TEXTUREBLOCK texture_for_block = texture[x->polyData->texture];
                minimal_texture_ids_set.insert(texture_for_block.texture);
                indices.emplace_back(x->polyData->vertex[0]);
                indices.emplace_back(x->polyData->vertex[1]);
                indices.emplace_back(x->polyData->vertex[2]);
                indices.emplace_back(x->polyData->vertex[0]);
                indices.emplace_back(x->polyData->vertex[2]);
                indices.emplace_back(x->polyData->vertex[3]);
                uvs.emplace_back(1.0f - texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                uvs.emplace_back(1.0f - texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
                uvs.emplace_back(1.0f - texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                uvs.emplace_back(1.0f - texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                uvs.emplace_back(1.0f - texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                uvs.emplace_back(1.0f - texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
            }
            // Get ordered list of unique texture id's present in block
            std::vector<short> texture_ids = RemapNormals(minimal_texture_ids_set, normals);
            Model xobj_model = Model("XOBJ" + std::to_string(i), verts, uvs, normals, indices, true, texture_ids);
            xobj_model.enable();
            xobj_model.track = true;
            xobj_models.emplace_back(xobj_model);
        }
    }
    return xobj_models;
}

std::vector<Model> trk_loader::GetTRKModels() {
    std::vector<Model> trk_blocks;

    /* TRKBLOCKS - BASE TRACK GEOMETRY */
    for (int i = 0; i < nBlocks; i++) {
        // Get Verts from Trk block, indices from associated polygon block
        TRKBLOCK trk_block = trk[i];
        POLYGONBLOCK polygon_block = poly[i];

        if (trk_block.nXobj == 0) {
            // Get Object vertices
            std::vector<glm::vec3> obj_verts;
            for (int v = 0; v < trk_block.nObjectVert; v++) {
                obj_verts.emplace_back(glm::vec3(trk_block.vert[v].x / 10,
                                                 trk_block.vert[v].y / 10,
                                                 trk_block.vert[v].z / 10));
            }
            // 4 OBJ Poly blocks
            for (int j = 0; j < 4; j++) {
                OBJPOLYBLOCK obj_polygon_block = polygon_block.obj[j];
                if (obj_polygon_block.n1 > 0) {
                    // Iterate through objects in objpoly block up to num objects
                    for (int k = 0; k < obj_polygon_block.n2; k++) {
                        if (obj_polygon_block.types[k] == 1) {
                            // Keep track of unique textures in trackblock for later OpenGL bind
                            std::set<short> minimal_texture_ids_set;
                            // Mesh Data
                            std::vector<unsigned int> indices;
                            std::vector<glm::vec2> uvs;
                            std::vector<glm::vec3> normals;
                            // Get Polygons in object
                            LPPOLYGONDATA object_polys = obj_polygon_block.poly[k];
                            for (int p = 0; p < obj_polygon_block.numpoly[k]; p++) {
                                TEXTUREBLOCK texture_for_block = texture[object_polys[p].texture];
                                minimal_texture_ids_set.insert(texture_for_block.texture);
                                indices.emplace_back(object_polys[p].vertex[0]);
                                indices.emplace_back(object_polys[p].vertex[1]);
                                indices.emplace_back(object_polys[p].vertex[2]);
                                indices.emplace_back(object_polys[p].vertex[0]);
                                indices.emplace_back(object_polys[p].vertex[2]);
                                indices.emplace_back(object_polys[p].vertex[3]);
                                uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                                uvs.emplace_back(texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
                                uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                                uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                                uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                                uvs.emplace_back(texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
                                // Use TextureID in place of normal
                                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                            }
                            // Get ordered list of unique texture id's present in block
                            std::vector<short> texture_ids = RemapNormals(minimal_texture_ids_set, normals);
                            Model current_trk_block_model = Model("ObjBlock" + std::to_string((i*(j+k))*3), obj_verts, uvs, normals, indices, true, texture_ids);
                            current_trk_block_model.enable();
                            current_trk_block_model.track = true;
                            trk_blocks.emplace_back(current_trk_block_model);
                        }
                    }
                }
            }
        }

        // Keep track of unique textures in trackblock for later OpenGL bind
        std::set<short> minimal_texture_ids_set;
        // Mesh Data
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> verts;
        for (int j = 0; j < trk_block.nHiResVert; j++) {
            verts.emplace_back(glm::vec3(trk_block.vert[j].x / 10, trk_block.vert[j].y / 10, trk_block.vert[j].z / 10));
        }
        // Get indices from Chunk 4 for High Res polys
        LPPOLYGONDATA poly_chunk = polygon_block.poly[4];
        for (int chnk = 0; chnk < 6; chnk++) {
            for (int k = 0; k < polygon_block.sz[chnk]; k++) {
                TEXTUREBLOCK texture_for_block = texture[poly_chunk[k].texture];
                minimal_texture_ids_set.insert(texture_for_block.texture);
                indices.emplace_back(poly_chunk[k].vertex[0]);
                indices.emplace_back(poly_chunk[k].vertex[1]);
                indices.emplace_back(poly_chunk[k].vertex[2]);
                indices.emplace_back(poly_chunk[k].vertex[0]);
                indices.emplace_back(poly_chunk[k].vertex[2]);
                indices.emplace_back(poly_chunk[k].vertex[3]);
                uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                uvs.emplace_back(texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
                uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                uvs.emplace_back(texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
                // Use TextureID in place of normal
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
                normals.emplace_back(glm::vec3(texture_for_block.texture, 0, 0));
            }
        }
        // Get ordered list of unique texture id's present in block
        std::vector<short> texture_ids = RemapNormals(minimal_texture_ids_set, normals);
        Model current_trk_block_model = Model("TrkBlock" + std::to_string(i), verts, uvs, normals, indices, true, texture_ids);
        current_trk_block_model.enable();
        current_trk_block_model.track = true;
        trk_blocks.emplace_back(current_trk_block_model);
    }

    return trk_blocks;
}

trk_loader::trk_loader(const std::string &frd_path) {
    if (LoadFRD(frd_path)) {
        if (LoadCOL("../resources/TRK006/TR06.COL")) {
            std::cout << "Successful track load!" << std::endl;
            texture_gl_mappings = GenTrackTextures(textures);
        } else
            return;
    } else
        return;

    std::vector<Model> col_models = GetCOLModels();
    //track_models.insert(track_models.begin(), col_models.begin(), col_models.end());
    std::vector<Model> xobj_models = GetXOBJModels();
    track_models.insert(track_models.begin(), xobj_models.begin(), xobj_models.end());
    std::vector<Model> trk_models = GetTRKModels();
    track_models.insert(track_models.begin(), trk_models.begin(), trk_models.end());
}

vector<Model> trk_loader::getTrackBlocks() {
    return track_models;
}

std::map<short, GLuint> trk_loader::getTextureGLMap() {
    return texture_gl_mappings;
}

std::map<short, Texture> trk_loader::getTextures() {
    return textures;
}
