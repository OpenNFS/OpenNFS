//
// Created by Amrik.Sadhra on 20/06/2018.
//

#include "nfs3_loader.h"
#include "../Util/Raytracer.h"

using namespace TrackUtils;

// CAR
std::shared_ptr<Car> NFS3::LoadCar(const std::string &car_base_path) {
    boost::filesystem::path p(car_base_path);
    std::string car_name = p.filename().string();

    std::stringstream viv_path, car_out_path, fce_path;
    viv_path << car_base_path << "/car.viv";
    car_out_path << CAR_PATH << ToString(NFS_3) << "/" << car_name << "/";
    fce_path << CAR_PATH << ToString(NFS_3) << "/" << car_name << "/car.fce";

    ASSERT(Utils::ExtractVIV(viv_path.str(), car_out_path.str()), "Unable to extract " << viv_path.str() << " to " << car_out_path.str());

    return std::make_shared<Car>(LoadFCE(fce_path.str()), NFS_3, car_name);
}

void NFS3::ConvertFCE(const std::string &fce_path, const std::string &obj_out_path) {
    std::shared_ptr<Car> car(new Car(LoadFCE(fce_path), NFS_3, "Converted"));
    car->writeObj(obj_out_path);
}

std::vector<CarModel> NFS3::LoadFCE(const std::string &fce_path) {
    LOG(INFO) << "Parsing FCE File located at " << fce_path;
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI/2,0,0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    std::vector<CarModel> meshes;

    ifstream fce(fce_path, ios::in | ios::binary);

    auto *fceHeader = new FCE::NFS3::HEADER();
    fce.read((char *) fceHeader, sizeof(FCE::NFS3::HEADER));

    for (uint32_t part_Idx = 0; part_Idx < fceHeader->nParts; ++part_Idx) {
        float specularDamper = 0.2f;
        float specularReflectivity = 0.02f;
        float envReflectivity = 0.4f;

        std::vector<uint32_t> indices;
        std::vector<uint32_t> polygonFlags;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::string part_name(fceHeader->partNames[part_Idx]);
        glm::vec3 center = rotationMatrix * glm::vec3(fceHeader->partCoords[part_Idx].x/10, fceHeader->partCoords[part_Idx].y/10, fceHeader->partCoords[part_Idx].z/10);

        auto *partVertices = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partNormals = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partTriangles = new FCE::TRIANGLE[fceHeader->partNumTriangles[part_Idx]];

        fce.seekg(sizeof(FCE::NFS3::HEADER) + fceHeader->vertTblOffset + (fceHeader->partFirstVertIndices[part_Idx] * sizeof(FLOATPT)), ios_base::beg);
        fce.read((char *) partVertices, fceHeader->partNumVertices[part_Idx] * sizeof(FLOATPT));
        for (uint32_t vert_Idx = 0; vert_Idx < fceHeader->partNumVertices[part_Idx]; ++vert_Idx) {
            vertices.emplace_back(rotationMatrix * glm::vec3(partVertices[vert_Idx].x /10, partVertices[vert_Idx].y/10, partVertices[vert_Idx].z/10));
        }

        fce.seekg(sizeof(FCE::NFS3::HEADER) + fceHeader->normTblOffset + (fceHeader->partFirstVertIndices[part_Idx] * sizeof(FLOATPT)), ios_base::beg);
        fce.read((char *) partNormals, fceHeader->partNumVertices[part_Idx] * sizeof(FLOATPT));
        for (uint32_t normal_Idx = 0; normal_Idx < fceHeader->partNumVertices[part_Idx]; ++normal_Idx) {
            normals.emplace_back(rotationMatrix * glm::vec3(partNormals[normal_Idx].x, partNormals[normal_Idx].y, partNormals[normal_Idx].z));
        }

        fce.seekg(sizeof(FCE::NFS3::HEADER) + fceHeader->triTblOffset + (fceHeader->partFirstTriIndices[part_Idx] * sizeof(FCE::TRIANGLE)), ios_base::beg);
        fce.read((char *) partTriangles, fceHeader->partNumTriangles[part_Idx] * sizeof(FCE::TRIANGLE));
        for (uint32_t tri_Idx = 0; tri_Idx < fceHeader->partNumTriangles[part_Idx]; ++tri_Idx) {
            polygonFlags.emplace_back(partTriangles[tri_Idx].polygonFlags);
            polygonFlags.emplace_back(partTriangles[tri_Idx].polygonFlags);
            polygonFlags.emplace_back(partTriangles[tri_Idx].polygonFlags);
            indices.emplace_back(partTriangles[tri_Idx].vertex[0]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[1]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[2]);
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[0], partTriangles[tri_Idx].uvTable[3]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[1], partTriangles[tri_Idx].uvTable[4]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[2], partTriangles[tri_Idx].uvTable[5]));
        }

        meshes.emplace_back(CarModel(part_name, vertices, uvs, normals, indices, polygonFlags, center, specularDamper, specularReflectivity, envReflectivity));
        LOG(INFO) << "Loaded Mesh: " << meshes[part_Idx].m_name << " UVs: " << meshes[part_Idx].m_uvs.size() << " Verts: " << meshes[part_Idx].m_vertices.size() << " Indices: " << meshes[part_Idx].m_vertex_indices.size() << " Normals: " << meshes[part_Idx].m_normals.size();

        delete[] partNormals;
        delete[] partVertices;
        delete[] partTriangles;
    }

    fce.close();

    delete fceHeader;
    return meshes;
}

// TRACK
std::shared_ptr<TRACK> NFS3::LoadTrack(const std::string &track_base_path) {
    LOG(INFO) << "Loading Track located at " << track_base_path;
    auto track = make_shared<TRACK>(TRACK());

    boost::filesystem::path p(track_base_path);
    track->name = p.filename().string();
    stringstream frd_path, col_path, can_path, hrz_path;
    string strip = "k0";
    size_t pos = track->name.find(strip);
    if (pos != string::npos)
        track->name.replace(pos, strip.size(), "");

    frd_path << track_base_path << "/"  << track->name << ".frd";
    col_path << track_base_path << "/"  << track->name << ".col";
    can_path << track_base_path << "/"  << track->name << "00a.can";
    hrz_path << track_base_path << "/3" << track->name << ".hrz";

    ASSERT(ExtractTrackTextures(track_base_path, track->name, NFSVer::NFS_3), "Could not extract " << track->name << " QFS texture pack.");
    ASSERT(LoadFRD(frd_path.str(), track->name, track), "Could not load FRD file: " << frd_path.str()); // Load FRD file to get track block specific data
    ASSERT(LoadCOL(col_path.str(), track), "Could not load COL file: " << col_path.str()); // Load Catalogue file to get global (non trkblock specific) data
    ASSERT(LoadCAN(can_path.str(), track->cameraAnimation), "Could not load CAN file (camera animation): " << can_path.str()); // Load camera intro/outro animation data
    ASSERT(LoadHRZ(hrz_path.str(), track), "Could not load HRZ file (skybox/lighting):" << hrz_path.str()); // Load HRZ Data

    track->textureArrayID = MakeTextureArray(track->textures, false);
    track->track_blocks = ParseTRKModels(track);
    track->global_objects = ParseCOLModels(track);

    LOG(INFO) << "Track loaded successfully";
    return track;
}

void NFS3::FreeTrack(const std::shared_ptr<TRACK> &track) {
    FreeFRD(track);
    FreeCOL(track);
}

bool NFS3::LoadFRD(std::string frd_path, const std::string &track_name, const std::shared_ptr<TRACK> &track) {
    ifstream ar(frd_path, ios::in | ios::binary);

    char header[28]; /* file header */
    SAFE_READ(ar, header, 28); // header & numblocks
    SAFE_READ(ar, &track->nBlocks, 4);
    track->nBlocks++;
    if ((track->nBlocks < 1) || (track->nBlocks > 500)) return false; // 1st sanity check

    LOG(INFO) << "Loading FRD File located at " << frd_path;

    track->trk = new TRKBLOCK[track->nBlocks]();
    track->poly = new POLYGONBLOCK[track->nBlocks]();
    track->xobj = new XOBJBLOCK[4 * track->nBlocks + 1]();

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
        if (trackBlock->nVertices == 0) return false;
        trackBlock->vert = new FLOATPT[trackBlock->nVertices];;

        SAFE_READ(ar, trackBlock->vert, 12 * trackBlock->nVertices);
        trackBlock->unknVertices = new uint32_t[trackBlock->nVertices];
        SAFE_READ(ar, trackBlock->unknVertices, 4 * trackBlock->nVertices);
        SAFE_READ(ar, trackBlock->nbdData, 4 * 0x12c);
        // nStartPos & various blk sizes == 32 bytes
        SAFE_READ(ar, &(trackBlock->nStartPos), 32);

        if (block_Idx > 0)
            if (trackBlock->nStartPos !=
                track->trk[block_Idx - 1].nStartPos + track->trk[block_Idx - 1].nPositions)
                return false;
        trackBlock->posData = new POSITIONDATA[trackBlock->nPositions];
        SAFE_READ(ar, trackBlock->posData, 8 * trackBlock->nPositions);

        trackBlock->polyData = new POLYVROADDATA[trackBlock->nPolygons];

        for (uint32_t j = 0; j < trackBlock->nPolygons; j++)
            SAFE_READ(ar, trackBlock->polyData + j, 8);

        trackBlock->vroadData = new VROADDATA[trackBlock->nVRoad];
        SAFE_READ(ar, trackBlock->vroadData, 12 * trackBlock->nVRoad);

        if (trackBlock->nXobj > 0) {
            trackBlock->xobj = new REFXOBJ[trackBlock->nXobj];
            SAFE_READ(ar, trackBlock->xobj, 20 * trackBlock->nXobj);
        }
        if (trackBlock->nPolyobj > 0) {
            ar.seekg(20 * trackBlock->nPolyobj, ios_base::cur);
        }
        trackBlock->nPolyobj = 0;
        if (trackBlock->nSoundsrc > 0) {
            trackBlock->soundsrc = new SOUNDSRC[trackBlock->nSoundsrc];
            SAFE_READ(ar, trackBlock->soundsrc, 16 * trackBlock->nSoundsrc);
        }
        if (trackBlock->nLightsrc > 0) {
            trackBlock->lightsrc = new LIGHTSRC[trackBlock->nLightsrc];
            SAFE_READ(ar, trackBlock->lightsrc, 16 * trackBlock->nLightsrc);
        }
    }

    // POLYGONBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
        POLYGONBLOCK *p = &(track->poly[block_Idx]);
        for (uint32_t j = 0; j < 7; j++) {
            SAFE_READ(ar, &(p->sz[j]), 0x4);
            if (p->sz[j] != 0) {
                SAFE_READ(ar, &(p->szdup[j]), 0x4);
                if (p->szdup[j] != p->sz[j]) return false;
                p->poly[j] = static_cast<LPPOLYGONDATA>(calloc(p->sz[j], sizeof(POLYGONDATA)));
                SAFE_READ(ar, p->poly[j], 14 * p->sz[j]);
            }
        }
        if (p->sz[4] != track->trk[block_Idx].nPolygons) return false; // sanity check
        for (uint32_t obj_Idx = 0; obj_Idx < 4; obj_Idx++) {
            OBJPOLYBLOCK *o = &(p->obj[obj_Idx]);
            SAFE_READ(ar, &(o->n1), 0x4);
            if (o->n1 > 0) {
                SAFE_READ(ar, &(o->n2), 0x4);
                o->types = new uint32_t[o->n2];
                o->numpoly = new uint32_t[o->n2];
                o->poly = new LPPOLYGONDATA[o->n2];
                o->nobj = 0;
                l = 0;
                for (uint32_t k = 0; k < o->n2; k++) {
                    SAFE_READ(ar, o->types + k, 0x4);
                    if (o->types[k] == 1) {
                        SAFE_READ(ar, o->numpoly + o->nobj, 0x4);
                        o->poly[o->nobj] = static_cast<LPPOLYGONDATA>(calloc(o->numpoly[o->nobj], sizeof(POLYGONDATA)));
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
            track->xobj[xblock_Idx].obj = new XOBJDATA[track->xobj[xblock_Idx].nobj];
        }
        for (uint32_t xobj_Idx = 0; xobj_Idx < track->xobj[xblock_Idx].nobj; xobj_Idx++) {
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
                x->animData = new ANIMDATA[x->nAnimLength];
                SAFE_READ(ar, x->animData, 20 * x->nAnimLength);
                // make a ref point from first anim position
                x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
            } else return false; // unknown object type

            // common part : vertices & polygons
            SAFE_READ(ar, &(x->nVertices), 4);
            x->vert = new FLOATPT[x->nVertices];
            SAFE_READ(ar, x->vert, 12 * x->nVertices);
            x->unknVertices = new uint32_t[x->nVertices];
            SAFE_READ(ar, x->unknVertices, 4 * x->nVertices);
            SAFE_READ(ar, &(x->nPolygons), 4);
            x->polyData = new POLYGONDATA[x->nPolygons];
            SAFE_READ(ar, x->polyData, 14 * x->nPolygons);
        }
    }

    // TEXTUREBLOCKs
    SAFE_READ(ar, &track->nTextures, 4);
    track->texture = new TEXTUREBLOCK[track->nTextures];
    for (uint32_t tex_Idx = 0; tex_Idx < track->nTextures; tex_Idx++) {
        SAFE_READ(ar, &(track->texture[tex_Idx]), 47);
        track->textures[track->texture[tex_Idx].texture] = LoadTexture(track->texture[tex_Idx], track_name);
    }

    uint32_t pad;
    return ar.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
}

bool NFS3::LoadCOL(std::string col_path, const std::shared_ptr<TRACK> &track) {
    ifstream coll(col_path, ios::in | ios::binary);

    COLOBJECT *o;

    track->col.hs_extra = NULL;
    if (coll.read((char *) &track->col, 16).gcount() != 16) return false;
    if (memcmp(track->col.collID, "COLL", sizeof(track->col.collID[0])) != 0) {
        LOG(WARNING) << "Invalid COL file";
        return false;
    }

    LOG(INFO) << "Loading COL File located at " << col_path;

    if (track->col.version != 11) return false;
    if ((track->col.nBlocks != 2) && (track->col.nBlocks != 4) && (track->col.nBlocks != 5)) return false;
    SAFE_READ(coll, track->col.xbTable, 4 * track->col.nBlocks);

    // texture XB
    SAFE_READ(coll, &track->col.textureHead, 8);
    if (track->col.textureHead.xbid != XBID_TEXTUREINFO) return false;

    track->col.texture = new COLTEXTUREINFO[track->col.textureHead.nrec];
    SAFE_READ(coll, track->col.texture, 8 * track->col.textureHead.nrec);

    // struct3D XB
    if (track->col.nBlocks >= 4) {
        SAFE_READ(coll, &track->col.struct3DHead, 8);
        if (track->col.struct3DHead.xbid != XBID_STRUCT3D) return false;
        COLSTRUCT3D *s = track->col.struct3D = new COLSTRUCT3D[track->col.struct3DHead.nrec];
        int delta;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.struct3DHead.nrec; colRec_Idx++, s++) {
            SAFE_READ(coll, s, 8);
            delta = (8 + 16 * s->nVert + 6 * s->nPoly) % 4;
            delta = (4 - delta) % 4;
            if (s->size != 8 + 16 * s->nVert + 6 * s->nPoly + delta) return false;
            s->vertex = new COLVERTEX[s->nVert];
            SAFE_READ(coll, s->vertex, 16 * s->nVert);
            s->polygon = new COLPOLYGON[s->nPoly];
            SAFE_READ(coll, s->polygon, 6 * s->nPoly);
            int dummy;
            if (delta > 0) SAFE_READ(coll, &dummy, delta);
        }

        // object XB
        SAFE_READ(coll, &track->col.objectHead, 8);
        if ((track->col.objectHead.xbid != XBID_OBJECT) && (track->col.objectHead.xbid != XBID_OBJECT2))
            return false;
        o = track->col.object = new COLOBJECT[track->col.objectHead.nrec];

        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.objectHead.nrec; colRec_Idx++, o++) {
            SAFE_READ(coll, o, 4);
            if (o->type == 1) {
                if (o->size != 16) return false;
                SAFE_READ(coll, &(o->ptRef), 12);
            } else if (o->type == 3) {
                SAFE_READ(coll, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength) return false;
                o->animData = new ANIMDATA[o->animLength];
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
        if ((track->col.object2Head.xbid != XBID_OBJECT) && (track->col.object2Head.xbid != XBID_OBJECT2))
            return false;
        o = track->col.object2 = new COLOBJECT[track->col.object2Head.nrec];

        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.object2Head.nrec; colRec_Idx++, o++) {
            SAFE_READ(coll, o, 4);
            if (o->type == 1) {
                if (o->size != 16) return false;
                SAFE_READ(coll, &(o->ptRef), 12);
            } else if (o->type == 3) {
                SAFE_READ(coll, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength) return false;
                o->animData = new ANIMDATA[o->animLength];
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
    track->col.vroad = new COLVROAD[track->col.vroadHead.nrec];
    SAFE_READ(coll, track->col.vroad, 36 * track->col.vroadHead.nrec);

    uint32_t pad;
    return coll.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
}

bool NFS3::LoadHRZ(std::string hrz_path, const std::shared_ptr<TRACK> &track) {
    ifstream hrz(hrz_path, ios::in | ios::binary);
    if(!hrz.is_open()) return false;
    LOG(INFO) << "Loading HRZ File located at " << hrz_path;

    std::string str, skyTopColour, skyBottomColour;

    while (std::getline(hrz, str))
    {
        if(str.find("/* r,g,b value at top of Gourad shaded SKY area */") != std::string::npos){
            std::getline(hrz, skyTopColour);
        }
        if(str.find("/* r,g,b values for base of Gourad shaded SKY area */") != std::string::npos){
            std::getline(hrz, skyBottomColour);
        }
    }

    track->sky_top_colour = parseRGBString(skyTopColour);
    track->sky_bottom_colour = parseRGBString(skyBottomColour);

    hrz.close();

    return true;
}

std::vector<TrackBlock> NFS3::ParseTRKModels(const std::shared_ptr<TRACK> &track) {
    LOG(INFO) << "Parsing TRK file into ONFS GL structures";

    std::vector<TrackBlock> track_blocks = std::vector<TrackBlock>();
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI/2,0,0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    /* TRKBLOCKS - BASE TRACK GEOMETRY */
    for (uint32_t i = 0; i < track->nBlocks; i++) {
        // Get Verts from Trk block, indices from associated polygon block
        TRKBLOCK trk_block = track->trk[i];
        POLYGONBLOCK polygon_block = track->poly[i];
        TrackBlock current_track_block(i, rotationMatrix * glm::vec3(trk_block.ptCentre.x/ 10, trk_block.ptCentre.y/ 10, trk_block.ptCentre.z/ 10));
        glm::vec3 trk_block_center = rotationMatrix * glm::vec3(0, 0, 0);

        // Light sources
        for (uint32_t j = 0; j < trk_block.nLightsrc; j++) {
            glm::vec3 light_center = rotationMatrix * glm::vec3((trk_block.lightsrc[j].refpoint.x / 65536.0) / 10,
                                                                (trk_block.lightsrc[j].refpoint.y / 65536.0) / 10,
                                                                (trk_block.lightsrc[j].refpoint.z / 65536.0) / 10);
            current_track_block.lights.emplace_back(Entity(i, j, NFS_3, LIGHT, MakeLight(light_center, trk_block.lightsrc[j].type)));
        }

        for (uint32_t s = 0; s < trk_block.nSoundsrc; s++) {
            glm::vec3 sound_center = rotationMatrix * glm::vec3((trk_block.soundsrc[s].refpoint.x / 65536.0) / 10,
                                                                (trk_block.soundsrc[s].refpoint.y / 65536.0) / 10,
                                                                (trk_block.soundsrc[s].refpoint.z / 65536.0) / 10);
            current_track_block.sounds.emplace_back(Entity(i, s, NFS_3, SOUND, Sound(sound_center, trk_block.soundsrc[s].type)));
        }

        // Get Object vertices
        std::vector<glm::vec3> obj_verts;
        std::vector<glm::vec4> obj_shading_verts;
        for (uint32_t v = 0; v < trk_block.nObjectVert; v++) {
            obj_verts.emplace_back(rotationMatrix * glm::vec3(trk_block.vert[v].x / 10, trk_block.vert[v].y / 10, trk_block.vert[v].z / 10));
            uint32_t shading_data = trk_block.unknVertices[v];
            obj_shading_verts.emplace_back(glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f, (shading_data & 0xFF) / 255.0f, ((shading_data >> 24) & 0xFF) / 255.0f));
        }
        // 4 OBJ Poly blocks
        for (uint32_t j = 0; j < 4; j++) {
            OBJPOLYBLOCK obj_polygon_block = polygon_block.obj[j];
            if (obj_polygon_block.n1 > 0) {
                // Iterate through objects in objpoly block up to num objects
                for (uint32_t k = 0; k < obj_polygon_block.nobj; k++) {
                    //TODO: Animated objects here, obj_polygon_block.types
                    // Mesh Data
                    std::vector<unsigned int> vertex_indices;
                    std::vector<glm::vec2> uvs;
                    std::vector<unsigned int> texture_indices;
                    std::vector<glm::vec3> norms;
					uint32_t accumulatedObjectFlags = 0u;
                    // Get Polygons in object
                    LPPOLYGONDATA object_polys = obj_polygon_block.poly[k];
                    for (uint32_t p = 0; p < obj_polygon_block.numpoly[k]; p++) {
                        TEXTUREBLOCK texture_for_block = track->texture[object_polys[p].texture];
                        Texture gl_texture = track->textures[texture_for_block.texture];

                        glm::vec3 normal = rotationMatrix * calculateQuadNormal( pointToVec(trk_block.vert[object_polys[p].vertex[0]]),  pointToVec(trk_block.vert[object_polys[p].vertex[1]]), pointToVec(trk_block.vert[object_polys[p].vertex[2]]), pointToVec(trk_block.vert[object_polys[p].vertex[3]]));
                        norms.emplace_back(normal);
                        norms.emplace_back(normal);
                        norms.emplace_back(normal);
                        norms.emplace_back(normal);
                        norms.emplace_back(normal);
                        norms.emplace_back(normal);

                        vertex_indices.emplace_back(object_polys[p].vertex[0]);
                        vertex_indices.emplace_back(object_polys[p].vertex[1]);
                        vertex_indices.emplace_back(object_polys[p].vertex[2]);
                        vertex_indices.emplace_back(object_polys[p].vertex[0]);
                        vertex_indices.emplace_back(object_polys[p].vertex[2]);
                        vertex_indices.emplace_back(object_polys[p].vertex[3]);

                        std::vector<glm::vec2> transformedUVs = nfsUvGenerate(NFS_3, OBJ_POLY, object_polys[p].hs_texflags, gl_texture, texture_for_block);
                        uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);
                        texture_indices.emplace_back(texture_for_block.texture);

                        accumulatedObjectFlags |= object_polys[p].flags;
                    }
                    current_track_block.objects.emplace_back(Entity(i,  (j + 1) * (k + 1), NFS_3, OBJ_POLY, Track(obj_verts, norms, uvs, texture_indices, vertex_indices, obj_shading_verts, trk_block_center), accumulatedObjectFlags));
                }
            }
        }

        /* XOBJS - EXTRA OBJECTS */
        for (uint32_t l = (i * 4); l < (i * 4) + 4; l++) {
            for (uint32_t j = 0; j < track->xobj[l].nobj; j++) {
                XOBJDATA *x = &(track->xobj[l].obj[j]);
                if (x->crosstype == 4) { // basic objects
                } else if (x->crosstype == 3) { // animated objects
                }
                // common part : vertices & polygons
                std::vector<glm::vec3> verts;
                std::vector<glm::vec4> xobj_shading_verts;
                for (uint32_t k = 0; k < x->nVertices; k++, x->vert++) {
                    verts.emplace_back(rotationMatrix * glm::vec3(x->vert->x / 10, x->vert->y / 10, x->vert->z / 10));
                    uint32_t shading_data = x->unknVertices[k];
                    //RGBA
                    xobj_shading_verts.emplace_back(glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f, (shading_data & 0xFF) / 255.0f, ((shading_data >> 24) & 0xFF) / 255.0f));
                }
                std::vector<unsigned int> vertex_indices;
                std::vector<glm::vec2> uvs;
                std::vector<unsigned int> texture_indices;
                std::vector<glm::vec3> norms;
                uint32_t accumulatedObjectFlags = 0u;
                for (uint32_t k = 0; k < x->nPolygons; k++, x->polyData++) {
                    TEXTUREBLOCK texture_for_block = track->texture[x->polyData->texture];
                    Texture gl_texture = track->textures[texture_for_block.texture];

                    glm::vec3 normal = rotationMatrix * calculateQuadNormal( pointToVec(verts[x->polyData->vertex[0]]),  pointToVec(verts[x->polyData->vertex[1]]), pointToVec(verts[x->polyData->vertex[2]]), pointToVec(verts[x->polyData->vertex[3]]));
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);

                    vertex_indices.emplace_back(x->polyData->vertex[0]);
                    vertex_indices.emplace_back(x->polyData->vertex[1]);
                    vertex_indices.emplace_back(x->polyData->vertex[2]);
                    vertex_indices.emplace_back(x->polyData->vertex[0]);
                    vertex_indices.emplace_back(x->polyData->vertex[2]);
                    vertex_indices.emplace_back(x->polyData->vertex[3]);

                    std::vector<glm::vec2> transformedUVs = nfsUvGenerate(NFS_3, XOBJ, x->polyData->hs_texflags, gl_texture, texture_for_block);
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);
                    texture_indices.emplace_back(texture_for_block.texture);

                    accumulatedObjectFlags |= x->polyData->flags;
                }
                glm::vec3 xobj_center = rotationMatrix * glm::vec3(x->ptRef.x / 10, x->ptRef.y / 10, x->ptRef.z / 10 );
                current_track_block.objects.emplace_back(Entity(i, l, NFS_3, XOBJ, Track(verts, norms, uvs, texture_indices, vertex_indices, xobj_shading_verts, xobj_center), accumulatedObjectFlags));
            }
        }

        // Mesh Data
        std::vector<unsigned int> vertex_indices;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> texture_indices;
        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> trk_block_shading_verts;
        std::vector<glm::vec3> norms;
        uint32_t accumulatedObjectFlags = 0u;
        for (int32_t j = 0; j < trk_block.nVertices; j++) {
            verts.emplace_back(rotationMatrix * glm::vec3(trk_block.vert[j].x / 10, trk_block.vert[j].y / 10, trk_block.vert[j].z / 10));
            // Break uint32_t of RGB into 4 normalised floats and store into vec4
            uint32_t shading_data = trk_block.unknVertices[j];
            trk_block_shading_verts.emplace_back(glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f, (shading_data & 0xFF) / 255.0f, ((shading_data >> 24) & 0xFF) / 255.0f));
        }
        // Get indices from Chunk 4 and 5 for High Res polys, Chunk 6 for Road Lanes
        for (uint32_t chnk = 4; chnk <= 6; chnk++) {
            if ((chnk == 6) && (trk_block.nVertices <= trk_block.nHiResVert))
                continue;
            LPPOLYGONDATA poly_chunk = polygon_block.poly[chnk];
            for (uint32_t k = 0; k < polygon_block.sz[chnk]; k++) {
                TEXTUREBLOCK texture_for_block = track->texture[poly_chunk[k].texture];
                Texture gl_texture = track->textures[texture_for_block.texture];

                glm::vec3 normal = rotationMatrix * calculateQuadNormal( pointToVec(trk_block.vert[poly_chunk[k].vertex[0]]),  pointToVec(trk_block.vert[poly_chunk[k].vertex[1]]), pointToVec(trk_block.vert[poly_chunk[k].vertex[2]]), pointToVec(trk_block.vert[poly_chunk[k].vertex[3]]));
                norms.emplace_back(normal);
                norms.emplace_back(normal);
                norms.emplace_back(normal);
                norms.emplace_back(normal);
                norms.emplace_back(normal);
                norms.emplace_back(normal);

                vertex_indices.emplace_back(poly_chunk[k].vertex[0]);
                vertex_indices.emplace_back(poly_chunk[k].vertex[1]);
                vertex_indices.emplace_back(poly_chunk[k].vertex[2]);
                vertex_indices.emplace_back(poly_chunk[k].vertex[0]);
                vertex_indices.emplace_back(poly_chunk[k].vertex[2]);
                vertex_indices.emplace_back(poly_chunk[k].vertex[3]);

                std::vector<glm::vec2> transformedUVs = nfsUvGenerate(NFS_3,  chnk == 6 ? LANE : ROAD, poly_chunk[k].hs_texflags, gl_texture, texture_for_block);
                uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);
                texture_indices.emplace_back(texture_for_block.texture);

                accumulatedObjectFlags |= poly_chunk[k].flags;
            }

            if(chnk == 6){
                current_track_block.lanes.emplace_back(Entity(i, -1, NFS_3, LANE, Track(verts, norms, uvs, texture_indices, vertex_indices, trk_block_shading_verts, trk_block_center), accumulatedObjectFlags));
            } else {
                current_track_block.track.emplace_back(Entity(i, -1, NFS_3, ROAD, Track(verts, norms, uvs, texture_indices, vertex_indices, trk_block_shading_verts, trk_block_center), accumulatedObjectFlags));
            }
        }
        track_blocks.emplace_back(current_track_block);
    }
    return track_blocks;
}

std::vector<Entity> NFS3::ParseCOLModels(const std::shared_ptr<TRACK> &track) {
    LOG(INFO) << "Parsing COL file into ONFS GL structures";

    std::vector<Entity> col_entities;
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI/2,0,0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    COLOBJECT *o = track->col.object;
    /* COL DATA - TODO: Come back for VROAD AI/Collision data */
    for (uint32_t i = 0; i < track->col.objectHead.nrec; i++, o++) {
        COLSTRUCT3D s = track->col.struct3D[o->struct3D];
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> texture_indices;
        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> shading_data;
        std::vector<glm::vec3> norms;
        for (uint32_t j = 0; j < s.nVert; j++, s.vertex++) {
            verts.emplace_back(rotationMatrix * glm::vec3(s.vertex->pt.x / 10, s.vertex->pt.y / 10, s.vertex->pt.z / 10));
            shading_data.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
        }
        for (uint32_t k = 0; k < s.nPoly; k++, s.polygon++) {
            // Remap the COL TextureID's using the COL texture block (XBID2)
            COLTEXTUREINFO col_texture = track->col.texture[s.polygon->texture];
            TEXTUREBLOCK texture_for_block;
            // Find the texture by it's file name, but use the Texture table to get the block. TODO: Not mapping this so, must do a manual search.
            for (uint32_t t = 0; t < track->nTextures; t++) {
                if (track->texture[t].texture == col_texture.texture) {
                    texture_for_block = track->texture[t];
                }
            }
            Texture gl_texture = track->textures[texture_for_block.texture];
            indices.emplace_back(s.polygon->v[0]);
            indices.emplace_back(s.polygon->v[1]);
            indices.emplace_back(s.polygon->v[2]);
            indices.emplace_back(s.polygon->v[0]);
            indices.emplace_back(s.polygon->v[2]);
            indices.emplace_back(s.polygon->v[3]);

            glm::vec3 normal = rotationMatrix * calculateQuadNormal( pointToVec(verts[s.polygon->v[0]]),  pointToVec(verts[s.polygon->v[1]]), pointToVec(verts[s.polygon->v[2]]), pointToVec(verts[s.polygon->v[3]]));
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);

            uvs.emplace_back(texture_for_block.corners[0] * gl_texture.max_u, (1.0f - texture_for_block.corners[1]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[2] * gl_texture.max_u, (1.0f - texture_for_block.corners[3]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[4] * gl_texture.max_u, (1.0f - texture_for_block.corners[5]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[0] * gl_texture.max_u, (1.0f - texture_for_block.corners[1]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[4] * gl_texture.max_u, (1.0f - texture_for_block.corners[5]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[6] * gl_texture.max_u, (1.0f - texture_for_block.corners[7]) * gl_texture.max_v);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
        }
        glm::vec3 position = rotationMatrix * glm::vec3(static_cast<float>(o->ptRef.x / 65536.0) / 10, static_cast<float>(o->ptRef.y / 65536.0) / 10, static_cast<float>(o->ptRef.z / 65536.0) / 10);
        col_entities.emplace_back(Entity(-1, i, NFS_3, GLOBAL, Track(verts, norms, uvs, texture_indices, indices, shading_data, position)));
    }
    return col_entities;
}

Texture NFS3::LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name) {
    std::stringstream filename;
    std::stringstream filename_alpha;

    if (track_texture.islane) {
        filename << "../resources/sfx/" << setfill('0') << setw(4) << track_texture.texture + 9 << ".BMP";
        filename_alpha << "../resources/sfx/" << setfill('0') << setw(4) << track_texture.texture + 9 << "-a.BMP";
    } else {
        filename << TRACK_PATH << ToString(NFS_3) << "/" << track_name << "/textures/" << setfill('0') << setw(4) << track_texture.texture << ".BMP";
        filename_alpha << TRACK_PATH << ToString(NFS_3) << "/" << track_name << "/textures/" << setfill('0') << setw(4) << track_texture.texture << "-a.BMP";
    }

    GLubyte *data;
    GLsizei width = track_texture.width;
    GLsizei height = track_texture.height;

    if (!Utils::LoadBmpWithAlpha(filename.str().c_str(), filename_alpha.str().c_str(), &data, &width, &height)) {
        LOG(WARNING) << "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!";
        // If the texture is missing, load a "MISSING" texture of identical size.
        ASSERT(Utils::LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", &data, &width, &height), "Even the 'missing' texture is missing!");
        return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
    }

    return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(track_texture.width), static_cast<unsigned int>(track_texture.height));
}

void NFS3::FreeFRD(const std::shared_ptr<TRACK> &track) {
    // Free FRD data
    // TRKBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
        TRKBLOCK *trackBlock = &(track->trk[block_Idx]);
        delete[]trackBlock->vert;
        delete[]trackBlock->unknVertices;
        delete[]trackBlock->posData;
        delete[] trackBlock->polyData;
        delete[] trackBlock->vroadData;
        if (trackBlock->nXobj > 0) {
            delete[]trackBlock->xobj;
        }
        if (trackBlock->nSoundsrc > 0) {
            delete[]trackBlock->soundsrc;
        }
        if (trackBlock->nLightsrc > 0) {
            delete[]trackBlock->lightsrc;
        }
        //delete trackBlock;
    }
    //delete []track->trk;

    // POLYGONBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
        POLYGONBLOCK *p = &(track->poly[block_Idx]);
        for (uint32_t j = 0; j < 7; j++) {
            if (p->sz[j] != 0) {
                free(p->poly[j]);
            }
        }
        for (uint32_t obj_Idx = 0; obj_Idx < 4; obj_Idx++) {
            OBJPOLYBLOCK *o = &(p->obj[obj_Idx]);
            if (o->n1 > 0) {
                delete[]o->types;
                delete[]o->numpoly;
                for (uint32_t k = 0; k < o->n2; k++) {
                    if (o->types[k] == 1) {
                        // free(o->poly[o->nobj]);
                    }
                }
            }
        }
    }

    // XOBJBLOCKs
    for (uint32_t xblock_Idx = 0; xblock_Idx <= 4 * track->nBlocks; xblock_Idx++) {
        if (track->xobj[xblock_Idx].nobj > 0) {
            delete[]track->xobj[xblock_Idx].obj;
        }
        for (uint32_t xobj_Idx = 0; xobj_Idx < track->xobj[xblock_Idx].nobj; xobj_Idx++) {
            XOBJDATA *x = &(track->xobj[xblock_Idx].obj[xobj_Idx]);
            if (x->crosstype == 3) { // animated objects
                delete[]x->animData;
            }
            //delete[]x->vert;
            delete[]x->unknVertices;
            delete[]x->polyData;
            //delete x;
        }
    }

    // TEXTUREBLOCKs
    delete[]track->texture;
    delete[]track->trk;
    delete[]track->poly;
    delete[]track->xobj;
}

void NFS3::FreeCOL(const std::shared_ptr<TRACK> &track) {
    // Free COL Data
    delete[]track->col.texture;
    // struct3D XB
    if (track->col.nBlocks >= 4) {
        COLSTRUCT3D *s = track->col.struct3D;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.struct3DHead.nrec; colRec_Idx++, s++) {
            delete[]s->vertex;
            delete[]s->polygon;
        }
        delete[] track->col.struct3D;

        // Object XB
        COLOBJECT *o = track->col.object;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.objectHead.nrec; colRec_Idx++, o++) {
            if (o->type == 3) {
                delete[]o->animData;
            }
        }
        delete[]track->col.object;
    }

    // object2 XB
    if (track->col.nBlocks == 5) {
        COLOBJECT *o = track->col.object2;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.object2Head.nrec; colRec_Idx++, o++) {
            if (o->type == 3) {
                delete[]o->animData;
            }
        }
        delete[]track->col.object2;
    }
    delete track->col.vroad;
}
