#include "NFS3Loader.h"

#include "../Shared/CanFile.h"
#include "../Shared/HrzFile.h"
#include "../../Renderer/Texture.h"

// CAR
std::shared_ptr<Car> NFS3::LoadCar(const std::string &carBasePath)
{
    boost::filesystem::path p(carBasePath);
    std::string car_name = p.filename().string();

    std::stringstream vivPath, carOutPath, fcePath, fedataPath;
    vivPath << carBasePath << "/car.viv";
    carOutPath << CAR_PATH << ToString(NFS_3) << "/" << car_name << "/";
    fcePath << CAR_PATH << ToString(NFS_3) << "/" << car_name << "/car.fce";
    fedataPath << carOutPath.str() << "/fedata.eng";

    FceFile fceFile;
    FedataFile fedataFile;

    ASSERT(Utils::ExtractVIV(vivPath.str(), carOutPath.str()), "Unable to extract " << vivPath.str() << " to " << carOutPath.str());
    ASSERT(FceFile::Load(fcePath.str(), fceFile), "Could not load FCE file: " << fcePath.str());
    ASSERT(FedataFile::Load(fedataPath.str(), fedataFile, fceFile.nPriColours), "Could not load FeData file: " << fedataPath.str());

    CarData carData = _ParseFCEModels(fceFile);

    // Go get car metadata from FEDATA
    carData.carName = fedataFile.menuName;
    for (uint8_t colourIdx = 0; colourIdx < fceFile.nPriColours; ++colourIdx)
    {
        carData.colours[colourIdx].colourName = fedataFile.primaryColourNames[colourIdx];
    }

    return std::make_shared<Car>(carData, NFS_3, car_name);
}

std::shared_ptr<TRACK> NFS3::LoadTrack(const std::string &trackBasePath)
{
    LOG(INFO) << "Loading Track located at " << trackBasePath;
    auto track = std::make_shared<TRACK>(TRACK());

    boost::filesystem::path p(trackBasePath);
    track->name = p.filename().string();
    std::stringstream frdPath, colPath, canPath, hrzPath;
    std::string strip = "k0";
    size_t pos = track->name.find(strip);
    if (pos != std::string::npos)
    {
        track->name.replace(pos, strip.size(), "");
    }

    frdPath << trackBasePath << "/" << track->name << ".frd";
    colPath << trackBasePath << "/" << track->name << ".col";
    canPath << trackBasePath << "/" << track->name << "00a.can";
    hrzPath << trackBasePath << "/3" << track->name << ".hrz";

    FrdFile frdFile;
    ColFile colFile;
    CanFile canFile;
    HrzFile hrzFile;

    // Load QFS texture into GL objects
    for (auto &frdTexBlock : frdFile.textureBlocks)
    {
        track->textures[frdTexBlock.texture] = Texture::LoadTexture(frdTexBlock, track->name);
    }

    ASSERT(Texture::ExtractTrackTextures(trackBasePath, track->name, NFSVer::NFS_3), "Could not extract " << track->name << " QFS texture pack.");
    ASSERT(FrdFile::Load(frdPath.str(), frdFile), "Could not load FRD file: " << frdPath.str()); // Load FRD file to get track block specific data
    ASSERT(ColFile::Load(colPath.str(), colFile), "Could not load COL file: " << colPath.str()); // Load Catalogue file to get global (non trkblock specific) data
    ASSERT(CanFile::Load(canPath.str(), canFile), "Could not load CAN file (camera animation): " << canPath.str()); // Load camera intro/outro animation data
    ASSERT(HrzFile::Load(hrzPath.str(), hrzFile), "Could not load HRZ file (skybox/lighting):" << hrzPath.str()); // Load HRZ Data

    track->cameraAnimation = canFile.animPoints;
    track->textureArrayID = Texture::MakeTextureArray(track->textures, false);
    track->trackBlocks = _ParseTRKModels(frdFile, track);
    track->globalObjects = _ParseCOLModels(colFile, track);

    LOG(INFO) << "Track loaded successfully";

    return track;
}

CarData NFS3::_ParseFCEModels(const FceFile &fceFile)
{
    LOG(INFO) << "Parsing FCE File into ONFS Structures";
    // All Vertices are stored so that the model is rotated 90 degs on X, 180 on Z. Remove this at Vert load time.
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(glm::radians(90.f), 0, glm::radians(180.f))));
    CarData carData;

    // Grab colours
    for (uint8_t colourIdx = 0; colourIdx < fceFile.nPriColours; ++colourIdx)
    {
        Colour primaryColour = fceFile.primaryColours[colourIdx];
        CarColour originalPrimaryColour("", Utils::HSLToRGB(glm::vec4(primaryColour.H, primaryColour.S, primaryColour.B, primaryColour.T)));
        carData.colours.emplace_back(originalPrimaryColour);
    }

    for (uint32_t dummyIdx = 0; dummyIdx < fceFile.nDummies; ++dummyIdx)
    {
        Dummy dummy(fceFile.dummyNames[dummyIdx], rotationMatrix * glm::vec3(fceFile.dummyCoords[dummyIdx].x / 10, fceFile.dummyCoords[dummyIdx].y / 10, fceFile.dummyCoords[dummyIdx].z / 10));
        carData.dummies.emplace_back(dummy);
    }

    for (uint32_t part_Idx = 0; part_Idx < fceFile.nParts; ++part_Idx)
    {
        float specularDamper = 0.2f;
        float specularReflectivity = 0.02f;
        float envReflectivity = 0.4f;

        std::vector<uint32_t> indices;
        std::vector<uint32_t> polygonFlags;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::string part_name(fceFile.partNames[part_Idx]);
        glm::vec3 center = rotationMatrix * (fceFile.partCoords[part_Idx] / NFS3_SCALE_FACTOR);

        for (uint32_t vert_Idx = 0; vert_Idx < fceFile.partNumVertices[part_Idx]; ++vert_Idx)
        {
            vertices.emplace_back(rotationMatrix * (partVertices[vertIdx] / NFS3_SCALE_FACTOR));
        }

        for (uint32_t normal_Idx = 0; normal_Idx < fceFile.partNumVertices[part_Idx]; ++normal_Idx)
        {
            normals.emplace_back(rotationMatrix * partNormals[normal_Idx]);
        }
        for (uint32_t tri_Idx = 0; tri_Idx < fceFile.partNumTriangles[part_Idx]; ++tri_Idx)
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
    }

    return carData;
}

std::vector<TrackBlock> NFS3::_ParseTRKModels(const FrdFile &frdFile)
{
    LOG(INFO) << "Parsing TRK file into ONFS GL structures";
    std::vector<TrackBlock> trackBlocks(frdFile.nBlocks);

    /* TRKBLOCKS - BASE TRACK GEOMETRY */
    for (uint32_t trackblockIdx = 0; trackblockIdx < frdFile.nBlocks; ++trackblockIdx)
    {
        // Get Verts from Trk block, indices from associated polygon block
        TrkBlock rawTrackBlock = frdFile.trackBlocks[trackblockIdx];
        PolyBlock trackPolygonBlock = frdFile.polygonBlocks[trackblockIdx];

        glm::vec3 rawTrackBlockCenter = rawTrackBlock.ptCentre / NFS3_SCALE_FACTOR;
        std::vector<glm::vec3> trackBlockVerts;
        std::vector<glm::vec4> trackBlockShadingData;

        TrackBlock trackBlock(trackblockIdx, rawTrackBlockCenter);

        // Light and sound sources
        for (uint32_t lightNum = 0; lightNum < rawTrackBlock.nLightsrc; ++lightNum)
        {
            glm::vec3 lightCenter = Utils::FixedToFloat(rawTrackBlock.lightsrc[lightNum].refpoint) / NFS3_SCALE_FACTOR;
            trackBlock.lights.emplace_back(Entity(trackblockIdx, lightNum, NFS_3, LIGHT, TrackUtils::MakeLight(lightCenter, rawTrackBlock.lightsrc[lightNum].type), 0));
        }
        for (uint32_t soundNum = 0; soundNum < rawTrackBlock.nSoundsrc; ++soundNum)
        {
            glm::vec3 soundCenter = Utils::FixedToFloat(rawTrackBlock.soundsrc[soundNum].refpoint) / NFS3_SCALE_FACTOR;
            trackBlock.sounds.emplace_back(Entity(trackblockIdx, soundNum, NFS_3, SOUND, Sound(soundCenter, rawTrackBlock.soundsrc[soundNum].type), 0));
        }

        // Get Trackblock roadVertices and per-vertex shading data
        for (uint32_t vertIdx = 0; vertIdx < rawTrackBlock.nObjectVert; ++vertIdx)
        {
            trackBlockVerts.emplace_back((rawTrackBlock.vert[vertIdx] / NFS3_SCALE_FACTOR) - rawTrackBlockCenter);
            trackBlockShadingData.emplace_back(TrackUtils::ShadingDataToVec4(rawTrackBlock.vertShading[vertIdx]));
        }

        // 4 OBJ Poly blocks
        for (uint32_t j = 0; j < 4; ++j)
        {
            ObjectPolyBlock polygonBlock = trackPolygonBlock.obj[j];

            if (polygonBlock.n1 > 0)
            {
                // Iterate through objects in objpoly block up to num objects
                for (uint32_t objectIdx = 0; objectIdx < polygonBlock.nobj; ++objectIdx)
                {
                    // Mesh Data
                    std::vector<unsigned int> vertexIndices;
                    std::vector<unsigned int> textureIndices;
                    std::vector<glm::vec2> uvs;
                    std::vector<glm::vec3> normals;
                    uint32_t accumulatedObjectFlags = 0u;

                    // Get Polygons in object
                    std::vector<PolygonData> objectPolygons = polygonBlock.poly[objectIdx];

                    for (uint32_t polyIdx = 0; polyIdx < polygonBlock.numpoly[objectIdx]; ++polyIdx)
                    {
                        // Texture for this polygon and it's loaded OpenGL equivalent
                        TexBlock polygonTexture = frdFile.textureBlocks[objectPolygons[polyIdx].texture];
                        Texture glTexture = track->textures[polygonTexture.texture];

                        // Calculate the normal, as the provided data is a little suspect
                        glm::vec3 normal = Utils::CalculateQuadNormal(rawTrackBlock.vert[objectPolygons[polyIdx].vertex[0]],
                                                                      rawTrackBlock.vert[objectPolygons[polyIdx].vertex[1]],
                                                                      rawTrackBlock.vert[objectPolygons[polyIdx].vertex[2]],
                                                                      rawTrackBlock.vert[objectPolygons[polyIdx].vertex[3]]);

                        // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                        for (uint8_t i = 0; i < 6; ++i)
                        {
                            normals.emplace_back(normal);
                            vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[quadToTriVertNumbers[i]]);
                            textureIndices.emplace_back(polygonTexture.texture);
                        }

                        // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                        std::vector<glm::vec2> transformedUVs = GenerateUVs(NFS_3, OBJ_POLY, objectPolygons[polyIdx].hs_texflags, glTexture, polygonTexture);
                        uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

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
            for (uint32_t j = 0; j < frdFile.extraObjectBlocks[l].nobj; ++j)
            {
                // Mesh Data
                std::vector<glm::vec3> extraObjectVerts;
                std::vector<glm::vec4> extraObjectShadingData;
                std::vector<unsigned int> vertexIndices;
                std::vector<unsigned int> textureIndices;
                std::vector<glm::vec2> uvs;
                std::vector<glm::vec3> normals;
                uint32_t accumulatedObjectFlags = 0u;

                // Get the Extra object data for this trackblock object from the global xobj table
                ExtraObjectData extraObjectData = frdFile.extraObjectBlocks[l].obj[j];

                for (uint32_t vertIdx = 0; vertIdx < extraObjectData.nVertices; vertIdx++)
                {
                    extraObjectVerts.emplace_back(extraObjectData.vert[vertIdx] / NFS3_SCALE_FACTOR);
                    extraObjectShadingData.emplace_back(TrackUtils::ShadingDataToVec4(extraObjectData.vertShading[vertIdx]));
                }

                for (uint32_t k = 0; k < extraObjectData.nPolygons; k++)
                {
                    TexBlock blockTexture = frdFile.textureBlocks[extraObjectData.polyData[k].texture];
                    Texture gl_texture = track->textures[blockTexture.texture];
                    glm::vec3 normal = Utils::CalculateQuadNormal(extraObjectVerts[extraObjectData.polyData[k].vertex[0]],
                                                                  extraObjectVerts[extraObjectData.polyData[k].vertex[1]],
                                                                  extraObjectVerts[extraObjectData.polyData[k].vertex[2]],
                                                                  extraObjectVerts[extraObjectData.polyData[k].vertex[3]]);

                    // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                    for (uint8_t i = 0; i < 6; ++i)
                    {
                        normals.emplace_back(normal);
                        vertexIndices.emplace_back(extraObjectData.polyData[k].vertex[quadToTriVertNumbers[i]]);
                        textureIndices.emplace_back(blockTexture.texture);
                    }

                    std::vector<glm::vec2> transformedUVs = GenerateUVs(NFS_3, XOBJ, extraObjectData.polyData[k].hs_texflags, gl_texture, blockTexture);
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    accumulatedObjectFlags |= extraObjectData.polyData[k].flags;
                }
                glm::vec3 extraObjectCenter = extraObjectData.ptRef / NFS3_SCALE_FACTOR;
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

        for (uint32_t vertIdx = 0; vertIdx < rawTrackBlock.nVertices; ++vertIdx)
        {
            roadVertices.emplace_back((rawTrackBlock.vert[vertIdx] / NFS3_SCALE_FACTOR) - rawTrackBlockCenter);
            roadShadingData.emplace_back(TrackUtils::ShadingDataToVec4(rawTrackBlock.vertShading[vertIdx]));
        }
        // Get indices from Chunk 4 and 5 for High Res polys, Chunk 6 for Road Lanes
        for (uint32_t lodChunkIdx = 4; lodChunkIdx <= 6; lodChunkIdx++)
        {
            // If there are no lane markers in the lane chunk, skip
            if ((lodChunkIdx == 6) && (rawTrackBlock.nVertices <= rawTrackBlock.nHiResVert))
            {
                continue;
            }

            // Get the polygon data for this road section
            std::vector<PolygonData> chunkPolygonData = trackPolygonBlock.poly[lodChunkIdx];

            for (uint32_t polyIdx = 0; polyIdx < trackPolygonBlock.sz[lodChunkIdx]; polyIdx++)
            {
                TexBlock polygonTexture = frdFile.textureBlocks[chunkPolygonData[polyIdx].texture];
                Texture gl_texture = track->textures[polygonTexture.texture];

                glm::vec3 normal = Utils::CalculateQuadNormal(rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[0]],
                                                              rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[1]],
                                                              rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[2]],
                                                              rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[3]]);

                // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                for (uint8_t i = 0; i < 6; ++i)
                {
                    normals.emplace_back(normal);
                    vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[quadToTriVertNumbers[i]]);
                    textureIndices.emplace_back(polygonTexture.texture);
                }

                std::vector<glm::vec2> transformedUVs = GenerateUVs(NFS_3, lodChunkIdx == 6 ? LANE : ROAD, chunkPolygonData[polyIdx].hs_texflags, gl_texture, polygonTexture);
                uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                accumulatedObjectFlags |= chunkPolygonData[polyIdx].flags;
            }
            Track roadModel = Track(roadVertices, normals, uvs, textureIndices, vertexIndices, roadShadingData, rawTrackBlockCenter);
            if (lodChunkIdx == 6)
            {
                Entity laneEntity = Entity(trackblockIdx, -1, NFS_3, LANE, roadModel, accumulatedObjectFlags);
                trackBlock.lanes.emplace_back(laneEntity);
            } else
            {
                Entity roadEntity = Entity(trackblockIdx, -1, NFS_3, ROAD, roadModel, accumulatedObjectFlags);
                trackBlock.track.emplace_back(roadEntity);
            }
        }
        trackBlocks.emplace_back(trackBlock);
    }
    return trackBlocks;
}

std::vector<Entity> NFS3::_ParseCOLModels(const ColFile &colFile)
{
    LOG(INFO) << "Parsing COL file into ONFS GL structures";
    std::vector<Entity> colEntities;

    /* COL DATA - TODO: Come back for VROAD AI/Collision data */
    for (uint32_t i = 0; i < colFile.objectHead.nrec; ++i)
    {
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> texture_indices;
        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> shading_data;
        std::vector<glm::vec3> norms;

        ColStruct3D s = colFile.struct3D[colFile.object[i].struct3D];

        for (uint32_t vertIdx = 0; vertIdx < s.nVert; ++vertIdx)
        {
            verts.emplace_back(s.vertex[vertIdx].pt / NFS3_SCALE_FACTOR);
            shading_data.emplace_back(TrackUtils::ShadingDataToVec4(s.vertex[vertIdx].unknown));
        }
        for (uint32_t polyIdx = 0; polyIdx < s.nPoly; ++polyIdx)
        {
            // Remap the COL TextureID's using the COL texture block (XBID2)
            ColTextureInfo col_texture = colFile.texture[s.polygon[polyIdx].texture];
            // Lookup the remapped COL->FRD texture ID in the FRD texture table
            TEXTUREBLOCK blockTexture = track->textures[col_texture.texture].rawTextureInfo;
            // Retrieve the GL texture for it so can scale UVs into texture array
            Texture glTexture = track->textures[blockTexture.texture];

            glm::vec3 normal = Utils::CalculateQuadNormal(verts[s.polygon[polyIdx].v[0]], verts[s.polygon[polyIdx].v[1]], verts[s.polygon[polyIdx].v[2]], verts[s.polygon[polyIdx].v[3]]);

            // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
            for (uint8_t i = 0; i < 6; ++i)
            {
                indices.emplace_back(s.polygon[polyIdx].v[quadToTriVertNumbers[i]]);
                norms.emplace_back(normal);
                texture_indices.emplace_back(blockTexture.texture);
            }

            uvs.emplace_back(blockTexture.corners[0] * glTexture.max_u, (1.0f - blockTexture.corners[1]) * glTexture.max_v);
            uvs.emplace_back(blockTexture.corners[2] * glTexture.max_u, (1.0f - blockTexture.corners[3]) * glTexture.max_v);
            uvs.emplace_back(blockTexture.corners[4] * glTexture.max_u, (1.0f - blockTexture.corners[5]) * glTexture.max_v);
            uvs.emplace_back(blockTexture.corners[0] * glTexture.max_u, (1.0f - blockTexture.corners[1]) * glTexture.max_v);
            uvs.emplace_back(blockTexture.corners[4] * glTexture.max_u, (1.0f - blockTexture.corners[5]) * glTexture.max_v);
            uvs.emplace_back(blockTexture.corners[6] * glTexture.max_u, (1.0f - blockTexture.corners[7]) * glTexture.max_v);
        }
        glm::vec3 position = glm::vec3(colFile.object[i].ptRef) / NFS3_SCALE_FACTOR;
        colEntities.emplace_back(Entity(-1, i, NFS_3, GLOBAL, Track(verts, norms, uvs, texture_indices, indices, shading_data, position), 0));
    }

    return colEntities;
}