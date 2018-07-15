//
// Created by Amrik on 03/07/2018.
//

#include "nfs4_loader.h"
#include "trk_loader.h"
#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>

NFS4_Loader::NFS4_Loader(const std::string &car_base_path, std::string *car_name) {
    boost::filesystem::path p(car_base_path);
    *car_name = p.filename().string();

    std::stringstream viv_path, car_out_path, fce_path;
    viv_path << car_base_path << "/car.viv";
    car_out_path << CAR_PATH << *car_name << "/";
    fce_path << CAR_PATH << *car_name << "/car.fce";

    ASSERT(ExtractVIV(viv_path.str(), car_out_path.str()),
           "Unable to extract " << viv_path.str() << " to " << car_out_path.str());
    ASSERT(LoadFCE(fce_path.str()), "Unable to load " << fce_path.str());
}

NFS4_Loader::NFS4_Loader(const std::string &track_base_path) {
    std::cout << "--- Loading NFS4 Track ---" << std::endl;

    boost::filesystem::path p(track_base_path);
    std::string track_name = p.filename().string();
    stringstream frd_path;
    string strip = "K0";
    unsigned int pos = track_name.find(strip);
    if (pos != string::npos)
        track_name.replace(pos, strip.size(), "");

    frd_path << track_base_path << "/TR.frd";

    ASSERT(ExtractTrackTextures(track_base_path, track_name, NFSVer::NFS_4),
           "Could not extract " << track_name << " QFS texture pack.");
    ASSERT(LoadFRD(frd_path.str(), track_name),
           "Could not load FRD file: " << frd_path.str()); // Load FRD file to get track block specific data

    track->texture_gl_mappings = GenTrackTextures(track->textures);
    track->track_blocks = ParseTRKModels();

    std::cout << "Successful track load!" << std::endl;
}

bool NFS4_Loader::LoadFCE(const std::string &fce_path) {
    std::cout << "- Parsing FCE File: " << fce_path << std::endl;
    ifstream fce(fce_path, ios::in | ios::binary);

    auto *fceHeader = new FCE::NFS4::HEADER();
    fce.read((char *) fceHeader, sizeof(FCE::NFS4::HEADER));

    for (int part_Idx = 0; part_Idx < fceHeader->nParts; ++part_Idx) {
        float specularDamper = 0.2;
        float specularReflectivity = 0.02;
        float envReflectivity = 0.4;

        std::vector<uint32_t> indices;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::string part_name(fceHeader->partNames[part_Idx]);
        glm::vec3 center(fceHeader->partCoords[part_Idx].x, fceHeader->partCoords[part_Idx].y,
                         fceHeader->partCoords[part_Idx].z);
        center /= 10;

        auto *partVertices = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partNormals = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partTriangles = new FCE::TRIANGLE[fceHeader->partNumTriangles[part_Idx]];

        fce.seekg(sizeof(FCE::NFS4::HEADER) + fceHeader->vertTblOffset +
                  (fceHeader->partFirstVertIndices[part_Idx] * sizeof(FLOATPT)), ios_base::beg);
        fce.read((char *) partVertices, fceHeader->partNumVertices[part_Idx] * sizeof(FLOATPT));
        for (int vert_Idx = 0; vert_Idx < fceHeader->partNumVertices[part_Idx]; ++vert_Idx) {
            vertices.emplace_back(
                    glm::vec3(partVertices[vert_Idx].x, partVertices[vert_Idx].y, partVertices[vert_Idx].z));
        }

        fce.seekg(sizeof(FCE::NFS4::HEADER) + fceHeader->normTblOffset +
                  (fceHeader->partFirstVertIndices[part_Idx] * sizeof(FLOATPT)), ios_base::beg);
        fce.read((char *) partNormals, fceHeader->partNumVertices[part_Idx] * sizeof(FLOATPT));
        for (int normal_Idx = 0; normal_Idx < fceHeader->partNumVertices[part_Idx]; ++normal_Idx) {
            normals.emplace_back(
                    glm::vec3(partNormals[normal_Idx].x, partNormals[normal_Idx].y, partNormals[normal_Idx].z));
        }

        fce.seekg(sizeof(FCE::NFS4::HEADER) + fceHeader->triTblOffset +
                  (fceHeader->partFirstTriIndices[part_Idx] * sizeof(FCE::TRIANGLE)), ios_base::beg);
        fce.read((char *) partTriangles, fceHeader->partNumTriangles[part_Idx] * sizeof(FCE::TRIANGLE));
        for (int tri_Idx = 0; tri_Idx < fceHeader->partNumTriangles[part_Idx]; ++tri_Idx) {
            indices.emplace_back(partTriangles[tri_Idx].vertex[0]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[1]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[2]);
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[0], 1.0f - partTriangles[tri_Idx].uvTable[3]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[1], 1.0f - partTriangles[tri_Idx].uvTable[4]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[2], 1.0f - partTriangles[tri_Idx].uvTable[5]));
        }

        meshes.emplace_back(CarModel(part_name, part_Idx, vertices, uvs, normals, indices, center, specularDamper,
                                     specularReflectivity, envReflectivity));
        std::cout << "Mesh: " << meshes[part_Idx].m_name << " UVs: " << meshes[part_Idx].m_uvs.size() << " Verts: "
                  << meshes[part_Idx].m_vertices.size() << " Indices: " << meshes[part_Idx].m_vertex_indices.size()
                  << " Normals: "
                  << meshes[part_Idx].m_normals.size() << std::endl;

        delete[] partNormals;
        delete[] partVertices;
        delete[] partTriangles;
    }

    fce.close();

    delete fceHeader;
    return true;
}

bool NFS4_Loader::LoadFRD(const std::string &frd_path, const std::string &track_name) {
    ifstream ar(frd_path, ios::in | ios::binary);
    uint32_t nPos;
    unsigned char ptrspace[44]; // some useless data from HS FRDs

    char header[28]; /* file header */
    SAFE_READ(ar, header, 28); // header & numblocks
    SAFE_READ(ar, &track->nBlocks, 4);
    track->nBlocks++;
    if ((track->nBlocks < 1) || (track->nBlocks > 500)) return false; // 1st sanity check

    track->trk = new TRKBLOCK[track->nBlocks]();
    track->poly = new POLYGONBLOCK[track->nBlocks]();
    track->xobj = new XOBJBLOCK[4 * track->nBlocks + 1]();

    SAFE_READ(ar, &nPos, 4); // choose between NFS3_4 & NFSHS
    if (nPos > 5000) track->bHSMode = false;
    else if (((nPos + 7) / 8) == track->nBlocks) track->bHSMode = true;
    else return false; // unknown file type

    // emulate the COL file
    memcpy(track->col.collID, "COLL", 4);
    track->col.version = 11;
    track->col.fileLength = 36 * nPos + 48;
    track->col.nBlocks = 2;
    track->col.xbTable[0] = 8;
    track->col.xbTable[1] = 24;
    // fake a texture table with only one entry to please NFS3_4/T3ED
    track->col.textureHead.size = 16;
    track->col.textureHead.xbid = XBID_TEXTUREINFO;
    track->col.textureHead.nrec = 1;
    track->col.texture = new COLTEXTUREINFO();
    // vroad XB
    track->col.vroadHead.size = 8 + 36 * nPos;
    track->col.vroadHead.xbid = XBID_VROAD;
    track->col.vroadHead.nrec = (uint16_t) nPos;
    track->col.vroad = new COLVROAD[track->nBlocks * 8]();
    track->col.hs_extra = new uint32_t[7 * nPos];

    for (int i = 0; i < nPos; i++) {
        COLVROAD vr = track->col.vroad[i];
        HS_VROADBLOCK vroadblk;
        ar.read((char *) &vroadblk, 84);
        vr.refPt.x = (long) (vroadblk.refPt.x * 65536);
        vr.refPt.z = (long) (vroadblk.refPt.z * 65536);
        vr.refPt.y = (long) (vroadblk.refPt.y * 65536);
        // a wild guess
        vr.unknown = (uint32_t) ((vroadblk.unknown2[3] & 0xFFFF) +       // unknownLanes[2]
                                 ((vroadblk.unknown2[4] & 0xF) << 16) +    // wallKinds[0]
                                 ((vroadblk.unknown2[4] & 0xF00) << 20)); // wallKinds[1]
        if (vroadblk.normal.x >= 1.0) vr.normal.x = 127;
        else vr.normal.x = (signed char) (vroadblk.normal.x * 128);
        if (vroadblk.normal.z >= 1.0) vr.normal.z = 127;
        else vr.normal.z = (signed char) (vroadblk.normal.z * 128);
        if (vroadblk.normal.y >= 1.0) vr.normal.y = 127;
        else vr.normal.y = (signed char) (vroadblk.normal.y * 128);
        if (vroadblk.forward.x >= 1.0) vr.forward.x = 127;
        else vr.forward.x = (signed char) (vroadblk.forward.x * 128);
        if (vroadblk.forward.z >= 1.0) vr.forward.z = 127;
        else vr.forward.z = (signed char) (vroadblk.forward.z * 128);
        if (vroadblk.forward.y >= 1.0) vr.forward.y = 127;
        else vr.forward.y = (signed char) (vroadblk.forward.y * 128);
        if (vroadblk.right.x >= 1.0) vr.right.x = 127;
        else vr.right.x = (signed char) (vroadblk.right.x * 128);
        if (vroadblk.right.z >= 1.0) vr.right.z = 127;
        else vr.right.z = (signed char) (vroadblk.right.z * 128);
        if (vroadblk.right.y >= 1.0) vr.right.y = 127;
        else vr.right.y = (signed char) (vroadblk.right.y * 128);
        vr.leftWall = (long) (vroadblk.leftWall * 65536);
        vr.rightWall = (long) (vroadblk.rightWall * 65536);
        memcpy(track->col.hs_extra + 7 * i, &(vroadblk.unknown1[0]), 28);
    }

    // TRKBLOCKs
    for (int block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
        TRKBLOCK *b = &(track->trk[block_Idx]);
        POLYGONBLOCK *p = &(track->poly[block_Idx]);
        // 7 track polygon numbers
        ar.read((char *) p->sz, 28);
        memcpy(p->szdup, p->sz, 28);
        // 4 object polygon numbers
        for (int j = 0; j < 4; j++) {
            ar.read((char *) &(p->obj[j].n1), 4);
        }
        // pointer space
        ar.read((char *) ptrspace, 44);
        // 6 nVertices
        ar.read((char *) &(b->nVertices), 24);
        if (b->nVertices < 0) {/*||(b->nVertices>1000)*/
            free(track->col.hs_extra);
            free(track->col.texture);
            return false;
        }
        // pointer space
        ar.read((char *) ptrspace, 8);
        // ptCentre, ptBounding == 60 bytes
        ar.read((char *) b, 60);
        // nbdData
        ar.read((char *) b->nbdData, 4 * 0x12C);
        // xobj numbers
        for (int j = 4 * block_Idx; j < 4 * block_Idx + 4; j++) {
            ar.read((char *) &(track->xobj[j].nobj), 4);
            ar.read((char *) ptrspace, 4);
        }
        // nVRoad is not the same as in NFS3, will change later
        ar.read((char *) &(b->nVRoad), 4);
        // 2 unknown specific FLOATPTs
        ar.read((char *) &(b->hs_ptMin), 24);
        ar.read((char *) ptrspace, 4);
        // nPositions
        ar.read((char *) &(b->nPositions), 4);
        if (block_Idx == 0) b->nStartPos = 0;
        else b->nStartPos = track->trk[block_Idx - 1].nStartPos + track->trk[block_Idx - 1].nPositions;
        b->nPolygons = p->sz[4];
        // nXobj etc...
        ar.read((char *) &(b->nXobj), 4);
        ar.read((char *) ptrspace, 4);
        ar.read((char *) &(b->nPolyobj), 4);
        ar.read((char *) ptrspace, 4);
        ar.read((char *) &(b->nSoundsrc), 4);
        ar.read((char *) ptrspace, 4);
        ar.read((char *) &(b->nLightsrc), 4);
        ar.read((char *) ptrspace, 4);
        // neighbor data
        ar.read((char *) b->hs_neighbors, 32);
    }

    // TRKBLOCKDATA
    for (int block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
        TRKBLOCK *b = &(track->trk[block_Idx]);
        POLYGONBLOCK *p = &(track->poly[block_Idx]);
        // vertices
        b->vert = new FLOATPT[b->nVertices]();
        ar.read((char *) b->vert, 12 * b->nVertices);
        b->unknVertices = new uint32_t[b->nVertices];
        ar.read((char *) b->unknVertices, 4 * b->nVertices);
        // polyData is a bit tricky
        b->polyData = new POLYVROADDATA[b->nPolygons]();
        b->vroadData = new VROADDATA[b->nPolygons]();

        for (int j = 0; j < b->nPolygons; j++) {
            b->polyData[j].vroadEntry = j;
            b->polyData[j].flags = 0xE; // not passable
        }
        for (int j = 0; j < b->nVRoad; j++) {
            ar.read((char *) ptrspace, 10);
            int k = 0;
            ar.read((char *) &k, 2);
            memcpy(b->polyData[k].hs_minmax, ptrspace, 8);
            b->polyData[k].flags = ptrspace[8];
            b->polyData[k].hs_unknown = ptrspace[9];
            if ((ptrspace[8] & 15) == 14) {
                free(track->col.hs_extra);
                free(track->col.texture);
                return false;
            }
            ar.read((char *) b->vroadData + k, 12);
        }
        b->nVRoad = b->nPolygons;

        // the 4 misc. tables
        if (b->nXobj > 0) {
            b->xobj = new REFXOBJ[b->nXobj]();
            ar.read((char *) b->xobj, 20 * b->nXobj);
            // crossindex is f***ed up, but we don't really care
        }
        if (b->nPolyobj > 0) {
            char *buffer = (char *) malloc(b->nPolyobj * 20);
            ar.read(buffer, 20 * b->nPolyobj);
            free(buffer);
        }
        b->nPolyobj = 0;
        if (b->nSoundsrc > 0) {
            b->soundsrc = new SOUNDSRC[b->nSoundsrc]();
            ar.read((char *) b->soundsrc, 16 * b->nSoundsrc);
        }
        if (b->nLightsrc > 0) {
            b->lightsrc = new LIGHTSRC[b->nLightsrc]();
            ar.read((char *) b->lightsrc, 16 * b->nLightsrc);
        }

        // track polygons
        for (int j = 0; j < 7; j++)
            if (p->sz[j] != 0) {
                p->poly[j] = new POLYGONDATA[p->sz[j]]();
                for (int k = 0; k < p->sz[j]; k++) {
                    POLYGONDATA tmppoly;
                    ar.read((char *) &tmppoly, 13);
                    for (int m = 0; m < 4; m++) p->poly[j][k].vertex[m] = tmppoly.vertex[m ^ 1];
                    memcpy(&(p->poly[j][k].texture), &(tmppoly.texture), 5);
                    p->poly[j][k].unknown2 = 0xF9; //Nappe1: fixed for correct animation reading... (originally 0xF9)
                }
            }

        // make up some fake posData
        b->posData = new POSITIONDATA[b->nPositions]();
        uint32_t k = 0;
        LPPOLYGONDATA pp = p->poly[4];
        for (int j = 0; j < b->nPositions; j++) {
            b->posData[j].polygon = k;
            b->posData[j].unknown = 0;
            b->posData[j].extraNeighbor1 = -1;
            b->posData[j].extraNeighbor2 = -1;
            int l;
            do {
                l = 0;
                do {
                    if ((b->polyData[k].flags & 0x0f) % 14) l++;
                    k++;
                    pp++;
                } while ((k < b->nPolygons) && (pp->vertex[0] == (pp - 1)->vertex[1]) &&
                         (pp->vertex[3] == (pp - 1)->vertex[2]));
                if (((j == b->nPositions - 1) && (k < b->nPolygons)) || (k > b->nPolygons)) {
                    k = b->nPolygons;
                }
            } while ((l == 0) && (k < b->nPolygons));
            b->posData[j].nPolygons = k - b->posData[j].polygon;
        }

        // still vroadData is missing for unpassable polygons
        for (int j = 0; j < b->nPolygons; j++) {
            if (b->polyData[j].flags == 0xE) {
                FLOATPT v1, v2, norm;
                VROADDATA *v = b->vroadData + j;
                uint16_t *vno = p->poly[4][j].vertex;
                v1.x = b->vert[vno[1]].x - b->vert[vno[3]].x;
                v1.z = b->vert[vno[1]].z - b->vert[vno[3]].z;
                v1.y = b->vert[vno[1]].y - b->vert[vno[3]].y;
                v2.x = b->vert[vno[2]].x - b->vert[vno[0]].x;
                v2.z = b->vert[vno[2]].z - b->vert[vno[0]].z;
                v2.y = b->vert[vno[2]].y - b->vert[vno[0]].y;
                norm.x = -v1.y * v2.z + v1.z * v2.y;
                norm.y = -v1.z * v2.x + v1.x * v2.z;
                norm.z = -v1.x * v2.y + v1.y * v2.x;
                float len = (float) sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
                v->xNorm = (uint16_t) (norm.x * 32767 / len);
                v->zNorm = (uint16_t) (norm.z * 32767 / len);
                v->yNorm = (uint16_t) (norm.y * 32767 / len);
                v1.x = (float) track->col.vroad[b->nStartPos].forward.x;
                v1.z = (float) track->col.vroad[b->nStartPos].forward.z;
                v1.y = (float) track->col.vroad[b->nStartPos].forward.y;
                len = (float) sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
                v->xForw = (uint16_t) (v1.x * 32767 / len);
                v->zForw = (uint16_t) (v1.z * 32767 / len);
                v->yForw = (uint16_t) (v1.y * 32767 / len);
            }
        }
        // POLYGONBLOCK OBJECTS
        OBJPOLYBLOCK *o = p->obj;
        unsigned char *belong = (unsigned char *) malloc(b->nObjectVert);
        for (int j = 0; j < 4; j++, o++) {
            if (o->n1 > 0) {
                memset(belong, 0xFF, b->nObjectVert);
                pp = (LPPOLYGONDATA) malloc(14 * o->n1);
                for (k = 0; k < o->n1; k++) {
                    POLYGONDATA tmppoly;
                    ar.read((char *) &tmppoly, 13);
                    for (int m = 0; m < 4; m++) pp[k].vertex[m] = tmppoly.vertex[m ^ 1];
                    memcpy(&(pp[k].texture), &(tmppoly.texture), 5);
                    pp[k].unknown2 = 0xFF; // will temporarily store object's #
                    //Nappe1: Destroys AnimData??! ah... it sets it to 0xF9 later... fixing There...
                }
                uint32_t remn = o->n1;
                o->nobj = 0;
                while (remn > 0) {  // there are still polygons to be connected
                    k = 0;
                    while (pp[k].unknown2 != 0xFF) k++;
                    pp[k].unknown2 = (unsigned char) o->nobj;
                    remn--;
                    for (int l = 0; l < 4; l++) belong[pp[k].vertex[l]] = (unsigned char) o->nobj;
                    int m;
                    int l;
                    do {
                        m = 0;
                        for (k = 0; k < o->n1; k++)
                            if (pp[k].unknown2 == 0xFF) {
                                for (l = 0; l < 4; l++)
                                    if (belong[pp[k].vertex[l]] == (unsigned char) o->nobj) break;
                                if (l < 4) {
                                    remn--;
                                    m++;
                                    pp[k].unknown2 = (unsigned char) o->nobj;
                                    for (l = 0; l < 4; l++)
                                        belong[pp[k].vertex[l]] = (unsigned char) o->nobj;
                                }
                            }
                    } while (m > 0); // we've been adding more polygons
                    o->nobj++;
                }
                o->n2 = o->nobj + track->xobj[4 * block_Idx + j].nobj;
                o->types = new uint32_t[o->n2];
                o->numpoly = new uint32_t[o->nobj];
                o->poly = new LPPOLYGONDATA[4 * o->nobj];
                for (int l = 0; l < o->nobj; l++) {
                    remn = 0;
                    for (k = 0; k < o->n1; k++) if (pp[k].unknown2 == l) remn++;
                    o->numpoly[l] = remn;
                    o->poly[l] = (LPPOLYGONDATA) malloc(remn * sizeof(struct POLYGONDATA));
                    remn = 0;
                    for (k = 0; k < o->n1; k++)
                        if (pp[k].unknown2 == l) {
                            memcpy(o->poly[l] + remn, pp + k, sizeof(struct POLYGONDATA));
                            o->poly[l][remn].unknown2 = 0xF9; //Nappe1: fixed: Loads AnimData right. Didn't work??!
                            remn++;
                        }
                }
                free(pp);
                // there used to be something with REFPOLYOBJs if chunk 0
                for (k = 0; k < o->nobj; k++) {
                    o->types[k] = 1;
                }
                for (k = o->nobj; k < o->n2; k++) {
                    o->types[k] = 4; // to be replaced by 3/... later
                }
            }
        }
        free(belong);
        // XOBJs
        for (int j = 4 * block_Idx; j < 4 * block_Idx + 4; j++) {
            if (track->xobj[j].nobj > 0) {
                track->xobj[j].obj = new XOBJDATA[track->xobj[j].nobj]();
                for (k = 0; k < track->xobj[j].nobj; k++) {
                    XOBJDATA *x = &(track->xobj[j].obj[k]);
                    // 3 headers == 12 bytes
                    ar.read((char *) x, 12);
                    if ((x->crosstype == 4) || (x->crosstype == 2)) // basic objects
                    { ar.read((char *) &(x->ptRef), 12); }
                    else if (x->crosstype == 3) { // animated objects
                        // unkn3 instead of ptRef
                        ar.read((char *) x->unknown3, 12);
                    } else {
                        free(track->col.hs_extra);
                        free(track->col.texture);
                        return false; // unknown object type
                    }
                    if (p->obj[j & 3].nobj != 0) {
                        p->obj[j & 3].types[p->obj[j & 3].nobj + k] = x->crosstype;
                    }
                    // common part : vertices & polygons
                    ar.read((char *) &(x->AnimMemory), 4);
                    ar.read((char *) ptrspace, 4);
                    ar.read((char *) &(x->nVertices), 4);
                    ar.read((char *) ptrspace, 8);
                    x->vert = new FLOATPT[12 * x->nVertices]();
                    x->unknVertices = new uint32_t[4 * x->nVertices];
                    ar.read((char *) &(x->nPolygons), 4);
                    ar.read((char *) ptrspace, 4);
                    x->polyData = new POLYGONDATA[x->nPolygons * 14]();
                }
                // now the xobjdata
                for (k = 0; k < track->xobj[j].nobj; k++) {
                    XOBJDATA *x = &(track->xobj[j].obj[k]);
                    if (x->crosstype == 3) { // animated-specific header
                        ar.read((char *) x->unknown3 + 6, 2);
                        // if (x->unknown3[6]!=4) return false;  // fails
                        // type3, objno, animLength, unknown4
                        ar.read((char *) &(x->type3), 6);
                        if (x->type3 != 3) {
                            free(track->col.hs_extra);
                            free(track->col.texture);
                            return false;
                        }
                        x->animData = new ANIMDATA[20 * x->nAnimLength]();
                        ar.read((char *) x->animData, 20 * x->nAnimLength);
                        // make a ref point from first anim position
                        x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                        x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                        x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
                    }
// appears in REFPOLYOBJ & REFXOBJ but not in XOBJs !
/*				if (x->crosstype==6) { // object with byte data
					x->hs_type6=(char *)malloc(x->unknown2);
					if ((long)ar.read((char*)x->hs_type6,x->unknown2)!=x->unknown2) return false;
				}
*/                ar.read((char *) x->vert, 12 * x->nVertices);
                    ar.read((char *) x->unknVertices, 4 * x->nVertices);
                    for (int l = 0; l < x->nPolygons; l++) {
                        POLYGONDATA tmppoly;
                        ar.read((char *) &tmppoly, 13);
                        for (int m = 0; m < 4; m++) x->polyData[l].vertex[m] = tmppoly.vertex[m ^ 1];
                        memcpy(&(x->polyData[l].texture), &(tmppoly.texture), 5);
                        x->polyData[l].unknown2 = 0xF9; //Nappe1: Fixed AnimData load. Didn't work??
                        // what on earth for these Unknown2 definitions are used for internal checkings?
                        //Now it doesn't crash, but doesn't Imply Necromancers definitions... wierd...
                    }
                }
            }
        }
    }

    int j = 4 * track->nBlocks; //Global Objects
    ar.read((char *) &track->xobj[j], 4);
    if (track->xobj[j].nobj > 0) {
        track->xobj[j].obj = new XOBJDATA[track->xobj[j].nobj]();
        for (int k = 0; k < track->xobj[j].nobj; k++) {
            XOBJDATA *x = &(track->xobj[j].obj[k]);
            // 3 headers == 12 bytes
            ar.read((char *) x, 12);
            if ((x->crosstype == 4) || (x->crosstype == 2) || (x->crosstype == 1)) // basic objects + crosstype 1
            { ar.read((char *) &(x->ptRef), 12); }
            else if (x->crosstype == 3) { // animated objects
                // unkn3 instead of ptRef
                ar.read((char *) x->unknown3, 12);
            } else return false; // unknown object type
            /*if (p->obj[j&3].nobj!=0)
                p->obj[j&3].types[p->obj[j&3].nobj+k]=x->crosstype;*/
            // common part : vertices & polygons
            ar.read((char *) &(x->AnimMemory), 4);
            ar.read((char *) ptrspace, 4);
            ar.read((char *) &(x->nVertices), 4);
            ar.read((char *) ptrspace, 8);
            x->vert = new FLOATPT[12 * x->nVertices]();
            x->unknVertices = new uint32_t[4 * x->nVertices]();
            ar.read((char *) &(x->nPolygons), 4);
            ar.read((char *) ptrspace, 4);
            x->polyData = new POLYGONDATA[x->nPolygons * 14]();
            if (x->polyData == NULL) return false;
        }
        // now the xobjdata
        for (int k = 0; k < track->xobj[j].nobj; k++) {
            XOBJDATA *x = &(track->xobj[j].obj[k]);
            if (x->crosstype == 3) { // animated-specific header
                ar.read((char *) x->unknown3 + 6, 2);
                // if (x->unknown3[6]!=4) return false;  // fails
                // type3, objno, animLength, unknown4
                ar.read((char *) &(x->type3), 6);
                if (x->type3 != 3) return false;
                x->animData = new ANIMDATA[20 * x->nAnimLength]();
                ar.read((char *) x->animData, 20 * x->nAnimLength);
                // make a ref point from first anim position
                x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
            }
// appears in REFPOLYOBJ & REFXOBJ but not in XOBJs !
/*				if (x->crosstype==6) { // object with byte data
					x->hs_type6=(char *)malloc(x->unknown2);
					if ((long)ar.read((char*)x->hs_type6,x->unknown2)!=x->unknown2) return false;
				}
*/               ar.read((char *) x->vert, 12 * x->nVertices);
            ar.read((char *) x->unknVertices, 4 * x->nVertices);
            for (int l = 0; l < x->nPolygons; l++) {
                POLYGONDATA tmppoly;
                ar.read((char *) &tmppoly, 13);
                for (int m = 0; m < 4; m++) x->polyData[l].vertex[m] = tmppoly.vertex[m ^ 1];
                memcpy(&(x->polyData[l].texture), &(tmppoly.texture), 5);
                x->polyData[l].unknown2 = 0xF9; //Nappe1: Fixed AnimData load. Didn't work??
                // what on earth for these Unknown2 definitions are used for internal checkings?
                //Now it doesn't crash, but doesn't Imply Necromancers definitions... wierd...
            }
        }
    }


    // remainder of the FRD file
    /*char * hs_morexobj = (char *) malloc(100000); // no more than 100K, I hope !
    long hs_morexobjlen = ar.read((char*)hs_morexobj, 100000);
    if (hs_morexobjlen == 100000) return false;
    hs_morexobj = (char *) realloc(hs_morexobj, hs_morexobjlen);*/

    // TEXTUREBLOCKs
    int m = 0;
    for (int i = 0; i < track->nBlocks; i++) {
        for (j = 0; j < 7; j++) {
            LPPOLYGONDATA pp;
            int k;
            for (k = 0, pp = track->poly[i].poly[j]; k < track->poly[i].sz[j]; k++, pp++)
                if (pp->texture > m) m = pp->texture;
        }
        for (j = 0; j < 4; j++) {
            for (int k = 0; k < track->poly[i].obj[j].nobj; k++) {
                LPPOLYGONDATA pp;
                int l;
                for (l = 0, pp = track->poly[i].obj[j].poly[k]; l < track->poly[i].obj[j].numpoly[k]; l++, pp++)
                    if (pp->texture > m) m = pp->texture;
            }
        }
    }
    for (int i = 0; i < 4 * track->nBlocks; i++) {
        for (j = 0; j < track->xobj[i].nobj; j++) {
            LPPOLYGONDATA pp;
            int k;
            for (k = 0, pp = track->xobj[i].obj[j].polyData; k < track->xobj[i].obj[j].nPolygons; k++, pp++)
                if (pp->texture > m) m = pp->texture;
        }
    }
    //HOO: This is changed because some pStockBitmap can not be seen (3)
    track->nTextures = m += 15;
    //HOO: (3)
    track->texture = new TEXTUREBLOCK[m]();
    for (int i = 0; i < m; i++) {
        track->texture[i].width = 16;
        track->texture[i].height = 16; // WHY ?????
        track->texture[i].corners[2] = 1.0; // (1,0)
        track->texture[i].corners[4] = 1.0; // (1,1)
        track->texture[i].corners[5] = 1.0;
        track->texture[i].corners[7] = 1.0; // (0,1)
        track->texture[i].texture = i & 0x7FF;  // ANYWAY WE CAN'T FIND IT !
        track->texture[i].islane = i >> 11;
        track->textures[track->texture[i].texture] = LoadTexture(track->texture[i], track_name);
    }

    //CorrectVirtualRoad();
    return true;
}


std::vector<glm::vec2> highStakesToGLUV(POLYGONDATA poly) {
    glm::vec2 converted_uvs[6] = {};
    int map[4], hold;
    //(flags>>2)&3 indicates the multiple of 90° by which the
    //texture should be rotated (0 for no rotation, 1 for 90°,
    //2 for 180°, 3 for 270°) ; a non-zero value of flags&0x10
    //indicates that the texture is horizontally flipped ; a
    //non-zero value of flags&0x20 indicates that the texture
    //is vertically flipped. The value of (flags>>6)&7 indicates
    //the scaling factor : 0 is no scaling ; 1 means that the
    //texture is tiled twice horizontally ; 2 that the texture
    //is tile twice vertically ; 3 indicates 4x horizontal
    //tiling, 4 indicates 4x vertical tiling. Finally, a non-zero
    //value of flags&0x8000 indicates that the polygon is one-sided.
    //ox, oy, and oz :: Origin of the wrap.
    //dx, dy, and dz :: The z-axis of the wrap.
    //ux, uy, and uz ::	The y-axis of the wrap.
    //ou and ov :: Origin in the texture.
    //su and sv :: Scale factor in the texture
    float oi[2], oo[2], ii[2], io[2];
    switch ((poly.hs_texflags >> 2) & 3) {
        case 0:
            map[0] = 0;
            map[1] = 1;
            map[2] = 2;
            map[3] = 3;
            break;
        case 1:
            map[0] = 3;
            map[1] = 0;
            map[2] = 1;
            map[3] = 2;
            break;
        case 2:
            map[0] = 2;
            map[1] = 3;
            map[2] = 0;
            map[3] = 1;
            break;
        case 3:
            map[0] = 1;
            map[1] = 2;
            map[2] = 3;
            map[3] = 0;
            break;
    }
    switch ((poly.hs_texflags >> 4) & 3) {
        case 1:
            hold = map[0];
            map[0] = map[1];
            map[1] = hold;
            hold = map[2];
            map[2] = map[3];
            map[3] = hold;
            break;
        case 2:
            hold = map[0];
            map[0] = map[3];
            map[3] = hold;
            hold = map[2];
            map[2] = map[1];
            map[1] = hold;
            break;
        case 3:
            hold = map[1];
            map[1] = map[3];
            map[3] = hold;
            hold = map[2];
            map[2] = map[0];
            map[0] = hold;
            break;
    }
    // Scale Factor
    switch ((poly.hs_texflags >> 6) & 7) {
        case 0:
            oi[1] = 1.0f;
            ii[0] = 1.0f;
            ii[1] = 1.0f;
            io[0] = 1.0f;
            break;
        case 1:
            oi[1] = 1.0f;
            ii[0] = 0.5f;
            ii[1] = 1.0f;
            io[0] = 0.5f;
            break;
        case 2:
            oi[1] = 0.5f;
            ii[0] = 1.0f;
            ii[1] = 0.5f;
            io[0] = 1.0f;
            break;
        case 3:
            oi[1] = 1.0f;
            ii[0] = 0.25f;
            ii[1] = 1.0f;
            io[0] = 0.25f;
            break;
        case 4:
            oi[1] = 0.25f;
            ii[0] = 1.0f;
            ii[1] = 0.25f;
            io[0] = 1.0f;
            break;
    }
    oi[0] = 0.0f;
    io[1] = 0.0f;
    oo[0] = 0.0f;
    oo[1] = 0.0f;

    converted_uvs[map[0]] = glm::vec2(oi[0], oi[1]);
    converted_uvs[map[1]] = glm::vec2(ii[0], ii[1]);
    converted_uvs[map[2]] = glm::vec2(io[0], io[1]);
    converted_uvs[map[0]] = glm::vec2(oi[0], oi[1]);
    converted_uvs[map[2]] = glm::vec2(io[0], io[1]);
    converted_uvs[map[3]] = glm::vec2(oo[0], oo[1]);

    return std::vector<glm::vec2>(converted_uvs, converted_uvs + 6);
}

std::vector<TrackBlock> NFS4_Loader::ParseTRKModels() {
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

        for (int s = 0; s < trk_block.nSoundsrc; s++) {
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
                        minimal_texture_ids_set.insert(object_polys[p].texture);
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

                        std::vector<glm::vec2> converted_uvs = highStakesToGLUV(object_polys[p]);
                        uvs.insert(uvs.end(), converted_uvs.begin(), converted_uvs.end());

                        // Use TextureID in place of normal
                        texture_indices.emplace_back(object_polys[p].texture);
                        texture_indices.emplace_back(object_polys[p].texture);
                        texture_indices.emplace_back(object_polys[p].texture);
                        texture_indices.emplace_back(object_polys[p].texture);
                        texture_indices.emplace_back(object_polys[p].texture);
                        texture_indices.emplace_back(object_polys[p].texture);
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
                for (int k = 0; k < x->nPolygons; k++) {
                    POLYGONDATA poly = x->polyData[k];
                    TEXTUREBLOCK texture_for_block = track->texture[x->polyData->texture];
                    minimal_texture_ids_set.insert(poly.texture);
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
                    vertex_indices.emplace_back(poly.vertex[0]);
                    vertex_indices.emplace_back(poly.vertex[1]);
                    vertex_indices.emplace_back(poly.vertex[2]);
                    vertex_indices.emplace_back(poly.vertex[0]);
                    vertex_indices.emplace_back(poly.vertex[2]);
                    vertex_indices.emplace_back(poly.vertex[3]);

                    std::vector<glm::vec2> converted_uvs = highStakesToGLUV(poly);
                    uvs.insert(uvs.end(), converted_uvs.begin(), converted_uvs.end());

                    texture_indices.emplace_back(poly.texture);
                    texture_indices.emplace_back(poly.texture);
                    texture_indices.emplace_back(poly.texture);
                    texture_indices.emplace_back(poly.texture);
                    texture_indices.emplace_back(poly.texture);
                    texture_indices.emplace_back(poly.texture);
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
            if ((chnk == 6) && (trk_block.nVertices <= trk_block.nHiResVert))
                continue;
            LPPOLYGONDATA poly_chunk = polygon_block.poly[chnk];
            for (int k = 0; k < polygon_block.sz[chnk]; k++) {
                //TEXTUREBLOCK texture_for_block = track->texture[poly_chunk[k].texture];
                minimal_texture_ids_set.insert(poly_chunk[k].texture);
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

                std::vector<glm::vec2> converted_uvs = highStakesToGLUV(poly_chunk[k]);
                uvs.insert(uvs.end(), converted_uvs.begin(), converted_uvs.end());

                texture_indices.emplace_back(poly_chunk[k].texture);
                texture_indices.emplace_back(poly_chunk[k].texture);
                texture_indices.emplace_back(poly_chunk[k].texture);
                texture_indices.emplace_back(poly_chunk[k].texture);
                texture_indices.emplace_back(poly_chunk[k].texture);
                texture_indices.emplace_back(poly_chunk[k].texture);
            }
            // Get ordered list of unique texture id's present in block
            std::vector<short> texture_ids = RemapTextureIDs(minimal_texture_ids_set, texture_indices);
            Track current_trk_block_model = Track("TrkBlock", i, verts, uvs, texture_indices, vertex_indices,
                                                  texture_ids,
                                                  trk_block_shading_verts,
                                                  trk_block_center);
            current_trk_block_model.enable();
            if (chnk != 6)
                current_track_block.track.emplace_back(current_trk_block_model);
            current_track_block.objects.emplace_back(current_trk_block_model);
        }
        track_blocks.emplace_back(current_track_block);
    }
    return track_blocks;
}

Texture NFS4_Loader::LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name) {
    std::stringstream filename;
    std::stringstream filename_alpha;

    if (track_texture.islane) {
        int texNum = track_texture.texture + 9;
        /*if(track_texture.texture < 14){
            if (track_texture.texture > 9)
            {
                texNum = nData-1;
            } else {
                texNum = track_texture.texture+nData-11;
            }
        }*/
        filename << "../resources/sfx/" << setfill('0') << setw(4) << texNum << ".BMP";
        filename_alpha << "../resources/sfx/" << setfill('0') << setw(4) << texNum << "-a.BMP";
    } else {
        filename << TRACK_PATH << "NFS4/" << track_name << "/textures/" << setfill('0') << setw(4) << track_texture.texture << ".BMP";
        filename_alpha << TRACK_PATH << "NFS4/" << track_name << "/textures/" << setfill('0') << setw(4) << track_texture.texture << "-a.BMP";
    }

    // Width and height data isn't set properly in FRD loader so deduce from bmp
    GLubyte *data;
    GLsizei width;
    GLsizei height;

    if (!Utils::LoadBmpCustomAlpha(filename.str().c_str(), &data, &width, &height, 0)) {
        std::cerr << "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!" << std::endl;
        // If the texture is missing, load a "MISSING" texture of identical size.
        ASSERT(LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", &data, width, height), "Even the 'missing' texture is missing!");
        return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(track_texture.width), static_cast<unsigned int>(track_texture.height));
    }

    return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}