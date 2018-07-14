//
// Created by Amrik on 03/07/2018.
//

#include "nfs4_loader.h"
#include "trk_loader.h"


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
    //track->track_blocks = ParseTRKModels();
    //std::vector<Track> col_models = ParseCOLModels();
    //track->track_blocks[0].objects.insert(track->track_blocks[0].objects.end(), col_models.begin(), col_models.end()); // Insert the COL models into track block 0 for now

    std::cout << "Successful track load!" << std::endl;
}

bool NFS4_Loader::LoadFCE(const std::string fce_path) {
    std::cout << "- Parsing FCE File: " << fce_path << std::endl;
    ifstream fce(fce_path, ios::in | ios::binary);

    auto *fceHeader = new NFS4::FCE::HEADER();
    fce.read((char *) fceHeader, sizeof(NFS4::FCE::HEADER));

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

        auto *partVertices = new NFS4::FCE::VECTOR[fceHeader->partNumVertices[part_Idx]];
        auto *partNormals = new NFS4::FCE::VECTOR[fceHeader->partNumVertices[part_Idx]];
        auto *partTriangles = new NFS4::FCE::TRIANGLE[fceHeader->partNumTriangles[part_Idx]];

        fce.seekg(sizeof(NFS4::FCE::HEADER) + fceHeader->vertTblOffset +
                  (fceHeader->partFirstVertIndices[part_Idx] * sizeof(NFS4::FCE::VECTOR)), ios_base::beg);
        fce.read((char *) partVertices, fceHeader->partNumVertices[part_Idx] * sizeof(NFS4::FCE::VECTOR));
        for (int vert_Idx = 0; vert_Idx < fceHeader->partNumVertices[part_Idx]; ++vert_Idx) {
            vertices.emplace_back(
                    glm::vec3(partVertices[vert_Idx].x, partVertices[vert_Idx].y, partVertices[vert_Idx].z));
        }

        fce.seekg(sizeof(NFS4::FCE::HEADER) + fceHeader->normTblOffset +
                  (fceHeader->partFirstVertIndices[part_Idx] * sizeof(NFS4::FCE::VECTOR)), ios_base::beg);
        fce.read((char *) partNormals, fceHeader->partNumVertices[part_Idx] * sizeof(NFS4::FCE::VECTOR));
        for (int normal_Idx = 0; normal_Idx < fceHeader->partNumVertices[part_Idx]; ++normal_Idx) {
            normals.emplace_back(
                    glm::vec3(partNormals[normal_Idx].x, partNormals[normal_Idx].y, partNormals[normal_Idx].z));
        }

        fce.seekg(sizeof(NFS4::FCE::HEADER) + fceHeader->triTblOffset +
                  (fceHeader->partFirstTriIndices[part_Idx] * sizeof(NFS4::FCE::TRIANGLE)), ios_base::beg);
        fce.read((char *) partTriangles, fceHeader->partNumTriangles[part_Idx] * sizeof(NFS4::FCE::TRIANGLE));
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

bool NFS4_Loader::LoadFRD(const std::string frd_path, const std::string &track_name) {
    ifstream ar(frd_path, ios::in | ios::binary);

    int i, j, k, l, m, remn, nPos;
    struct TRKBLOCK *b;
    struct POLYGONBLOCK *p;
    struct XOBJDATA *x;
    struct OBJPOLYBLOCK *o;
    struct COLVROAD *vr;
    struct HS_VROADBLOCK vroadblk;
    struct VROADDATA *v;
    LPPOLYGONDATA pp;
    uint16_t *vno;
    unsigned char ptrspace[44]; // some useless data from HS FRDs
    struct FLOATPT v1, v2, norm;
    unsigned char *belong;
    float len;
    struct POLYGONDATA tmppoly;

    struct COLFILE col;

    char header[28]; /* file header */
    SAFE_READ(ar, header, 28); // header & numblocks
    SAFE_READ(ar, &track->nBlocks, 4);
    track->nBlocks++;
    if ((track->nBlocks < 1) || (track->nBlocks > 500)) return false; // 1st sanity check

    track->trk = new TRKBLOCK[track->nBlocks];
    track->poly = new POLYGONBLOCK[track->nBlocks];
    track->xobj = new XOBJBLOCK[4 * track->nBlocks + 1];

    SAFE_READ(ar, &l, 4); // choose between NFS3 & NFSHS
    if ((l < 0) || (l > 5000)) track->bHSMode = false;
    else if (((l + 7) / 8) == track->nBlocks) track->bHSMode = true;
    else return false; // unknown file type

    nPos = l;

    // emulate the COL file
    memcpy(col.collID, "COLL", 4);
    col.version = 11;
    col.fileLength = 36 * nPos + 48;
    col.nBlocks = 2;
    col.xbTable[0] = 8;
    col.xbTable[1] = 24;
    // fake a texture table with only one entry to please NFS3/T3ED
    col.textureHead.size = 16;
    col.textureHead.xbid = XBID_TEXTUREINFO;
    col.textureHead.nrec = 1;
    col.texture = new COLTEXTUREINFO();
    if (col.texture == NULL) return false;
    memset(col.texture, 0, 8);
    // vroad XB
    col.vroadHead.size = 8 + 36 * nPos;
    col.vroadHead.xbid = XBID_VROAD;
    col.vroadHead.nrec = (uint16_t) nPos;
    col.vroad = new COLVROAD[track->nBlocks * 8];

    col.hs_extra = new uint32_t[7 * nPos];

    for (i = 0, vr = col.vroad; i < nPos; i++, vr++) {
        ar.read((char *) &vroadblk, 84);
        vr->refPt.x = (long) (vroadblk.refPt.x * 65536);
        vr->refPt.z = (long) (vroadblk.refPt.z * 65536);
        vr->refPt.y = (long) (vroadblk.refPt.y * 65536);
        // a wild guess
        vr->unknown = (long) ((vroadblk.unknown2[3] & 0xFFFF) +       // unknownLanes[2]
                              ((vroadblk.unknown2[4] & 0xF) << 16) +    // wallKinds[0]
                              ((vroadblk.unknown2[4] & 0xF00) << 20)); // wallKinds[1]
        if (vroadblk.normal.x >= 1.0) vr->normal.x = 127;
        else vr->normal.x = (signed char) (vroadblk.normal.x * 128);
        if (vroadblk.normal.z >= 1.0) vr->normal.z = 127;
        else vr->normal.z = (signed char) (vroadblk.normal.z * 128);
        if (vroadblk.normal.y >= 1.0) vr->normal.y = 127;
        else vr->normal.y = (signed char) (vroadblk.normal.y * 128);
        if (vroadblk.forward.x >= 1.0) vr->forward.x = 127;
        else vr->forward.x = (signed char) (vroadblk.forward.x * 128);
        if (vroadblk.forward.z >= 1.0) vr->forward.z = 127;
        else vr->forward.z = (signed char) (vroadblk.forward.z * 128);
        if (vroadblk.forward.y >= 1.0) vr->forward.y = 127;
        else vr->forward.y = (signed char) (vroadblk.forward.y * 128);
        if (vroadblk.right.x >= 1.0) vr->right.x = 127;
        else vr->right.x = (signed char) (vroadblk.right.x * 128);
        if (vroadblk.right.z >= 1.0) vr->right.z = 127;
        else vr->right.z = (signed char) (vroadblk.right.z * 128);
        if (vroadblk.right.y >= 1.0) vr->right.y = 127;
        else vr->right.y = (signed char) (vroadblk.right.y * 128);
        vr->leftWall = (long) (vroadblk.leftWall * 65536);
        vr->rightWall = (long) (vroadblk.rightWall * 65536);
        memcpy(col.hs_extra + 7 * i, &(vroadblk.unknown1[0]), 28);
    }

    // TRKBLOCKs
    for (i = 0; i < track->nBlocks; i++) {
        b = &(track->trk[i]);
        p = &(track->poly[i]);
        // 7 track polygon numbers
        ar.read((char *) p->sz, 28);
        memcpy(p->szdup, p->sz, 28);
        // 4 object polygon numbers
        for (j = 0; j < 4; j++) {
            ar.read((char *) &(p->obj[j].n1), 4);
        }
        // pointer space
        ar.read((char *) ptrspace, 44);
        // 6 nVertices
        ar.read((char *) &(b->nVertices), 24);
        if (b->nVertices < 0) {/*||(b->nVertices>1000)*/
            free(col.hs_extra);
            free(col.texture);
            return false;
        }
        // pointer space
        ar.read((char *) ptrspace, 8);
        // ptCentre, ptBounding == 60 bytes
        ar.read((char *) b, 60);
        // nbdData
        ar.read((char *) b->nbdData, 4 * 0x12C);
        // xobj numbers
        for (j = 4 * i; j < 4 * i + 4; j++) {
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
        if (i == 0) b->nStartPos = 0;
        else b->nStartPos = track->trk[i - 1].nStartPos + track->trk[i - 1].nPositions;
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
    for (i = 0; i < track->nBlocks; i++) {
        b = &(track->trk[i]);
        p = &(track->poly[i]);
        // vertices
        b->vert = new FLOATPT[b->nVertices];
        ar.read((char *) b->vert, 12 * b->nVertices);
        b->unknVertices = new uint32_t[b->nVertices];
        ar.read((char *) b->unknVertices, 4 * b->nVertices);
        // polyData is a bit tricky
        b->polyData = new POLYVROADDATA[b->nPolygons];
        memset(b->polyData, 0, b->nPolygons * sizeof(struct POLYVROADDATA));
        b->vroadData = new VROADDATA[b->nPolygons];

        for (j = 0; j < b->nPolygons; j++) {
            b->polyData[j].vroadEntry = j;
            b->polyData[j].flags = 0xE; // not passable
        }
        for (j = 0; j < b->nVRoad; j++) {
            ar.read((char *) ptrspace, 10);
            k = 0;
            ar.read((char *) &k, 2);
            memcpy(b->polyData[k].hs_minmax, ptrspace, 8);
            b->polyData[k].flags = ptrspace[8];
            b->polyData[k].hs_unknown = ptrspace[9];
            if ((ptrspace[8] & 15) == 14) {
                free(col.hs_extra);
                free(col.texture);
                return false;
            }
            ar.read((char *) b->vroadData + k, 12);
        }
        b->nVRoad = b->nPolygons;

        // the 4 misc. tables
        if (b->nXobj > 0) {
            b->xobj = new REFXOBJ[b->nXobj];
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
            b->soundsrc = new SOUNDSRC[b->nSoundsrc];
            ar.read((char *) b->soundsrc, 16 * b->nSoundsrc);
        }
        if (b->nLightsrc > 0) {
            b->lightsrc = new LIGHTSRC[b->nLightsrc];
            ar.read((char *) b->lightsrc, 16 * b->nLightsrc);
        }

        // track polygons
        for (j = 0; j < 7; j++)
            if (p->sz[j] != 0) {
                p->poly[j] = (LPPOLYGONDATA) malloc(p->sz[j] * sizeof(struct POLYGONDATA));
                for (k = 0; k < p->sz[j]; k++) {
                    ar.read((char *) &tmppoly, 13);
                    for (m = 0; m < 4; m++) p->poly[j][k].vertex[m] = tmppoly.vertex[m ^ 1];
                    memcpy(&(p->poly[j][k].texture), &(tmppoly.texture), 5);
                    p->poly[j][k].unknown2 = 0xF9; //Nappe1: fixed for correct animation reading... (originally 0xF9)
                }
            }

        // make up some fake posData
        b->posData = new POSITIONDATA[b->nPositions];
        k = 0;
        pp = p->poly[4];
        for (j = 0; j < b->nPositions; j++) {
            b->posData[j].polygon = k;
            b->posData[j].unknown = 0;
            b->posData[j].extraNeighbor1 = -1;
            b->posData[j].extraNeighbor2 = -1;
            do {
                l = 0;
                do {
                    if ((b->polyData[k].flags & 0x0f) % 14) l++;
                    k++;
                    pp++;
                } while ((k < b->nPolygons) && (pp->vertex[0] == (pp - 1)->vertex[1]) &&
                         (pp->vertex[3] == (pp - 1)->vertex[2]));
                if (((j == b->nPositions - 1) && (k < b->nPolygons)) || (k > b->nPolygons)) {
//					MessageBeep(MB_ICONEXCLAMATION);
//					afxDump << "failed " << i << " : " << k << " instead of " << b->nPolygons << "\n";
//					for (l=0;l<b->nPositions;l++) afxDump << b->posData[l].polygon << " ";
//					afxDump << "\n";
                    k = b->nPolygons;
                }
            } while ((l == 0) && (k < b->nPolygons));
            b->posData[j].nPolygons = k - b->posData[j].polygon;
        }

        // still vroadData is missing for unpassable polygons
        for (j = 0; j < b->nPolygons; j++)
            if (b->polyData[j].flags == 0xE) {
                v = b->vroadData + j;
                vno = p->poly[4][j].vertex;
                v1.x = b->vert[vno[1]].x - b->vert[vno[3]].x;
                v1.z = b->vert[vno[1]].z - b->vert[vno[3]].z;
                v1.y = b->vert[vno[1]].y - b->vert[vno[3]].y;
                v2.x = b->vert[vno[2]].x - b->vert[vno[0]].x;
                v2.z = b->vert[vno[2]].z - b->vert[vno[0]].z;
                v2.y = b->vert[vno[2]].y - b->vert[vno[0]].y;
                norm.x = -v1.y * v2.z + v1.z * v2.y;
                norm.y = -v1.z * v2.x + v1.x * v2.z;
                norm.z = -v1.x * v2.y + v1.y * v2.x;
                len = (float) sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
                v->xNorm = (uint16_t) (norm.x * 32767 / len);
                v->zNorm = (uint16_t) (norm.z * 32767 / len);
                v->yNorm = (uint16_t) (norm.y * 32767 / len);
                v1.x = (float) col.vroad[b->nStartPos].forward.x;
                v1.z = (float) col.vroad[b->nStartPos].forward.z;
                v1.y = (float) col.vroad[b->nStartPos].forward.y;
                len = (float) sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
                v->xForw = (uint16_t) (v1.x * 32767 / len);
                v->zForw = (uint16_t) (v1.z * 32767 / len);
                v->yForw = (uint16_t) (v1.y * 32767 / len);
            }

        // POLYGONBLOCK OBJECTS
        o = p->obj;
        belong = (unsigned char *) malloc(b->nObjectVert);
        for (j = 0; j < 4; j++, o++) {
            if (o->n1 > 0) {
                memset(belong, 0xFF, b->nObjectVert);
                pp = (LPPOLYGONDATA) malloc(14 * o->n1);
                for (k = 0; k < o->n1; k++) {
                    ar.read((char *) &tmppoly, 13);
                    for (m = 0; m < 4; m++) pp[k].vertex[m] = tmppoly.vertex[m ^ 1];
                    memcpy(&(pp[k].texture), &(tmppoly.texture), 5);
                    pp[k].unknown2 = 0xFF; // will temporarily store object's #
                    //Nappe1: Destroys AnimData??! ah... it sets it to 0xF9 later... fixing There...

                }
                remn = o->n1;
                o->nobj = 0;
                while (remn > 0) {  // there are still polygons to be connected
                    k = 0;
                    while (pp[k].unknown2 != 0xFF) k++;
                    pp[k].unknown2 = (unsigned char) o->nobj;
                    remn--;
                    for (l = 0; l < 4; l++) belong[pp[k].vertex[l]] = (unsigned char) o->nobj;
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
                o->n2 = o->nobj + track->xobj[4 * i + j].nobj;
                o->types = new uint32_t[o->n2];
                o->numpoly = new uint32_t[o->nobj];
                o->poly = new LPPOLYGONDATA[4 * o->nobj];
                for (l = 0; l < o->nobj; l++) {
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
        for (j = 4 * i; j < 4 * i + 4; j++) {
            if (track->xobj[j].nobj > 0) {
                track->xobj[j].obj = new XOBJDATA[track->xobj[j].nobj];
                for (k = 0; k < track->xobj[j].nobj; k++) {
                    x = &(track->xobj[j].obj[k]);
                    // 3 headers == 12 bytes
                    ar.read((char *) x, 12);
                    if ((x->crosstype == 4) || (x->crosstype == 2)) // basic objects
                    { ar.read((char *) &(x->ptRef), 12); }
                    else if (x->crosstype == 3) { // animated objects
                        // unkn3 instead of ptRef
                        ar.read((char *) x->unknown3, 12);
                    } else return false; // unknown object type
                    if (p->obj[j & 3].nobj != 0)
                        p->obj[j & 3].types[p->obj[j & 3].nobj + k] = x->crosstype;
                    // common part : vertices & polygons
                    ar.read((char *) &(x->AnimMemory), 4);
                    ar.read((char *) ptrspace, 4);
                    ar.read((char *) &(x->nVertices), 4);
                    ar.read((char *) ptrspace, 8);
                    x->vert = new FLOATPT[12 * x->nVertices];
                    x->unknVertices = new uint32_t[4 * x->nVertices];
                    ar.read((char *) &(x->nPolygons), 4);
                    ar.read((char *) ptrspace, 4);
                    x->polyData = new POLYGONDATA[x->nPolygons * 14];
                }
                // now the xobjdata
                for (k = 0; k < track->xobj[j].nobj; k++) {
                    x = &(track->xobj[j].obj[k]);
                    if (x->crosstype == 3) { // animated-specific header
                        ar.read((char *) x->unknown3 + 6, 2);
                        // if (x->unknown3[6]!=4) return false;  // fails
                        // type3, objno, animLength, unknown4
                        ar.read((char *) &(x->type3), 6);
                        if (x->type3 != 3) return false;
                        x->animData = new ANIMDATA[20 * x->nAnimLength];
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
                    for (l = 0; l < x->nPolygons; l++) {
                        ar.read((char *) &tmppoly, 13);
                        for (m = 0; m < 4; m++) x->polyData[l].vertex[m] = tmppoly.vertex[m ^ 1];
                        memcpy(&(x->polyData[l].texture), &(tmppoly.texture), 5);
                        x->polyData[l].unknown2 = 0xF9; //Nappe1: Fixed AnimData load. Didn't work??
                        // what on earth for these Unknown2 definitions are used for internal checkings?
                        //Now it doesn't crash, but doesn't Imply Necromancers definitions... wierd...
                    }
                }
            }
        }
    }

    j = 4 * track->nBlocks; //Global Objects
    ar.read((char *) &track->xobj[j], 4);
    if (track->xobj[j].nobj > 0) {
        track->xobj[j].obj = new XOBJDATA[track->xobj[j].nobj];
        for (k = 0; k < track->xobj[j].nobj; k++) {
            x = &(track->xobj[j].obj[k]);
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
            x->vert = new FLOATPT[12 * x->nVertices];
            x->unknVertices = new uint32_t[4 * x->nVertices];
            ar.read((char *) &(x->nPolygons), 4);
            ar.read((char *) ptrspace, 4);
            x->polyData = new POLYGONDATA[x->nPolygons * 14];
            if (x->polyData == NULL) return false;
        }
        // now the xobjdata
        for (k = 0; k < track->xobj[j].nobj; k++) {
            x = &(track->xobj[j].obj[k]);
            if (x->crosstype == 3) { // animated-specific header
                ar.read((char *) x->unknown3 + 6, 2);
                // if (x->unknown3[6]!=4) return false;  // fails
                // type3, objno, animLength, unknown4
                ar.read((char *) &(x->type3), 6);
                if (x->type3 != 3) return false;
                x->animData = new ANIMDATA[20 * x->nAnimLength];
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
            for (l = 0; l < x->nPolygons; l++) {
                ar.read((char *) &tmppoly, 13);
                for (m = 0; m < 4; m++) x->polyData[l].vertex[m] = tmppoly.vertex[m ^ 1];
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
    m = 0;
    for (i = 0; i < track->nBlocks; i++) {
        for (j = 0; j < 7; j++)
            for (k = 0, pp = track->poly[i].poly[j]; k < track->poly[i].sz[j]; k++, pp++)
                if (pp->texture > m) m = pp->texture;
        for (j = 0; j < 4; j++)
            for (k = 0; k < track->poly[i].obj[j].nobj; k++)
                for (l = 0, pp = track->poly[i].obj[j].poly[k]; l < track->poly[i].obj[j].numpoly[k]; l++, pp++)
                    if (pp->texture > m) m = pp->texture;
    }
    for (i = 0; i < 4 * track->nBlocks; i++)
        for (j = 0; j < track->xobj[i].nobj; j++)
            for (k = 0, pp = track->xobj[i].obj[j].polyData; k < track->xobj[i].obj[j].nPolygons; k++, pp++)
                if (pp->texture > m) m = pp->texture;
    //HOO: This is changed because some pStockBitmap can not be seen (3)
    track->nTextures = m += 15;
    //HOO: (3)
    track->texture = (struct TEXTUREBLOCK *) malloc(m * sizeof(struct TEXTUREBLOCK));
    for (i = 0; i < m; i++) {
        track->texture[i].width = 16;
        track->texture[i].height = 16; // WHY ?????
        track->texture[i].corners[2] = 1.0; // (1,0)
        track->texture[i].corners[4] = 1.0; // (1,1)
        track->texture[i].corners[5] = 1.0;
        track->texture[i].corners[7] = 1.0; // (0,1)
        track->texture[i].texture = i & 0x7FF;  // ANYWAY WE CAN'T FIND IT !
        track->texture[i].islane = i >> 11;
    }

    //CorrectVirtualRoad();
    return true;
}