#include "NFS2Loader.h"

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
    /*for (uint32_t texIdx = 0; texIdx < track->nTextures; texIdx++)
    {
        track->textureMap[colFile.polyToQFStexTable[texIdx].texNumber] = Texture::LoadTexture(track->polyToQFStexTable[texIdx], track->name, track->tag);
    }*/

    track->textureArrayID  = Texture::MakeTextureArray(track->textureMap, false);
    track->nBlocks         = trkFile.nBlocks;
    track->cameraAnimation = canFile.animPoints;
    track->trackBlocks     = _ParseTRKModels(trkFile, track);
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
    colPath = trackBasePath + ".COL";
    colPath.replace(colPath.find("ZZ"), 2, "");

    TrkFile<PS1> trkFile;
    ColFile<PS1> colFile;

    // ASSERT(Texture::ExtractTrackTextures(trackBasePath, track->name, track->tag), "Could not extract " << track->name << " texture pack.");
    ASSERT(TrkFile<PS1>::Load(trkPath, trkFile), "Could not load TRK file: " << trkPath); // Load TRK file to get track block specific data
    ASSERT(ColFile<PS1>::Load(colPath, colFile), "Could not load COL file: " << colPath); // Load Catalogue file to get global (non block specific) data

    // Load up the textures
    /*for (uint32_t texIdx = 0; texIdx < track->nTextures; texIdx++)
    {
        track->textureMap[colFile.polyToQFStexTable[texIdx].texNumber] = Texture::LoadTexture(track->polyToQFStexTable[texIdx], track->name, track->tag);
    }*/

    track->textureArrayID = Texture::MakeTextureArray(track->textureMap, false);
    track->nBlocks        = trkFile.nBlocks;
    track->trackBlocks    = _ParseTRKModels(trkFile, track);
    track->globalObjects  = _ParseCOLModels(colFile, track);
    track->virtualRoad    = _ParseVirtualRoad(colFile);

    LOG(INFO) << "Track loaded successfully";

    return track;
}

template <typename Platform>
std::vector<OpenNFS::TrackBlock> NFS2Loader<Platform>::_ParseTRKModels(const TrkFile<Platform> &trkFile, const std::shared_ptr<Track> &track)
{
    LOG(INFO) << "Parsing TRK file into ONFS GL structures";

    glm::quat rotationMatrix =
      glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    // Parse out TRKBlock data
    /*for (uint32_t superBlock_Idx = 0; superBlock_Idx < trkFile.nSuperBlocks; ++superBlock_Idx)
    {
        SuperBlock<Platform> superBlock = trkFile.superBlocks[superBlock_Idx];

        for (uint32_t blockIdx = 0; blockIdx < trkFile.nBlocks; ++blockIdx)
        {
            // Base Track Geometry
            auto trkBlock = superBlock.trackBlocks[blockIdx];
            VERT_HIGHP blockReferenceCoord;

            glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
            OpenNFS::TrackBlock trackBlock(trkBlock.blockSerial,
                                           orientation * glm::vec3(trkFile.blockReferenceCoords[trkBlock.blockSerial].x / NFS2_SCALE_FACTOR,
                                                                   trkFile.blockReferenceCoords[trkBlock.blockSerial].y / NFS2_SCALE_FACTOR,
                                                                   trkFile.blockReferenceCoords[trkBlock.blockSerial].z / NFS2_SCALE_FACTOR));

            if (trkBlock.nStructures != trkBlock.nStructureReferences)
            {
                LOG(WARNING) << "Trk block " << (int) trkBlock.header->blockSerial << " is missing " << trkBlock.nStructures - trkBlock.nStructureReferences
                             << " structure locations!";
            }

            // Structures
            for (uint32_t structure_Idx = 0; structure_Idx < trkBlock.nStructures; ++structure_Idx)
            {
                // Mesh Data
                std::vector<unsigned int> vertex_indices;
                std::vector<glm::vec2> uvs;
                std::vector<unsigned int> texture_indices;
                std::vector<glm::vec3> verts;
                std::vector<glm::vec4> shading_verts;
                std::vector<glm::vec3> norms;
                std::vector<uint32_t> debug_data;

                VERT_HIGHP *structureReferenceCoordinates = &track->blockReferenceCoords[trkBlock.header->blockSerial];
                // Find the structure reference that matches this structure, else use block default
                for (auto &structure : trkBlock.structureRefData)
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
                if (structureReferenceCoordinates == &track->blockReferenceCoords[trkBlock.header->blockSerial])
                {
                    LOG(WARNING) << "Couldn't find a reference coordinate for Structure " << structure_Idx << " in SB" << superBlock_Idx << "TB" << blockIdx;
                }
                for (uint16_t vert_Idx = 0; vert_Idx < trkBlock.structures[structure_Idx].nVerts; ++vert_Idx)
                {
                    int32_t x = structureReferenceCoordinates->x + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].x);
                    int32_t y = structureReferenceCoordinates->y + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].y);
                    int32_t z = structureReferenceCoordinates->z + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].z);
                    verts.emplace_back(rotationMatrix * glm::vec3(x / NFS2_SCALE_FACTOR, y / NFS2_SCALE_FACTOR, z / NFS2_SCALE_FACTOR));
                    shading_verts.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
                }
                for (uint32_t poly_Idx = 0; poly_Idx < trkBlock.structures[structure_Idx].nPoly; ++poly_Idx)
                {
                    // Remap the COL TextureID's using the COL texture block (XBID2)
                    TEXTURE_BLOCK texture_for_block = track->polyToQFStexTable[trkBlock.structures[structure_Idx].polygonTable[poly_Idx].texture];
                    Texture gl_texture              = track->textures[texture_for_block.texNumber];
                    vertex_indices.emplace_back(trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[0]);
                    vertex_indices.emplace_back(trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[1]);
                    vertex_indices.emplace_back(trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[2]);
                    vertex_indices.emplace_back(trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[0]);
                    vertex_indices.emplace_back(trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[2]);
                    vertex_indices.emplace_back(trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[3]);
                    // TODO: Use textures alignment data to modify these UV's
                    debug_data.emplace_back(texture_for_block.alignmentData);
                    debug_data.emplace_back(texture_for_block.alignmentData);
                    debug_data.emplace_back(texture_for_block.alignmentData);
                    debug_data.emplace_back(texture_for_block.alignmentData);
                    debug_data.emplace_back(texture_for_block.alignmentData);
                    debug_data.emplace_back(texture_for_block.alignmentData);

                    std::vector<glm::vec2> transformedUVs = GenerateUVs(std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, XOBJ, texture_for_block.alignmentData, gl_texture);
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    texture_indices.emplace_back(texture_for_block.texNumber);
                    texture_indices.emplace_back(texture_for_block.texNumber);
                    texture_indices.emplace_back(texture_for_block.texNumber);
                    texture_indices.emplace_back(texture_for_block.texNumber);
                    texture_indices.emplace_back(texture_for_block.texNumber);
                    texture_indices.emplace_back(texture_for_block.texNumber);
                    // TODO: Calculate normals properly
                    norms.emplace_back(glm::vec3(1, 1, 1));
                    norms.emplace_back(glm::vec3(1, 1, 1));
                    norms.emplace_back(glm::vec3(1, 1, 1));
                    norms.emplace_back(glm::vec3(1, 1, 1));
                    norms.emplace_back(glm::vec3(1, 1, 1));
                    norms.emplace_back(glm::vec3(1, 1, 1));
                }

                trackBlock.objects.emplace_back(Entity(superBlock_Idx,
                                                                (trkBlock.header->blockSerial * trkBlock.nStructures) * structure_Idx,
                                                                std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2,
                                                                XOBJ,
                                                                TrackModel(verts, norms, uvs, texture_indices, vertex_indices, shading_verts, debug_data, glm::vec3(0, 0, 0)),
                                                                0));
            }

            // Mesh Data
            std::vector<unsigned int> vertex_indices;
            std::vector<glm::vec2> uvs;
            std::vector<unsigned int> texture_indices;
            std::vector<glm::vec3> verts;
            std::vector<glm::vec4> trk_block_shading_verts;
            std::vector<glm::vec3> norms;
            std::vector<uint32_t> debug_data;

            for (int32_t i = 0; i < trkBlock.header->nStickToNextVerts + trkBlock.header->nHighResVert; i++)
            {
                if (i < trkBlock.header->nStickToNextVerts)
                {
                    // If in last block go get ref coord of first block, else get ref of next block
                    blockReferenceCoord =
                      (trkBlock.header->blockSerial == track->nBlocks - 1) ? track->blockReferenceCoords[0] : track->blockReferenceCoords[trkBlock.header->blockSerial + 1];
                }
                else
                {
                    blockReferenceCoord = track->blockReferenceCoords[trkBlock.header->blockSerial];
                }
                int32_t x = (blockReferenceCoord.x + (256 * trkBlock.vertexTable[i].x));
                int32_t y = (blockReferenceCoord.y + (256 * trkBlock.vertexTable[i].y));
                int32_t z = (blockReferenceCoord.z + (256 * trkBlock.vertexTable[i].z));
                verts.emplace_back(rotationMatrix * glm::vec3(x / NFS2_SCALE_FACTOR, y / NFS2_SCALE_FACTOR, z / NFS2_SCALE_FACTOR));
                trk_block_shading_verts.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
            }
            for (int32_t poly_Idx = (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly);
                 poly_Idx < (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly + trkBlock.header->nHighResPoly);
                 ++poly_Idx)
            {
                // Remap the COL TextureID's using the COL texture block (XBID2)
                TEXTURE_BLOCK texture_for_block = track->polyToQFStexTable[trkBlock.polygonTable[poly_Idx].texture];
                Texture gl_texture              = track->textures[texture_for_block.texNumber];
                vertex_indices.emplace_back(trkBlock.polygonTable[poly_Idx].vertex[0]);
                vertex_indices.emplace_back(trkBlock.polygonTable[poly_Idx].vertex[1]);
                vertex_indices.emplace_back(trkBlock.polygonTable[poly_Idx].vertex[2]);
                vertex_indices.emplace_back(trkBlock.polygonTable[poly_Idx].vertex[0]);
                vertex_indices.emplace_back(trkBlock.polygonTable[poly_Idx].vertex[2]);
                vertex_indices.emplace_back(trkBlock.polygonTable[poly_Idx].vertex[3]);

                // TODO: Use textures alignment data to modify these UV's
                debug_data.emplace_back(texture_for_block.alignmentData);
                debug_data.emplace_back(texture_for_block.alignmentData);
                debug_data.emplace_back(texture_for_block.alignmentData);
                debug_data.emplace_back(texture_for_block.alignmentData);
                debug_data.emplace_back(texture_for_block.alignmentData);
                debug_data.emplace_back(texture_for_block.alignmentData);

                std::vector<glm::vec2> transformedUVs = GenerateUVs(std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, ROAD, texture_for_block.alignmentData, gl_texture);
                uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                texture_indices.emplace_back(texture_for_block.texNumber);
                texture_indices.emplace_back(texture_for_block.texNumber);
                texture_indices.emplace_back(texture_for_block.texNumber);
                texture_indices.emplace_back(texture_for_block.texNumber);
                texture_indices.emplace_back(texture_for_block.texNumber);
                texture_indices.emplace_back(texture_for_block.texNumber);
                // TODO: Calculate normals properly
                norms.emplace_back(glm::vec3(1, 1, 1));
                norms.emplace_back(glm::vec3(1, 1, 1));
                norms.emplace_back(glm::vec3(1, 1, 1));
                norms.emplace_back(glm::vec3(1, 1, 1));
                norms.emplace_back(glm::vec3(1, 1, 1));
                norms.emplace_back(glm::vec3(1, 1, 1));
            }
            trackBlock.track.emplace_back(Entity(superBlock_Idx,
                                                          trkBlock.blockSerial,
                                                          std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2,
                                                          ROAD,
                                                          TrackModel(verts, norms, uvs, texture_indices, vertex_indices, trk_block_shading_verts, debug_data, glm::vec3(0, 0, 0)),
                                                          0));

            track->track_blocks.emplace_back(trackBlock);
        }
    }*/
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
/*

template <typename Platform>
Texture NFS2<Platform>::LoadTexture(TEXTURE_BLOCK track_texture, const std::string &track_name, NFSVer nfs_version)
{
    std::stringstream filename;
    uint8_t alphaColour = 0;
    filename << TRACK_PATH << ToString(nfs_version) << "/";

    switch (nfs_version)
    {
    case NFS_2:
        alphaColour = 0u;
        break;
    case NFS_2_SE:
        alphaColour = 248u;
        break;
    case NFS_3_PS1:
        break;
    case UNKNOWN:
    default:
        ASSERT(false, "Trying to load texture from unknown NFS version");
        break;
    }
    filename << track_name << "/textures/" << std::setfill('0') << std::setw(4) << track_texture.texNumber << ".BMP";

    GLubyte *data;
    GLsizei width;
    GLsizei height;

    ASSERT(ImageLoader::LoadBmpCustomAlpha(filename.str().c_str(), &data, &width, &height, alphaColour), "Texture " << filename.str() << " did not load succesfully!");

    return Texture((unsigned int) track_texture.texNumber, data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}
*/
