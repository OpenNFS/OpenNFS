#include "NFS2Loader.h"

#include <array>
#include "../../Renderer/Texture.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
std::shared_ptr<Car> NFS2Loader<Platform>::LoadCar(const std::string &carBasePath)
{
    /*boost::filesystem::path p(car_base_path);
    std::string car_name = p.filename().string();

    std::stringstream geo_path, psh_path, qfs_path, car_out_path;
    geo_path << car_base_path << ".GEO";
    psh_path << car_base_path << ".PSH";
    qfs_path << car_base_path << ".QFS";

    // For every file in here that's a BMP, load the data into a Texture object. This lets us easily access textures by an ID.
    std::map<unsigned int, Texture> car_textures;
    std::map<std::string, uint32_t> remapped_texture_ids;
    uint32_t remappedTextureID = 0;

    switch (version)
    {
    case NFS_3_PS1:
    case NFS_2_PS1:
        car_out_path << CAR_PATH << ToString(version) << "/" << car_name << "/";
        ImageLoader::ExtractPSH(psh_path.str(), car_out_path.str());
        break;
    case NFS_2:
    case NFS_2_SE:
        car_out_path << CAR_PATH << ToString(version) << "/" << car_name << "/";
        ImageLoader::ExtractQFS(qfs_path.str(), car_out_path.str());
        break;
    default:
        ASSERT(false, "I shouldn't be loading this version (" << version << ") and you know it.");
    }

    for (boost::filesystem::directory_iterator itr(car_out_path.str()); itr != boost::filesystem::directory_iterator(); ++itr)
    {
        if (itr->path().filename().string().find("BMP") != std::string::npos && itr->path().filename().string().find("-a") == std::string::npos)
        {
            // Map texture names, strings, into numbers so I can use them for indexes into the eventual Texture Array
            remapped_texture_ids[itr->path().filename().replace_extension("").string()] = remappedTextureID++;
            switch (version)
            {
            case NFS_3_PS1:
            case NFS_2_PS1:
                GLubyte *data;
                GLsizei width;
                GLsizei height;
                ASSERT(ImageLoader::LoadBmpCustomAlpha(itr->path().string().c_str(), &data, &width, &height, 0u),
                       "Texture " << itr->path().string() << " did not load succesfully!");
                car_textures[remapped_texture_ids[itr->path().filename().replace_extension("").string()]] =
                  Texture(remapped_texture_ids[itr->path().filename().replace_extension("").string()], data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
                break;
            case NFS_2:
            case NFS_2_SE:
                bmpread_t bmpAttr; // This will leak.
                ASSERT(bmpread(itr->path().string().c_str(), BMPREAD_ANY_SIZE | BMPREAD_ALPHA, &bmpAttr), "Texture " << itr->path().string() << " did not load succesfully!");
                car_textures[remapped_texture_ids[itr->path().filename().replace_extension("").string()]] =
                  Texture(remapped_texture_ids[itr->path().filename().replace_extension("").string()],
                          bmpAttr.data,
                          static_cast<unsigned int>(bmpAttr.width),
                          static_cast<unsigned int>(bmpAttr.height));
                break;
            default:
                ASSERT(false, "I shouldn't be loading this version (" << version << ") and you know it.");
            }
        }
    }

    // This must run before geometry load, as it will affect UV's
    GLint textureArrayID = MakeTextureArray(car_textures, false);
    CarData carData;
    carData.meshes = LoadGEO(geo_path.str(), car_textures, remapped_texture_ids);
    return std::make_shared<Car>(carData, version, car_name, textureArrayID);*/
}

template <>
std::shared_ptr<Track> NFS2Loader<PC>::LoadTrack(const std::string &trackBasePath)
{
    LOG(INFO) << "Loading Track located at " << trackBasePath;

    auto track = std::make_shared<Track>(Track());
    track->tag = (trackBasePath.find(ToString(NFS_2_SE)) != std::string::npos) ? NFS_2_SE : NFS_2;

    boost::filesystem::path p(trackBasePath);
    track->name = p.filename().string();
    std::string trkPath, colPath, canPath;

    trkPath = trackBasePath + ".TRK";
    colPath = trackBasePath + ".COL";
    canPath = RESOURCE_PATH + ToString(track->tag) + "/GAMEDATA/TRACKS/DATA/PC/" + track->name + "00.CAN";

    TrkFile<PC> trkFile;
    ColFile<PC> colFile;
    CanFile canFile;

    // ASSERT(Texture::ExtractTrackTextures(trackBasePath, track->name, track->tag), "Could not extract " << track->name << " texture pack.");
    ASSERT(CanFile::Load(canPath, canFile), "Could not load CAN file (camera animation): " << canPath); // Load camera intro/outro animation data
    ASSERT(TrkFile<PC>::Load(trkPath, trkFile), "Could not load TRK file: " << trkPath);                // Load TRK file to get track block specific data
    ASSERT(ColFile<PC>::Load(colPath, colFile), "Could not load COL file: " << colPath);                // Load Catalogue file to get global (non block specific) data

    // Load up the textures
    auto &textureBlock = colFile.extraObjectBlocks[ExtraBlockID::TEXTURE_BLOCK_ID];
    for (uint32_t texIdx = 0; texIdx < textureBlock.nTextures; texIdx++)
    {
        TEXTURE_BLOCK test = textureBlock.polyToQfsTexTable[texIdx];
        track->textureMap[textureBlock.polyToQfsTexTable[texIdx].texNumber] = Texture::LoadTexture(track->tag, textureBlock.polyToQfsTexTable[texIdx], track->name);
    }

    track->textureArrayID  = Texture::MakeTextureArray(track->textureMap, false);
    track->nBlocks         = trkFile.nBlocks;
    track->cameraAnimation = canFile.animPoints;
    track->trackBlocks     = _ParseTRKModels(trkFile, colFile, track);
    track->globalObjects   = _ParseCOLModels(colFile, track);
    track->virtualRoad     = _ParseVirtualRoad(colFile);

    LOG(INFO) << "Track loaded successfully";

    return track;
}

template <>
std::shared_ptr<Track> NFS2Loader<PS1>::LoadTrack(const std::string &trackBasePath)
{
    LOG(INFO) << "Loading Track located at " << trackBasePath;
    auto track = std::make_shared<Track>(Track());
    track->tag = NFS_3_PS1;

    boost::filesystem::path p(trackBasePath);
    track->name = p.filename().string();
    std::string trkPath, colPath;

    trkPath = trackBasePath + ".TRK";
    colPath = trackBasePath + ".COL";
    colPath.replace(colPath.find("ZZ"), 2, "");

    TrkFile<PS1> trkFile;
    ColFile<PS1> colFile;

    // ASSERT(Texture::ExtractTrackTextures(trackBasePath, track->name, track->tag), "Could not extract " << track->name << " texture pack.");
    ASSERT(TrkFile<PS1>::Load(trkPath, trkFile), "Could not load TRK file: " << trkPath); // Load TRK file to get track block specific data
    ASSERT(ColFile<PS1>::Load(colPath, colFile), "Could not load COL file: " << colPath); // Load Catalogue file to get global (non block specific) data

    // Load up the textures
    auto &textureBlock = colFile.extraObjectBlocks[ExtraBlockID::TEXTURE_BLOCK_ID];
    for (uint32_t texIdx = 0; texIdx < textureBlock.nTextures; texIdx++)
    {
        track->textureMap[textureBlock.polyToQfsTexTable[texIdx].texNumber] = Texture::LoadTexture(track->tag, textureBlock.polyToQfsTexTable[texIdx], track->name);
    }

    track->textureArrayID = Texture::MakeTextureArray(track->textureMap, false);
    track->nBlocks        = trkFile.nBlocks;
    track->trackBlocks    = _ParseTRKModels(trkFile, colFile, track);
    track->globalObjects  = _ParseCOLModels(colFile, track);
    track->virtualRoad    = _ParseVirtualRoad(colFile);

    LOG(INFO) << "Track loaded successfully";

    return track;
}

// One might question why a TRK parsing function requires the COL file too. Simples, we need XBID 2 for Texture remapping during ONFS texgen.
template <typename Platform>
std::vector<OpenNFS::TrackBlock> NFS2Loader<Platform>::_ParseTRKModels(const TrkFile<Platform> &trkFile, ColFile<Platform> &colFile, const std::shared_ptr<Track> &track)
{
    LOG(INFO) << "Parsing TRK file into ONFS GL structures";
    std::vector<OpenNFS::TrackBlock> trackBlocks;

    // Pull out a shorter reference to the texture table
    auto &polyToQfsTexTable = colFile.extraObjectBlocks[ExtraBlockID::TEXTURE_BLOCK_ID].polyToQfsTexTable;

    // Parse out TRKBlock data
    for (const auto &superBlock : trkFile.superBlocks)
    {
        for (auto rawTrackBlock : superBlock.trackBlocks)
        {
            // Get position all vertices need to be relative to
            glm::quat orientation         = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
            glm::vec3 rawTrackBlockCenter = orientation * (Utils::PointToVec(trkFile.blockReferenceCoords[rawTrackBlock.serialNum]) / NFS2_SCALE_FACTOR);
            // Convert the neighbor uint16_t's to uint32_t for OFNS trackblock representation
            std::vector<uint32_t> trackBlockNeighbourIds(rawTrackBlock.extraObjectBlocks[ExtraBlockID::NEIGHBOUR_BLOCK_ID].blockNeighbours.begin(),
                                                         rawTrackBlock.extraObjectBlocks[ExtraBlockID::NEIGHBOUR_BLOCK_ID].blockNeighbours.end());

            OpenNFS::TrackBlock trackBlock(rawTrackBlock.serialNum, rawTrackBlockCenter, rawTrackBlock.serialNum, 1, trackBlockNeighbourIds);

            // Collate all available Structure References, 3 different ID types can store this information, check them all
            std::vector<StructureRefBlock> structureReferences;
            std::array<ExtraBlockID, 3> structRefBlockIds = {ExtraBlockID::STRUCTURE_REF_BLOCK_A_ID, ExtraBlockID::STRUCTURE_REF_BLOCK_B_ID,
                                                             ExtraBlockID::STRUCTURE_REF_BLOCK_C_ID};
            for (auto &structRefBlockId : structRefBlockIds)
            {
                if (rawTrackBlock.extraObjectBlocks.count(structRefBlockId))
                {
                    structureReferences.insert(structureReferences.end(),
                                               rawTrackBlock.extraObjectBlocks[structRefBlockId].structureReferences.begin(),
                                               rawTrackBlock.extraObjectBlocks[structRefBlockId].structureReferences.end());
                }
            }

            if (rawTrackBlock.extraObjectBlocks[ExtraBlockID::STRUCTURE_BLOCK_ID].nStructures != structureReferences.size())
            {
                LOG(WARNING) << "Trk block " << (int) rawTrackBlock.serialNum << " is missing "
                             << rawTrackBlock.extraObjectBlocks[ExtraBlockID::STRUCTURE_BLOCK_ID].nStructures - structureReferences.size() << " structure locations!";
            }

            // Structures
            for (uint32_t structureIdx = 0; structureIdx < rawTrackBlock.extraObjectBlocks[ExtraBlockID::STRUCTURE_BLOCK_ID].nStructures; ++structureIdx)
            {
                // Mesh Data
                std::vector<unsigned int> structureVertexIndices;
                std::vector<glm::vec2> structureUVs;
                std::vector<unsigned int> structureTextureIndices;
                std::vector<glm::vec3> structureVertices;
                std::vector<glm::vec4> structureShadingData;
                std::vector<glm::vec3> structureNormals;

                // Shorter reference to structures for trackblock
                auto &structures = rawTrackBlock.extraObjectBlocks[ExtraBlockID::STRUCTURE_BLOCK_ID].structures;

                VERT_HIGHP structureReferenceCoordinates = trkFile.blockReferenceCoords[rawTrackBlock.serialNum];
                bool refCoordsFound                      = false;
                // Find the structure reference that matches this structure, else use block default
                for (auto &structureReference : structureReferences)
                {
                    // Only check fixed type structure references
                    if (structureReference.structureRef == structureIdx)
                    {
                        if (structureReference.recType == 1 || structureReference.recType == 4)
                        {
                            structureReferenceCoordinates = structureReference.refCoordinates;
                            refCoordsFound                = true;
                            break;
                        }
                        else if (structureReference.recType == 3)
                        {
                            // For now, if animated, use position 0 of animation sequence
                            structureReferenceCoordinates = structureReference.animationData[0].position;
                            refCoordsFound                = true;
                            break;
                        }
                    }
                }
                if (!refCoordsFound)
                {
                    LOG(WARNING) << "Couldn't find a reference coordinate for Structure " << structureIdx << " in TB" << rawTrackBlock.serialNum;
                }
                for (uint16_t vertIdx = 0; vertIdx < structures[structureIdx].nVerts; ++vertIdx)
                {
                    structureVertices.emplace_back(orientation *
                                                   (((256.f * Utils::PointToVec(structures[structureIdx].vertexTable[vertIdx])) / NFS2_SCALE_FACTOR) - rawTrackBlockCenter));
                    structureShadingData.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
                }
                for (uint32_t polyIdx = 0; polyIdx < structures[structureIdx].nPoly; ++polyIdx)
                {
                    // Remap the COL TextureID's using the COL texture block (XBID2)
                    TEXTURE_BLOCK polygonTexture = polyToQfsTexTable[structures[structureIdx].polygonTable[polyIdx].texture];
                    Texture glTexture            = track->textureMap[polygonTexture.texNumber];
                    // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                    std::vector<glm::vec2> transformedUVs = glTexture.GenerateUVs(XOBJ, polygonTexture.alignmentData);
                    structureUVs.insert(structureUVs.end(), transformedUVs.begin(), transformedUVs.end());

                    // Calculate the normal, as no provided data
                    glm::vec3 normal = Utils::CalculateQuadNormal(verts[structures[structureIdx].polygonTable[polyIdx].vertex[0]],
                                                                  verts[structures[structureIdx].polygonTable[polyIdx].vertex[1]],
                                                                  verts[structures[structureIdx].polygonTable[polyIdx].vertex[2]],
                                                                  verts[structures[structureIdx].polygonTable[polyIdx].vertex[3]]);

                    // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                    for (auto &quadToTriVertNumber : quadToTriVertNumbers)
                    {
                        structureNormals.emplace_back(normal);
                        structureVertexIndices.emplace_back(structures[structureIdx].polygonTable[polyIdx].vertex[quadToTriVertNumber]);
                        structureTextureIndices.emplace_back(polygonTexture.texNumber);
                    }
                }

                TrackModel trackBlockModel =
                  TrackModel(structureVertices, structureNormals, structureUVs, structureTextureIndices, structureVertexIndices, structureShadingData, rawTrackBlockCenter);
                Entity trackBlockEntity = Entity(rawTrackBlock.serialNum, structureIdx, track->tag, OBJ_POLY, trackBlockModel, 0);
                trackBlock.objects.emplace_back(trackBlockEntity);
            }

            // Track Mesh Data
            std::vector<unsigned int> trackBlockVertexIndices;
            std::vector<glm::vec2> trackBlockUVs;
            std::vector<unsigned int> trackBlockTextureIndices;
            std::vector<glm::vec3> trackBlockVertices;
            std::vector<glm::vec4> trackBlockShadingData;
            std::vector<glm::vec3> trackBlockNormals;

            for (int32_t i = 0; i < rawTrackBlock.nStickToNextVerts + rawTrackBlock.nHighResVert; i++)
            {
                // Base Track Geometry
                VERT_HIGHP blockRefCoord;

                if (i < rawTrackBlock.nStickToNextVerts)
                {
                    // If in last block go get ref coord of first block, else get ref of next block
                    blockRefCoord = (rawTrackBlock.serialNum == track->nBlocks - 1) ? trkFile.blockReferenceCoords[0] : trkFile.blockReferenceCoords[rawTrackBlock.serialNum + 1];
                }
                else
                {
                    blockRefCoord = trkFile.blockReferenceCoords[rawTrackBlock.serialNum];
                }

                trackBlockVertices.emplace_back(
                  orientation * ((Utils::PointToVec(blockRefCoord) + (256.f * Utils::PointToVec(rawTrackBlock.vertexTable[i])) / NFS2_SCALE_FACTOR) - rawTrackBlockCenter));
                trackBlockShadingData.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
            }
            for (int32_t polyIdx = (rawTrackBlock.nLowResPoly + rawTrackBlock.nMedResPoly);
                 polyIdx < (rawTrackBlock.nLowResPoly + rawTrackBlock.nMedResPoly + rawTrackBlock.nHighResPoly);
                 ++polyIdx)
            {
                // Remap the COL TextureID's using the COL texture block (XBID2)
                TEXTURE_BLOCK polygonTexture = polyToQfsTexTable[rawTrackBlock.polygonTable[polyIdx].texture];
                Texture glTexture            = track->textureMap[polygonTexture.texNumber];
                // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                std::vector<glm::vec2> transformedUVs = glTexture.GenerateUVs(ROAD, polygonTexture.alignmentData);
                trackBlockUVs.insert(trackBlockUVs.end(), transformedUVs.begin(), transformedUVs.end());
                // Calculate the normal, as no provided data
                glm::vec3 normal = Utils::CalculateQuadNormal(trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[0]],
                                                              trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[1]],
                                                              trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[2]],
                                                              trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[3]]);

                // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                for (auto &quadToTriVertNumber : quadToTriVertNumbers)
                {
                    trackBlockNormals.emplace_back(normal);
                    trackBlockVertexIndices.emplace_back(rawTrackBlock.polygonTable[polyIdx].vertex[quadToTriVertNumber]);
                    trackBlockTextureIndices.emplace_back(polygonTexture.texNumber);
                }
            }

            TrackModel trackBlockModel =
              TrackModel(trackBlockVertices, trackBlockNormals, trackBlockUVs, trackBlockTextureIndices, trackBlockVertexIndices, trackBlockShadingData, rawTrackBlockCenter);
            Entity trackBlockEntity = Entity(rawTrackBlock.serialNum, rawTrackBlock.serialNum, track->tag, OBJ_POLY, trackBlockModel, 0);
            trackBlock.track.push_back(trackBlockEntity);

            // Add the parsed ONFS trackblock to the list of trackblocks
            trackBlocks.push_back(trackBlock);
        }
    }
}

template <typename Platform>
std::vector<VirtualRoad> NFS2Loader<Platform>::_ParseVirtualRoad(const ColFile<Platform> &colFile)
{
    std::vector<VirtualRoad> virtualRoad;

    return virtualRoad;
}

template <typename Platform>
std::vector<Entity> NFS2Loader<Platform>::_ParseCOLModels(const ColFile<Platform> &colFile, const std::shared_ptr<Track> &track)
{
    /*LOG(INFO) << "Parsing COL file into ONFS GL structures";

    glm::quat rotationMatrix =
      glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    std::vector<Entity> col_entities;

    // Parse out COL data
    for (uint32_t structure_Idx = 0; structure_Idx < track->nColStructures; ++structure_Idx)
    {
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> texture_indices;
        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> shading_data;

        auto *structureReferenceCoordinates = static_cast<VERT_HIGHP *>(calloc(1, sizeof(VERT_HIGHP)));
        // Find the structure reference that matches this structure, else use block default
        for (auto &structure : track->colStructureRefData)
        {
            // Only check fixed type structure references
            if (structure.structureRef == structure_Idx)
            {
                if (structure.recType == 1 || structure.recType == 4)
                {
                    structureReferenceCoordinates = &structure.refCoordinates;
                    break;
                }
                else if (structure.recType == 3)
                {
                    // For now, if animated, use position 0 of animation sequence
                    structureReferenceCoordinates = &structure.animationData[0].position;
                    break;
                }
            }
        }
        if (!structureReferenceCoordinates->x || !structureReferenceCoordinates->y || !structureReferenceCoordinates->z)
        {
            LOG(WARNING) << "Couldn't find a reference coordinate for Structure " << structure_Idx << " in COL file";
        }
        for (uint16_t vert_Idx = 0; vert_Idx < track->colStructures[structure_Idx].nVerts; ++vert_Idx)
        {
            int32_t x = ((256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].x));
            int32_t y = ((256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].y));
            int32_t z = ((256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].z));
            verts.emplace_back(rotationMatrix * glm::vec3(x / NFS2_SCALE_FACTOR, z / NFS2_SCALE_FACTOR, y / NFS2_SCALE_FACTOR));
            shading_data.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
        }

        for (uint32_t poly_Idx = 0; poly_Idx < track->colStructures[structure_Idx].nPoly; ++poly_Idx)
        {
            // Remap the COL TextureID's using the COL texture block (XBID2)
            TEXTURE_BLOCK texture_for_block = track->polyToQFStexTable[track->colStructures[structure_Idx].polygonTable[poly_Idx].texture];
            Texture gl_texture              = track->textures[texture_for_block.texNumber];
            indices.emplace_back(track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[0]);
            indices.emplace_back(track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[1]);
            indices.emplace_back(track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[2]);
            indices.emplace_back(track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[0]);
            indices.emplace_back(track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[2]);
            indices.emplace_back(track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[3]);
            // TODO: Use textures alignment data to modify these UV's
            uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
            uvs.emplace_back(0.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
            uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
            uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
            uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
            uvs.emplace_back(1.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
            texture_indices.emplace_back(texture_for_block.texNumber);
            texture_indices.emplace_back(texture_for_block.texNumber);
            texture_indices.emplace_back(texture_for_block.texNumber);
            texture_indices.emplace_back(texture_for_block.texNumber);
            texture_indices.emplace_back(texture_for_block.texNumber);
            texture_indices.emplace_back(texture_for_block.texNumber);
        }
        glm::vec3 position = rotationMatrix * glm::vec3(structureReferenceCoordinates->x / NFS2_SCALE_FACTOR,
                                                        structureReferenceCoordinates->y / NFS2_SCALE_FACTOR,
                                                        structureReferenceCoordinates->z / NFS2_SCALE_FACTOR);
        col_entities.emplace_back(Entity(0, structure_Idx, NFS_2, GLOBAL, TrackModel(verts, uvs, texture_indices, indices, shading_data, position), 0));
        // free(structureReferenceCoordinates);
    }
    return col_entities;*/
}
