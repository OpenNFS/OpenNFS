#include "NFS3Loader.h"
#include "../Util/Raytracer.h"

using namespace Utils;
using namespace TrackUtils;

// CAR
std::shared_ptr<Car> NFS3::LoadCar(const std::string &car_base_path)
{
    boost::filesystem::path p(car_base_path);
    std::string car_name = p.filename().string();

    std::stringstream viv_path, car_out_path, fce_path;
    viv_path << car_base_path << "/car.viv";
    car_out_path << CAR_PATH << ToString(NFS_3) << "/" << car_name << "/";
    fce_path << CAR_PATH << ToString(NFS_3) << "/" << car_name << "/car.fce";

    ASSERT(ExtractVIV(viv_path.str(), car_out_path.str()),
           "Unable to extract " << viv_path.str() << " to " << car_out_path.str());

    return std::make_shared<Car>(LoadFCE(fce_path.str()), NFS_3, car_name);
}

CarData NFS3::LoadFCE(const std::string &fce_path)
{
    LOG(INFO) << "Parsing FCE File located at " << fce_path;
    glm::quat rotationMatrix = glm::normalize(
            glm::quat(glm::vec3(glm::radians(90.f), 0, glm::radians(180.f)))); // All Vertices are stored so that the model is rotated 90 degs on X, 180 on Z. Remove this at Vert load time.

    CarData carData;
    std::ifstream fce(fce_path, std::ios::in | std::ios::binary);

    auto fceHeader = std::make_shared<FCE::NFS3::HEADER>();
    fce.read((char *) fceHeader.get(), sizeof(FCE::NFS3::HEADER));

    // Grab colours
    for (uint8_t colourIdx = 0; colourIdx < fceHeader->nPriColours; ++colourIdx)
    {
        FCE::NFS3::COLOUR primaryColour = fceHeader->primaryColours[colourIdx];
        CarColour originalPrimaryColour("", HSLToRGB(glm::vec4(primaryColour.H, primaryColour.S, primaryColour.B, primaryColour.T)));
        carData.colours.emplace_back(originalPrimaryColour);
    }

    for (uint32_t dummyIdx = 0; dummyIdx < fceHeader->nDummies; ++dummyIdx)
    {
        Dummy dummy(fceHeader->dummyNames[dummyIdx],
                    rotationMatrix * glm::vec3(fceHeader->dummyCoords[dummyIdx].x / 10, fceHeader->dummyCoords[dummyIdx].y / 10, fceHeader->dummyCoords[dummyIdx].z / 10));
        carData.dummies.emplace_back(dummy);
    }

    for (uint32_t part_Idx = 0; part_Idx < fceHeader->nParts; ++part_Idx)
    {
        float specularDamper = 0.2f;
        float specularReflectivity = 0.02f;
        float envReflectivity = 0.4f;

        std::vector<uint32_t> indices;
        std::vector<uint32_t> polygonFlags;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::string part_name(fceHeader->partNames[part_Idx]);
        glm::vec3 center = rotationMatrix *
                           glm::vec3(fceHeader->partCoords[part_Idx].x / 10, fceHeader->partCoords[part_Idx].y / 10,
                                     fceHeader->partCoords[part_Idx].z / 10);

        auto *partVertices = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partNormals = new FLOATPT[fceHeader->partNumVertices[part_Idx]];
        auto *partTriangles = new FCE::TRIANGLE[fceHeader->partNumTriangles[part_Idx]];

        fce.seekg(sizeof(FCE::NFS3::HEADER) + fceHeader->vertTblOffset +
                  (fceHeader->partFirstVertIndices[part_Idx] * sizeof(FLOATPT)), std::ios_base::beg);
        fce.read((char *) partVertices, fceHeader->partNumVertices[part_Idx] * sizeof(FLOATPT));
        for (uint32_t vert_Idx = 0; vert_Idx < fceHeader->partNumVertices[part_Idx]; ++vert_Idx)
        {
            vertices.emplace_back(rotationMatrix *
                                  glm::vec3(partVertices[vert_Idx].x / 10, partVertices[vert_Idx].y / 10,
                                            partVertices[vert_Idx].z / 10));
        }

        fce.seekg(sizeof(FCE::NFS3::HEADER) + fceHeader->normTblOffset +
                  (fceHeader->partFirstVertIndices[part_Idx] * sizeof(FLOATPT)), std::ios_base::beg);
        fce.read((char *) partNormals, fceHeader->partNumVertices[part_Idx] * sizeof(FLOATPT));
        for (uint32_t normal_Idx = 0; normal_Idx < fceHeader->partNumVertices[part_Idx]; ++normal_Idx)
        {
            normals.emplace_back(rotationMatrix * glm::vec3(partNormals[normal_Idx].x, partNormals[normal_Idx].y,
                                                            partNormals[normal_Idx].z));
        }

        fce.seekg(sizeof(FCE::NFS3::HEADER) + fceHeader->triTblOffset +
                  (fceHeader->partFirstTriIndices[part_Idx] * sizeof(FCE::TRIANGLE)), std::ios_base::beg);
        fce.read((char *) partTriangles, fceHeader->partNumTriangles[part_Idx] * sizeof(FCE::TRIANGLE));
        for (uint32_t tri_Idx = 0; tri_Idx < fceHeader->partNumTriangles[part_Idx]; ++tri_Idx)
        {
            polygonFlags.emplace_back(partTriangles[tri_Idx].polygonFlags);
            polygonFlags.emplace_back(partTriangles[tri_Idx].polygonFlags);
            polygonFlags.emplace_back(partTriangles[tri_Idx].polygonFlags);
            indices.emplace_back(partTriangles[tri_Idx].vertex[0]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[1]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[2]);
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[0], 1.0f - partTriangles[tri_Idx].uvTable[3]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[1], 1.0f - partTriangles[tri_Idx].uvTable[4]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[2], 1.0f - partTriangles[tri_Idx].uvTable[5]));
        }

        carData.meshes.emplace_back(CarModel(part_name, vertices, uvs, normals, indices, polygonFlags, center, specularDamper, specularReflectivity, envReflectivity));
        LOG(INFO) << "Loaded Mesh: " << carData.meshes[part_Idx].m_name << " UVs: " << carData.meshes[part_Idx].m_uvs.size()
                  << " Verts: " << carData.meshes[part_Idx].m_vertices.size() << " Indices: "
                  << carData.meshes[part_Idx].m_vertex_indices.size() << " Normals: " << carData.meshes[part_Idx].m_normals.size();

        delete[] partNormals;
        delete[] partVertices;
        delete[] partTriangles;
    }
    fce.close();

    // Go get car metadata from FEDATA
    boost::filesystem::path fcePath(fce_path);
    boost::filesystem::path fceBaseDir = fcePath.parent_path();

    std::ifstream fedata(fceBaseDir.string() + "/fedata.eng", std::ios::in | std::ios::binary);
    // Go get the offset of car name
    fedata.seekg(FEDATA::NFS3::MENU_NAME_FILEPOS_OFFSET, std::ios::beg);
    uint32_t menuNameOffset = 0;
    fedata.read((char *) &menuNameOffset, sizeof(uint32_t));
    fedata.seekg(menuNameOffset, std::ios::beg);
    char carMenuName[64];
    fedata.read((char *) carMenuName, 64u);
    std::string carMenuNameStr(carMenuName);
    carData.carName = carMenuNameStr;


    // Jump to location of FILEPOS table for car colour names
    fedata.seekg(FEDATA::NFS3::COLOUR_TABLE_OFFSET, std::ios::beg);
    // Read that table in
    auto *colourNameOffsets = new uint32_t[fceHeader->nPriColours];
    fedata.read((char *) colourNameOffsets, fceHeader->nPriColours * sizeof(uint32_t));

    for (uint8_t colourIdx = 0; colourIdx < fceHeader->nPriColours; ++colourIdx)
    {
        fedata.seekg(colourNameOffsets[colourIdx]);
        uint32_t colourNameLength = colourIdx < (fceHeader->nPriColours - 1) ? (colourNameOffsets[colourIdx + 1] - colourNameOffsets[colourIdx]) : 32;
        char *colourName = new char[colourNameLength];
        fedata.read((char *) colourName, colourNameLength);
        std::string colourNameStr(colourName);
        carData.colours[colourIdx].colourName = colourNameStr;
        delete[]colourName;
    }
    delete[]colourNameOffsets;

    return carData;
}

void DumpMeshes(CarData car, const std::string &objFilePath)
{
    // Make the directories if they're not there
    if (!(boost::filesystem::exists(objFilePath)))
    {
        boost::filesystem::create_directories(objFilePath);
    }

    // Open up the obj file for writing
    std::ofstream obj_dump(objFilePath);
    ASSERT(obj_dump.is_open(), "Couldn't open obj file at " << objFilePath);

    // For every part
    for (auto &part : car.meshes)
    {
        // Write the name into the file
        obj_dump << "o " << part.m_name << std::endl;
        // And all of its vertices
        for (auto &vertex : part.m_vertices)
        {
            obj_dump << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
        }
        // You can do the indices spark!
        for (auto &index : part.m_vertex_indices)
        {
            // Same as before, but follow obj format. 'v' should be 'f'
        }
    }
    obj_dump.close();
}

// TRACK
std::shared_ptr<TRACK> NFS3::LoadTrack(const std::string &track_base_path)
{
    LOG(INFO) << "Loading Track located at " << track_base_path;
    auto track = std::make_shared<TRACK>(TRACK());

    boost::filesystem::path p(track_base_path);
    track->name = p.filename().string();
    std::stringstream frd_path, col_path, can_path, hrz_path;
    std::string strip = "k0";
    size_t pos = track->name.find(strip);
    if (pos != std::string::npos)
    {
        track->name.replace(pos, strip.size(), "");
    }

    frd_path << track_base_path << "/" << track->name << ".frd";
    col_path << track_base_path << "/" << track->name << ".col";
    can_path << track_base_path << "/" << track->name << "00a.can";
    hrz_path << track_base_path << "/3" << track->name << ".hrz";

    ASSERT(ExtractTrackTextures(track_base_path, track->name, NFSVer::NFS_3),
           "Could not extract " << track->name << " QFS texture pack.");
    ASSERT(LoadFRD(frd_path.str(), track->name, track),
           "Could not load FRD file: " << frd_path.str()); // Load FRD file to get track block specific data
    ASSERT(LoadCOL(col_path.str(), track), "Could not load COL file: "
            << col_path.str()); // Load Catalogue file to get global (non trkblock specific) data
    ASSERT(LoadCAN(can_path.str(), track->cameraAnimation),
           "Could not load CAN file (camera animation): " << can_path.str()); // Load camera intro/outro animation data
    ASSERT(LoadHRZ(hrz_path.str(), track),
           "Could not load HRZ file (skybox/lighting):" << hrz_path.str()); // Load HRZ Data

    track->textureArrayID = MakeTextureArray(track->textures, false);
    track->track_blocks = ParseTRKModels(track);
    track->global_objects = ParseCOLModels(track);

    LOG(INFO) << "Track loaded successfully";
    return track;
}

bool NFS3::LoadFFN(const std::string &ffn_path)
{
    std::ifstream ffn(ffn_path, std::ios::in | std::ios::binary);

    if (!ffn.is_open())
    {
        return false;
    }

    LOG(INFO) << "Loading FFN File located at " << ffn_path;

    // Get filesize so can check have parsed all bytes
    FFN::HEADER *header = new FFN::HEADER;
    ffn.read((char *) header, sizeof(FFN::HEADER));

    if (memcmp(header->fntfChk, "FNTF", sizeof(header->fntfChk)) != 0)
    {
        LOG(WARNING) << "Invalid FFN Header.";
        delete header;
        return false;
    }

    FFN::CHAR_TABLE_ENTRY *charTable = new FFN::CHAR_TABLE_ENTRY[header->numChars];
    ffn.read((char *) charTable, sizeof(FFN::CHAR_TABLE_ENTRY) * header->numChars);

    uint32_t predictedAFontOffset = header->fontMapOffset;

    for (uint8_t char_Idx = 0; char_Idx < header->numChars; ++char_Idx)
    {
        FFN::CHAR_TABLE_ENTRY character = charTable[char_Idx];
    }
    header->numChars = 400;
    header->version = 164;

    //streamoff readBytes = ffn.tellg();
    //ASSERT(readBytes == header->fileSize, "Missing " << header->fileSize - readBytes << " bytes from loaded FFN file: " << ffn_path);

    ffn.seekg(header->fontMapOffset, std::ios_base::beg);
    uint32_t *pixels = new uint32_t[header->version * header->numChars];
    uint16_t *paletteColours = new uint16_t[0xFF];
    uint8_t *indexes = new uint8_t[header->version * header->numChars]; // Only used if indexed

    for (int pal_Idx = 0; pal_Idx < 255; ++pal_Idx)
    {
        paletteColours[pal_Idx] = 65535;
    }

    for (int y = 0; y < header->numChars; y++)
    {
        for (int x = 0; x < header->version; x++)
        {
            ffn.read((char *) &indexes[(x + y * header->version)], sizeof(uint8_t));
        }
    }

    // Rewrite the pixels using the palette data
    for (int y = 0; y < header->numChars; y++)
    {
        for (int x = 0; x < header->version; x++)
        {
            uint32_t pixel = ImageLoader::abgr1555ToARGB8888(paletteColours[indexes[(x + y * header->version)]]);
            pixels[(x + y * header->version)] = pixel;
        }
    }

    ImageLoader::SaveImage("C:/Users/Amrik/Desktop/test.bmp", pixels, header->version, header->numChars);
    delete[]pixels;

    delete header;

    //ASSERT(readBytes == header->fileSize, "Missing " << header->fileSize - readBytes << " bytes from loaded FFN file: " << ffn_path);
    return true;
}

void NFS3::FreeTrack(const std::shared_ptr<TRACK> &track)
{
    FreeFRD(track);
    FreeCOL(track);
}

bool NFS3::LoadFRD(std::string frd_path, const std::string &track_name, const std::shared_ptr<TRACK> &track)
{
    std::ifstream ar(frd_path, std::ios::in | std::ios::binary);

    char header[28]; /* file header */
    SAFE_READ(ar, header, 28); // header & numblocks
    SAFE_READ(ar, &track->nBlocks, 4);
    track->nBlocks++;
    if ((track->nBlocks < 1) || (track->nBlocks > 500))
    { return false; } // 1st sanity check

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
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++)
    {
        TRKBLOCK *trackBlock = &(track->trk[block_Idx]);
        // ptCentre, ptBounding, 6 nVertices == 84 bytes
        if (block_Idx != 0)
        { SAFE_READ(ar, trackBlock, 84); }
        if (trackBlock->nVertices == 0) return false;
        trackBlock->vert = new FLOATPT[trackBlock->nVertices];

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

        if (trackBlock->nXobj > 0)
        {
            trackBlock->xobj = new REFXOBJ[trackBlock->nXobj];
            SAFE_READ(ar, trackBlock->xobj, 20 * trackBlock->nXobj);
        }
        if (trackBlock->nPolyobj > 0)
        {
            ar.seekg(20 * trackBlock->nPolyobj, std::ios_base::cur);
        }
        trackBlock->nPolyobj = 0;
        if (trackBlock->nSoundsrc > 0)
        {
            trackBlock->soundsrc = new SOUNDSRC[trackBlock->nSoundsrc];
            SAFE_READ(ar, trackBlock->soundsrc, 16 * trackBlock->nSoundsrc);
        }
        if (trackBlock->nLightsrc > 0)
        {
            trackBlock->lightsrc = new LIGHTSRC[trackBlock->nLightsrc];
            SAFE_READ(ar, trackBlock->lightsrc, 16 * trackBlock->nLightsrc);
        }
    }

    // POLYGONBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++)
    {
        POLYGONBLOCK *p = &(track->poly[block_Idx]);
        for (uint32_t j = 0; j < 7; j++)
        {
            SAFE_READ(ar, &(p->sz[j]), 0x4);
            if (p->sz[j] != 0)
            {
                SAFE_READ(ar, &(p->szdup[j]), 0x4);
                if (p->szdup[j] != p->sz[j]) return false;
                p->poly[j] = static_cast<LPPOLYGONDATA>(calloc(p->sz[j], sizeof(POLYGONDATA)));
                SAFE_READ(ar, p->poly[j], 14 * p->sz[j]);
            }
        }
        if (p->sz[4] != track->trk[block_Idx].nPolygons) return false; // sanity check
        for (uint32_t obj_Idx = 0; obj_Idx < 4; obj_Idx++)
        {
            OBJPOLYBLOCK *o = &(p->obj[obj_Idx]);
            SAFE_READ(ar, &(o->n1), 0x4);
            if (o->n1 > 0)
            {
                SAFE_READ(ar, &(o->n2), 0x4);
                o->types = new uint32_t[o->n2];
                o->numpoly = new uint32_t[o->n2];
                o->poly = new LPPOLYGONDATA[o->n2];
                o->nobj = 0;
                l = 0;
                for (uint32_t k = 0; k < o->n2; k++)
                {
                    SAFE_READ(ar, o->types + k, 0x4);
                    if (o->types[k] == 1)
                    {
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
    for (uint32_t xblock_Idx = 0; xblock_Idx <= 4 * track->nBlocks; xblock_Idx++)
    {
        SAFE_READ(ar, &(track->xobj[xblock_Idx].nobj), 4);
        if (track->xobj[xblock_Idx].nobj > 0)
        {
            track->xobj[xblock_Idx].obj = new XOBJDATA[track->xobj[xblock_Idx].nobj];
        }
        for (uint32_t xobj_Idx = 0; xobj_Idx < track->xobj[xblock_Idx].nobj; xobj_Idx++)
        {
            XOBJDATA *x = &(track->xobj[xblock_Idx].obj[xobj_Idx]);
            // 3 headers == 12 bytes
            SAFE_READ(ar, x, 12);
            if (x->crosstype == 4)
            { // basic objects
                SAFE_READ(ar, &(x->ptRef), 12);
                SAFE_READ(ar, &(x->AnimMemory), 4);
            }
            else if (x->crosstype == 3)
            { // animated objects
                // unkn3, type3, objno, nAnimLength, unkn4 == 24 bytes
                SAFE_READ(ar, x->unknown3, 24);
                if (x->type3 != 3) return false;
                x->animData = new ANIMDATA[x->nAnimLength];
                SAFE_READ(ar, x->animData, 20 * x->nAnimLength);
                // make a ref point from first anim position
                x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
            }
            else return false; // unknown object type

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
    for (uint32_t tex_Idx = 0; tex_Idx < track->nTextures; tex_Idx++)
    {
        SAFE_READ(ar, &(track->texture[tex_Idx]), 47);
        track->textures[track->texture[tex_Idx].texture] = LoadTexture(track->texture[tex_Idx], track_name);
    }

    uint32_t pad;
    return ar.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
}

bool NFS3::LoadCOL(std::string col_path, const std::shared_ptr<TRACK> &track)
{
    std::ifstream coll(col_path, std::ios::in | std::ios::binary);

    COLOBJECT *o;

    track->col.hs_extra = NULL;
    if (coll.read((char *) &track->col, 16).gcount() != 16)
    { return false; }
    if (memcmp(track->col.collID, "COLL", sizeof(track->col.collID[0])) != 0)
    {
        LOG(WARNING) << "Invalid COL file";
        return false;
    }

    LOG(INFO) << "Loading COL File located at " << col_path;

    if (track->col.version != 11)
    { return false; }
    if ((track->col.nBlocks != 2) && (track->col.nBlocks != 4) && (track->col.nBlocks != 5))
    { return false; }
    SAFE_READ(coll, track->col.xbTable, 4 * track->col.nBlocks);

    // texture XB
    SAFE_READ(coll, &track->col.textureHead, 8);
    if (track->col.textureHead.xbid != XBID_TEXTUREINFO)
    { return false; }

    track->col.texture = new COLTEXTUREINFO[track->col.textureHead.nrec];
    SAFE_READ(coll, track->col.texture, 8 * track->col.textureHead.nrec);

    // struct3D XB
    if (track->col.nBlocks >= 4)
    {
        SAFE_READ(coll, &track->col.struct3DHead, 8);
        if (track->col.struct3DHead.xbid != XBID_STRUCT3D)
        { return false; }
        COLSTRUCT3D *s = track->col.struct3D = new COLSTRUCT3D[track->col.struct3DHead.nrec];
        int delta;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.struct3DHead.nrec; colRec_Idx++, s++)
        {
            SAFE_READ(coll, s, 8);
            delta = (8 + 16 * s->nVert + 6 * s->nPoly) % 4;
            delta = (4 - delta) % 4;
            if (s->size != 8 + 16 * s->nVert + 6 * s->nPoly + delta)
            { return false; }
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
        {
            return false;
        }
        o = track->col.object = new COLOBJECT[track->col.objectHead.nrec];

        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.objectHead.nrec; colRec_Idx++, o++)
        {
            SAFE_READ(coll, o, 4);
            if (o->type == 1)
            {
                if (o->size != 16)
                { return false; }
                SAFE_READ(coll, &(o->ptRef), 12);
            }
            else if (o->type == 3)
            {
                SAFE_READ(coll, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength)
                { return false; }
                o->animData = new ANIMDATA[o->animLength];
                SAFE_READ(coll, o->animData, 20 * o->animLength);
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            }
            else
            { return false; } // unknown object type
        }
    }

    // object2 XB
    if (track->col.nBlocks == 5)
    {
        SAFE_READ(coll, &track->col.object2Head, 8);
        if ((track->col.object2Head.xbid != XBID_OBJECT) && (track->col.object2Head.xbid != XBID_OBJECT2))
        {
            return false;
        }
        o = track->col.object2 = new COLOBJECT[track->col.object2Head.nrec];

        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.object2Head.nrec; colRec_Idx++, o++)
        {
            SAFE_READ(coll, o, 4);
            if (o->type == 1)
            {
                if (o->size != 16)
                { return false; }
                SAFE_READ(coll, &(o->ptRef), 12);
            }
            else if (o->type == 3)
            {
                SAFE_READ(coll, &(o->animLength), 4);
                if (o->size != 8 + 20 * o->animLength)
                { return false; }
                o->animData = new ANIMDATA[o->animLength];
                SAFE_READ(coll, o->animData, 20 * o->animLength);
                o->ptRef.x = o->animData->pt.x;
                o->ptRef.z = o->animData->pt.z;
                o->ptRef.y = o->animData->pt.y;
            }
            else
            { return false; } // unknown object type
        }
    }

    // vroad XB
    SAFE_READ(coll, &track->col.vroadHead, 8);
    if (track->col.vroadHead.xbid != XBID_VROAD)
    { return false; }
    if (track->col.vroadHead.size != 8 + 36 * track->col.vroadHead.nrec)
    { return false; }
    track->col.vroad = new COLVROAD[track->col.vroadHead.nrec];
    SAFE_READ(coll, track->col.vroad, 36 * track->col.vroadHead.nrec);

    uint32_t pad;
    return coll.read((char *) &pad, 4).gcount() == 0; // we ought to be at EOF now
}

bool NFS3::LoadHRZ(std::string hrz_path, const std::shared_ptr<TRACK> &track)
{
    std::ifstream hrz(hrz_path, std::ios::in | std::ios::binary);
    if (!hrz.is_open())
    { return false; }
    LOG(INFO) << "Loading HRZ File located at " << hrz_path;

    std::string str, skyTopColour, skyBottomColour;

    while (std::getline(hrz, str))
    {
        if (str.find("/* r,g,b value at top of Gourad shaded SKY area */") != std::string::npos)
        {
            std::getline(hrz, skyTopColour);
        }
        if (str.find("/* r,g,b values for base of Gourad shaded SKY area */") != std::string::npos)
        {
            std::getline(hrz, skyBottomColour);
        }
    }

    track->sky_top_colour = ParseRGBString(skyTopColour);
    track->sky_bottom_colour = ParseRGBString(skyBottomColour);

    hrz.close();

    return true;
}

std::vector<TrackBlock> NFS3::ParseTRKModels(const std::shared_ptr<TRACK> &track)
{
    LOG(INFO) << "Parsing TRK file into ONFS GL structures";

    std::vector<TrackBlock> trackBlocks = std::vector<TrackBlock>();
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    /* TRKBLOCKS - BASE TRACK GEOMETRY */
    for (uint32_t trackblockIdx = 0; trackblockIdx < track->nBlocks; ++trackblockIdx)
    {
        // Get Verts from Trk block, indices from associated polygon block
        TRKBLOCK rawTrackBlock = track->trk[trackblockIdx];
        POLYGONBLOCK trackPolygonBlock = track->poly[trackblockIdx];

        glm::vec3 rawTrackBlockCenter = rotationMatrix * (Utils::PointToVec(rawTrackBlock.ptCentre) / NFS3_SCALE_FACTOR);
        std::vector<glm::vec3> trackBlockVerts;
        std::vector<glm::vec4> trackBlockShadingData;

        TrackBlock trackBlock(trackblockIdx, rawTrackBlockCenter);

        // Light and sound sources
        for (uint32_t lightNum = 0; lightNum < rawTrackBlock.nLightsrc; ++lightNum)
        {
            glm::vec3 lightCenter = rotationMatrix * (Utils::FixedToFloat(Utils::PointToVec(rawTrackBlock.lightsrc[lightNum].refpoint)) / NFS3_SCALE_FACTOR);
            trackBlock.lights.emplace_back(Entity(trackblockIdx, lightNum, NFS_3, LIGHT, MakeLight(lightCenter, rawTrackBlock.lightsrc[lightNum].type), 0));
        }
        for (uint32_t soundNum = 0; soundNum < rawTrackBlock.nSoundsrc; ++soundNum)
        {
            glm::vec3 soundCenter = rotationMatrix * (Utils::FixedToFloat(Utils::PointToVec(rawTrackBlock.soundsrc[soundNum].refpoint)) / NFS3_SCALE_FACTOR);
            trackBlock.sounds.emplace_back(Entity(trackblockIdx, soundNum, NFS_3, SOUND, Sound(soundCenter, rawTrackBlock.soundsrc[soundNum].type), 0));
        }

        // Get Trackblock roadVertices and per-vertex shading data
        for (uint32_t vertIdx = 0; vertIdx < rawTrackBlock.nObjectVert; ++vertIdx)
        {
            glm::vec3 trackBlockVertex = (rotationMatrix * (Utils::PointToVec(rawTrackBlock.vert[vertIdx]) / NFS3_SCALE_FACTOR)) - rawTrackBlockCenter;
            trackBlockVerts.emplace_back(trackBlockVertex);

            // Packed RGBA per vertex colour data for baked lighting
            uint32_t trackBlockVertexShadingRaw = rawTrackBlock.unknVertices[vertIdx];
            // Break uint32_t of RGB into 4 normalised floats and store into vec4
            glm::vec4 trackBlockVertexShadingColour = glm::vec4(((trackBlockVertexShadingRaw >> 16) & 0xFF) / 255.0f,
                                                                ((trackBlockVertexShadingRaw >> 8) & 0xFF) / 255.0f,
                                                                (trackBlockVertexShadingRaw & 0xFF) / 255.0f,
                                                                ((trackBlockVertexShadingRaw >> 24) & 0xFF) / 255.0f);
            trackBlockShadingData.emplace_back(trackBlockVertexShadingColour);
        }

        // 4 OBJ Poly blocks
        for (uint32_t j = 0; j < 4; j++)
        {
            OBJPOLYBLOCK polygonBlock = trackPolygonBlock.obj[j];

            if (polygonBlock.n1 > 0)
            {
                // Iterate through objects in objpoly block up to num objects
                for (uint32_t objectIdx = 0; objectIdx < polygonBlock.nobj; ++objectIdx)
                {
                    // Get Polygons in object
                    LPPOLYGONDATA objectPolygons = polygonBlock.poly[objectIdx];

                    // Mesh Data
                    std::vector<unsigned int> vertexIndices;
                    std::vector<unsigned int> textureIndices;
                    std::vector<glm::vec2> uvs;
                    std::vector<glm::vec3> normals;
                    uint32_t accumulatedObjectFlags = 0u;

                    for (uint32_t polyIdx = 0; polyIdx < polygonBlock.numpoly[objectIdx]; polyIdx++)
                    {
                        // Texture for this polygon and it's loaded OpenGL equivalent
                        TEXTUREBLOCK polygonTexture = track->texture[objectPolygons[polyIdx].texture];
                        Texture glTexture = track->textures[polygonTexture.texture];

                        // Calculate the normal, as the provided data is a little suspect
                        glm::vec3 normal = rotationMatrix *
                                           CalculateQuadNormal(PointToVec(rawTrackBlock.vert[objectPolygons[polyIdx].vertex[0]]),
                                                               PointToVec(rawTrackBlock.vert[objectPolygons[polyIdx].vertex[1]]),
                                                               PointToVec(rawTrackBlock.vert[objectPolygons[polyIdx].vertex[2]]),
                                                               PointToVec(rawTrackBlock.vert[objectPolygons[polyIdx].vertex[3]]));

                        normals.emplace_back(normal);
                        normals.emplace_back(normal);
                        normals.emplace_back(normal);
                        normals.emplace_back(normal);
                        normals.emplace_back(normal);
                        normals.emplace_back(normal);

                        vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[0]);
                        vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[1]);
                        vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[2]);
                        vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[0]);
                        vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[2]);
                        vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[3]);

                        // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                        std::vector<glm::vec2> transformedUVs = GenerateUVs(NFS_3, OBJ_POLY,
                                                                            objectPolygons[polyIdx].hs_texflags, glTexture,
                                                                            polygonTexture);
                        uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                        textureIndices.emplace_back(polygonTexture.texture);
                        textureIndices.emplace_back(polygonTexture.texture);
                        textureIndices.emplace_back(polygonTexture.texture);
                        textureIndices.emplace_back(polygonTexture.texture);
                        textureIndices.emplace_back(polygonTexture.texture);
                        textureIndices.emplace_back(polygonTexture.texture);

                        accumulatedObjectFlags |= objectPolygons[polyIdx].flags;
                    }
                    Track trackBlockModel = Track(trackBlockVerts, normals, uvs, textureIndices, vertexIndices, trackBlockShadingData, rawTrackBlockCenter);
                    Entity trackBlockEntity = Entity(trackblockIdx, (j + 1) * (objectIdx + 1), NFS_3, OBJ_POLY, trackBlockModel, accumulatedObjectFlags);
                    trackBlock.objects.emplace_back(trackBlockEntity);
                }
            }
        }

        /* XOBJS - EXTRA OBJECTS */
        for (uint32_t l = (trackblockIdx * 4); l < (trackblockIdx * 4) + 4; ++l)
        {
            for (uint32_t j = 0; j < track->xobj[l].nobj; ++j)
            {
                // Get the Extra object data for this trackblock object from the global xobj table
                XOBJDATA *pExtraObjectData = &(track->xobj[l].obj[j]);

                // Mesh Data
                std::vector<glm::vec3> extraObjectVerts;
                std::vector<glm::vec4> extraObjectShadingData;
                std::vector<unsigned int> vertexIndices;
                std::vector<unsigned int> textureIndices;
                std::vector<glm::vec2> uvs;
                std::vector<glm::vec3> normals;
                uint32_t accumulatedObjectFlags = 0u;

                for (uint32_t vertIdx = 0; vertIdx < pExtraObjectData->nVertices; vertIdx++, pExtraObjectData->vert++)
                {
                    glm::vec3 extraObjectVertex = rotationMatrix * (glm::vec3(pExtraObjectData->vert->x, pExtraObjectData->vert->y, pExtraObjectData->vert->z) / NFS3_SCALE_FACTOR);
                    extraObjectVerts.emplace_back(extraObjectVertex);

                    uint32_t extraObjectVertexShadingRaw = pExtraObjectData->unknVertices[vertIdx];
                    glm::vec4 extraObjectVertexShadingColour = glm::vec4(((extraObjectVertexShadingRaw >> 16) & 0xFF) / 255.0f,
                                                                         ((extraObjectVertexShadingRaw >> 8) & 0xFF) / 255.0f,
                                                                         (extraObjectVertexShadingRaw & 0xFF) / 255.0f,
                                                                         ((extraObjectVertexShadingRaw >> 24) & 0xFF) / 255.0f);
                    extraObjectShadingData.emplace_back(extraObjectVertexShadingColour);
                }

                for (uint32_t k = 0; k < pExtraObjectData->nPolygons; k++, pExtraObjectData->polyData++)
                {
                    TEXTUREBLOCK texture_for_block = track->texture[pExtraObjectData->polyData->texture];
                    Texture gl_texture = track->textures[texture_for_block.texture];

                    glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(extraObjectVerts[pExtraObjectData->polyData->vertex[0]]),
                                                                            PointToVec(extraObjectVerts[pExtraObjectData->polyData->vertex[1]]),
                                                                            PointToVec(extraObjectVerts[pExtraObjectData->polyData->vertex[2]]),
                                                                            PointToVec(extraObjectVerts[pExtraObjectData->polyData->vertex[3]]));
                    normals.emplace_back(normal);
                    normals.emplace_back(normal);
                    normals.emplace_back(normal);
                    normals.emplace_back(normal);
                    normals.emplace_back(normal);
                    normals.emplace_back(normal);

                    vertexIndices.emplace_back(pExtraObjectData->polyData->vertex[0]);
                    vertexIndices.emplace_back(pExtraObjectData->polyData->vertex[1]);
                    vertexIndices.emplace_back(pExtraObjectData->polyData->vertex[2]);
                    vertexIndices.emplace_back(pExtraObjectData->polyData->vertex[0]);
                    vertexIndices.emplace_back(pExtraObjectData->polyData->vertex[2]);
                    vertexIndices.emplace_back(pExtraObjectData->polyData->vertex[3]);

                    std::vector<glm::vec2> transformedUVs = GenerateUVs(NFS_3, XOBJ, pExtraObjectData->polyData->hs_texflags,
                                                                        gl_texture, texture_for_block);
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    textureIndices.emplace_back(texture_for_block.texture);
                    textureIndices.emplace_back(texture_for_block.texture);
                    textureIndices.emplace_back(texture_for_block.texture);
                    textureIndices.emplace_back(texture_for_block.texture);
                    textureIndices.emplace_back(texture_for_block.texture);
                    textureIndices.emplace_back(texture_for_block.texture);

                    accumulatedObjectFlags |= pExtraObjectData->polyData->flags;
                }
                glm::vec3 extraObjectCenter = rotationMatrix * Utils::PointToVec(pExtraObjectData->ptRef) / NFS3_SCALE_FACTOR;
                Track extraObjectModel = Track(extraObjectVerts, normals, uvs, textureIndices, vertexIndices, extraObjectShadingData, extraObjectCenter);
                Entity extraObjectEntity = Entity(trackblockIdx, l, NFS_3, XOBJ, extraObjectModel, accumulatedObjectFlags);
                trackBlock.objects.emplace_back(extraObjectEntity);
            }
        }

        // Road Mesh data
        std::vector<glm::vec3> roadVertices;
        std::vector<glm::vec4> roadShadingData;
        std::vector<unsigned int> vertexIndices;
        std::vector<unsigned int> textureIndices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        uint32_t accumulatedObjectFlags = 0u;

        for (int32_t vertIdx = 0; vertIdx < rawTrackBlock.nVertices; ++vertIdx)
        {
            glm::vec3 roadVertex = (rotationMatrix * (Utils::PointToVec(rawTrackBlock.vert[vertIdx]) / NFS3_SCALE_FACTOR)) - rawTrackBlockCenter;
            roadVertices.emplace_back(roadVertex);

            uint32_t roadVertexShadingRaw = rawTrackBlock.unknVertices[vertIdx];
            glm::vec4 roadVertexShadingColour = glm::vec4(((roadVertexShadingRaw >> 16) & 0xFF) / 255.0f,
                                                          ((roadVertexShadingRaw >> 8) & 0xFF) / 255.0f,
                                                          (roadVertexShadingRaw & 0xFF) / 255.0f,
                                                          ((roadVertexShadingRaw >> 24) & 0xFF) / 255.0f);
            roadShadingData.emplace_back(roadVertexShadingColour);
        }
        // Get indices from Chunk 4 and 5 for High Res polys, Chunk 6 for Road Lanes
        for (int32_t lodChunkIdx = 4; lodChunkIdx <= 6; lodChunkIdx++)
        {
            // If there are no lane markers in the lane chunk, skip
            if ((lodChunkIdx == 6) && (rawTrackBlock.nVertices <= rawTrackBlock.nHiResVert))
            {
                continue;
            }

            // Get the polygon data for this road section
            LPPOLYGONDATA chunkPolygonData = trackPolygonBlock.poly[lodChunkIdx];

            for (uint32_t polyIdx = 0; polyIdx < trackPolygonBlock.sz[lodChunkIdx]; polyIdx++)
            {
                TEXTUREBLOCK polygonTexture = track->texture[chunkPolygonData[polyIdx].texture];
                Texture gl_texture = track->textures[polygonTexture.texture];

                glm::vec3 normal = rotationMatrix *
                                   CalculateQuadNormal(PointToVec(rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[0]]),
                                                       PointToVec(rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[1]]),
                                                       PointToVec(rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[2]]),
                                                       PointToVec(rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[3]]));

                normals.emplace_back(normal);
                normals.emplace_back(normal);
                normals.emplace_back(normal);
                normals.emplace_back(normal);
                normals.emplace_back(normal);
                normals.emplace_back(normal);

                vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[0]);
                vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[1]);
                vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[2]);
                vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[0]);
                vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[2]);
                vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[3]);

                std::vector<glm::vec2> transformedUVs = GenerateUVs(NFS_3, lodChunkIdx == 6 ? LANE : ROAD,
                                                                    chunkPolygonData[polyIdx].hs_texflags, gl_texture,
                                                                    polygonTexture);
                uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                textureIndices.emplace_back(polygonTexture.texture);
                textureIndices.emplace_back(polygonTexture.texture);
                textureIndices.emplace_back(polygonTexture.texture);
                textureIndices.emplace_back(polygonTexture.texture);
                textureIndices.emplace_back(polygonTexture.texture);
                textureIndices.emplace_back(polygonTexture.texture);

                accumulatedObjectFlags |= chunkPolygonData[polyIdx].flags;
            }
            Track roadModel = Track(roadVertices, normals, uvs, textureIndices, vertexIndices, roadShadingData, rawTrackBlockCenter);
            if (lodChunkIdx == 6)
            {
                Entity laneEntity = Entity(trackblockIdx, -1, NFS_3, LANE, roadModel, accumulatedObjectFlags);
                trackBlock.lanes.emplace_back(laneEntity);
            }
            else
            {
                Entity roadEntity = Entity(trackblockIdx, -1, NFS_3, ROAD, roadModel, accumulatedObjectFlags);
                trackBlock.track.emplace_back(roadEntity);
            }
        }
        trackBlocks.emplace_back(trackBlock);
    }
    return trackBlocks;
}

std::vector<Entity> NFS3::ParseCOLModels(const std::shared_ptr<TRACK> &track)
{
    LOG(INFO) << "Parsing COL file into ONFS GL structures";

    std::vector<Entity> col_entities;
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0,
                                                                  0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    COLOBJECT *o = track->col.object;
    /* COL DATA - TODO: Come back for VROAD AI/Collision data */
    for (uint32_t i = 0; i < track->col.objectHead.nrec; i++, o++)
    {
        COLSTRUCT3D s = track->col.struct3D[o->struct3D];
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> texture_indices;
        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> shading_data;
        std::vector<glm::vec3> norms;
        for (uint32_t j = 0; j < s.nVert; j++, s.vertex++)
        {
            verts.emplace_back(
                    rotationMatrix * glm::vec3(s.vertex->pt.x / 10, s.vertex->pt.y / 10, s.vertex->pt.z / 10));
            shading_data.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
        }
        for (uint32_t k = 0; k < s.nPoly; k++, s.polygon++)
        {
            // Remap the COL TextureID's using the COL texture block (XBID2)
            COLTEXTUREINFO col_texture = track->col.texture[s.polygon->texture];
            TEXTUREBLOCK texture_for_block;
            // Find the texture by it's file name, but use the Texture table to get the block. TODO: Not mapping this so, must do a manual search.
            for (uint32_t t = 0; t < track->nTextures; t++)
            {
                if (track->texture[t].texture == col_texture.texture)
                {
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

            glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(verts[s.polygon->v[0]]),
                                                                    PointToVec(verts[s.polygon->v[1]]),
                                                                    PointToVec(verts[s.polygon->v[2]]),
                                                                    PointToVec(verts[s.polygon->v[3]]));
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);
            norms.emplace_back(normal);

            uvs.emplace_back(texture_for_block.corners[0] * gl_texture.max_u,
                             (1.0f - texture_for_block.corners[1]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[2] * gl_texture.max_u,
                             (1.0f - texture_for_block.corners[3]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[4] * gl_texture.max_u,
                             (1.0f - texture_for_block.corners[5]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[0] * gl_texture.max_u,
                             (1.0f - texture_for_block.corners[1]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[4] * gl_texture.max_u,
                             (1.0f - texture_for_block.corners[5]) * gl_texture.max_v);
            uvs.emplace_back(texture_for_block.corners[6] * gl_texture.max_u,
                             (1.0f - texture_for_block.corners[7]) * gl_texture.max_v);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
            texture_indices.emplace_back(texture_for_block.texture);
        }
        glm::vec3 position = rotationMatrix * Utils::FixedToFloat(Utils::PointToVec(o->ptRef)) / NFS3_SCALE_FACTOR;
        col_entities.emplace_back(Entity(-1, i, NFS_3, GLOBAL,Track(verts, norms, uvs, texture_indices, indices, shading_data, position), 0));
    }
    return col_entities;
}

Texture NFS3::LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name)
{
    std::stringstream filename;
    std::stringstream filename_alpha;

    if (track_texture.islane)
    {
        filename << "../resources/sfx/" << std::setfill('0') << std::setw(4) << track_texture.texture + 9 << ".BMP";
        filename_alpha << "../resources/sfx/" << std::setfill('0') << std::setw(4) << track_texture.texture + 9 << "-a.BMP";
    }
    else
    {
        filename << TRACK_PATH << ToString(NFS_3) << "/" << track_name << "/textures/" << std::setfill('0') << std::setw(4)
                 << track_texture.texture << ".BMP";
        filename_alpha << TRACK_PATH << ToString(NFS_3) << "/" << track_name << "/textures/" << std::setfill('0') << std::setw(4)
                       << track_texture.texture << "-a.BMP";
    }

    GLubyte *data;
    GLsizei width = track_texture.width;
    GLsizei height = track_texture.height;

    if (!ImageLoader::LoadBmpWithAlpha(filename.str().c_str(), filename_alpha.str().c_str(), &data, &width, &height))
    {
        LOG(WARNING) << "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!";
        // If the texture is missing, load a "MISSING" texture of identical size.
        ASSERT(ImageLoader::LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", &data,
                                             &width, &height), "Even the 'missing' texture is missing!");
        return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(width),
                       static_cast<unsigned int>(height));
    }

    return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(track_texture.width),
                   static_cast<unsigned int>(track_texture.height));
}

void NFS3::FreeFRD(const std::shared_ptr<TRACK> &track)
{
    // Free FRD data
    // TRKBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++)
    {
        TRKBLOCK *trackBlock = &(track->trk[block_Idx]);
        delete[]trackBlock->vert;
        delete[]trackBlock->unknVertices;
        delete[]trackBlock->posData;
        delete[] trackBlock->polyData;
        delete[] trackBlock->vroadData;
        if (trackBlock->nXobj > 0)
        {
            delete[]trackBlock->xobj;
        }
        if (trackBlock->nSoundsrc > 0)
        {
            delete[]trackBlock->soundsrc;
        }
        if (trackBlock->nLightsrc > 0)
        {
            delete[]trackBlock->lightsrc;
        }
        //delete trackBlock;
    }
    //delete []track->trk;

    // POLYGONBLOCKs
    for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++)
    {
        POLYGONBLOCK *p = &(track->poly[block_Idx]);
        for (uint32_t j = 0; j < 7; j++)
        {
            if (p->sz[j] != 0)
            {
                free(p->poly[j]);
            }
        }
        for (uint32_t obj_Idx = 0; obj_Idx < 4; obj_Idx++)
        {
            OBJPOLYBLOCK *o = &(p->obj[obj_Idx]);
            if (o->n1 > 0)
            {
                delete[]o->types;
                delete[]o->numpoly;
                for (uint32_t k = 0; k < o->n2; k++)
                {
                    if (o->types[k] == 1)
                    {
                        // free(o->poly[o->nobj]);
                    }
                }
            }
        }
    }

    // XOBJBLOCKs
    for (uint32_t xblock_Idx = 0; xblock_Idx <= 4 * track->nBlocks; xblock_Idx++)
    {
        if (track->xobj[xblock_Idx].nobj > 0)
        {
            delete[]track->xobj[xblock_Idx].obj;
        }
        for (uint32_t xobj_Idx = 0; xobj_Idx < track->xobj[xblock_Idx].nobj; xobj_Idx++)
        {
            XOBJDATA *x = &(track->xobj[xblock_Idx].obj[xobj_Idx]);
            if (x->crosstype == 3)
            { // animated objects
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

void NFS3::FreeCOL(const std::shared_ptr<TRACK> &track)
{
    // Free COL Data
    delete[]track->col.texture;
    // struct3D XB
    if (track->col.nBlocks >= 4)
    {
        COLSTRUCT3D *s = track->col.struct3D;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.struct3DHead.nrec; colRec_Idx++, s++)
        {
            delete[]s->vertex;
            delete[]s->polygon;
        }
        delete[] track->col.struct3D;

        // Object XB
        COLOBJECT *o = track->col.object;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.objectHead.nrec; colRec_Idx++, o++)
        {
            if (o->type == 3)
            {
                delete[]o->animData;
            }
        }
        delete[]track->col.object;
    }

    // object2 XB
    if (track->col.nBlocks == 5)
    {
        COLOBJECT *o = track->col.object2;
        for (uint32_t colRec_Idx = 0; colRec_Idx < track->col.object2Head.nrec; colRec_Idx++, o++)
        {
            if (o->type == 3)
            {
                delete[]o->animData;
            }
        }
        delete[]track->col.object2;
    }
    delete track->col.vroad;
}
