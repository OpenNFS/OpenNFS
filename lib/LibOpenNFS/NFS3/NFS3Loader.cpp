#include "NFS3Loader.h"
#include <filesystem>
#include "LibOpenNFS.h"

#include <Entities/TrackSound.h>
#include <Entities/TrackGeometry.h>
#include <Entities/TrackEntity.h>

namespace LibOpenNFS::NFS3 {
    Car Loader::LoadCar(const std::string &carBasePath) {
        std::filesystem::path p(carBasePath);
        std::string carName = p.filename().string();

        std::stringstream vivPath, carOutPath, fcePath, fedataPath;
        vivPath << carBasePath << "/car.viv";
        carOutPath << CAR_PATH << get_string(NFSVersion::NFS_3) << "/" << carName << "/";
        fcePath << CAR_PATH << get_string(NFSVersion::NFS_3) << "/" << carName << "/car.fce";
        fedataPath << carOutPath.str() << "fedata.eng";

        Shared::VivFile vivFile;
        FceFile fceFile;
        FedataFile fedataFile;

        ASSERT(Shared::VivFile::Load(vivPath.str(), vivFile), "Could not open VIV file: " << vivPath.str());
        ASSERT(Shared::VivFile::Extract(carOutPath.str(), vivFile), "Could not extract VIV file: " << vivPath.str() << "to: " << carOutPath.str());
        ASSERT(FceFile::Load(fcePath.str(), fceFile), "Could not load FCE file: " << fcePath.str());
        if (!FedataFile::Load(fedataPath.str(), fedataFile, fceFile.nPriColours)) {
            // LOG(WARNING) << "Could not load FeData file: " << fedataPath.str();
        }

        Car::MetaData carData = _ParseAssetData(fceFile, fedataFile);

        return Car(carData, NFSVersion::NFS_3, carName);
    }

    Track Loader::LoadTrack(const std::string &trackBasePath) {
        // LOG(INFO) << "Loading Track located at " << trackBasePath;
        std::filesystem::path p(trackBasePath);
        std::string trackName = p.filename().string();
        std::string frdPath, colPath, canPath, hrzPath, binPath;
        std::string strip = "k0", trackNameStripped = trackName;
        size_t pos = trackName.find(strip);
        if (pos != std::string::npos) {
            trackNameStripped.replace(pos, strip.size(), "");
        }
        Track track(NFSVersion::NFS_3, trackNameStripped, trackBasePath);

        frdPath = trackBasePath + "/" + trackNameStripped + ".frd";
        colPath = trackBasePath + "/" + trackNameStripped + ".col";
        canPath = trackBasePath + "/" + trackNameStripped + "00a.can";
        hrzPath = trackBasePath + "/3" + trackNameStripped + ".hrz";
        binPath = trackBasePath + "/speedsf.bin";

        FrdFile frdFile;
        ColFile colFile;
        CanFile canFile;
        HrzFile hrzFile;
        SpeedsFile speedFile;

        ASSERT(FrdFile::Load(frdPath, frdFile), "Could not load FRD file: " << frdPath);                    // Load FRD file to get track block specific data
        ASSERT(ColFile::Load(colPath, colFile), "Could not load COL file: " << colPath);                    // Load Catalogue file to get global (non trkblock specific) data
        ASSERT(CanFile::Load(canPath, canFile), "Could not load CAN file (camera animation): " << canPath); // Load camera intro/outro animation data
        ASSERT(HrzFile::Load(hrzPath, hrzFile), "Could not load HRZ file (skybox/lighting):" << hrzPath);   // Load HRZ Data
        ASSERT(SpeedsFile::Load(binPath, speedFile), "Could not load speedsf.bin file (AI vroad speeds:" << binPath); // Load AI speed data

        track.nBlocks            = frdFile.nBlocks;
        track.cameraAnimation    = canFile.animPoints;
        track.trackTextureAssets = _ParseTextures(frdFile, track);
        track.trackBlocks        = _ParseTRKModels(frdFile, track);
        track.globalObjects      = _ParseCOLModels(colFile, track, frdFile.textureBlocks);
        track.virtualRoad        = _ParseVirtualRoad(colFile);

        // LOG(INFO) << "Track loaded successfully";

        return track;
    }

    Car::MetaData Loader::_ParseAssetData(const FceFile &fceFile, const FedataFile &fedataFile) {
        // LOG(INFO) << "Parsing FCE File into ONFS Structures";
        // All Vertices are stored so that the model is rotated 90 degs on X, 180 on Z. Remove this at Vert load time.
        Car::MetaData carMetadata;

        // Go get car metadata from FEDATA
        carMetadata.name = fedataFile.menuName;

        // Grab colours
        for (uint8_t colourIdx = 0; colourIdx < fceFile.nPriColours; ++colourIdx) {
            FceFile::Colour primaryColour = fceFile.primaryColours[colourIdx];
            Car::Colour originalPrimaryColour(fedataFile.primaryColourNames[colourIdx],
                                              TextureUtils::HSLToRGB(glm::vec4(primaryColour.H, primaryColour.S, primaryColour.B, primaryColour.T)));
            carMetadata.colours.emplace_back(originalPrimaryColour);
        }

        for (uint32_t dummyIdx = 0; dummyIdx < fceFile.nDummies; ++dummyIdx) {
            Car::Dummy dummy(fceFile.dummyNames[dummyIdx], fceFile.dummyCoords[dummyIdx] * NFS3_SCALE_FACTOR);
            carMetadata.dummies.emplace_back(dummy);
        }

        for (uint32_t partIdx = 0; partIdx < fceFile.nParts; ++partIdx) {
            std::vector<uint32_t> indices;
            std::vector<uint32_t> polygonFlags;
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> uvs;

            std::string part_name(fceFile.partNames[partIdx]);
            glm::vec3 center = fceFile.partCoords[partIdx] * NFS3_SCALE_FACTOR;

            for (uint32_t vert_Idx = 0; vert_Idx < fceFile.partNumVertices[partIdx]; ++vert_Idx) {
                vertices.emplace_back(fceFile.carParts[partIdx].vertices[vert_Idx] * NFS3_SCALE_FACTOR);
            }
            for (uint32_t normal_Idx = 0; normal_Idx < fceFile.partNumVertices[partIdx]; ++normal_Idx) {
                normals.emplace_back(fceFile.carParts[partIdx].normals[normal_Idx]);
            }
            for (uint32_t tri_Idx = 0; tri_Idx < fceFile.partNumTriangles[partIdx]; ++tri_Idx) {
                polygonFlags.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].polygonFlags);
                polygonFlags.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].polygonFlags);
                polygonFlags.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].polygonFlags);
                indices.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].vertex[0]);
                indices.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].vertex[1]);
                indices.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].vertex[2]);
                uvs.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].uvTable[0], 1.0f - fceFile.carParts[partIdx].triangles[tri_Idx].uvTable[3]);
                uvs.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].uvTable[1], 1.0f - fceFile.carParts[partIdx].triangles[tri_Idx].uvTable[4]);
                uvs.emplace_back(fceFile.carParts[partIdx].triangles[tri_Idx].uvTable[2], 1.0f - fceFile.carParts[partIdx].triangles[tri_Idx].uvTable[5]);
            }
            carMetadata.meshes.emplace_back(part_name, vertices, uvs, normals, indices, polygonFlags, center);
        }

        return carMetadata;
    }

    std::map<uint32_t, TrackTextureAsset> Loader::_ParseTextures(const FrdFile &frdFile, const Track &track) {
        std::map<uint32_t, TrackTextureAsset> textureAssetMap;
        size_t max_width = 0, max_height = 0;

        // Load QFS texture information into ONFS texture objects
        for (auto &frdTexBlock : frdFile.textureBlocks) {
            // Some TexBlocks don't appear to be genuine, though their QfsIndex seems sane. Skip over them, as their height is
            // disproportionate to the rest (approaching UINT16_MAX vs <= 256). This upsets Texture Array scaling.
            if (frdTexBlock.unknown1 == 0xFF000000 && frdTexBlock.unknown2 == 0xFF000000) {
                continue;
            }
            std::stringstream fileReference;
            std::stringstream alphaFileReference;

            // Find the maximum width and height, so we can avoid overestimating with blanket values (256x256) and thereby scale UV's uneccesarily
            max_width  = frdTexBlock.width > max_width ? frdTexBlock.width : max_width;
            max_height = frdTexBlock.height > max_height ? frdTexBlock.height : max_height;

            if (frdTexBlock.isLane) {
                fileReference << "../resources/sfx/" << std::setfill('0') << std::setw(4) << frdTexBlock.qfsIndex + 9 << ".BMP";
                alphaFileReference << "../resources/sfx/" << std::setfill('0') << std::setw(4) << frdTexBlock.qfsIndex + 9 << "-a.BMP";
            } else {
                fileReference << LibOpenNFS::TRACK_PATH << get_string(NFSVersion::NFS_3) << "/" << track.name << "/textures/" << std::setfill('0') << std::setw(4)
                              << frdTexBlock.qfsIndex << ".BMP";
                alphaFileReference << LibOpenNFS::TRACK_PATH << get_string(NFSVersion::NFS_3) << "/" << track.name << "/textures/" << std::setfill('0') << std::setw(4)
                                   << frdTexBlock.qfsIndex << "-a.BMP";
            }

            textureAssetMap[frdTexBlock.qfsIndex] = TrackTextureAsset(frdTexBlock.qfsIndex, frdTexBlock.width, frdTexBlock.height, fileReference.str(), alphaFileReference.str());
        }

        // Now that maximum width/height is known, set the Max U/V for the texture
        for (auto &[id, textureAsset] : textureAssetMap) {
            // Attempt to remove potential for sampling texture from transparent area
            textureAsset.maxU = (static_cast<float>(textureAsset.width) / static_cast<float>(max_width)) - 0.005f;
            textureAsset.maxV = (static_cast<float>(textureAsset.height) / static_cast<float>(max_height)) - 0.005f;
        }

        return textureAssetMap;
    }

    std::vector<TrackBlock> Loader::_ParseTRKModels(const FrdFile &frdFile, const Track &track) {
        // LOG(INFO) << "Parsing TRK file into ONFS GL structures";
        std::vector<TrackBlock> trackBlocks;
        trackBlocks.reserve(frdFile.nBlocks);

        /* TRKBLOCKS - BASE TRACK GEOMETRY */
        for (uint32_t trackblockIdx = 0; trackblockIdx < frdFile.nBlocks; ++trackblockIdx) {
            // Get Verts from Trk block, indices from associated polygon block
            TrkBlock rawTrackBlock      = frdFile.trackBlocks[trackblockIdx];
            PolyBlock trackPolygonBlock = frdFile.polygonBlocks[trackblockIdx];

            glm::vec3 rawTrackBlockCenter = rawTrackBlock.ptCentre * NFS3_SCALE_FACTOR;
            std::vector<uint32_t> trackBlockNeighbourIds;
            std::vector<glm::vec3> trackBlockVerts;
            std::vector<glm::vec4> trackBlockShadingData;

            // Get neighbouring block IDs
            for (auto &neighbourBlockData : frdFile.trackBlocks[trackblockIdx].nbdData) {
                if (neighbourBlockData.blk == -1) {
                    break;
                } else {
                    trackBlockNeighbourIds.emplace_back(neighbourBlockData.blk);
                }
            }

            // Build the base OpenNFS trackblock, to hold all the geometry and virtual road data, lights, sounds etc. for this portion of track
            TrackBlock trackBlock(trackblockIdx, rawTrackBlockCenter, rawTrackBlock.nStartPos, rawTrackBlock.nPositions, trackBlockNeighbourIds);

            // Light and sound sources
            for (uint32_t lightNum = 0; lightNum < rawTrackBlock.nLightsrc; ++lightNum) {
                glm::vec3 lightCenter = Utils::FixedToFloat(rawTrackBlock.lightsrc[lightNum].refpoint) * NFS3_SCALE_FACTOR;
                trackBlock.lights.emplace_back(lightNum, lightCenter, rawTrackBlock.lightsrc[lightNum].type);
            }
            for (uint32_t soundNum = 0; soundNum < rawTrackBlock.nSoundsrc; ++soundNum) {
                glm::vec3 soundCenter = Utils::FixedToFloat(rawTrackBlock.soundsrc[soundNum].refpoint) * NFS3_SCALE_FACTOR;
                trackBlock.sounds.emplace_back(soundNum, soundCenter, rawTrackBlock.soundsrc[soundNum].type);
            }

            // Get Trackblock roadVertices and per-vertex shading data
            for (uint32_t vertIdx = 0; vertIdx < rawTrackBlock.nObjectVert; ++vertIdx) {
                trackBlockVerts.emplace_back((rawTrackBlock.vert[vertIdx] * NFS3_SCALE_FACTOR) - rawTrackBlockCenter);
                trackBlockShadingData.emplace_back(TextureUtils::ShadingDataToVec4(rawTrackBlock.vertShading[vertIdx]));
            }

            // 4 OBJ Poly blocks
            for (uint32_t j = 0; j < 4; ++j) {
                ObjectPolyBlock polygonBlock = trackPolygonBlock.obj[j];

                if (polygonBlock.n1 > 0) {
                    // Iterate through objects in objpoly block up to num objects
                    for (uint32_t objectIdx = 0; objectIdx < polygonBlock.nobj; ++objectIdx) {
                        // Mesh Data
                        std::vector<uint32_t> vertexIndices;
                        std::vector<uint32_t> textureIndices;
                        std::vector<glm::vec2> uvs;
                        std::vector<glm::vec3> normals;
                        uint32_t accumulatedObjectFlags = 0u;

                        // Get Polygons in object
                        std::vector<PolygonData> objectPolygons = polygonBlock.poly[objectIdx];

                        for (uint32_t polyIdx = 0; polyIdx < polygonBlock.numpoly[objectIdx]; ++polyIdx) {
                            // Texture for this polygon and it's loaded OpenGL equivalent
                            TexBlock polygonTexture = frdFile.textureBlocks[objectPolygons[polyIdx].textureId];
                            // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                            TrackTextureAsset trackTextureAsset   = track.trackTextureAssets.at(polygonTexture.qfsIndex);
                            std::vector<glm::vec2> transformedUVs = trackTextureAsset.ScaleUVs(polygonTexture.GetUVs(), false, true);
                            uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                            // Calculate the normal, as the provided data is a little suspect
                            glm::vec3 normal = Utils::CalculateQuadNormal(rawTrackBlock.vert[objectPolygons[polyIdx].vertex[0]],
                                                                          rawTrackBlock.vert[objectPolygons[polyIdx].vertex[1]],
                                                                          rawTrackBlock.vert[objectPolygons[polyIdx].vertex[2]],
                                                                          rawTrackBlock.vert[objectPolygons[polyIdx].vertex[3]]);

                            // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                            for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                                normals.emplace_back(normal);
                                vertexIndices.emplace_back(objectPolygons[polyIdx].vertex[quadToTriVertNumber]);
                                textureIndices.emplace_back(polygonTexture.qfsIndex);
                            }

                            accumulatedObjectFlags |= objectPolygons[polyIdx].flags;
                        }
                        TrackGeometry trackBlockModel(trackBlockVerts, normals, uvs, textureIndices, vertexIndices, trackBlockShadingData, rawTrackBlockCenter);
                        trackBlock.objects.emplace_back((j + 1) * (objectIdx + 1), EntityType::OBJ_POLY, trackBlockModel, accumulatedObjectFlags);
                    }
                }
            }

            /* XOBJS - EXTRA OBJECTS */
            for (uint32_t l = (trackblockIdx * 4); l < (trackblockIdx * 4) + 4; ++l) {
                for (uint32_t j = 0; j < frdFile.extraObjectBlocks[l].nobj; ++j) {
                    // Mesh Data
                    std::vector<glm::vec3> extraObjectVerts;
                    std::vector<glm::vec4> extraObjectShadingData;
                    std::vector<uint32_t> vertexIndices;
                    std::vector<uint32_t> textureIndices;
                    std::vector<glm::vec2> uvs;
                    std::vector<glm::vec3> normals;
                    uint32_t accumulatedObjectFlags = 0u;

                    // Get the Extra object data for this trackblock object from the global xobj table
                    ExtraObjectData extraObjectData = frdFile.extraObjectBlocks[l].obj[j];

                    for (uint32_t vertIdx = 0; vertIdx < extraObjectData.nVertices; vertIdx++) {
                        extraObjectVerts.emplace_back(extraObjectData.vert[vertIdx] * NFS3_SCALE_FACTOR);
                        extraObjectShadingData.emplace_back(TextureUtils::ShadingDataToVec4(extraObjectData.vertShading[vertIdx]));
                    }

                    for (uint32_t k = 0; k < extraObjectData.nPolygons; k++) {
                        TexBlock blockTexture                 = frdFile.textureBlocks[extraObjectData.polyData[k].textureId];
                        TrackTextureAsset trackTextureAsset   = track.trackTextureAssets.at(blockTexture.qfsIndex);
                        std::vector<glm::vec2> transformedUVs = trackTextureAsset.ScaleUVs(blockTexture.GetUVs(), true, true);
                        uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                        glm::vec3 normal = Utils::CalculateQuadNormal(extraObjectVerts[extraObjectData.polyData[k].vertex[0]],
                                                                      extraObjectVerts[extraObjectData.polyData[k].vertex[1]],
                                                                      extraObjectVerts[extraObjectData.polyData[k].vertex[2]],
                                                                      extraObjectVerts[extraObjectData.polyData[k].vertex[3]]);

                        // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                        for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                            normals.emplace_back(normal);
                            vertexIndices.emplace_back(extraObjectData.polyData[k].vertex[quadToTriVertNumber]);
                            textureIndices.emplace_back(blockTexture.qfsIndex);
                        }

                        accumulatedObjectFlags |= extraObjectData.polyData[k].flags;
                    }
                    glm::vec3 extraObjectCenter    = extraObjectData.ptRef * NFS3_SCALE_FACTOR;
                    TrackGeometry extraObjectModel = TrackGeometry(extraObjectVerts, normals, uvs, textureIndices, vertexIndices, extraObjectShadingData, extraObjectCenter);
                    TrackEntity extraObjectEntity  = TrackEntity(l, EntityType::XOBJ, extraObjectModel, accumulatedObjectFlags);
                    trackBlock.objects.emplace_back(extraObjectEntity);
                }
            }

            // Road Mesh data
            std::vector<glm::vec3> roadVertices;
            std::vector<glm::vec4> roadShadingData;
            std::vector<uint32_t> vertexIndices;
            std::vector<uint32_t> textureIndices;
            std::vector<glm::vec2> uvs;
            std::vector<glm::vec3> normals;
            uint32_t accumulatedObjectFlags = 0u;

            for (uint32_t vertIdx = 0; vertIdx < rawTrackBlock.nVertices; ++vertIdx) {
                roadVertices.emplace_back((rawTrackBlock.vert[vertIdx] * NFS3_SCALE_FACTOR) - rawTrackBlockCenter);
                roadShadingData.emplace_back(TextureUtils::ShadingDataToVec4(rawTrackBlock.vertShading[vertIdx]));
            }
            // Get indices from Chunk 4 and 5 for High Res polys, Chunk 6 for Road Lanes
            for (uint32_t lodChunkIdx = 4; lodChunkIdx <= 6; lodChunkIdx++) {
                // If there are no lane markers in the lane chunk, skip
                if ((lodChunkIdx == 6) && (rawTrackBlock.nVertices <= rawTrackBlock.nHiResVert)) {
                    continue;
                }

                // Get the polygon data for this road section
                std::vector<PolygonData> chunkPolygonData = trackPolygonBlock.poly[lodChunkIdx];

                for (uint32_t polyIdx = 0; polyIdx < trackPolygonBlock.sz[lodChunkIdx]; polyIdx++) {
                    TexBlock polygonTexture               = frdFile.textureBlocks[chunkPolygonData[polyIdx].textureId];
                    TrackTextureAsset trackTextureAsset   = track.trackTextureAssets.at(polygonTexture.qfsIndex);
                    std::vector<glm::vec2> transformedUVs = trackTextureAsset.ScaleUVs(polygonTexture.GetUVs(), false, true);
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    glm::vec3 normal = Utils::CalculateQuadNormal(rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[0]],
                                                                  rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[1]],
                                                                  rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[2]],
                                                                  rawTrackBlock.vert[chunkPolygonData[polyIdx].vertex[3]]);

                    // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                    for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                        normals.emplace_back(normal);
                        vertexIndices.emplace_back(chunkPolygonData[polyIdx].vertex[quadToTriVertNumber]);
                        textureIndices.emplace_back(polygonTexture.qfsIndex);
                    }

                    accumulatedObjectFlags |= chunkPolygonData[polyIdx].flags;
                }
                TrackGeometry roadModel = TrackGeometry(roadVertices, normals, uvs, textureIndices, vertexIndices, roadShadingData, rawTrackBlockCenter);
                if (lodChunkIdx == 6) {
                    trackBlock.lanes.emplace_back(-1, EntityType::LANE, roadModel, accumulatedObjectFlags);
                } else {
                    trackBlock.track.emplace_back(-1, EntityType::ROAD, roadModel, accumulatedObjectFlags);
                }
            }
            trackBlocks.emplace_back(trackBlock);
        }
        return trackBlocks;
    }

    std::vector<TrackVRoad> Loader::_ParseVirtualRoad(const ColFile &colFile) {
        std::vector<TrackVRoad> virtualRoad;

        for (uint16_t vroadIdx = 0; vroadIdx < colFile.vroadHead.nrec; ++vroadIdx) {
            ColVRoad vroad = colFile.vroad[vroadIdx];

            // Transform NFS3/4 coords into ONFS 3d space
            glm::vec3 position = Utils::FixedToFloat(vroad.refPt) * NFS3_SCALE_FACTOR;
            position.y += 0.2f;

            // Get VROAD right vector
            glm::vec3 right   = glm::vec3(vroad.right) / 128.f;
            glm::vec3 forward = glm::vec3(vroad.forward);
            glm::vec3 normal  = glm::vec3(vroad.normal);

            glm::vec3 leftWall  = ((vroad.leftWall / 65536.0f) * NFS3_SCALE_FACTOR) * right;
            glm::vec3 rightWall = ((vroad.rightWall / 65536.0f) * NFS3_SCALE_FACTOR) * right;

            virtualRoad.emplace_back(position, glm::vec3(0, 0, 0), normal, forward, right, leftWall, rightWall, vroad.unknown);
        }

        return virtualRoad;
    }

    std::vector<TrackEntity> Loader::_ParseCOLModels(const ColFile &colFile, const Track &track, std::vector<TexBlock> &texBlocks) {
        // LOG(INFO) << "Parsing COL file into ONFS GL structures";
        std::vector<TrackEntity> colEntities;

        for (uint32_t i = 0; i < colFile.objectHead.nrec; ++i) {
            std::vector<uint32_t> indices;
            std::vector<glm::vec2> uvs;
            std::vector<uint32_t> texture_indices;
            std::vector<glm::vec3> verts;
            std::vector<glm::vec4> shading_data;
            std::vector<glm::vec3> norms;

            ColStruct3D s = colFile.struct3D[colFile.object[i].struct3D];

            for (uint32_t vertIdx = 0; vertIdx < s.nVert; ++vertIdx) {
                verts.emplace_back(s.vertex[vertIdx].pt * NFS3_SCALE_FACTOR);
                shading_data.emplace_back(TextureUtils::ShadingDataToVec4(s.vertex[vertIdx].unknown));
            }
            for (uint32_t polyIdx = 0; polyIdx < s.nPoly; ++polyIdx) {
                // Remap the COL TextureID's using the COL texture block (XBID2)
                ColTextureInfo colTexture = colFile.texture[s.polygon[polyIdx].texture];
                TexBlock frdTexture       = texBlocks.at(colTexture.id);
                // Retrieve the GL texture for it so can scale UVs into texture array
                TrackTextureAsset trackTextureAsset   = track.trackTextureAssets.at(colTexture.id);
                std::vector<glm::vec2> transformedUVs = trackTextureAsset.ScaleUVs(frdTexture.GetUVs(), false, true);
                uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                glm::vec3 normal =
                  Utils::CalculateQuadNormal(verts[s.polygon[polyIdx].v[0]], verts[s.polygon[polyIdx].v[1]], verts[s.polygon[polyIdx].v[2]], verts[s.polygon[polyIdx].v[3]]);

                // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                    indices.emplace_back(s.polygon[polyIdx].v[quadToTriVertNumber]);
                    norms.emplace_back(normal);
                    texture_indices.emplace_back(trackTextureAsset.id);
                }
            }
            glm::vec3 position = glm::vec3(colFile.object[i].ptRef) * NFS3_SCALE_FACTOR;
            colEntities.emplace_back(i, EntityType::GLOBAL, TrackGeometry(verts, norms, uvs, texture_indices, indices, shading_data, position), 0);
        }

        return colEntities;
    }
} // namespace LibOpenNFS::NFS3
