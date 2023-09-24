#include "NFS4PS1Loader.h"
#include "Texture.h"

constexpr float carScaleFactor = 1000.f;

std::shared_ptr<Car> NFS4PS1::LoadCar(const std::string &carVivPath) {
    boost::filesystem::path p(carVivPath);
    std::string vivName = p.filename().string();
    std::string carName = vivName.substr(3, vivName.size() - 7);
    for (char &letter : carName) {
        if (letter <= 'Z' && letter >= 'A')
            letter -= ('Z' - 'z');
    }

    std::stringstream carOutPath, geoPath, pshPath;
    carOutPath << CAR_PATH << ToString(NFS_4_PS1) << "/" << carName << "/";
    geoPath << CAR_PATH << ToString(NFS_4_PS1) << "/" << carName << "/zz" << carName << ".geo";
    pshPath << CAR_PATH << ToString(NFS_4_PS1) << "/" << carName << "/zz" << carName << "d.psh";

    ASSERT(Utils::ExtractVIV(carVivPath, carOutPath.str()), "Unable to extract " << carVivPath << " to " << carOutPath.str());

    // For every file in here that's a BMP, load the data into a Texture object. This lets us easily access textures by an ID.
    std::map<unsigned int, Texture> carTextures;
    std::map<std::string, uint32_t> remappedTextureIds;
    uint32_t remappedTextureID = 0;
    carOutPath << "Textures/";

    ImageLoader::ExtractPSH(pshPath.str(), carOutPath.str());

    for (boost::filesystem::directory_iterator itr(carOutPath.str()); itr != boost::filesystem::directory_iterator(); ++itr) {
        if (itr->path().filename().string().find("BMP") != std::string::npos && itr->path().filename().string().find("-a") == std::string::npos) {
            // Map texture names, strings, into numbers so I can use them for indexes into the eventual Texture Array
            remappedTextureIds[itr->path().filename().replace_extension("").string()] = remappedTextureID++;
            GLubyte *data;
            GLsizei width;
            GLsizei height;
            ASSERT(ImageLoader::LoadBmpCustomAlpha(itr->path().string().c_str(), &data, &width, &height, 0u), "Texture " << itr->path().string() << " did not load succesfully!");
            carTextures[remappedTextureIds[itr->path().filename().replace_extension("").string()]] =
              Texture(remappedTextureIds[itr->path().filename().replace_extension("").string()], data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
        }
    }

    GLuint textureArrayId = TrackUtils::MakeTextureArray(carTextures, false);

    CarData carData;
    carData.meshes = LoadGEO(geoPath.str(), carTextures);
    return std::make_shared<Car>(carData, NFS_4_PS1, carName, textureArrayId);
}

std::shared_ptr<NFS3_4_DATA::PS1::TRACK> NFS4PS1::LoadTrack(const std::string &trackGrpPath) {
    std::cout << "--- Loading NFS4 PS1  ---" << std::endl;
    auto track = std::make_shared<TRACK>(TRACK());

    boost::filesystem::path p(trackGrpPath);
    track->name              = p.filename().replace_extension("").string();
    std::string trackOutPath = TRACK_PATH + ToString(NFS_4_PS1) + "/" + track->name;

    LoadGRP(trackGrpPath);
    /* frd_path << track_base_path << "/TR.frd";

     ASSERT(ExtractTrackTextures(track_base_path, track->name, NFSVer::NFS_4_PS1), "Could not extract " << track->name << " QFS texture
     pack"); ASSERT(LoadFRD(frd_path.str(), track->name, track), "Could not load FRD file: " << frd_path.str()); // Load FRD file to get
     track block specific data
 */
    std::cout << "Successful track load!" << std::endl;
    return track;
}

void NFS4PS1::LoadGRP(const std::string &grpPath) {
    // Build a buffer of the file in memory
    struct stat fstat {};
    stat(grpPath.c_str(), &fstat);
    char *grp = (char *) malloc(static_cast<size_t>(fstat.st_size + 1));

    FILE *grpFile = fopen(grpPath.c_str(), "rb");
    ASSERT((int) fread(grp, 1, fstat.st_size, grpFile) == fstat.st_size, "Couldn't allocate buffer for " << grpPath);

    int fileOffset = 0x24c;

    /*

        short *psVar1;
        void *pvVar2;
        this_00;
        SaveSurface *this_01;
        undefined2 *puVar4;
        CVECTOR *pCVar5;
        int iVar6;
        ushort *puVar7;
        int iVar8;
        int iVar9;
        CVECTOR CVar10;
        CVECTOR CVar11;
        CVECTOR CVar12;
        Chunk *this_02;
        SerializedGroup *pSVar14;
        int iVar15;
        char trackName[128];

        // Loading a light table, somewhere down the line
        Chunk_lightTable = (CVECTOR *) reservememadr(s_lighttbl_80056668, 0x404, 0);
        // Load the whole chunk into a SimpleMem
        int len = fstat.st_size;
        int someOffset = len + 0x9080;


        this = (SimpleMem *)__builtin_new(0xc);
        psVar1 = reservememadr(s_Track_mem_80056674,offset,0);
        *(short **)&this->heap = psVar1;
        *(short **)&this->freeMem = psVar1;
        if (psVar1 == (short *)0x0) {
            offset = 0;
        }
        this->freeMemSize = offset;
        DAT_00000f8c = this;
        pvVar2 = FeignAlloc__9SimpleMemi(this,trackFileLength);

        SimpleMem *DAT_00000f8c = SimpleMem{};
        pvVar2 = FeignAlloc__9SimpleMemi(this, len);
        //SerializedGroup *trackGroup = (SerializedGroup *) loadfileatadr(trackName, (int) pvVar2 + 0x9080);
        SerializedGroup *trackGroup = reinterpret_cast<SerializedGroup *>(grp);
        Group *DAT_00000f68 = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(trackGroup, 0x1f, DAT_00000f8c, 0);
        DAT_00000f68 = DAT_00000f68 + 1;
        Group *chunkSerialGroupOffset = LocateCreateGroupType__15SerializedGroupiP9SimpleMemi(trackGroup, 0x20, DAT_00000f8c, 0);
        coorddef *Chunk_chunkCenters = (coorddef *) (chunkSerialGroupOffset + 1);
        SerializedGroup *perGroup = LocateGroupType__15SerializedGroupii(trackGroup, 0x23, 0);
        pSVar14 = perGroup + 1;
        pCVar5 = Chunk_lightTable;
        if ((((uint8_t) pSVar14 | (uint) Chunk_lightTable) & 3) == 0) {
            do {
                CVar10 = *(CVECTOR * ) & pSVar14->m_length;
                CVar11 = *(CVECTOR * ) & pSVar14->dummy;
                CVar12 = *(CVECTOR * ) & pSVar14->m_num_elements;
                *pCVar5 = *(CVECTOR * ) & pSVar14->m_type;
                pCVar5[1] = CVar10;
                pCVar5[2] = CVar11;
                pCVar5[3] = CVar12;
                pSVar14 = pSVar14 + 1;
                pCVar5 = pCVar5 + 4;
            } while (pSVar14 != perGroup + 0x41);
        } else {
            do {
                CVar10 = *(CVECTOR * ) & pSVar14->m_length;
                CVar11 = *(CVECTOR * ) & pSVar14->dummy;
                CVar12 = *(CVECTOR * ) & pSVar14->m_num_elements;
                *pCVar5 = *(CVECTOR * ) & pSVar14->m_type;
                pCVar5[1] = CVar10;
                pCVar5[2] = CVar11;
                pCVar5[3] = CVar12;
                pSVar14 = pSVar14 + 1;
                pCVar5 = pCVar5 + 4;
            } while (pSVar14 != perGroup + 0x41);
        }
        perGroup = LocateGroupType__15SerializedGroupii(trackGroup, 0x23, 0);
        len = 0;
        someOffset = 0;
        iVar15 = 0;
        DAT_00000fa0 = perGroup->m_length - 0x10U >> 2;
        DAT_00000f60 = Alloc__9SimpleMemii(DAT_00000f8c, DAT_00000f68[7].m_num_elements * 0x48, 0);
        DAT_00000f64 = Alloc__9SimpleMemii(DAT_00000f8c, DAT_00000f68[7].m_num_elements, 0);
        DAT_00000f6c = Alloc__9SimpleMemii(DAT_00000f8c, DAT_00000f68[7].m_num_elements * 0x70, 0);
        Chunk_Init__Fv();
        perGroup = LocateGroupType__15SerializedGroupii(trackGroup, 0x21, 0);
        trackGroup = LocateGroupType__15SerializedGroupii(trackGroup, 0x1d, 0);
        while (len < DAT_00000f68[7].m_num_elements) {
            this_02 = (Chunk *) ((int) DAT_00000f6c + iVar15);
            pSVar14 = (SerializedGroup *) 0x0;
            if (len < DAT_00000f68[7].m_num_elements + -1) {
                pSVar14 = LocateNextGroupType__15SerializedGroupi(trackGroup, 0x1d);
            }
            InstanceGroup__5ChunkP15SerializedGroupP9SimpleMem(this_02, trackGroup, DAT_00000f8c);
            trackGroup = LocateGroupType__15SerializedGroupii(trackGroup, 4, 0);
            chunkSerialGroupOffset = DAT_00000f68;
            iVar9 = trackGroup->m_num_elements;
            trackGroup = trackGroup + 1;
            iVar8 = 0;
            if (0x24 < iVar9) {
                iVar9 = 0x24;
            }
            iVar6 = 0;
            if (0 < iVar9) {
                puVar7 = (ushort * )(someOffset + (int) DAT_00000f60);
                do {
                    if ((int) ((uint) * (ushort * ) & trackGroup->m_type & 0x3ff) < chunkSerialGroupOffset[7].m_num_elements) {
                        *puVar7 = *(ushort * ) & trackGroup->m_type;
                        puVar7 = puVar7 + 1;
                        iVar6 = iVar6 + 1;
                    }
                    iVar8 = iVar8 + 1;
                    trackGroup = (SerializedGroup *) ((int) &trackGroup->m_type + 2);
                } while (iVar8 < iVar9);
            }
            *(undefined * )((int) DAT_00000f64 + len) = (char) iVar6;
            if (iVar6 < 0x24) {
                puVar4 = (undefined2 * )((int) DAT_00000f60 + iVar6 * 2 + someOffset);
                do {
                    *puVar4 = 0x3ff;
                    iVar6 = iVar6 + 1;
                    puVar4 = puVar4 + 1;
                } while (iVar6 < 0x24);
            }
            someOffset = someOffset + 0x40;
            iVar15 = iVar15 + 0x70;
            len = len + 1;
            trackGroup = pSVar14;
        }*/

    free(grp);
    fclose(grpFile);
}

std::vector<CarModel> NFS4PS1::LoadGEO(const std::string &geoPath, std::map<unsigned int, Texture> carTextures) {
    // All Vertices are stored so that the model is rotated 90 degs on X, 180 on Z. Remove this at Vert load time.
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(glm::radians(90.f), 0, glm::radians(180.f))));
    LOG(INFO) << "Parsing NFS4 PS1 GEO File located at " << geoPath;
    std::vector<CarModel> carMeshes;

    // Build a buffer of the file in memory
    struct stat fstat {};
    stat(geoPath.c_str(), &fstat);
    char *mem = (char *) malloc(static_cast<size_t>(fstat.st_size + 1));

    FILE *geo = fopen(geoPath.c_str(), "rb");
    ASSERT((int) fread(mem, 1, fstat.st_size, geo) == fstat.st_size, "Couldn't allocate buffer for " << geoPath);

    int fileOffset = 0x24c;
    auto *scene    = reinterpret_cast<Transformer_zScene *>(mem);

    // These are special numbers from NFS4 PS1 runtime/The parent NFS4 PS1 Car_tObj object that affect parsing
    // Diablo == 0x0D
    int carType = 0; //(int)(carObj->render).palCopyNum[0xd];

    for (uint8_t partIdx = 0; partIdx < 57; ++partIdx) {
        // Prep the ONFS CarModel data
        float specularDamper       = 0.2f;
        float specularReflectivity = 0.02f;
        float envReflectivity      = 0.4f;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> polygonFlags;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> textureIndices;
        // Start reading the file
        auto *Nobj = reinterpret_cast<Transformer_zObj *>(mem + fileOffset);
        fileOffset += sizeof(Transformer_zObj);
        scene->obj[partIdx] = Nobj;
        // Apply some part specific translation offsets
        if (partIdx == 39) {
            Nobj->translation.x -= 0x7ae;
        } else if (partIdx == 40) {
            Nobj->translation.x += 0x7ae;
        }

        glm::vec3 center = rotationMatrix * glm::vec3{Nobj->translation.x >> 8, Nobj->translation.y >> 8, Nobj->translation.z >> 8};
        center /= carScaleFactor;

        if (Nobj->numVertex != 0) {
            Nobj->vertex = reinterpret_cast<COORD16 *>(mem + fileOffset);
            fileOffset += (uint32_t) Nobj->numVertex * sizeof(COORD16);
            // Alignment
            if (Nobj->numVertex % 2) {
                fileOffset += 2;
            }
            // Lets get those verts, regardless of whether normals are packed in the file
            for (uint16_t vertIdx = 0; vertIdx < Nobj->numVertex; ++vertIdx) {
                glm::vec3 vertex = glm::vec3(Nobj->vertex[vertIdx].x, Nobj->vertex[vertIdx].y, Nobj->vertex[vertIdx].z);
                vertex /= carScaleFactor;
                vertices.emplace_back(vertex);
            }
            if ((R3DCar_ObjectInfo[partIdx][1] & 1U) != 0 && carType < 0x1c) {
                Nobj->Nvertex = reinterpret_cast<COORD16 *>(mem + fileOffset);
                fileOffset += (uint32_t) Nobj->numVertex * sizeof(COORD16);
                // Alignment again
                if (Nobj->numVertex % 2) {
                    fileOffset += 2;
                }
                // Get normals
                // I think we're only good to write out these as normals if this is true?
                // Refer to disasm. Not important for now. Will just manually calculate them!
                // if ((objectInfo[partIdx][1] & 0x40U) != 0) {
                /*for (uint16_t vertIdx = 0; vertIdx < Nobj->numVertex; ++vertIdx) {*/
                /*    normals.emplace_back(glm::vec3((int) *(&Nobj->Nvertex->x + (vertIdx * sizeof(PS1::COORD16))),*/
                /*                                   (int) *(&Nobj->Nvertex->y + (vertIdx * sizeof(PS1::COORD16))),*/
                /*                                   (int) *(&Nobj->Nvertex->z + (vertIdx * sizeof(PS1::COORD16)))));*/
                /*}*/
            }
        }
        if (Nobj->numFacet != 0) {
            Nobj->facet = reinterpret_cast<Transformer_zFacet *>(mem + fileOffset);
            fileOffset += (uint32_t) Nobj->numFacet * sizeof(Transformer_zFacet);
            for (uint32_t facetIdx = 0; facetIdx < Nobj->numFacet; ++facetIdx) {
                Texture glTexture = carTextures[(Nobj->facet + facetIdx)->textureIndex];

                polygonFlags.emplace_back(Nobj->facet[facetIdx].flag);
                polygonFlags.emplace_back(Nobj->facet[facetIdx].flag);
                polygonFlags.emplace_back(Nobj->facet[facetIdx].flag);
                textureIndices.emplace_back(Nobj->facet[facetIdx].textureIndex);
                textureIndices.emplace_back(Nobj->facet[facetIdx].textureIndex);
                textureIndices.emplace_back(Nobj->facet[facetIdx].textureIndex);
                indices.emplace_back(Nobj->facet[facetIdx].vertexId0);
                indices.emplace_back(Nobj->facet[facetIdx].vertexId1);
                indices.emplace_back(Nobj->facet[facetIdx].vertexId2);
                uvs.emplace_back(glm::vec2(Nobj->facet[facetIdx].uv0.u * glTexture.max_u, Nobj->facet[facetIdx].uv0.v) * glTexture.max_v);
                uvs.emplace_back(glm::vec2(Nobj->facet[facetIdx].uv1.u * glTexture.max_u, Nobj->facet[facetIdx].uv1.v) * glTexture.max_v);
                uvs.emplace_back(glm::vec2(Nobj->facet[facetIdx].uv2.u * glTexture.max_u, Nobj->facet[facetIdx].uv2.v) * glTexture.max_v);
                // If no normals were packed in the file, lets generate some
                // TODO: For now generate them always
                // if ((R3DCar_ObjectInfo[partIdx][1] & 1U) == 0) {
                glm::vec3 normal = rotationMatrix * Utils::CalculateNormal(vertices[Nobj->facet[facetIdx].vertexId0], vertices[Nobj->facet[facetIdx].vertexId1],
                                                                           vertices[Nobj->facet[facetIdx].vertexId2]);
                normals.emplace_back(normal);
                normals.emplace_back(normal);
                normals.emplace_back(normal);
                //}
            }
        }
        // TODO: No polygonFlags for now
        if (Nobj->numVertex && Nobj->numFacet) {
            carMeshes.emplace_back(
              CarModel(std::string(geoPartNames[partIdx]), vertices, uvs, textureIndices, normals, indices, center, specularDamper, specularReflectivity, envReflectivity));
        }
    }
    fclose(geo);
    free(mem);

    return carMeshes;
}
