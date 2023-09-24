#include "GeoFile.h"

//#include "../../../../src/Scene/Models/CarModel.h"

namespace LibOpenNFS {
    namespace NFS2 {
        template <typename Platform>
        bool GeoFile<Platform>::Load(const std::string &geoPath, GeoFile &geoFile) {
            //LOG(INFO) << "Loading GEO File located at " << geoPath;
            std::ifstream geo(geoPath, std::ios::in | std::ios::binary);

            bool loadStatus = geoFile._SerializeIn(geo);
            geo.close();

            return loadStatus;
        }

        template <typename Platform>
        void GeoFile<Platform>::Save(const std::string &geoPath, GeoFile &geoFile) {
            //LOG(INFO) << "Saving FCE File to " << geoPath;
            std::ofstream geo(geoPath, std::ios::out | std::ios::binary);
            geoFile._SerializeOut(geo);
        }

        template <>
        bool GeoFile<PC>::_SerializeIn(std::ifstream &ifstream) {
            // std::vector<CarModel> NFS2<PC>::LoadGEO(const std::string &geo_path, std::map<unsigned int, Texture> car_textures, std::map<std::string, uint32_t>
            // remapped_texture_ids)
            float carScaleFactor = 2000.f;
            glm::quat rotationMatrix =
              glm::normalize(glm::quat(glm::vec3(0, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

            //std::vector<CarModel> car_meshes;

            /*SAFE_READ(ifstream, &header, sizeof(PC::HEADER));

            uint32_t part_Idx = -1;

            while (true)
            {
                std::streamoff start = ifstream.tellg();

                auto *geoBlockHeader = new PC::BLOCK_HEADER();
                while (geoBlockHeader->nVerts == 0)
                {
                    ++part_Idx;
                    geo.read((char *) geoBlockHeader, sizeof(PC::GEO::BLOCK_HEADER));
                    if (geo.eof())
                    {
                        delete geoBlockHeader;
                        delete geoFileHeader;
                        return car_meshes;
                    }
                }
                ASSERT(geoBlockHeader->pad0 == 0 && geoBlockHeader->pad1 == 1 && geoBlockHeader->pad2 == 1, "Corrupt GEO block header");

                std::vector<uint32_t> indices;
                std::vector<glm::vec3> verts;
                std::vector<glm::vec3> norms;
                std::vector<glm::vec2> uvs;
                std::vector<uint32_t> texture_indices;

                indices.resize(geoBlockHeader->nPolygons * 6);
                verts.resize(geoBlockHeader->nVerts);

                float specularDamper       = 0.2f;
                float specularReflectivity = 0.02f;
                float envReflectivity      = 0.4f;

                auto *vertices = new PC::GEO::BLOCK_3D[geoBlockHeader->nVerts];
                geo.read((char *) vertices, geoBlockHeader->nVerts * sizeof(PC::GEO::BLOCK_3D));

                std::streamoff end = geo.tellg();
                // Polygon Table start is aligned on 4 Byte boundary
                if (((end - start) % 4))
                {
                    LOG(DEBUG) << "Part " << part_Idx << " [" << PC_PART_NAMES[part_Idx] << "] Polygon Table Pre-Pad Contents: ";
                    uint16_t *pad = new uint16_t[3];
                    geo.read((char *) pad, sizeof(uint16_t) * 3);
                    for (uint32_t i = 0; i < 3; ++i)
                    {
                        LOG(DEBUG) << pad[i];
                    }
                    delete[] pad;
                }

                auto *polygons = new PC::GEO::POLY_3D[geoBlockHeader->nPolygons];
                geo.read((char *) polygons, geoBlockHeader->nPolygons * sizeof(PC::GEO::POLY_3D));

                for (uint32_t vert_Idx = 0; vert_Idx < geoBlockHeader->nVerts; ++vert_Idx)
                {
                    verts.emplace_back(rotationMatrix * glm::vec3(vertices[vert_Idx].x / carScaleFactor, vertices[vert_Idx].y / carScaleFactor, vertices[vert_Idx].z /
            carScaleFactor));
                }

                for (uint32_t poly_Idx = 0; poly_Idx < geoBlockHeader->nPolygons; ++poly_Idx)
                {
                    std::string textureName(polygons[poly_Idx].texName, polygons[poly_Idx].texName + 4);
                    Texture gl_texture = car_textures[remapped_texture_ids[textureName]];

                    indices.emplace_back(polygons[poly_Idx].vertex[0]);
                    indices.emplace_back(polygons[poly_Idx].vertex[1]);
                    indices.emplace_back(polygons[poly_Idx].vertex[2]);
                    indices.emplace_back(polygons[poly_Idx].vertex[0]);
                    indices.emplace_back(polygons[poly_Idx].vertex[2]);
                    indices.emplace_back(polygons[poly_Idx].vertex[3]);

                    uvs.emplace_back(0.0f * gl_texture.maxU, 0.0f * gl_texture.maxV);
                    uvs.emplace_back(1.0f * gl_texture.maxU, 0.0f * gl_texture.maxV);
                    uvs.emplace_back(1.0f * gl_texture.maxU, 1.0f * gl_texture.maxV);
                    uvs.emplace_back(0.0f * gl_texture.maxU, 0.0f * gl_texture.maxV);
                    uvs.emplace_back(1.0f * gl_texture.maxU, 1.0f * gl_texture.maxV);
                    uvs.emplace_back(0.0f * gl_texture.maxU, 1.0f * gl_texture.maxV);

                    glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(vertices[polygons[poly_Idx].vertex[0]]),
                                                                            PointToVec(vertices[polygons[poly_Idx].vertex[1]]),
                                                                            PointToVec(vertices[polygons[poly_Idx].vertex[2]]),
                                                                            PointToVec(vertices[polygons[poly_Idx].vertex[3]]));

                    // Use the R/L flag to flip normals
                    if (polygons[poly_Idx].texMapType & 0x4)
                    {
                        normal = -normal;
                    }

                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);

                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                }
                glm::vec3 center = glm::vec3(
                  (geoBlockHeader->position[0] / 256.f) / carScaleFactor, (geoBlockHeader->position[1] / 256.f) / carScaleFactor, (geoBlockHeader->position[2] / 256.f) /
            carScaleFactor); car_meshes.emplace_back(CarModel(PC_PART_NAMES[part_Idx], verts, uvs, texture_indices, norms, indices, center, specularDamper, specularReflectivity,
            envReflectivity));

                delete geoBlockHeader;
                delete[] vertices;
                delete[] polygons;
            }*/
            return false;
        }

        template <>
        bool GeoFile<PS1>::_SerializeIn(std::ifstream &ifstream) {
            // std::vector<CarModel> NFS2<PS1>::LoadGEO(const std::string &geo_path, std::map<unsigned int, Texture> car_textures, std::map<std::string, uint32_t>
            // remapped_texture_ids)
            /*glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(0, 0, 0)));
            float carScaleFactor     = 2000.f;

            std::vector<CarModel> car_meshes;

            auto *geoFileHeader = new PS1::HEADER();
            if (geo.read((char *) geoFileHeader, sizeof(PS1::GEO::HEADER)).gcount() != sizeof(PS1::GEO::HEADER))
            {
                LOG(WARNING) << "Couldn't open file/truncated";
                delete geoFileHeader;
                return car_meshes;
            }

            int32_t part_Idx = -1;

            while (true)
            {
                std::streamoff start = geo.tellg();
                LOG(DEBUG) << "Part " << part_Idx + 1 << " [" << PS1_PART_NAMES[part_Idx + 1] << "]";
                LOG(DEBUG) << "BlockStartOffset:   " << start;
                auto *geoBlockHeader = new PS1::GEO::BLOCK_HEADER();
                while (geoBlockHeader->nVerts == 0)
                {
                    ++part_Idx;
                    geo.read((char *) geoBlockHeader, sizeof(PS1::GEO::BLOCK_HEADER));
                    if (geo.eof())
                    {
                        delete geoBlockHeader;
                        delete geoFileHeader;
                        return car_meshes;
                    }
                }

                if ((geoBlockHeader->unknown[0] != 0) || (geoBlockHeader->unknown[1] != 1) || (geoBlockHeader->unknown[2] != 1))
                {
                    LOG(WARNING) << "Invalid geometry header. This file is special (or corrupt)";
                    delete geoBlockHeader;
                    delete geoFileHeader;
                    return car_meshes;
                }

                std::vector<uint32_t> indices;
                std::vector<uint32_t> texMapStuff;
                std::vector<glm::vec3> verts;
                std::vector<glm::vec3> norms;
                std::vector<glm::vec2> uvs;
                std::vector<uint32_t> texture_indices;

                indices.resize(geoBlockHeader->nPolygons * 6);
                verts.resize(geoBlockHeader->nVerts);

                float specularDamper       = 0.2f;
                float specularReflectivity = 0.02f;
                float envReflectivity      = 0.4f;

                auto *vertices = new PS1::GEO::BLOCK_3D[geoBlockHeader->nVerts];
                geo.read((char *) vertices, (geoBlockHeader->nVerts) * sizeof(PS1::GEO::BLOCK_3D));
                LOG(DEBUG) << "VertTblEndOffset:   " << geo.tellg() << " Size: " << geo.tellg() - start << std::endl;

                // If nVerts is ODD, we need to pad. Let's dump the contents of the pad though, in case there's data here
                if (geoBlockHeader->nVerts % 2)
                {
                    auto *pad = new uint16_t[3];
                    geo.read((char *) pad, sizeof(uint16_t) * 3);
                    if (pad[0] || pad[1] || pad[2])
                    {
                        LOG(DEBUG) << "Normal Table Pre-Pad Contents: " << std::endl;
                        for (uint32_t i = 0; i < 3; ++i)
                        {
                            LOG(DEBUG) << pad[i] << std::endl;
                        }
                    }
                    delete[] pad;
                }

                auto *normals = new PS1::GEO::BLOCK_3D[geoBlockHeader->nNormals];
                geo.read((char *) normals, (geoBlockHeader->nNormals) * sizeof(PS1::GEO::BLOCK_3D));
                LOG(DEBUG) << "NormTblEndOffset:   " << geo.tellg() << " Size: " << geo.tellg() - start;

                auto *xblock_1 = new PS1::GEO::XBLOCK_1();
                auto *xblock_2 = new PS1::GEO::XBLOCK_2();
                auto *xblock_3 = new PS1::GEO::XBLOCK_3();
                auto *xblock_4 = new PS1::GEO::XBLOCK_4();
                auto *xblock_5 = new PS1::GEO::XBLOCK_5();
                // Is this really a block type?
                switch (geoBlockHeader->unknown1)
                {
                case 1:
                    geo.read((char *) xblock_1, sizeof(PS1::GEO::XBLOCK_1));
                    break;
                case 2:
                    geo.read((char *) xblock_2, sizeof(PS1::GEO::XBLOCK_2));
                    break;
                case 3:
                    geo.read((char *) xblock_3, sizeof(PS1::GEO::XBLOCK_3));
                    break;
                case 4:
                    geo.read((char *) xblock_4, sizeof(PS1::GEO::XBLOCK_4));
                    break;
                case 5:
                    geo.read((char *) xblock_5, sizeof(PS1::GEO::XBLOCK_5));
                    break;
                default:
                    LOG(DEBUG) << "Unknown block type:  " << geoBlockHeader->unknown1;
                }

                std::streamoff end = geo.tellg();
                LOG(DEBUG) << "PolyTblStartOffset: " << end << " Size: " << end - start;
                // Polygon Table start is aligned on 4 Byte boundary
                if (((end - start) % 4))
                {
                    uint16_t *pad = new uint16_t[3];
                    geo.read((char *) pad, sizeof(uint16_t) * 3);
                    if (pad[0] || pad[1] || pad[2])
                    {
                        LOG(DEBUG) << "Polygon Table Pre-Pad Contents: ";
                        for (uint32_t i = 0; i < 3; ++i)
                        {
                            LOG(DEBUG) << pad[i];
                        }
                    }
                    delete[] pad;
                }

                auto *polygons = new PS1::GEO::POLY_3D[geoBlockHeader->nPolygons];
                geo.read((char *) polygons, geoBlockHeader->nPolygons * sizeof(PS1::GEO::POLY_3D));

                LOG(DEBUG) << "BlockEndOffset:     " << geo.tellg() << " Size: " << geo.tellg() - start;

                for (uint32_t vert_Idx = 0; vert_Idx < geoBlockHeader->nVerts; ++vert_Idx)
                {
                    verts.emplace_back(rotationMatrix * glm::vec3(vertices[vert_Idx].x / carScaleFactor, vertices[vert_Idx].y / carScaleFactor, vertices[vert_Idx].z /
            carScaleFactor));
                }

                for (uint32_t poly_Idx = 0; poly_Idx < geoBlockHeader->nPolygons; ++poly_Idx)
                {
                    std::string textureName(polygons[poly_Idx].texName, polygons[poly_Idx].texName + 4);
                    Texture gl_texture = car_textures[remapped_texture_ids[textureName]];

                    texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
                    texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
                    texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
                    texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
                    texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
                    texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);

                    // TODO: There's another set of indices at index [2], that form barely valid polygons. Middle set [1] are always numbers that
                    // match, 0000, 1111, 2222, 3333.
                    indices.emplace_back(polygons[poly_Idx].vertex[0][0]);
                    indices.emplace_back(polygons[poly_Idx].vertex[0][1]);
                    indices.emplace_back(polygons[poly_Idx].vertex[0][2]);
                    indices.emplace_back(polygons[poly_Idx].vertex[0][0]);
                    indices.emplace_back(polygons[poly_Idx].vertex[0][2]);
                    indices.emplace_back(polygons[poly_Idx].vertex[0][3]);

                    // TODO: Use Polygon TexMap type to fix texture mapping, use UV factory in trk utils
                    std::vector<glm::vec2> transformedUVs = GenerateUVs(NFS_3_PS1, CAR, polygons->texMap[0], gl_texture);
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(vertices[polygons[poly_Idx].vertex[0][0]]),
                                                                            PointToVec(vertices[polygons[poly_Idx].vertex[0][1]]),
                                                                            PointToVec(vertices[polygons[poly_Idx].vertex[0][2]]),
                                                                            PointToVec(vertices[polygons[poly_Idx].vertex[0][3]]));

                    // Use the R/L flag to flip normals
                    if (!(polygons[poly_Idx].texMap[0] & 0x4))
                    {
                        normal = -normal;
                    }

                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);
                    norms.emplace_back(normal);

                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                    texture_indices.emplace_back(remapped_texture_ids[textureName]);
                }
                glm::vec3 center = glm::vec3((geoBlockHeader->position[0] / 256.0f) / carScaleFactor,
                                             (geoBlockHeader->position[1] / 256.0f) / carScaleFactor,
                                             (geoBlockHeader->position[2] / 256.0f) / carScaleFactor);
                car_meshes.emplace_back(
                  CarModel(PS1_PART_NAMES[part_Idx], verts, uvs, texture_indices, texMapStuff, norms, indices, center, specularDamper, specularReflectivity, envReflectivity));

                // Dump GeoBlock data for correlating with geometry/LOD's/Special Cases
                LOG(DEBUG) << "nVerts:    " << geoBlockHeader->nVerts << std::endl;
                LOG(DEBUG) << "unknown1:  " << geoBlockHeader->unknown1 << std::endl;
                LOG(DEBUG) << "nNormals:  " << geoBlockHeader->nNormals << std::endl;
                LOG(DEBUG) << "nPolygons: " << geoBlockHeader->nPolygons << std::endl;

                switch (geoBlockHeader->unknown1)
                {
                case 1:
                    LOG(DEBUG) << "XBlock 1: ";
                    for (uint32_t i = 0; i < sizeof(xblock_1->unknown) / sizeof(xblock_1->unknown[0]); ++i)
                    {
                        LOG(DEBUG) << (int) xblock_1->unknown[i];
                    }
                    break;
                case 2:
                    LOG(DEBUG) << "XBlock 2: ";
                    for (uint32_t i = 0; i < sizeof(xblock_2->unknown) / sizeof(xblock_2->unknown[0]); ++i)
                    {
                        LOG(DEBUG) << (int) xblock_2->unknown[i];
                    }
                    break;
                case 3:
                    LOG(DEBUG) << "XBlock 3: ";
                    for (uint32_t i = 0; i < sizeof(xblock_3->unknown) / sizeof(xblock_3->unknown[0]); ++i)
                    {
                        LOG(DEBUG) << (int) xblock_3->unknown[i];
                    }
                    break;
                case 4:
                    LOG(DEBUG) << "XBlock 4: ";
                    for (uint32_t i = 0; i < sizeof(xblock_4->unknown) / sizeof(xblock_4->unknown[0]); ++i)
                    {
                        LOG(DEBUG) << (int) xblock_4->unknown[i];
                    }
                    break;
                case 5:
                    LOG(DEBUG) << "XBlock 5: ";
                    for (uint32_t i = 0; i < sizeof(xblock_5->unknown) / sizeof(xblock_5->unknown[0]); ++i)
                    {
                        LOG(DEBUG) << (int) xblock_5->unknown[i];
                    }
                    break;
                }
                LOG(DEBUG) << "--------------------------" << std::endl;

                delete geoBlockHeader;
                delete[] normals;
                delete[] vertices;
                delete[] polygons;
                delete xblock_1;
                delete xblock_2;
                delete xblock_3;
                delete xblock_4;
                delete xblock_5;
            }*/
            return false;
        }

        template <typename Platform>
        void GeoFile<Platform>::_SerializeOut(std::ofstream &ofstream) {
            ASSERT(false, "GEO output serialization is not currently implemented");
        }

        template class GeoFile<PS1>;
        template class GeoFile<PC>;
    } // namespace NFS2
} // namespace LibOpenNFS