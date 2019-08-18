#include "NFS4PS1Loader.h"

constexpr float carScaleFactor = 1000.f;

std::shared_ptr<Car> NFS4PS1::LoadCar(const std::string &carVivPath) {
    boost::filesystem::path p(carVivPath);
    std::string vivName = p.filename().string();
    std::string carName = vivName.substr(3, vivName.size() - 7);
    for (char &letter : carName) {
        if (letter <= 'Z' && letter >= 'A') letter -= ('Z' - 'z');
    }

    std::stringstream carOutPath, geoPath, pshPath;
    carOutPath << CAR_PATH << ToString(NFS_4_PS1) << "/" << carName << "/";
    geoPath << CAR_PATH << ToString(NFS_4_PS1) << "/" << carName << "/zz" << carName << ".geo";
    pshPath << CAR_PATH << ToString(NFS_4_PS1) << "/" << carName << "/zz" << carName << "d.psh";

    ASSERT(Utils::ExtractVIV(carVivPath, carOutPath.str()),
           "Unable to extract " << carVivPath << " to " << carOutPath.str());

    // For every file in here that's a BMP, load the data into a Texture object. This lets us easily access textures by an ID.
    std::map<unsigned int, Texture> carTextures;
    std::map<std::string, uint32_t> remappedTextureIds;
    uint32_t remappedTextureID = 0;
    carOutPath << "Textures/";

    ImageLoader::ExtractPSH(pshPath.str(), carOutPath.str());

    for (boost::filesystem::directory_iterator itr(carOutPath.str());
         itr != boost::filesystem::directory_iterator(); ++itr) {
        if (itr->path().filename().string().find("BMP") != std::string::npos &&
            itr->path().filename().string().find("-a") == std::string::npos) {
            // Map texture names, strings, into numbers so I can use them for indexes into the eventual Texture Array
            remappedTextureIds[itr->path().filename().replace_extension("").string()] = remappedTextureID++;
            GLubyte *data;
            GLsizei width;
            GLsizei height;
            ASSERT(ImageLoader::LoadBmpCustomAlpha(itr->path().string().c_str(), &data, &width, &height, 0u),
                   "Texture " << itr->path().string() << " did not load succesfully!");
            carTextures[remappedTextureIds[itr->path().filename().replace_extension("").string()]] = Texture(
                    remappedTextureIds[itr->path().filename().replace_extension("").string()], data,
                    static_cast<unsigned int>(width), static_cast<unsigned int>(height));
        }
    }

    GLuint textureArrayId = TrackUtils::MakeTextureArray(carTextures, false);

    CarData carData;
    carData.meshes = LoadGEO(geoPath.str(), carTextures);
    return std::make_shared<Car>(carData, NFS_4_PS1, carName, textureArrayId);
}

std::vector<CarModel> NFS4PS1::LoadGEO(const std::string &geoPath, std::map<unsigned int, Texture> carTextures) {
    // All Vertices are stored so that the model is rotated 90 degs on X, 180 on Z. Remove this at Vert load time.
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(glm::radians(90.f), 0, glm::radians(180.f))));
    LOG(INFO) << "Parsing NFS4 PS1 GEO File located at " << geoPath;
    std::vector<CarModel> carMeshes;

    // Build a buffer of the file in memory
    struct stat fstat{};
    stat(geoPath.c_str(), &fstat);
    char *mem = (char *) malloc(static_cast<size_t>(fstat.st_size + 1));

    FILE *geo = fopen(geoPath.c_str(), "rb");
    ASSERT((int) fread(mem, 1, fstat.st_size, geo) == fstat.st_size, "Couldn't allocate buffer for " << geoPath);

    int fileOffset = 0x24c;
    auto *scene = reinterpret_cast<PS1::Transformer_zScene *>(mem);

    // These are special numbers from NFS4 PS1 runtime/The parent NFS4 PS1 Car_tObj object that affect parsing
    // Diablo == 0x0D
    int carType = 0; //(int)(carObj->render).palCopyNum[0xd];

    for (uint8_t partIdx = 0; partIdx < 57; ++partIdx) {
        // Prep the ONFS CarModel data
        float specularDamper = 0.2f;
        float specularReflectivity = 0.02f;
        float envReflectivity = 0.4f;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> polygonFlags;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> textureIndices;
        // Start reading the file
        auto *Nobj = reinterpret_cast<PS1::Transformer_zObj *>(mem + fileOffset);
        fileOffset += sizeof(PS1::Transformer_zObj);
        scene->obj[partIdx] = Nobj;
        // Apply some part specific translation offsets
        if (partIdx == 39) {
            Nobj->translation.x -= 0x7ae;
        } else if (partIdx == 40) {
            Nobj->translation.x += 0x7ae;
        }

        glm::vec3 center = rotationMatrix * glm::vec3{
                Nobj->translation.x >> 8,
                Nobj->translation.y >> 8,
                Nobj->translation.z >> 8
        };
        center /= carScaleFactor;

        if (Nobj->numVertex != 0) {
            Nobj->vertex = reinterpret_cast<PS1::COORD16 *>(mem + fileOffset);
            fileOffset += (uint32_t) Nobj->numVertex * sizeof(PS1::COORD16);
            // Alignment
            if (Nobj->numVertex % 2) {
                fileOffset += 2;
            }
            // Lets get those verts, regardless of whether normals are packed in the file
            for (uint16_t vertIdx = 0; vertIdx < Nobj->numVertex; ++vertIdx) {
                glm::vec3 vertex = glm::vec3(Nobj->vertex[vertIdx].x,
                                             Nobj->vertex[vertIdx].y,
                                             Nobj->vertex[vertIdx].z);
                vertex /= carScaleFactor;
                vertices.emplace_back(vertex);
            }
            if ((R3DCar_ObjectInfo[partIdx][1] & 1U) != 0 && carType < 0x1c) {
                Nobj->Nvertex = reinterpret_cast<PS1::COORD16 *>(mem + fileOffset);
                fileOffset += (uint32_t) Nobj->numVertex * sizeof(PS1::COORD16);
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
            Nobj->facet = reinterpret_cast<PS1::Transformer_zFacet *>(mem + fileOffset);
            fileOffset += (uint32_t) Nobj->numFacet * sizeof(PS1::Transformer_zFacet);
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
                uvs.emplace_back(
                        glm::vec2(Nobj->facet[facetIdx].uv0.u * glTexture.max_u, Nobj->facet[facetIdx].uv0.v) *
                        glTexture.max_v);
                uvs.emplace_back(
                        glm::vec2(Nobj->facet[facetIdx].uv1.u * glTexture.max_u, Nobj->facet[facetIdx].uv1.v) *
                        glTexture.max_v);
                uvs.emplace_back(
                        glm::vec2(Nobj->facet[facetIdx].uv2.u * glTexture.max_u, Nobj->facet[facetIdx].uv2.v) *
                        glTexture.max_v);
                // If no normals were packed in the file, lets generate some
                // TODO: For now generate them always
                //if ((R3DCar_ObjectInfo[partIdx][1] & 1U) == 0) {
                    glm::vec3 normal =
                            rotationMatrix * Utils::CalculateNormal(vertices[Nobj->facet[facetIdx].vertexId0],
                                                                    vertices[Nobj->facet[facetIdx].vertexId1],
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
                    CarModel(std::string(geoPartNames[partIdx]), vertices, uvs, textureIndices, normals, indices,
                             center, specularDamper,
                             specularReflectivity, envReflectivity));
        }
    }
    fclose(geo);
    free(mem);

    return carMeshes;
}
