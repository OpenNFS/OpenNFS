//
// Created by Amrik on 16/01/2018.

#include "trk_loader.h"

using namespace std;

namespace NFS3{
    TRACK *trk_loader(const std::string &track_base_path) {
        std::cout << "--- Loading NFS3 Track ---" << std::endl;

		TRACK *track = new TRACK();
        boost::filesystem::path p(track_base_path);
        std::string track_name = p.filename().string();
        stringstream frd_path, col_path;
        string strip = "K0";
        unsigned int pos= track_name.find(strip);
        if(pos!= string::npos)
            track_name.replace(pos, strip.size(), "");

        frd_path << track_base_path <<  "/" << track_name << ".frd";
        col_path << track_base_path <<  "/" << track_name << ".col";


        if (LoadFRD(frd_path.str(), track)) {
            if (LoadCOL(col_path.str(), track)) {
                track->texture_gl_mappings = GenTrackTextures(track->textures);
                track->track_blocks = ParseTRKModels(track);
                std::vector<Track> col_models = ParseCOLModels(track);
                track->track_blocks[0].objects.insert(track->track_blocks[0].objects.end(), col_models.begin(), col_models.end()); // Insert the COL models into track block 0 for now
            }
            else {
                delete track;
                return nullptr ;
            }
        } else {
			delete track;
            return nullptr ;
        }

        std::cout << "Successful track load!" << std::endl;

        return track;
    }

    std::map<short, GLuint> GenTrackTextures(std::map<short, Texture> textures) {
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
            // TODO: Use Filtering for Textures with no alpha component
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE,
                              (const GLvoid *) texture.texture_data);
        }

        return gl_id_map;
    }

    std::vector<short> RemapTextureIDs(const std::set<short> &minimal_texture_ids_set, std::vector<unsigned int> &texture_indices) {
        // Get ordered list of unique texture id's present in block
        std::vector<short> texture_ids;
        texture_ids.assign(minimal_texture_ids_set.begin(), minimal_texture_ids_set.end());
        // Remap Normals to correspond to ordered texture ID's
        std::map<int, int> ordered_mapping;
        for (int t = 0; t < texture_ids.size(); ++t) {
            auto p = std::make_pair((int) texture_ids[t], t);
            ordered_mapping.insert(p);
        }
        for (auto &texture_index : texture_indices) {
            texture_index = static_cast<unsigned int>(ordered_mapping.find(texture_index)->second);
        }
        return texture_ids;
    }

    bool LoadFRD(std::string frd_path, TRACK *track) {
        // TODO: Wrap each fread with if(STATEMENT) != numElementsToRead) return false; MACRO?
        ifstream ar(frd_path, ios::in | ios::binary);

        int j, k, l;
        TRKBLOCK *trackBlock;
        POLYGONBLOCK *p;
        XOBJDATA *x;
        OBJPOLYBLOCK *o;

        char header[28]; /* file header */
        if (ar.read(header, 28).gcount() != 28) return false; // header & numblocks
        if (ar.read((char *) &track->nBlocks, 4).gcount() < 4) return false;
        track->nBlocks++;
        if ((track->nBlocks < 1) || (track->nBlocks > 500)) return false; // 1st sanity check

    	track->trk = (TRKBLOCK *) calloc(track->nBlocks, sizeof(TRKBLOCK));
        track->poly = (POLYGONBLOCK *) calloc(track->nBlocks, sizeof(POLYGONBLOCK));
        track->xobj = (XOBJBLOCK *) calloc((4 * track->nBlocks + 1), sizeof(XOBJBLOCK));

        if (ar.read((char *) &l, 4).gcount() < 4) return false; // choose between NFS3 & NFSHS
        if ((l < 0) || (l > 5000)) track->bHSMode = false;
        else if (((l + 7) / 8) == track->nBlocks) track->bHSMode = true;
        else return false; // unknown file type

        memcpy(track->trk, &l, 4);
        if (ar.read(((char *) track->trk) + 4, 80).gcount() != 80) return false;

        // TRKBLOCKs
        for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
            trackBlock = &(track->trk[block_Idx]);
            // ptCentre, ptBounding, 6 nVertices == 84 bytes
            if (block_Idx != 0) { if (ar.read((char *) trackBlock, 84).gcount() != 84) return false; }
            if ((trackBlock->nVertices < 0)) return false;
            trackBlock->vert = (FLOATPT *) calloc(trackBlock->nVertices, sizeof(FLOATPT));

            if ((uint32_t) ar.read((char *) trackBlock->vert, 12 * trackBlock->nVertices).gcount() != 12 * trackBlock->nVertices) return false;
            trackBlock->unknVertices = (uint32_t *) calloc(trackBlock->nVertices, sizeof(uint32_t));

            if ((uint32_t) ar.read((char *) trackBlock->unknVertices, 4 * trackBlock->nVertices).gcount() != 4 * trackBlock->nVertices) return false;
            if (ar.read((char *) trackBlock->nbdData, 4 * 0x12C).gcount() != 4 * 0x12C) return false;

            // nStartPos & various blk sizes == 32 bytes
            if (ar.read((char *) &(trackBlock->nStartPos), 32).gcount() != 32) return false;
            if (block_Idx > 0) if (trackBlock->nStartPos != track->trk[block_Idx - 1].nStartPos + track->trk[block_Idx - 1].nPositions) return false;
            trackBlock->posData = (POSITIONDATA *) malloc(trackBlock->nPositions * sizeof(POSITIONDATA));
            if (trackBlock->posData == nullptr) return false;
            if ((uint32_t) ar.read((char *) trackBlock->posData, 8 * trackBlock->nPositions).gcount() != 8 * trackBlock->nPositions) return false;
            trackBlock->polyData = (POLYVROADDATA *) malloc(trackBlock->nPolygons * sizeof(POLYVROADDATA));
            if (trackBlock->polyData == nullptr) return false;
            memset(trackBlock->polyData, 0, trackBlock->nPolygons * sizeof(POLYVROADDATA));
            for (j = 0; j < trackBlock->nPolygons; j++)
                if (ar.read((char *) trackBlock->polyData + j, 8).gcount() != 8) return false;
            trackBlock->vroadData = (VROADDATA *) malloc(trackBlock->nVRoad * sizeof(VROADDATA));
            if (trackBlock->vroadData == nullptr) return false;
            if ((uint32_t) ar.read((char *) trackBlock->vroadData, 12 * trackBlock->nVRoad).gcount() != 12 * trackBlock->nVRoad) return false;
            if (trackBlock->nXobj > 0) {
                trackBlock->xobj = (REFXOBJ *) malloc(trackBlock->nXobj * sizeof(REFXOBJ));
                if (trackBlock->xobj == nullptr) return false;
                if ((uint32_t) ar.read((char *) trackBlock->xobj, 20 * trackBlock->nXobj).gcount() != 20 * trackBlock->nXobj) return false;
            }
            if (trackBlock->nPolyobj > 0) {
                ar.seekg(20 * trackBlock->nPolyobj, ios_base::cur);
            }
            trackBlock->nPolyobj = 0;
            if (trackBlock->nSoundsrc > 0) {
                trackBlock->soundsrc = (SOUNDSRC *) malloc(trackBlock->nSoundsrc * sizeof(SOUNDSRC));
                if (trackBlock->soundsrc == nullptr) return false;
                if ((uint32_t) ar.read((char *) trackBlock->soundsrc, 16 * trackBlock->nSoundsrc).gcount() != 16 * trackBlock->nSoundsrc) return false;
            }
            if (trackBlock->nLightsrc > 0) {
                trackBlock->lightsrc = (LIGHTSRC *) malloc(trackBlock->nLightsrc * sizeof(LIGHTSRC));
                if (trackBlock->lightsrc == nullptr) return false;
                if ((uint32_t) ar.read((char *) trackBlock->lightsrc, 16 * trackBlock->nLightsrc).gcount() != 16 * trackBlock->nLightsrc) return false;
            }
        }

        // TODO: Identify in what cases stream reads garbage
        // This workaround is emblematic of a larger problem, why does the file stream 'ar' not read into these structs
        auto pos = static_cast<int>(ar.tellg());
        FILE *trk_file = fopen(frd_path.c_str(), "rb");
        fseek(trk_file, pos, SEEK_SET);

        // POLYGONBLOCKs
        for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
            p = &(track->poly[block_Idx]);
            for (j = 0; j < 7; j++) {
                fread(&(p->sz[j]), 0x4, 1, trk_file);
                if (p->sz[j] != 0) {
                    fread(&(p->szdup[j]), 0x4, 1, trk_file);
                    if (p->szdup[j] != p->sz[j]) return false;
                    p->poly[j] = (LPPOLYGONDATA) malloc(p->sz[j] * sizeof(POLYGONDATA));
                    if (p->poly[j] == nullptr) return false;
                    fread(p->poly[j], static_cast<size_t>(14 * p->sz[j]), 1, trk_file);
                }
            }
            if (p->sz[4] != track->trk[block_Idx].nPolygons) return false; // sanity check
            for (j = 0; j < 4; j++) {
                o = &(p->obj[j]);
                fread(&(o->n1), 0x4, 1, trk_file);
                if (o->n1 > 0) {
                    fread(&(o->n2), 0x4, 1, trk_file);
                    o->types = (uint32_t *) calloc(static_cast<size_t>(o->n2), sizeof(uint32_t));
                    if (o->types == nullptr) return false;
                    o->numpoly = (uint32_t *) malloc(o->n2 * sizeof(uint32_t));
                    if (o->numpoly == nullptr) return false;
                    o->poly = (LPPOLYGONDATA *) calloc(static_cast<size_t>(o->n2), sizeof(LPPOLYGONDATA));
                    if (o->poly == nullptr) return false;
                    o->nobj = 0;
                    l = 0;
                    for (k = 0; k < o->n2; k++) {
                        fread(o->types + k, 0x4, 1, trk_file);
                        if (o->types[k] == 1) {
                            fread(o->numpoly + o->nobj, 0x4, 1, trk_file);
                            o->poly[o->nobj] = (LPPOLYGONDATA) malloc(o->numpoly[o->nobj] * sizeof(POLYGONDATA));
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
        for (uint32_t xblock_Idx = 0; xblock_Idx <= 4 * track->nBlocks; xblock_Idx++) {
            if (ar.read((char *) &(track->xobj[xblock_Idx].nobj), 4).gcount() != 4) return false;
            if (track->xobj[xblock_Idx].nobj > 0) {
                track->xobj[xblock_Idx].obj = (XOBJDATA *) malloc(track->xobj[xblock_Idx].nobj * sizeof(XOBJDATA));
                if (track->xobj[xblock_Idx].obj == nullptr) return false;
                memset(track->xobj[xblock_Idx].obj, 0, track->xobj[xblock_Idx].nobj * sizeof(XOBJDATA));
            }
            for (j = 0; j < track->xobj[xblock_Idx].nobj; j++) {
                x = &(track->xobj[xblock_Idx].obj[j]);
                // 3 headers == 12 bytes
                if (ar.read((char *) x, 12).gcount() != 12) return false;
                if (x->crosstype == 4) { // basic objects
                    if (ar.read((char *) &(x->ptRef), 12).gcount() != 12) return false;
                    if (ar.read((char *) &(x->unknown2), 4).gcount() != 4) return false;
                } else if (x->crosstype == 3) { // animated objects
                    // unkn3, type3, objno, nAnimLength, unkn4 == 24 bytes
                    if (ar.read((char *) x->unknown3, 24).gcount() != 24) return false;
                    if (x->type3 != 3) return false;
                    x->animData = (ANIMDATA *) malloc(static_cast<size_t>(20 * x->nAnimLength));
                    if (x->animData == nullptr) return false;
                    if ((uint32_t) ar.read((char *) x->animData, 20 * x->nAnimLength).gcount() != 20 * x->nAnimLength)
                        return false;
                    // make a ref point from first anim position
                    x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                    x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                    x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
                } else return false; // unknown object type
                // common part : vertices & polygons
                if (ar.read((char *) &(x->nVertices), 4).gcount() != 4) return false;
                x->vert = (FLOATPT *) malloc(static_cast<size_t>(12 * x->nVertices));
                if (x->vert == nullptr) return false;
                if ((uint32_t) ar.read((char *) x->vert, 12 * x->nVertices).gcount() != 12 * x->nVertices) return false;
                x->unknVertices = (uint32_t *) malloc(static_cast<size_t>(4 * x->nVertices));
                if (x->unknVertices == nullptr) return false;
                if ((uint32_t) ar.read((char *) x->unknVertices, 4 * x->nVertices).gcount() != 4 * x->nVertices)
                    return false;
                if (ar.read((char *) &(x->nPolygons), 4).gcount() != 4) return false;
                x->polyData = (POLYGONDATA *) malloc(static_cast<size_t>(x->nPolygons * 14));
                if (x->polyData == nullptr) return false;
                if ((uint32_t) ar.read((char *) x->polyData, 14 * x->nPolygons).gcount() != 14 * x->nPolygons) return false;
            }
        }


        // TEXTUREBLOCKs
        if (ar.read((char *) &track->nTextures, 4).gcount() != 4) return false;
        track->texture = (TEXTUREBLOCK *) calloc(track->nTextures, sizeof(TEXTUREBLOCK));
        for (uint32_t tex_Idx = 0; tex_Idx < track->nTextures; tex_Idx++) {
            if (ar.read((char *) &(track->texture[tex_Idx]), 47).gcount() != 47) return false;
            auto p = std::make_pair(track->texture[tex_Idx].texture, LoadTexture(track->texture[tex_Idx]));
            track->textures.insert(p);
        }

        uint32_t pad;
        return ar.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
    }

    bool LoadCOL(std::string col_path, TRACK *track) {
        // TODO: Wrap each fread with if(STATEMENT) != numElementsToRead) return false; MACRO?
        ifstream coll(col_path, ios::in | ios::binary);

        COLOBJECT *o;

        track->col.hs_extra = NULL;
        if (coll.read((char *) &track->col, 16).gcount() != 16) return false;
        if ((track->col.collID[0] != 'C') || (track->col.collID[1] != 'O') ||
            (track->col.collID[2] != 'L') || (track->col.collID[3] != 'L'))
            return false;
        if (track->col.version != 11) return false;
        if ((track->col.nBlocks != 2) && (track->col.nBlocks != 4) && (track->col.nBlocks != 5)) return false;
        if ((uint32_t) coll.read((char *) track->col.xbTable, 4 * track->col.nBlocks).gcount() != 4 * track->col.nBlocks) return false;

        // texture XB
        if (coll.read((char *) &track->col.textureHead, 8).gcount() != 8) return false;
        if (track->col.textureHead.xbid != XBID_TEXTUREINFO) return false;
        //if (col.textureHead.size != 8 + 8 * col.textureHead.nrec) return false;
        track->col.texture = (COLTEXTUREINFO *) malloc(track->col.textureHead.nrec * sizeof(COLTEXTUREINFO));
        if (track->col.texture == NULL) return false;
        if ((uint32_t) coll.read((char *) track->col.texture, 8 * track->col.textureHead.nrec).gcount() != 8 * track->col.textureHead.nrec)
            return false;

        // struct3D XB
        if (track->col.nBlocks >= 4) {
            if (coll.read((char *) &track->col.struct3DHead, 8).gcount() != 8) return false;
            if (track->col.struct3DHead.xbid != XBID_STRUCT3D) return false;
            COLSTRUCT3D *s = track->col.struct3D = (COLSTRUCT3D *) malloc(track->col.struct3DHead.nrec * sizeof(COLSTRUCT3D));
            if (s == NULL) return false;
            memset(s, 0, track->col.struct3DHead.nrec * sizeof(COLSTRUCT3D));
            int delta;
            for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.struct3DHead.nrec; colRec_Idx++, s++) {
                if (coll.read((char *) s, 8).gcount() != 8) return false;
                delta = (8 + 16 * s->nVert + 6 * s->nPoly) % 4;
                delta = (4 - delta) % 4;
                if (s->size != 8 + 16 * s->nVert + 6 * s->nPoly + delta) return false;
                s->vertex = (COLVERTEX *) malloc(16 * s->nVert);
                if (s->vertex == NULL) return false;
                if ((uint32_t) coll.read((char *) s->vertex, 16 * s->nVert).gcount() != 16 * s->nVert) return false;
                s->polygon = (COLPOLYGON *) malloc(6 * s->nPoly);
                if (s->polygon == NULL) return false;
                if ((uint32_t) coll.read((char *) s->polygon, 6 * s->nPoly).gcount() != 6 * s->nPoly) return false;
                int dummy;
                if (delta > 0) if ((int) coll.read((char *) &dummy, delta).gcount() != delta) return false;
            }

            // object XB
            if (coll.read((char *) &track->col.objectHead, 8).gcount() != 8) return false;
            if ((track->col.objectHead.xbid != XBID_OBJECT) && (track->col.objectHead.xbid != XBID_OBJECT2)) return false;
            o = track->col.object = (COLOBJECT *)
                    malloc(track->col.objectHead.nrec * sizeof(COLOBJECT));
            if (o == NULL) return false;
            memset(o, 0, track->col.objectHead.nrec * sizeof(COLOBJECT));
            for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.objectHead.nrec; colRec_Idx++, o++) {
                if (coll.read((char *) o, 4).gcount() != 4) return false;
                if (o->type == 1) {
                    if (o->size != 16) return false;
                    if (coll.read((char *) &(o->ptRef), 12).gcount() != 12) return false;
                } else if (o->type == 3) {
                    if (coll.read((char *) &(o->animLength), 4).gcount() != 4) return false;
                    if (o->size != 8 + 20 * o->animLength) return false;
                    o->animData = (ANIMDATA *) malloc(20 * o->animLength);
                    if (o->animData == nullptr) return false;
                    if ((uint32_t) coll.read((char *) o->animData, 20 * o->animLength).gcount() != 20 * o->animLength)
                        return false;
                    o->ptRef.x = o->animData->pt.x;
                    o->ptRef.z = o->animData->pt.z;
                    o->ptRef.y = o->animData->pt.y;
                } else return false; // unknown object type
            }
        }

        // object2 XB
        if (track->col.nBlocks == 5) {
            if (coll.read((char *) &track->col.object2Head, 8).gcount() != 8) return false;
            if ((track->col.object2Head.xbid != XBID_OBJECT) && (track->col.object2Head.xbid != XBID_OBJECT2)) return false;
            o = track->col.object2 = (COLOBJECT *)
                    malloc(track->col.object2Head.nrec * sizeof(COLOBJECT));
            if (o == NULL) return false;
            memset(o, 0, track->col.object2Head.nrec * sizeof(COLOBJECT));
            for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.object2Head.nrec; colRec_Idx++, o++) {
                if (coll.read((char *) o, 4).gcount() != 4) return false;
                if (o->type == 1) {
                    if (o->size != 16) return false;
                    if (coll.read((char *) &(o->ptRef), 12).gcount() != 12) return false;
                } else if (o->type == 3) {
                    if (coll.read((char *) &(o->animLength), 4).gcount() != 4) return false;
                    if (o->size != 8 + 20 * o->animLength) return false;
                    o->animData = (ANIMDATA *) malloc(20 * o->animLength);
                    if (o->animData == NULL) return false;
                    if ((uint32_t) coll.read((char *) o->animData, 20 * o->animLength).gcount() != 20 * o->animLength)
                        return false;
                    o->ptRef.x = o->animData->pt.x;
                    o->ptRef.z = o->animData->pt.z;
                    o->ptRef.y = o->animData->pt.y;
                } else return false; // unknown object type
            }
        }

        // vroad XB
        if (coll.read((char *) &track->col.vroadHead, 8).gcount() != 8) return false;
        if (track->col.vroadHead.xbid != XBID_VROAD) return false;
        if (track->col.vroadHead.size != 8 + 36 * track->col.vroadHead.nrec) return false;
        //ASSERT(col.vroadHead.nrec==trk[nBlocks-1].nStartPos+trk[nBlocks-1].nPositions);
        track->col.vroad = (COLVROAD *) malloc(track->col.vroadHead.nrec * sizeof(COLVROAD));
        if (track->col.vroad == NULL) return false;
        if ((uint32_t) coll.read((char *) track->col.vroad, 36 * track->col.vroadHead.nrec).gcount() != 36 * track->col.vroadHead.nrec)
            return false;

        uint32_t pad;
        return coll.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
    }

    std::vector<Track> ParseCOLModels(TRACK *track) {
        std::vector<Track> col_models;
        COLOBJECT *o = track->col.object;
        /* COL DATA - TODO: Come back for VROAD AI/Collision data */
        for (int i = 0; i < track->col.objectHead.nrec; i++, o++) {
            COLSTRUCT3D s = track->col.struct3D[o->struct3D];
            // Keep track of unique textures in trackblock for later OpenGL bind
            std::set<short> minimal_texture_ids_set;
            std::vector<unsigned int> indices;
            std::vector<glm::vec2> uvs;
            std::vector<unsigned int> texture_indices;
            std::vector<glm::vec3> verts;
            std::vector<glm::vec4> shading_data;
            for (int j = 0; j < s.nVert; j++, s.vertex++) {
                verts.emplace_back(glm::vec3(s.vertex->pt.x / 10,
                                             s.vertex->pt.y / 10,
                                             s.vertex->pt.z / 10));
                shading_data.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
            }
            for (int k = 0; k < s.nPoly; k++, s.polygon++) {
                // Remap the COL TextureID's using the COL texture block (XBID2)
                COLTEXTUREINFO col_texture = track->col.texture[s.polygon->texture];
                TEXTUREBLOCK texture_for_block;
                // Find the texture by it's file name, but use the Texture table to get the block. TODO: Not mapping this so, must do a manual search.
                for (int t = 0; t < track->nTextures; t++) {
                    if (track->texture[t].texture == col_texture.texture) {
                        texture_for_block = track->texture[t];
                    }
                }
                minimal_texture_ids_set.insert(texture_for_block.texture);
                indices.emplace_back(s.polygon->v[0]);
                indices.emplace_back(s.polygon->v[1]);
                indices.emplace_back(s.polygon->v[2]);
                indices.emplace_back(s.polygon->v[0]);
                indices.emplace_back(s.polygon->v[2]);
                indices.emplace_back(s.polygon->v[3]);
                uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                uvs.emplace_back(texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
                uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                uvs.emplace_back(texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
            }
            // Get ordered list of unique texture id's present in block
            std::vector<short> texture_ids = RemapTextureIDs(minimal_texture_ids_set, texture_indices);
            glm::vec3 position = glm::vec3(static_cast<float>(o->ptRef.x / 65536.0)/10, static_cast<float>(o->ptRef.y / 65536.0)/10, static_cast<float>(o->ptRef.z / 65536.0)/10);
            Track col_model = Track("ColBlock", i, verts, uvs, texture_indices, indices, texture_ids, shading_data,
                                    glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * position);
            col_model.enable();
            col_models.emplace_back(col_model);
        }
        return col_models;
    }

    std::vector<TrackBlock> ParseTRKModels(TRACK *track) {
        std::vector<TrackBlock> track_blocks = std::vector<TrackBlock>();
        /* TRKBLOCKS - BASE TRACK GEOMETRY */
        for (int i = 0; i < track->nBlocks; i++) {
            // Get Verts from Trk block, indices from associated polygon block
            TRKBLOCK trk_block = track->trk[i];
            POLYGONBLOCK polygon_block = track->poly[i];
            TrackBlock current_track_block(i, trk_block);
            glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
            glm::vec3 trk_block_center = orientation * glm::vec3(0, 0, 0);

            // Light sources
            for (int j = 0; j < trk_block.nLightsrc; j++) {
                Light temp_light = Light(trk_block.lightsrc[j].refpoint, trk_block.lightsrc[j].type);
                temp_light.enable();
                current_track_block.lights.emplace_back(temp_light);
            }

            for(int s = 0; s < trk_block.nSoundsrc; s++){
                Light temp_light = Light(trk_block.soundsrc[s].refpoint, trk_block.soundsrc[s].type);
                temp_light.enable();
                current_track_block.lights.emplace_back(temp_light);
            }

            // Get Object vertices
            std::vector<glm::vec3> obj_verts;
            std::vector<glm::vec4> obj_shading_verts;
            for (int v = 0; v < trk_block.nObjectVert; v++) {
                obj_verts.emplace_back(glm::vec3(trk_block.vert[v].x / 10,
                                                 trk_block.vert[v].y / 10,
                                                 trk_block.vert[v].z / 10));
                uint32_t shading_data = trk_block.unknVertices[v];
                obj_shading_verts.emplace_back(
                        glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f,
                                  (shading_data & 0xFF) / 255.0f, ((shading_data >> 24) & 0xFF) / 255.0f));
            }
            // 4 OBJ Poly blocks
            for (int j = 0; j < 4; j++) {
                OBJPOLYBLOCK obj_polygon_block = polygon_block.obj[j];
                if (obj_polygon_block.n1 > 0) {
                    // Iterate through objects in objpoly block up to num objects
                    for (int k = 0; k < obj_polygon_block.nobj; k++) {
                        //TODO: Animated objects here, obj_polygon_block.types
                        // Keep track of unique textures in trackblock for later OpenGL bind
                        std::set<short> minimal_texture_ids_set;
                        // Mesh Data
                        std::vector<unsigned int> vertex_indices;
                        std::vector<glm::vec2> uvs;
                        std::vector<unsigned int> texture_indices;
                        std::vector<glm::vec3> norms;
                        FLOATPT norm_floatpt;
                        // Get Polygons in object
                        LPPOLYGONDATA object_polys = obj_polygon_block.poly[k];
                        for (int p = 0; p < obj_polygon_block.numpoly[k]; p++) {
                            TEXTUREBLOCK texture_for_block = track->texture[object_polys[p].texture];
                            minimal_texture_ids_set.insert(texture_for_block.texture);
                            norm_floatpt = VertexNormal(i, object_polys[p].vertex[0], track->trk, track->poly);
                            norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                            norm_floatpt = VertexNormal(i, object_polys[p].vertex[1], track->trk, track->poly);
                            norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                            norm_floatpt = VertexNormal(i, object_polys[p].vertex[2], track->trk, track->poly);
                            norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                            norm_floatpt = VertexNormal(i, object_polys[p].vertex[0], track->trk, track->poly);
                            norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                            norm_floatpt = VertexNormal(i, object_polys[p].vertex[2], track->trk, track->poly);
                            norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                            norm_floatpt = VertexNormal(i, object_polys[p].vertex[3], track->trk, track->poly);
                            norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                            vertex_indices.emplace_back(object_polys[p].vertex[0]);
                            vertex_indices.emplace_back(object_polys[p].vertex[1]);
                            vertex_indices.emplace_back(object_polys[p].vertex[2]);
                            vertex_indices.emplace_back(object_polys[p].vertex[0]);
                            vertex_indices.emplace_back(object_polys[p].vertex[2]);
                            vertex_indices.emplace_back(object_polys[p].vertex[3]);
                            uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                            uvs.emplace_back(texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
                            uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                            uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                            uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                            uvs.emplace_back(texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
                            // Use TextureID in place of normal
                            texture_indices.emplace_back(texture_for_block.texture);
                            texture_indices.emplace_back(texture_for_block.texture);
                            texture_indices.emplace_back(texture_for_block.texture);
                            texture_indices.emplace_back(texture_for_block.texture);
                            texture_indices.emplace_back(texture_for_block.texture);
                            texture_indices.emplace_back(texture_for_block.texture);
                        }
                        // Get ordered list of unique texture id's present in block
                        std::vector<short> texture_ids = RemapTextureIDs(minimal_texture_ids_set,
                                                                         texture_indices);
                        Track current_track_model = Track("ObjBlock", (j + 1) * (k + 1), obj_verts, uvs,
                                                          texture_indices, vertex_indices, texture_ids,
                                                          obj_shading_verts, trk_block_center);
                        current_track_model.enable();
                        current_track_block.objects.emplace_back(current_track_model);
                    }
                }
            }

            /* XOBJS - EXTRA OBJECTS */
            for (int l = (i * 4); l < (i * 4) + 4; l++) {
                for (int j = 0; j < track->xobj[l].nobj; j++) {
                    XOBJDATA *x = &(track->xobj[l].obj[j]);
                    if (x->crosstype == 4) { // basic objects
                    } else if (x->crosstype == 3) { // animated objects
                    }
                    // common part : vertices & polygons
                    std::vector<glm::vec3> verts;
                    std::vector<glm::vec4> xobj_shading_verts;
                    for (int k = 0; k < x->nVertices; k++, x->vert++) {
                        verts.emplace_back(glm::vec3(x->ptRef.x / 10 + x->vert->x / 10,
                                                     x->ptRef.y / 10 + x->vert->y / 10,
                                                     x->ptRef.z / 10 + x->vert->z / 10));
                        uint32_t shading_data = x->unknVertices[k];
                        //RGBA
                        xobj_shading_verts.emplace_back(
                                glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f,
                                          (shading_data & 0xFF) / 255.0f, ((shading_data >> 24) & 0xFF) / 255.0f));
                    }
                    std::set<short> minimal_texture_ids_set;
                    std::vector<unsigned int> vertex_indices;
                    std::vector<glm::vec2> uvs;
                    std::vector<unsigned int> texture_indices;
                    std::vector<glm::vec3> norms;
                    FLOATPT norm_floatpt;
                    for (int k = 0; k < x->nPolygons; k++, x->polyData++) {
                        TEXTUREBLOCK texture_for_block = track->texture[x->polyData->texture];
                        minimal_texture_ids_set.insert(texture_for_block.texture);
                        norm_floatpt = VertexNormal(i, x->polyData->vertex[0], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        norm_floatpt = VertexNormal(i, x->polyData->vertex[1], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        norm_floatpt = VertexNormal(i, x->polyData->vertex[2], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        norm_floatpt = VertexNormal(i, x->polyData->vertex[0], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        norm_floatpt = VertexNormal(i, x->polyData->vertex[2], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        norm_floatpt = VertexNormal(i, x->polyData->vertex[3], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        vertex_indices.emplace_back(x->polyData->vertex[0]);
                        vertex_indices.emplace_back(x->polyData->vertex[1]);
                        vertex_indices.emplace_back(x->polyData->vertex[2]);
                        vertex_indices.emplace_back(x->polyData->vertex[0]);
                        vertex_indices.emplace_back(x->polyData->vertex[2]);
                        vertex_indices.emplace_back(x->polyData->vertex[3]);
                        uvs.emplace_back(1.0f - texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                        uvs.emplace_back(1.0f - texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
                        uvs.emplace_back(1.0f - texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                        uvs.emplace_back(1.0f - texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                        uvs.emplace_back(1.0f - texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                        uvs.emplace_back(1.0f - texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                    }
                    // Get ordered list of unique texture id's present in block
                    std::vector<short> texture_ids = RemapTextureIDs(minimal_texture_ids_set, texture_indices);
                    Track xobj_model = Track("XOBJ", l, verts, norms, uvs, texture_indices, vertex_indices, texture_ids,
                                             xobj_shading_verts, trk_block_center);
                    xobj_model.enable();
                    current_track_block.objects.emplace_back(xobj_model);
                }
            }


            // Keep track of unique textures in trackblock for later OpenGL bind
            std::set<short> minimal_texture_ids_set;
            // Mesh Data
            std::vector<unsigned int> vertex_indices;
            std::vector<glm::vec2> uvs;
            std::vector<unsigned int> texture_indices;
            std::vector<glm::vec3> verts;
            std::vector<glm::vec4> trk_block_shading_verts;
            std::vector<glm::vec3> norms;
            for (int j = 0; j < trk_block.nVertices; j++) {
                verts.emplace_back(
                        glm::vec3(trk_block.vert[j].x / 10, trk_block.vert[j].y / 10, trk_block.vert[j].z / 10));
                // Break uint32_t of RGB into 4 normalised floats and store into vec4
                uint32_t shading_data = trk_block.unknVertices[j];
                trk_block_shading_verts.emplace_back(
                        glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f,
                                  (shading_data & 0xFF) / 255.0f, ((shading_data >> 24) & 0xFF) / 255.0f));
            }
            FLOATPT norm_floatpt;
            // Get indices from Chunk 4 and 5 for High Res polys, Chunk 6 for Road Lanes
            for (int chnk = 4; chnk <= 6; chnk++) {
                if((chnk == 6)&&(trk_block.nVertices <= trk_block.nHiResVert))
                    continue;
                LPPOLYGONDATA poly_chunk = polygon_block.poly[chnk];
                for (int k = 0; k < polygon_block.sz[chnk]; k++) {
                    TEXTUREBLOCK texture_for_block = track->texture[poly_chunk[k].texture];
                    minimal_texture_ids_set.insert(texture_for_block.texture);
                    //norm_floatpt = VertexNormal(i, poly_chunk[k].vertex[0]);
                    //norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, poly_chunk[k].vertex[1]);
                    //norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, poly_chunk[k].vertex[2]);
                    //norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, poly_chunk[k].vertex[0]);
                    //norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, poly_chunk[k].vertex[2]);
                    //norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, poly_chunk[k].vertex[3]);
                    //norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    vertex_indices.emplace_back(poly_chunk[k].vertex[0]);
                    vertex_indices.emplace_back(poly_chunk[k].vertex[1]);
                    vertex_indices.emplace_back(poly_chunk[k].vertex[2]);
                    vertex_indices.emplace_back(poly_chunk[k].vertex[0]);
                    vertex_indices.emplace_back(poly_chunk[k].vertex[2]);
                    vertex_indices.emplace_back(poly_chunk[k].vertex[3]);
                    uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                    uvs.emplace_back(texture_for_block.corners[2], 1.0f - texture_for_block.corners[3]);
                    uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                    uvs.emplace_back(texture_for_block.corners[0], 1.0f - texture_for_block.corners[1]);
                    uvs.emplace_back(texture_for_block.corners[4], 1.0f - texture_for_block.corners[5]);
                    uvs.emplace_back(texture_for_block.corners[6], 1.0f - texture_for_block.corners[7]);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                }
                // Get ordered list of unique texture id's present in block
                std::vector<short> texture_ids = RemapTextureIDs(minimal_texture_ids_set, texture_indices);
                Track current_trk_block_model = Track("TrkBlock", i, verts, uvs, texture_indices, vertex_indices,
                                                      texture_ids,
                                                      trk_block_shading_verts,
                                                      trk_block_center);
                current_trk_block_model.enable();
                if(chnk != 6)
                    current_track_block.track.emplace_back(current_trk_block_model);
                current_track_block.objects.emplace_back(current_trk_block_model);
            }
            track_blocks.emplace_back(current_track_block);
        }
        return track_blocks;
    }

    Texture LoadTexture(TEXTUREBLOCK track_texture) {
        std::stringstream filename;
        std::stringstream filename_alpha;
        if (track_texture.islane) {
            filename << "../resources/sfx/" << setfill('0') << setw(4) << track_texture.texture + 9 << ".BMP";
            filename_alpha << "../resources/sfx/" << setfill('0') << setw(4) << track_texture.texture + 9
                           << "-a.BMP";
        } else {
            filename << "../resources/TRK002/textures/" << setfill('0') << setw(4) << track_texture.texture << ".BMP";
            filename_alpha << "../resources/TRK002/textures/" << setfill('0') << setw(4) << track_texture.texture
                           << "-a.BMP";
        }
        GLubyte *data;
        GLsizei width = track_texture.width;
        GLsizei height = track_texture.height;

        ASSERT(Utils::LoadBmpWithAlpha(filename.str().c_str(), filename_alpha.str().c_str(), &data, width, height),
               "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!");

        return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(track_texture.width),
                       static_cast<unsigned int>(track_texture.height));
    }
}

namespace NFS2{
    TRACK *trk_loader(const std::string &track_base_path) {
        std::cout << "--- Loading NFS2 Track ---" << std::endl;

        TRACK *track = static_cast<TRACK *>(calloc(1, sizeof(TRACK)));
        boost::filesystem::path p(track_base_path);
        std::string track_name = p.filename().string();
        stringstream trk_path, col_path;

        trk_path << track_base_path <<  "/" << track_name << ".TRK";
        col_path << track_base_path <<  "/" << track_name << ".col";

        // WSL Linux Path: "/mnt/c/Users/Amrik/Development/OpenNFS3/resources/NFS2/tr00/TR00.TRK"
        if(LoadTRK(trk_path.str(), track)){
            LoadCOL(col_path.str(), track); // Load Catalogue file to get global (non trkblock specific) data
        };

        dbgPrintVerts(track, "C:/Users/Amrik/Desktop/Tr02b/");

        return track;
    }

    bool LoadTRK(std::string trk_path, TRACK *track) {
        std::cout << "- Parsing TRK File " << std::endl;
        ifstream trk(trk_path, ios::in | ios::binary);
        // TRK file header data
        unsigned char header[4];
        long unknownHeader[5];

        // Check we're in a valid TRK file
        if (trk.read(((char *) header), sizeof(unsigned char) * 4).gcount() != sizeof(unsigned char) * 4) {
            std::cout << trk_path << std::endl;
            return false;
        }
        // Header should contain TRAC
        if (memcmp(header, "TRAC", sizeof(header)) != 0){
            std::cout << "Invalid TRK file." << std::endl;
            return false;
        }

        // Unknown header data
        if (trk.read(((char *) unknownHeader), sizeof(uint32_t) * 5).gcount() != sizeof(uint32_t) * 5) return false;

        // Basic Track data
        trk.read((char *) &track->nSuperBlocks, sizeof(uint32_t));
        trk.read((char *) &track->nBlocks, sizeof(uint32_t));
        track->superblocks = static_cast<SUPERBLOCK *>(calloc(track->nBlocks, sizeof(SUPERBLOCK)));

        // Offsets of Superblocks in TRK file
        uint32_t *superblockOffsets = static_cast<uint32_t *>(calloc(track->nSuperBlocks, sizeof(uint32_t)));
        if (trk.read(((char *) superblockOffsets), track->nSuperBlocks * sizeof(uint32_t)).gcount() != track->nSuperBlocks * sizeof(uint32_t)){
            free(superblockOffsets);
            return false;
        }

        // Reference coordinates for each block
        track->blockReferenceCoords = static_cast<VERT_HIGHP *>(calloc(track->nBlocks, sizeof(VERT_HIGHP)));
        if (trk.read((char *)  track->blockReferenceCoords, track->nBlocks * sizeof(VERT_HIGHP)).gcount() != track->nBlocks * sizeof(VERT_HIGHP)){
            free(superblockOffsets);
            return false;
        }

        for (int superBlock_Idx = 0; superBlock_Idx < track->nSuperBlocks; ++superBlock_Idx) {
            std::cout << "SuperBlock " << superBlock_Idx+1 << " of " << track->nSuperBlocks << std::endl;
            // Get the superblock header
            SUPERBLOCK *superblock = &track->superblocks[superBlock_Idx];
            trk.seekg(superblockOffsets[superBlock_Idx], ios_base::beg);
            trk.read((char *) &superblock->superBlockSize, sizeof(uint32_t));
            trk.read((char *) &superblock->nBlocks, sizeof(uint32_t));
            trk.read((char *) &superblock->padding, sizeof(uint32_t));

            if (superblock->nBlocks != 0) {
                // Get the offsets of the child blocks within superblock
                uint32_t *blockOffsets = (uint32_t *) calloc(static_cast<size_t>(superblock->nBlocks), sizeof(uint32_t));
                trk.read((char *) blockOffsets, superblock->nBlocks * sizeof(uint32_t));
                superblock->trackBlocks = static_cast<TRKBLOCK *>(calloc(static_cast<size_t>(superblock->nBlocks), sizeof(TRKBLOCK)));

                for (int block_Idx = 0; block_Idx < superblock->nBlocks; ++block_Idx) {
                    std::cout << "  Block " << block_Idx+1 << " of " << superblock->nBlocks << std::endl;
                    TRKBLOCK *trackblock = &superblock->trackBlocks[block_Idx];
                    // Read Header
                    trackblock->header = static_cast<TRKBLOCK_HEADER *>(calloc(1, sizeof(TRKBLOCK_HEADER)));
                    trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx], ios_base::beg);
                    trk.read((char *) trackblock->header, sizeof(TRKBLOCK_HEADER));

                    // Sanity Checks
                    if((trackblock->header->blockSize != trackblock->header->blockSizeDup)||(trackblock->header->blockSerial > track->nBlocks)){
                        std::cout<< "   --- Bad Block" << std::endl;
                        free(superblockOffsets);
                        return false;
                    }

                    // Read 3D Data
                    trackblock->vertexTable = static_cast<VERT *>(calloc(static_cast<size_t>(trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert), sizeof(VERT)));
                    for(unsigned int vert_Idx = 0; vert_Idx < trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert; ++vert_Idx){
                        trk.read((char *) &trackblock->vertexTable[vert_Idx], sizeof(VERT));
                    }

                    trackblock->polygonTable =static_cast<POLYGONDATA *>(calloc(static_cast<size_t>(trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly), sizeof(POLYGONDATA)));
                    for(unsigned int poly_Idx = 0; poly_Idx < (trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly); ++poly_Idx){
                        trk.read((char *) &trackblock->polygonTable[poly_Idx], sizeof(POLYGONDATA));
                    }

                    // Read Extrablock data
                    trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + 64u + trackblock->header->extraBlockTblOffset, ios_base::beg);
                    // Get extrablock offsets (relative to beginning of TrackBlock)
                    uint32_t *extrablockOffsets = (uint32_t *) calloc(trackblock->header->nExtraBlocks, sizeof(uint32_t));
                    trk.read((char *) extrablockOffsets, trackblock->header->nExtraBlocks * sizeof(uint32_t));

                    for(int xblock_Idx = 0; xblock_Idx < trackblock->header->nExtraBlocks; ++xblock_Idx){
                        trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + extrablockOffsets[xblock_Idx], ios_base::beg);
                        EXTRABLOCK_HEADER *xblockHeader = static_cast<EXTRABLOCK_HEADER *>(calloc(1, sizeof(EXTRABLOCK_HEADER)));
                        trk.read((char*) xblockHeader, sizeof(EXTRABLOCK_HEADER));

                        switch(xblockHeader->XBID){
                            case 5:
                                trackblock->polyTypes = static_cast<POLY_TYPE *>(calloc(xblockHeader->nRecords, sizeof(POLY_TYPE)));
                                trk.read((char *) trackblock->polyTypes, xblockHeader->nRecords * sizeof(POLY_TYPE));
                                break;
                            case 4:
                                trackblock->nNeighbours = xblockHeader->nRecords;
                                trackblock->blockNeighbours = (uint16_t *) calloc(xblockHeader->nRecords, sizeof(uint16_t));
                                trk.read((char*) trackblock->blockNeighbours, xblockHeader->nRecords * sizeof(uint16_t));
                                break;
                            case 8:
                                trackblock->structures = static_cast<GEOM_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(GEOM_BLOCK)));
                                trackblock->nStructures = xblockHeader->nRecords;
                                for(int structure_Idx = 0; structure_Idx < trackblock->nStructures; ++structure_Idx){
                                    streamoff padCheck = trk.tellg();
                                    trk.read((char*) &trackblock->structures[structure_Idx].recSize, sizeof(uint32_t));
                                    trk.read((char*) &trackblock->structures[structure_Idx].nVerts, sizeof(uint16_t));
                                    trk.read((char*) &trackblock->structures[structure_Idx].nPoly, sizeof(uint16_t));
                                    trackblock->structures[structure_Idx].vertexTable = static_cast<VERT *>(calloc(trackblock->structures[structure_Idx].nVerts, sizeof(VERT)));
                                    for(int vert_Idx = 0; vert_Idx < trackblock->structures[structure_Idx].nVerts; ++vert_Idx){
                                        trk.read((char *) &trackblock->structures[structure_Idx].vertexTable[vert_Idx], sizeof(VERT));
                                    }
                                    trackblock->structures[structure_Idx].polygonTable = static_cast<POLYGONDATA *>(calloc(trackblock->structures[structure_Idx].nPoly, sizeof(POLYGONDATA)));
                                    for(int poly_Idx = 0; poly_Idx < trackblock->structures[structure_Idx].nPoly; ++poly_Idx){
                                        trk.read((char *) &trackblock->structures[structure_Idx].polygonTable[poly_Idx], sizeof(POLYGONDATA));
                                    }
                                    trk.seekg(trackblock->structures[structure_Idx].recSize - (trk.tellg() - padCheck), ios_base::cur); // Eat possible padding
                                }
                                break;
                            case 7:
                            case 18:
                                trackblock->structureRefData = static_cast<GEOM_REF_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(GEOM_REF_BLOCK)));
                                trackblock->nStructureReferences = xblockHeader->nRecords;
                                for(int structureRef_Idx = 0; structureRef_Idx < trackblock->nStructureReferences; ++structureRef_Idx){
                                    streamoff padCheck = trk.tellg();
                                    trk.read((char*) &trackblock->structureRefData[structureRef_Idx].recSize, sizeof(uint16_t));
                                    trk.read((char*) &trackblock->structureRefData[structureRef_Idx].recType, sizeof(uint8_t));
                                    trk.read((char*) &trackblock->structureRefData[structureRef_Idx].structureRef, sizeof(uint8_t));
                                    // Fixed type
                                    if(trackblock->structureRefData[structureRef_Idx].recType == 1){
                                        trk.read((char*) &trackblock->structureRefData[structureRef_Idx].refCoordinates, sizeof(VERT_HIGHP));
                                    }
                                    else if(trackblock->structureRefData[structureRef_Idx].recType == 3){ // Animated type
                                        trk.read((char*) &trackblock->structureRefData[structureRef_Idx].animLength, sizeof(uint16_t));
                                        trk.read((char*) &trackblock->structureRefData[structureRef_Idx].unknown, sizeof(uint16_t));
                                        trackblock->structureRefData[structureRef_Idx].animationData = static_cast<ANIM_POS *>(calloc(trackblock->structureRefData[structureRef_Idx].animLength, sizeof(ANIM_POS)));
                                        for(int animation_Idx = 0; animation_Idx < trackblock->structureRefData[structureRef_Idx].animLength; ++animation_Idx){
                                            trk.read((char*) &trackblock->structureRefData[structureRef_Idx].animationData[animation_Idx], sizeof(ANIM_POS));
                                        }
                                    } else {
                                        std::cout << "Unknown Structure Reference type: " << (int) trackblock->structureRefData[structureRef_Idx].recType << std::endl;
                                    }
                                    trk.seekg(trackblock->structureRefData[structureRef_Idx].recSize - (trk.tellg() - padCheck), ios_base::cur); // Eat possible padding
                                }
                                break;
                            case 6:
                                trackblock->medianData = static_cast<MEDIAN_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(MEDIAN_BLOCK)));
                                trk.read((char *) trackblock->medianData, xblockHeader->nRecords * sizeof(MEDIAN_BLOCK));
                                break;
                            case 13:
                                trackblock->nVroad = xblockHeader->nRecords;
                                trackblock->vroadData = static_cast<VROAD *>(calloc(xblockHeader->nRecords, sizeof(VROAD)));
                                trk.read((char *) trackblock->vroadData, trackblock->nVroad * sizeof(VROAD));
                                break;
                            case 9:
                                trackblock->nLanes = xblockHeader->nRecords;
                                trackblock->laneData = static_cast<LANE_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(LANE_BLOCK)));
                                trk.read((char *) trackblock->laneData, trackblock->nLanes * sizeof(LANE_BLOCK));
                                break;
                            default:
                                std::cout << "Unknown XBID: " << xblockHeader->XBID << std::endl;
                                break;
                        }
                        free(xblockHeader);
                    }
                    free(extrablockOffsets);
                }
                free(blockOffsets);
            }
        }
        free(superblockOffsets);
        trk.close();
        return true;
    }

    bool LoadCOL(std::string col_path, TRACK *track) {
        std::cout << "- Parsing COL File " << std::endl;
        ifstream col(col_path, ios::in | ios::binary);
        // Check we're in a valid TRK file
        unsigned char header[4];
        if (col.read(((char *) header), sizeof(unsigned char) * 4).gcount() != sizeof(unsigned char) * 4) return false;
        if (memcmp(header, "COLL", sizeof(header)) != 0) return false;

        uint32_t version;
        col.read((char *) &version, sizeof(uint32_t));
        if (version != 11) return false;

        uint32_t colSize;
        col.read((char *) &colSize, sizeof(uint32_t));

        uint32_t nExtraBlocks;
        col.read((char*) &nExtraBlocks, sizeof(uint32_t));

        uint32_t *extraBlockOffsets = (uint32_t *) calloc(nExtraBlocks, sizeof(uint32_t));
        col.read((char*) extraBlockOffsets, nExtraBlocks*sizeof(uint32_t));

        std::cout << "  Version: " << version << "\n  nExtraBlocks: " << nExtraBlocks << "\nParsing COL Extrablocks" << std::endl;

        for(int xBlock_Idx = 0; xBlock_Idx < nExtraBlocks; ++xBlock_Idx) {
            col.seekg(16 + extraBlockOffsets[xBlock_Idx], ios_base::beg);

            EXTRABLOCK_HEADER *xblockHeader = static_cast<EXTRABLOCK_HEADER *>(calloc(1, sizeof(EXTRABLOCK_HEADER)));
            col.read((char*) xblockHeader, sizeof(EXTRABLOCK_HEADER));

            std::cout << "  XBID " << (int) xblockHeader->XBID << " (XBlock " << xBlock_Idx + 1 << " of " << nExtraBlocks << ")" << std::endl;

            switch (xblockHeader->XBID) {
                case 2: // First xbock always texture table
                    track->nTextures = xblockHeader->nRecords;
                    track->polyToQFStexTable = static_cast<TEXTURE_BLOCK *>(calloc(track->nTextures, sizeof(TEXTURE_BLOCK)));
                    col.read((char *) track->polyToQFStexTable, track->nTextures * sizeof(TEXTURE_BLOCK));
                    break;
                case 8: // XBID 8 3D Structure data: This block is only present if nExtraBlocks != 2
                        track->nColStructures = xblockHeader->nRecords;
                        track->colStructures = static_cast<GEOM_BLOCK *>(calloc(track->nColStructures, sizeof(GEOM_BLOCK)));
                        for(int structure_Idx = 0; structure_Idx < track->nColStructures; ++structure_Idx){
                            streamoff padCheck = col.tellg();
                            col.read((char*) &track->colStructures[structure_Idx].recSize, sizeof(uint32_t));
                            col.read((char*) &track->colStructures[structure_Idx].nVerts, sizeof(uint16_t));
                            col.read((char*) &track->colStructures[structure_Idx].nPoly, sizeof(uint16_t));
                            track->colStructures[structure_Idx].vertexTable = static_cast<VERT *>(calloc(track->colStructures[structure_Idx].nVerts, sizeof(VERT)));
                            for(int vert_Idx = 0; vert_Idx < track->colStructures[structure_Idx].nVerts; ++vert_Idx){
                                col.read((char *) &track->colStructures[structure_Idx].vertexTable[vert_Idx], sizeof(VERT));
                            }
                            track->colStructures[structure_Idx].polygonTable = static_cast<POLYGONDATA *>(calloc(track->colStructures[structure_Idx].nPoly, sizeof(POLYGONDATA)));
                            for(int poly_Idx = 0; poly_Idx < track->colStructures[structure_Idx].nPoly; ++poly_Idx){
                                col.read((char *) &track->colStructures[structure_Idx].polygonTable[poly_Idx], sizeof(POLYGONDATA));
                            }
                            col.seekg(track->colStructures[structure_Idx].recSize - (col.tellg() - padCheck), ios_base::cur); // Eat possible padding
                        }
                    break;
                case 7: // XBID 7 3D Structure Reference: This block is only present if nExtraBlocks != 2
                        track->nColStructureReferences = xblockHeader->nRecords;
                        track->colStructureRefData = static_cast<GEOM_REF_BLOCK *>(calloc(track->nColStructureReferences, sizeof(GEOM_REF_BLOCK)));
                        for(int structureRef_Idx = 0; structureRef_Idx < track->nColStructures; ++structureRef_Idx){
                            streamoff padCheck = col.tellg();
                            col.read((char*) &track->colStructureRefData[structureRef_Idx].recSize, sizeof(uint16_t));
                            col.read((char*) &track->colStructureRefData[structureRef_Idx].recType, sizeof(uint8_t));
                            col.read((char*) &track->colStructureRefData[structureRef_Idx].structureRef, sizeof(uint8_t));
                            // Fixed type
                            if(track->colStructureRefData[structureRef_Idx].recType == 1){
                                col.read((char*) &track->colStructureRefData[structureRef_Idx].refCoordinates, sizeof(VERT_HIGHP));
                            }
                            else if(track->colStructureRefData[structureRef_Idx].recType == 3){ // Animated type
                                col.read((char*) &track->colStructureRefData[structureRef_Idx].animLength, sizeof(uint16_t));
                                col.read((char*) &track->colStructureRefData[structureRef_Idx].unknown, sizeof(uint16_t));
                                track->colStructureRefData[structureRef_Idx].animationData = static_cast<ANIM_POS *>(calloc(track->colStructureRefData[structureRef_Idx].animLength, sizeof(ANIM_POS)));
                                for(int animation_Idx = 0; animation_Idx < track->colStructureRefData[structureRef_Idx].animLength; ++animation_Idx){
                                    col.read((char*) &track->colStructureRefData[structureRef_Idx].animationData[animation_Idx], sizeof(ANIM_POS));
                                }
                            } else {
                                std::cout << "Unknown Structure Reference type: " << (int) track->colStructureRefData[structureRef_Idx].recType << std::endl;
                            }
                            col.seekg(track->colStructureRefData[structureRef_Idx].recSize - (col.tellg() - padCheck), ios_base::cur); // Eat possible padding
                        }
                    break;
                case 15:
                    track->nCollisionData  = xblockHeader->nRecords;
                    track->collisionData = static_cast<COLLISION_BLOCK *>(calloc(track->nCollisionData, sizeof(COLLISION_BLOCK)));
                    col.read((char*) track->collisionData, track->nCollisionData * sizeof(COLLISION_BLOCK));
                    break;
                default:break;
            }
        }
        col.close();
        return true;
    }

    void dbgPrintVerts(TRACK *track, const std::string &path) {
        std::ofstream obj_dump;

        float scaleFactor = 100000;

        // Parse out TRKBlock data
        for(int superBlock_Idx = 0; superBlock_Idx < track->nSuperBlocks; ++superBlock_Idx){
            SUPERBLOCK *superblock = &track->superblocks[superBlock_Idx];
            for (int block_Idx = 0; block_Idx < superblock->nBlocks; ++block_Idx) {
                TRKBLOCK trkBlock = superblock->trackBlocks[block_Idx];
                VERT_HIGHP blockReferenceCoord;
                // Print clipping rectangle
                //obj_dump << "o Block" << trkBlock.header->blockSerial << "ClippingRect" << std::endl;
                //for(int i = 0; i < 4; i++){
                //    obj_dump << "v " << trkBlock.header->clippingRect[i].x << " " << trkBlock.header->clippingRect[i].z << " " << trkBlock.header->clippingRect[i].y << std::endl;
                //}
                // obj_dump << "f " << 1+(4*trkBlock.header->blockSerial) << " " << 2+(4*trkBlock.header->blockSerial) << " " << 3+(4*trkBlock.header->blockSerial) << " " << 4+(4*trkBlock.header->blockSerial) << std::endl;
                std::ostringstream stringStream;
                stringStream << path << "TrackBlock" << trkBlock.header->blockSerial << ".obj";
                obj_dump.open(stringStream.str());
                obj_dump << "o TrackBlock" << trkBlock.header->blockSerial << std::endl;
                for (int i = 0; i < trkBlock.header->nStickToNextVerts + trkBlock.header->nHighResVert; i++) {
                    if (i < trkBlock.header->nStickToNextVerts) {
                        // If in last block go get ref coord of first block, else get ref of next block
                        blockReferenceCoord =  (trkBlock.header->blockSerial == track->nBlocks-1) ? track->blockReferenceCoords[0] :  track->blockReferenceCoords[trkBlock.header->blockSerial+1];
                    } else {
                        blockReferenceCoord = track->blockReferenceCoords[trkBlock.header->blockSerial];
                    }
                    int32_t x = (blockReferenceCoord.x + (256 * trkBlock.vertexTable[i].x));
                    int32_t y = (blockReferenceCoord.y + (256 * trkBlock.vertexTable[i].y));
                    int32_t z = (blockReferenceCoord.z + (256 * trkBlock.vertexTable[i].z));
                    obj_dump << "v " << x/scaleFactor << " " << z/scaleFactor << " " << y/scaleFactor << std::endl;
                }
                for (int poly_Idx = (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly); poly_Idx < (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly + trkBlock.header->nHighResPoly); ++poly_Idx)
                {
                    obj_dump << "f " << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[0]+1 << " " << (unsigned int)trkBlock.polygonTable[poly_Idx].vertex[1]+1 << " " << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[2]+1 << " " << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[3]+1<< std::endl;
                }
                obj_dump.close();
                for(int structure_Idx = 0; structure_Idx < trkBlock.nStructures; ++structure_Idx){
                    std::ostringstream stringStream1;
                    stringStream1 << path << "SB" << superBlock_Idx << "TB" << block_Idx << "S" << structure_Idx << ".obj";
                    obj_dump.open(stringStream1.str());
                    VERT_HIGHP *structureReferenceCoordinates = &track->blockReferenceCoords[trkBlock.header->blockSerial];
                    // Find the structure reference that matches this structure, else use block default
                    for(int structRef_Idx = 0; structRef_Idx < trkBlock.nStructureReferences; ++structRef_Idx){
                        // Only check fixed type structure references
                        if(trkBlock.structureRefData[structRef_Idx].structureRef == structure_Idx){
                            if(trkBlock.structureRefData[structRef_Idx].recType == 1){
                                structureReferenceCoordinates = &trkBlock.structureRefData[structure_Idx].refCoordinates;
                            }
                            else if(trkBlock.structureRefData[structRef_Idx].recType == 3) {
                                if(trkBlock.structureRefData[structure_Idx].animLength != 0){
                                    // For now, if animated, use position 0 of animation sequence
                                    structureReferenceCoordinates = &trkBlock.structureRefData[structure_Idx].animationData[0].position;
                                }
                            }
                        }
                    }
                    obj_dump << "o Struct" << &trkBlock.structures[structure_Idx] << std::endl;
                    for(uint16_t vert_Idx = 0; vert_Idx < trkBlock.structures[structure_Idx].nVerts; ++vert_Idx){
                        int32_t x = (structureReferenceCoordinates->x + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].x));
                        int32_t y = (structureReferenceCoordinates->y + (256 *trkBlock.structures[structure_Idx].vertexTable[vert_Idx].y));
                        int32_t z = (structureReferenceCoordinates->z + (256 *trkBlock.structures[structure_Idx].vertexTable[vert_Idx].z));
                        obj_dump << "v " << x/scaleFactor << " " << z/scaleFactor << " " << y/scaleFactor << std::endl;
                    }
                    for(int poly_Idx = 0; poly_Idx < trkBlock.structures[structure_Idx].nPoly; ++poly_Idx){
                        obj_dump << "f " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[0]+1 << " " << (unsigned int)trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[1]+1 << " " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[2]+1 << " " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[3]+1 << std::endl;
                    }
                    obj_dump.close();
                }
            }
        }

        // Parse out COL data
        for(int structure_Idx = 0; structure_Idx < track->nColStructures; ++structure_Idx){
            std::ostringstream stringStream1;
            stringStream1 << path << "COL" << structure_Idx << ".obj";
            obj_dump.open(stringStream1.str());
            VERT_HIGHP *structureReferenceCoordinates = static_cast<VERT_HIGHP *>(calloc(1, sizeof(VERT_HIGHP)));
            // Find the structure reference that matches this structure, else use block default
            for(int structRef_Idx = 0; structRef_Idx < track->nColStructureReferences; ++structRef_Idx){
                // Only check fixed type structure references
                if(track->colStructureRefData[structRef_Idx].structureRef == structure_Idx){
                    if(track->colStructureRefData[structRef_Idx].recType == 1){
                        structureReferenceCoordinates = &track->colStructureRefData[structure_Idx].refCoordinates;
                    }
                    else if(track->colStructureRefData[structRef_Idx].recType == 3) {
                        if(track->colStructureRefData[structure_Idx].animLength != 0){
                            // For now, if animated, use position 0 of animation sequence
                            structureReferenceCoordinates = &track->colStructureRefData[structure_Idx].animationData[0].position;
                        }
                    }
                }
            }
            obj_dump << "o ColStruct" << &track->colStructures[structure_Idx] << std::endl;
            for(uint16_t vert_Idx = 0; vert_Idx < track->colStructures[structure_Idx].nVerts; ++vert_Idx){
                int32_t x = (structureReferenceCoordinates->x + (256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].x));
                int32_t y = (structureReferenceCoordinates->y + (256 *track->colStructures[structure_Idx].vertexTable[vert_Idx].y));
                int32_t z = (structureReferenceCoordinates->z + (256 *track->colStructures[structure_Idx].vertexTable[vert_Idx].z));
                obj_dump << "v " << x/scaleFactor << " " << z/scaleFactor << " " << y/scaleFactor << std::endl;
            }
            for(int poly_Idx = 0; poly_Idx < track->colStructures[structure_Idx].nPoly; ++poly_Idx){
                obj_dump << "f " << (unsigned int) track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[0]+1 << " " << (unsigned int)track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[1]+1 << " " << (unsigned int) track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[2]+1 << " " << (unsigned int) track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[3]+1 << std::endl;
            }
            obj_dump.close();
        }
    }
}
