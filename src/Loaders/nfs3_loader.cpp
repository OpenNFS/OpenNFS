//
// Created by Amrik.Sadhra on 20/06/2018.
//

#include "nfs3_loader.h"

// CAR

NFS3_Loader::NFS3_Loader(const std::string &car_base_path, std::string *car_name){
    boost::filesystem::path p(car_base_path);
    *car_name = p.filename().string();

    std::stringstream viv_path, car_out_path, fce_path;
    viv_path << car_base_path << "/car.viv";
    car_out_path << CAR_PATH << *car_name << "/";
    fce_path << CAR_PATH << *car_name << "/car.fce";

    ASSERT(ExtractVIV(viv_path.str(), car_out_path.str()), "Unable to extract " << viv_path.str() << " to " << car_out_path.str());
    ASSERT(LoadFCE(fce_path.str()), "Unable to load " << fce_path.str());
}

bool NFS3_Loader::LoadFCE(const std::string fce_path) {
    std::cout << "- Parsing FCE File: "  << fce_path <<  std::endl;
    ifstream fce(fce_path, ios::in | ios::binary);

    auto *fceHeader = new FCE::HEADER();
    fce.read((char*) fceHeader, sizeof(FCE::HEADER));

    for(int part_Idx = 0; part_Idx < fceHeader->nParts; ++part_Idx){
        float specularDamper = 0.2;
        float specularReflectivity = 0.02;
        float envReflectivity = 0.4;

        std::vector<uint32_t> indices;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::string part_name(fceHeader->partNames[part_Idx]);
        glm::vec3 center(fceHeader->partCoords[part_Idx].x, fceHeader->partCoords[part_Idx].y, fceHeader->partCoords[part_Idx].z);
        center /= 10;

        auto *partVertices = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partNormals = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partTriangles = new FCE::TRIANGLE[fceHeader->partNumTriangles[part_Idx]];

        fce.seekg(sizeof(FCE::HEADER)+fceHeader->vertTblOffset+(fceHeader->partFirstVertIndices[part_Idx]*sizeof(FLOATPT)), ios_base::beg);
        fce.read((char*) partVertices, fceHeader->partNumVertices[part_Idx]*sizeof(FLOATPT));
        for(int vert_Idx = 0; vert_Idx < fceHeader->partNumVertices[part_Idx]; ++vert_Idx){
            vertices.emplace_back(glm::vec3(partVertices[vert_Idx].x, partVertices[vert_Idx].y, partVertices[vert_Idx].z));
        }

        fce.seekg(sizeof(FCE::HEADER)+fceHeader->normTblOffset+(fceHeader->partFirstVertIndices[part_Idx]*sizeof(FLOATPT)), ios_base::beg);
        fce.read((char*) partNormals, fceHeader->partNumVertices[part_Idx]*sizeof(FLOATPT));
        for(int normal_Idx = 0; normal_Idx < fceHeader->partNumVertices[part_Idx]; ++normal_Idx){
            normals.emplace_back(glm::vec3(partNormals[normal_Idx].x, partNormals[normal_Idx].y, partNormals[normal_Idx].z));
        }

        fce.seekg(sizeof(FCE::HEADER)+fceHeader->triTblOffset+(fceHeader->partFirstTriIndices[part_Idx]*sizeof(FCE::TRIANGLE)), ios_base::beg);
        fce.read((char*) partTriangles, fceHeader->partNumTriangles[part_Idx]*sizeof(FCE::TRIANGLE));
        for(int tri_Idx = 0; tri_Idx < fceHeader->partNumTriangles[part_Idx]; ++tri_Idx){
            indices.emplace_back(partTriangles[tri_Idx].vertex[0]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[1]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[2]);
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[0], partTriangles[tri_Idx].uvTable[3]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[1], partTriangles[tri_Idx].uvTable[4]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[2], partTriangles[tri_Idx].uvTable[5]));
        }

        meshes.emplace_back(CarModel(part_name, part_Idx, vertices, uvs, normals, indices, center, specularDamper, specularReflectivity, envReflectivity));
        std::cout << "Mesh: " << meshes[part_Idx].m_name << " UVs: " << meshes[part_Idx].m_uvs.size() << " Verts: "
                  << meshes[part_Idx].m_vertices.size() << " Indices: " << meshes[part_Idx].m_vertex_indices.size() << " Normals: "
                  << meshes[part_Idx].m_normals.size() << std::endl;
        meshes[part_Idx].enable();

        delete[] partNormals;
        delete[] partVertices;
        delete[] partTriangles;
    }

    fce.close();

    delete fceHeader;
    return true;
}

void ConvertFCE(const std::string &fce_path, const std::string &obj_out_path) {
    std::string car_name;
    NFS3_Loader fce_reader(fce_path, &car_name);
    fce_reader.WriteObj(obj_out_path);
}

void NFS3_Loader::WriteObj(const std::string &path) {
    std::cout << "Writing Meshes to " << path << std::endl;

    std::ofstream obj_dump;
    obj_dump.open(path);

    for (Model &mesh : meshes) {
        /* Print Part name*/
        obj_dump << "o " << mesh.m_name << std::endl;
        //Dump Vertices
        for (auto vertex : mesh.m_vertices) {
            obj_dump << "v " << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
        }
        //Dump UVs
        for (auto uv : mesh.m_uvs) {
            obj_dump << "vt " << uv[0] << " " << uv[1] << std::endl;
        }
        //Dump Indices
        for (auto vert_index : mesh.m_vertex_indices) {
            obj_dump << "f " << vert_index << std::endl;
        }
    }
    obj_dump.close();
}

// TRACK

NFS3_Loader::NFS3_Loader(const std::string &track_base_path) {
    std::cout << "--- Loading NFS3 Track ---" << std::endl;

    boost::filesystem::path p(track_base_path);
    std::string track_name = p.filename().string();
    stringstream frd_path, col_path;
    string strip = "K0";
    unsigned int pos= track_name.find(strip);
    if(pos!= string::npos)
        track_name.replace(pos, strip.size(), "");

    frd_path << track_base_path << ".frd";
    col_path << track_base_path << ".col";

    ASSERT(ExtractTrackTextures(track_base_path, track_name, NFSVer::NFS_3), "Could not extract " << track_name << " QFS texture pack.");
    ASSERT(LoadFRD(frd_path.str(), track_name), "Could not load FRD file: " << frd_path.str()); // Load FRD file to get track block specific data
    ASSERT(LoadCOL(col_path.str()), "Could not load COL file: " << col_path.str()); // Load Catalogue file to get global (non trkblock specific) data

    track->texture_gl_mappings = GenTrackTextures(track->textures);
    track->track_blocks = ParseTRKModels();
    std::vector<Track> col_models = ParseCOLModels();
    track->track_blocks[0].objects.insert(track->track_blocks[0].objects.end(), col_models.begin(), col_models.end()); // Insert the COL models into track block 0 for now

    std::cout << "Successful track load!" << std::endl;
}

bool NFS3_Loader::LoadFRD(std::string frd_path, const std::string &track_name) {
    ifstream ar(frd_path, ios::in | ios::binary);

    char header[28]; /* file header */
    SAFE_READ(ar, header, 28); // header & numblocks
    SAFE_READ(ar, &track->nBlocks, 4);
    track->nBlocks++;
    if ((track->nBlocks < 1) || (track->nBlocks > 500)) return false; // 1st sanity check

    track->trk = static_cast<TRKBLOCK *>(calloc(track->nBlocks, sizeof(TRKBLOCK)));
    track->poly = static_cast<POLYGONBLOCK *>(calloc(track->nBlocks, sizeof(POLYGONBLOCK)));
    track->xobj = static_cast<XOBJBLOCK *>(calloc((4 * track->nBlocks + 1), sizeof(XOBJBLOCK)));

    int l;
    SAFE_READ(ar, &l, 4); // choose between NFS3 & NFSHS
    if ((l < 0) || (l > 5000)) track->bHSMode = false;
    else if (((l + 7) / 8) == track->nBlocks) track->bHSMode = true;
    else return false; // unknown file type

    memcpy(track->trk, &l, 4);
    if (ar.read(((char *) track->trk) + 4, 80).gcount() != 80) return false;

    // TRKBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
        TRKBLOCK *trackBlock = &(track->trk[block_Idx]);
        // ptCentre, ptBounding, 6 nVertices == 84 bytes
        if (block_Idx != 0) { SAFE_READ(ar, trackBlock, 84); }
        if ((trackBlock->nVertices == 0)) return false;
        trackBlock->vert = static_cast<FLOATPT *>(calloc(trackBlock->nVertices, sizeof(FLOATPT)));

        SAFE_READ(ar, trackBlock->vert, 12 * trackBlock->nVertices);
        trackBlock->unknVertices = static_cast<uint32_t *>(calloc(trackBlock->nVertices, sizeof(uint32_t)));
        SAFE_READ(ar, trackBlock->unknVertices, 4 * trackBlock->nVertices);
        SAFE_READ(ar, trackBlock->nbdData, 4 * 0x12c);
        // nStartPos & various blk sizes == 32 bytes
        SAFE_READ(ar, &(trackBlock->nStartPos), 32);

        if (block_Idx > 0)
            if (trackBlock->nStartPos !=
                track->trk[block_Idx - 1].nStartPos + track->trk[block_Idx - 1].nPositions)
                return false;
        trackBlock->posData = static_cast<POSITIONDATA *>(calloc(trackBlock->nPositions, sizeof(POSITIONDATA)));
        SAFE_READ(ar, trackBlock->posData, 8 * trackBlock->nPositions);

        trackBlock->polyData = static_cast<POLYVROADDATA *>(calloc(trackBlock->nPolygons, sizeof(POLYVROADDATA)));

        for (int j = 0; j < trackBlock->nPolygons; j++)
            SAFE_READ(ar, trackBlock->polyData + j, 8);

        trackBlock->vroadData = static_cast<VROADDATA *>(calloc(trackBlock->nVRoad, sizeof(VROADDATA)));
        SAFE_READ(ar, trackBlock->vroadData, 12 * trackBlock->nVRoad);

        if (trackBlock->nXobj > 0) {
            trackBlock->xobj = static_cast<REFXOBJ *>(calloc(trackBlock->nXobj, sizeof(REFXOBJ)));
            SAFE_READ(ar, trackBlock->xobj, 20 * trackBlock->nXobj);
        }
        if (trackBlock->nPolyobj > 0) {
            ar.seekg(20 * trackBlock->nPolyobj, ios_base::cur);
        }
        trackBlock->nPolyobj = 0;
        if (trackBlock->nSoundsrc > 0) {
            trackBlock->soundsrc = static_cast<SOUNDSRC *>(calloc(trackBlock->nSoundsrc, sizeof(SOUNDSRC)));
            SAFE_READ(ar, trackBlock->soundsrc, 16 * trackBlock->nSoundsrc);
        }
        if (trackBlock->nLightsrc > 0) {
            trackBlock->lightsrc = static_cast<LIGHTSRC *>(calloc(trackBlock->nLightsrc, sizeof(LIGHTSRC)));
            SAFE_READ(ar, trackBlock->lightsrc, 16 * trackBlock->nLightsrc);
        }
    }

    // POLYGONBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
        POLYGONBLOCK *p = &(track->poly[block_Idx]);
        for (int j = 0; j < 7; j++) {
            SAFE_READ(ar, &(p->sz[j]), 0x4);
            if (p->sz[j] != 0) {
                SAFE_READ(ar, &(p->szdup[j]), 0x4);
                if (p->szdup[j] != p->sz[j]) return false;
                p->poly[j] = static_cast<LPPOLYGONDATA>(calloc(p->sz[j], sizeof(POLYGONDATA)));
                SAFE_READ(ar, p->poly[j], 14 * p->sz[j]);
            }
        }
        if (p->sz[4] != track->trk[block_Idx].nPolygons) return false; // sanity check
        for (int obj_Idx = 0; obj_Idx < 4; obj_Idx++) {
            OBJPOLYBLOCK *o = &(p->obj[obj_Idx]);
            SAFE_READ(ar, &(o->n1), 0x4);
            if (o->n1 > 0) {
                SAFE_READ(ar, &(o->n2), 0x4);
                o->types = static_cast<uint32_t *>(calloc(static_cast<size_t>(o->n2), sizeof(uint32_t)));
                o->numpoly = static_cast<uint32_t *>(malloc(o->n2 * sizeof(uint32_t)));
                o->poly = static_cast<LPPOLYGONDATA *>(calloc(static_cast<size_t>(o->n2), sizeof(LPPOLYGONDATA)));
                o->nobj = 0;
                l = 0;
                for (int k = 0; k < o->n2; k++) {
                    SAFE_READ(ar, o->types + k, 0x4);
                    if (o->types[k] == 1) {
                        SAFE_READ(ar, o->numpoly + o->nobj, 0x4);
                        o->poly[o->nobj] = static_cast<LPPOLYGONDATA>(calloc(o->numpoly[o->nobj],
                                                                             sizeof(POLYGONDATA)));
                        SAFE_READ(ar, o->poly[o->nobj], 14 * o->numpoly[o->nobj]);
                        l += o->numpoly[o->nobj];
                        o->nobj++;
                    }
                }
                if (l != o->n1) return false; // n1 == total nb polygons
            }
        }
    }

    // XOBJBLOCKs
    for (uint32_t xblock_Idx = 0; xblock_Idx <= 4 * track->nBlocks; xblock_Idx++) {
        SAFE_READ(ar, &(track->xobj[xblock_Idx].nobj), 4);
        if (track->xobj[xblock_Idx].nobj > 0) {
            track->xobj[xblock_Idx].obj = static_cast<XOBJDATA *>(calloc(track->xobj[xblock_Idx].nobj,
                                                                         sizeof(XOBJDATA)));
        }
        for (int xobj_Idx = 0; xobj_Idx < track->xobj[xblock_Idx].nobj; xobj_Idx++) {
            XOBJDATA *x = &(track->xobj[xblock_Idx].obj[xobj_Idx]);
            // 3 headers == 12 bytes
            SAFE_READ(ar, x, 12);
            if (x->crosstype == 4) { // basic objects
                SAFE_READ(ar, &(x->ptRef), 12);
                SAFE_READ(ar, &(x->AnimMemory), 4);
            } else if (x->crosstype == 3) { // animated objects
                // unkn3, type3, objno, nAnimLength, unkn4 == 24 bytes
                SAFE_READ(ar, x->unknown3, 24);
                if (x->type3 != 3) return false;
                x->animData = static_cast<ANIMDATA *>(calloc(20, x->nAnimLength));
                SAFE_READ(ar, x->animData, 20 * x->nAnimLength);
                // make a ref point from first anim position
                x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
            } else return false; // unknown object type

            // common part : vertices & polygons
            SAFE_READ(ar, &(x->nVertices), 4);
            x->vert = static_cast<FLOATPT *>(calloc(12, x->nVertices));
            SAFE_READ(ar, x->vert, 12 * x->nVertices);
            x->unknVertices = static_cast<uint32_t *>(calloc(4, x->nVertices));
            SAFE_READ(ar, x->unknVertices, 4 * x->nVertices);
            SAFE_READ(ar, &(x->nPolygons), 4);
            x->polyData = static_cast<POLYGONDATA *>(calloc(x->nPolygons, 14));
            SAFE_READ(ar, x->polyData, 14 * x->nPolygons);
        }
    }

    // TEXTUREBLOCKs
    SAFE_READ(ar, &track->nTextures, 4);
    track->texture = static_cast<TEXTUREBLOCK *>(calloc(track->nTextures, sizeof(TEXTUREBLOCK)));
    for (uint32_t tex_Idx = 0; tex_Idx < track->nTextures; tex_Idx++) {
        SAFE_READ(ar, &(track->texture[tex_Idx]), 47);
        track->textures[track->texture[tex_Idx].texture] = LoadTexture(track->texture[tex_Idx], track_name);
    }

    uint32_t pad;
    return ar.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
}

bool NFS3_Loader::LoadCOL(std::string col_path) {
    ifstream coll(col_path, ios::in | ios::binary);

    COLOBJECT *o;

    track->col.hs_extra = NULL;
    if (coll.read((char *) &track->col, 16).gcount() != 16) return false;
    if (memcmp(track->col.collID, "COLL", sizeof(track->col.collID[0])) != 0){
        std::cout << "Invalid COL file." << std::endl;
        return false;
    }

    if (track->col.version != 11) return false;
    if ((track->col.nBlocks != 2) && (track->col.nBlocks != 4) && (track->col.nBlocks != 5)) return false;
    SAFE_READ(coll, track->col.xbTable, 4 * track->col.nBlocks);

    // texture XB
    SAFE_READ(coll, &track->col.textureHead, 8);
    if (track->col.textureHead.xbid != XBID_TEXTUREINFO) return false;

    track->col.texture = static_cast<COLTEXTUREINFO *>(calloc(track->col.textureHead.nrec, sizeof(COLTEXTUREINFO)));
    SAFE_READ(coll, track->col.texture, 8 * track->col.textureHead.nrec);

    // struct3D XB
    if (track->col.nBlocks >= 4) {
        SAFE_READ(coll, &track->col.struct3DHead, 8);
        if (track->col.struct3DHead.xbid != XBID_STRUCT3D) return false;
        COLSTRUCT3D *s = track->col.struct3D = static_cast<COLSTRUCT3D *>(calloc(track->col.struct3DHead.nrec, sizeof(COLSTRUCT3D)));
        int delta;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.struct3DHead.nrec; colRec_Idx++, s++) {
            SAFE_READ(coll, s, 8);
            delta = (8 + 16 * s->nVert + 6 * s->nPoly) % 4;
            delta = (4 - delta) % 4;
            if (s->size != 8 + 16 * s->nVert + 6 * s->nPoly + delta) return false;
            s->vertex = static_cast<COLVERTEX *>(calloc(16, s->nVert));
            SAFE_READ(coll, s->vertex, 16 * s->nVert);
            s->polygon = static_cast<COLPOLYGON *>(calloc(6, s->nPoly));
            SAFE_READ(coll, s->polygon, 6*s->nPoly);
            int dummy;
            if (delta > 0) SAFE_READ(coll, &dummy, delta);
        }

        // object XB
        SAFE_READ(coll, &track->col.objectHead, 8);
        if ((track->col.objectHead.xbid != XBID_OBJECT) && (track->col.objectHead.xbid != XBID_OBJECT2)) return false;
        o = track->col.object = static_cast<COLOBJECT *>(calloc(track->col.objectHead.nrec, sizeof(COLOBJECT)));

        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.objectHead.nrec; colRec_Idx++, o++) {
            SAFE_READ(coll, o, 4);
            if (o->type == 1) {
                if (o->size != 16) return false;
                SAFE_READ(coll, &(o->ptRef), 12);
            } else if (o->type == 3) {
                SAFE_READ(coll, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength) return false;
                o->animData = static_cast<ANIMDATA *>(calloc(20, o->animLength));
                SAFE_READ(coll, o->animData, 20 * o->animLength);
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            } else return false; // unknown object type
        }
    }

    // object2 XB
    if (track->col.nBlocks == 5) {
        SAFE_READ(coll, &track->col.object2Head, 8);
        if ((track->col.object2Head.xbid != XBID_OBJECT) && (track->col.object2Head.xbid != XBID_OBJECT2)) return false;
        o = track->col.object2 = static_cast<COLOBJECT *>(calloc(track->col.object2Head.nrec, sizeof(COLOBJECT)));

        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.object2Head.nrec; colRec_Idx++, o++) {
            SAFE_READ(coll, o, 4);
            if (o->type == 1) {
                if (o->size != 16) return false;
                SAFE_READ(coll, &(o->ptRef), 12);
            } else if (o->type == 3) {
                SAFE_READ(coll, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength) return false;
                o->animData = static_cast<ANIMDATA *>(calloc(20, o->animLength));
                SAFE_READ(coll, o->animData, 20 * o->animLength);
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            } else return false; // unknown object type
        }
    }

    // vroad XB
    SAFE_READ(coll, &track->col.vroadHead, 8);
    if (track->col.vroadHead.xbid != XBID_VROAD) return false;
    if (track->col.vroadHead.size != 8 + 36 * track->col.vroadHead.nrec) return false;
    track->col.vroad = static_cast<COLVROAD *>(calloc(track->col.vroadHead.nrec, sizeof(COLVROAD)));
    SAFE_READ(coll, track->col.vroad, 36 * track->col.vroadHead.nrec);

    uint32_t pad;
    return coll.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
}

std::vector<TrackBlock> NFS3_Loader::ParseTRKModels() {
    std::vector<TrackBlock> track_blocks = std::vector<TrackBlock>();
    /* TRKBLOCKS - BASE TRACK GEOMETRY */
    for (int i = 0; i < track->nBlocks; i++) {
        // Get Verts from Trk block, indices from associated polygon block
        TRKBLOCK trk_block = track->trk[i];
        POLYGONBLOCK polygon_block = track->poly[i];
        TrackBlock current_track_block(i, glm::vec3(trk_block.ptCentre.x, trk_block.ptCentre.y, trk_block.ptCentre.z));
        glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
        glm::vec3 trk_block_center = orientation * glm::vec3(0, 0, 0);

        // Light sources
        for (int j = 0; j < trk_block.nLightsrc; j++) {
            // Light temp_light = Light(trk_block.lightsrc[j].refpoint, trk_block.lightsrc[j].type);
            //temp_light.enable();
            //current_track_block.lights.emplace_back(temp_light);
        }

        for(int s = 0; s < trk_block.nSoundsrc; s++){
            //Light temp_light = Light(trk_block.soundsrc[s].refpoint, trk_block.soundsrc[s].type);
            //temp_light.enable();
            //current_track_block.lights.emplace_back(temp_light);
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
                    glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f,
                              ((shading_data >> 8) & 0xFF) / 255.0f,
                              (shading_data & 0xFF) / 255.0f,
                              ((shading_data >> 24) & 0xFF) / 255.0f));
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
                        //norm_floatpt = VertexNormal(i, object_polys[p].vertex[0], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        //norm_floatpt = VertexNormal(i, object_polys[p].vertex[1], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        //norm_floatpt = VertexNormal(i, object_polys[p].vertex[2], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        //norm_floatpt = VertexNormal(i, object_polys[p].vertex[0], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        //norm_floatpt = VertexNormal(i, object_polys[p].vertex[2], track->trk, track->poly);
                        norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                        //norm_floatpt = VertexNormal(i, object_polys[p].vertex[3], track->trk, track->poly);
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
                    //norm_floatpt = VertexNormal(i, x->polyData->vertex[0], track->trk, track->poly);
                    norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, x->polyData->vertex[1], track->trk, track->poly);
                    norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, x->polyData->vertex[2], track->trk, track->poly);
                    norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, x->polyData->vertex[0], track->trk, track->poly);
                    norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, x->polyData->vertex[2], track->trk, track->poly);
                    norms.emplace_back(glm::vec3(norm_floatpt.x, norm_floatpt.y, norm_floatpt.z));
                    //norm_floatpt = VertexNormal(i, x->polyData->vertex[3], track->trk, track->poly);
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

std::vector<Track> NFS3_Loader::ParseCOLModels() {
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

Texture NFS3_Loader::LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name) {
    std::stringstream filename;
    std::stringstream filename_alpha;

    if (track_texture.islane) {
        filename << "../resources/sfx/" << setfill('0') << setw(4) << track_texture.texture + 9 << ".BMP";
        filename_alpha << "../resources/sfx/" << setfill('0') << setw(4) << track_texture.texture + 9 << "-a.BMP";
    } else {
        filename << TRACK_PATH << "NFS3/" << track_name << "/textures/" << setfill('0') << setw(4) << track_texture.texture << ".BMP";
        filename_alpha << TRACK_PATH << "NFS3/" << track_name << "/textures/" << setfill('0') << setw(4) << track_texture.texture << "-a.BMP";
    }

    GLubyte *data;
    GLsizei width = track_texture.width;
    GLsizei height = track_texture.height;

    if(!LoadBmpWithAlpha(filename.str().c_str(), filename_alpha.str().c_str(), &data, width, height)){
        std::cerr << "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!" << std::endl;
        // If the texture is missing, load a "MISSING" texture of identical size.
        ASSERT(LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", &data, width, height), "Even the 'missing' texture is missing!");
        return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(track_texture.width),
                       static_cast<unsigned int>(track_texture.height));
    }

    return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(track_texture.width),
                   static_cast<unsigned int>(track_texture.height));
}