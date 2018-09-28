//
// Created by Amrik.Sadhra on 20/06/2018.
//

#include "nfs2_loader.h"

using namespace TrackUtils;

void DumpToObj(int block_Idx, PS1::GEO::BLOCK_HEADER *geoBlockHeader, PS1::GEO::BLOCK_3D *vertices, PS1::GEO::BLOCK_3D *normals, PS1::GEO::POLY_3D *polygons) {
    std::ofstream obj_dump;
    std::stringstream obj_name;
    obj_name << "./assets/psx_test/ps1_geo_" << block_Idx << ".obj";
    obj_dump.open(obj_name.str());

    /* Print Part name*/
    obj_dump << "o " << "PS1_Test" << std::endl;
    for (uint32_t i = 0; i < geoBlockHeader->nVerts; ++i) {
        obj_dump << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
    }

    // TODO: How can these be normals if there aren't enough for Per vertex? On PSX they're likely per polygon
    for (uint32_t i = 0; i < geoBlockHeader->nNormals; ++i) {
        obj_dump << "vn " << normals[i].x << " " << normals[i].y << " " << normals[i].z << std::endl;
    }

    obj_dump.close();
}

template<>
std::vector<CarModel> NFS2<PC>::LoadGEO(const std::string &geo_path, std::map<unsigned int, Texture> car_textures, std::map<std::string, uint32_t> remapped_texture_ids) {
    // Mike Thompson CarEd disasm parts table for NFS2 Cars
    std::string PC_PART_NAMES[32]{
            "High Additional Body Part",
            "High Main Body Part",
            "High Ground Part",
            "High Front Part",
            "High Back Part",
            "High Left Side Part",
            "High Right Side Part",
            "High Additional Left Side Part",
            "High Additional Right Side Part",
            "High Spoiler Part",
            "High Additional Part",
            "High Backlights",
            "High Front Right Wheel",
            "High Front Right Wheel Part",
            "High Front Left Wheel",
            "High Front Left Wheel Part",
            "High Rear Right Wheel",
            "High Rear Right Wheel Part",
            "High Rear Left Wheel",
            "High Rear Left Wheel Part",
            "Medium Additional Body Part",
            "Medium Main Body Part",
            "Medium Ground Part",
            "Low Wheel Part",
            "Low Main Part",
            "Low Side Part",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
    };

    float carScaleFactor = 2000.f;
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(0, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    std::cout << "- Parsing GEO File " << std::endl;
    std::vector<CarModel> car_meshes;
    ifstream geo(geo_path, ios::in | ios::binary);

    auto *geoFileHeader = new PC::GEO::HEADER();
    if (geo.read((char *) geoFileHeader, sizeof(PC::GEO::HEADER)).gcount() != sizeof(PC::GEO::HEADER)) {
        std::cout << "Couldn't open file/truncated." << std::endl;
        delete geoFileHeader;
        return car_meshes;
    }

    uint32_t part_Idx = -1;

    while (true) {
        streamoff start = geo.tellg();

        auto *geoBlockHeader = new PC::GEO::BLOCK_HEADER();
        while (geoBlockHeader->nVerts == 0) {
            ++part_Idx;
            geo.read((char *) geoBlockHeader, sizeof(PC::GEO::BLOCK_HEADER));
            if (geo.eof()) {
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
        std::vector<unsigned int> texture_indices;

        indices.reserve(geoBlockHeader->nPolygons * 6);
        verts.reserve(geoBlockHeader->nVerts);

        float specularDamper = 0.2f;
        float specularReflectivity = 0.02f;
        float envReflectivity = 0.4f;

        auto *vertices = new PC::GEO::BLOCK_3D[geoBlockHeader->nVerts];
        geo.read((char *) vertices, geoBlockHeader->nVerts * sizeof(PC::GEO::BLOCK_3D));

        streamoff end = geo.tellg();
        // Polygon Table start is aligned on 4 Byte boundary
        if (((end - start) % 4)) {
            std::cout << "Part " << part_Idx << " [" << PC_PART_NAMES[part_Idx] << "] Polygon Table Pre-Pad Contents: " << std::endl;
            uint16_t *pad = new uint16_t[3];
            geo.read((char *) pad, sizeof(uint16_t) * 3);
            for (uint32_t i = 0; i < 3; ++i) {
                std::cout << pad[i] << std::endl;
            }
            delete[] pad;
        }

        auto *polygons = new PC::GEO::POLY_3D[geoBlockHeader->nPolygons];
        geo.read((char *) polygons, geoBlockHeader->nPolygons * sizeof(PC::GEO::POLY_3D));

        for (uint32_t vert_Idx = 0; vert_Idx < geoBlockHeader->nVerts; ++vert_Idx) {
            verts.emplace_back(rotationMatrix * glm::vec3(vertices[vert_Idx].x / carScaleFactor, vertices[vert_Idx].y / carScaleFactor, vertices[vert_Idx].z / carScaleFactor));
        }

        for (uint32_t poly_Idx = 0; poly_Idx < geoBlockHeader->nPolygons; ++poly_Idx) {
            std::string textureName(polygons[poly_Idx].texName, polygons[poly_Idx].texName + 4);
            Texture gl_texture = car_textures[remapped_texture_ids[textureName]];

            indices.emplace_back(polygons[poly_Idx].vertex[0]);
            indices.emplace_back(polygons[poly_Idx].vertex[1]);
            indices.emplace_back(polygons[poly_Idx].vertex[2]);
            indices.emplace_back(polygons[poly_Idx].vertex[0]);
            indices.emplace_back(polygons[poly_Idx].vertex[2]);
            indices.emplace_back(polygons[poly_Idx].vertex[3]);

            uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
            uvs.emplace_back(1.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
            uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
            uvs.emplace_back(0.0f * gl_texture.max_u, 0.0f * gl_texture.max_v);
            uvs.emplace_back(1.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);
            uvs.emplace_back(0.0f * gl_texture.max_u, 1.0f * gl_texture.max_v);

            glm::vec3 normal = rotationMatrix * calculateQuadNormal(pointToVec(vertices[polygons[poly_Idx].vertex[0]]), pointToVec(vertices[polygons[poly_Idx].vertex[1]]), pointToVec(vertices[polygons[poly_Idx].vertex[2]]), pointToVec(vertices[polygons[poly_Idx].vertex[3]]));

            // Use the R/L flag to flip normals
            if(polygons[poly_Idx].texMapType & 0x4){
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
        glm::vec3 center = glm::vec3((geoBlockHeader->position[0] / 256.f) / carScaleFactor, (geoBlockHeader->position[1] / 256.f) / carScaleFactor, (geoBlockHeader->position[2] / 256.f) / carScaleFactor);
        car_meshes.emplace_back(CarModel(PC_PART_NAMES[part_Idx], verts, uvs, texture_indices, norms, indices, center, specularDamper, specularReflectivity, envReflectivity));

        delete geoBlockHeader;
        delete[] vertices;
        delete[] polygons;
    }
}

template<>
std::vector<CarModel> NFS2<PS1>::LoadGEO(const std::string &geo_path, std::map<unsigned int, Texture> car_textures, std::map<std::string, uint32_t> remapped_texture_ids) {
    std::string PS1_PART_NAMES[33]{
            "High Additional Body Part",
            "High Main Body Part",
            "High Ground Part",
            "High Front Part",
            "High Rear Part",
            "High Left Side Part",
            "High Right Side Part",
            "High Additional Left Side Part",
            "High Additional Right Side Part",
            "High Front Rear Grilles",
            "High Extra Side Parts",
            "High Spoiler Part",
            "High Additional Part",
            "High Backlights",
            "High Front Right Wheel",
            "High Front Right Wheel Part",
            "High Front Left Wheel",
            "High Front Left Wheel Part",
            "High Rear Right Wheel",
            "High Rear Right Wheel Part",
            "High Rear Left Wheel",
            "High Rear Left Wheel Part",
            "Medium Additional Body Part",
            "Medium Main Body Part",
            "Medium Ground Part",
            "Wheel Positions",
            "Medium/Low Side Parts",
            "Low Main Part",
            "Low Side Part",
            "Headlight Positions",
            "Backlight Positions",
            "Reserved",
            "Reserved"
    };
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(0, 0, 0)));
    float carScaleFactor = 2000.f;

    std::cout << "- Parsing GEO File " << std::endl;
    std::vector<CarModel> car_meshes;

    ifstream geo(geo_path, ios::in | ios::binary);

    auto *geoFileHeader = new PS1::GEO::HEADER();
    if (geo.read((char *) geoFileHeader, sizeof(PS1::GEO::HEADER)).gcount() != sizeof(PS1::GEO::HEADER)) {
        std::cout << "Couldn't open file/truncated." << std::endl;
        delete geoFileHeader;
        return car_meshes;
    }

    int32_t part_Idx = -1;

    while (true) {
        streamoff start = geo.tellg();
        std::cout << "Part " << part_Idx + 1 << " [" << PS1_PART_NAMES[part_Idx + 1] << "]" << std::endl;
        std::cout << "BlockStartOffset:   " << start << std::endl;
        auto *geoBlockHeader = new PS1::GEO::BLOCK_HEADER();
        while (geoBlockHeader->nVerts == 0) {
            ++part_Idx;
            geo.read((char *) geoBlockHeader, sizeof(PS1::GEO::BLOCK_HEADER));
            if (geo.eof()) {
                delete geoBlockHeader;
                delete geoFileHeader;
                return car_meshes;
            }
        }

        if ((geoBlockHeader->unknown[0] != 0) || (geoBlockHeader->unknown[1] != 1) || (geoBlockHeader->unknown[2] != 1)) {
            std::cout << "Invalid geometry header. This file is special (or corrupt)" << std::endl;
            delete geoBlockHeader;
            delete geoFileHeader;
            return car_meshes;
        }

        std::vector<uint32_t> indices;
        std::vector<uint32_t> texMapStuff;
        std::vector<glm::vec3> verts;
        std::vector<glm::vec3> norms;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> texture_indices;

        indices.reserve(geoBlockHeader->nPolygons * 6);
        verts.reserve(geoBlockHeader->nVerts);

        float specularDamper = 0.2f;
        float specularReflectivity = 0.02f;
        float envReflectivity = 0.4f;

        auto *vertices = new PS1::GEO::BLOCK_3D[geoBlockHeader->nVerts];
        geo.read((char *) vertices, (geoBlockHeader->nVerts) * sizeof(PS1::GEO::BLOCK_3D));
        std::cout << "VertTblEndOffset:   " << geo.tellg() << " Size: " << geo.tellg() - start << std::endl;

        // If nVerts is ODD, we need to pad. Let's dump the contents of the pad though, in case there's data here
        if (geoBlockHeader->nVerts % 2) {
            auto *pad = new uint16_t[3];
            geo.read((char *) pad, sizeof(uint16_t) * 3);
            if (pad[0] || pad[1] || pad[2]) {
                std::cout << "Normal Table Pre-Pad Contents: " << std::endl;
                for (uint32_t i = 0; i < 3; ++i) {
                    std::cout << pad[i] << std::endl;
                }
            }
            delete[] pad;
        }

        auto *normals = new PS1::GEO::BLOCK_3D[geoBlockHeader->nNormals];
        geo.read((char *) normals, (geoBlockHeader->nNormals) * sizeof(PS1::GEO::BLOCK_3D));
        std::cout << "NormTblEndOffset:   " << geo.tellg() << " Size: " << geo.tellg() - start << std::endl;

        auto *xblock_1 = new PS1::GEO::XBLOCK_1();
        auto *xblock_2 = new PS1::GEO::XBLOCK_2();
        auto *xblock_3 = new PS1::GEO::XBLOCK_3();
        auto *xblock_4 = new PS1::GEO::XBLOCK_4();
        auto *xblock_5 = new PS1::GEO::XBLOCK_5();
        // Is this really a block type?
        switch (geoBlockHeader->unknown1) {
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
                std::cout << "Unknown block type:  " << geoBlockHeader->unknown1 << std::endl;
        }

        streamoff end = geo.tellg();
        std::cout << "PolyTblStartOffset: " << end << " Size: " << end - start << std::endl;
        // Polygon Table start is aligned on 4 Byte boundary
        if (((end - start) % 4)) {
            uint16_t *pad = new uint16_t[3];
            geo.read((char *) pad, sizeof(uint16_t) * 3);
            if (pad[0] || pad[1] || pad[2]) {
                std::cout << "Polygon Table Pre-Pad Contents: " << std::endl;
                for (uint32_t i = 0; i < 3; ++i) {
                    std::cout << pad[i] << std::endl;
                }
            }
            delete[] pad;
        }

        auto *polygons = new PS1::GEO::POLY_3D[geoBlockHeader->nPolygons];
        geo.read((char *) polygons, geoBlockHeader->nPolygons * sizeof(PS1::GEO::POLY_3D));

        std::cout << "BlockEndOffset:     " << geo.tellg() << " Size: " << geo.tellg() - start << std::endl;

        for (uint32_t vert_Idx = 0; vert_Idx < geoBlockHeader->nVerts; ++vert_Idx) {
            verts.emplace_back(rotationMatrix * glm::vec3(vertices[vert_Idx].x / carScaleFactor, vertices[vert_Idx].y / carScaleFactor, vertices[vert_Idx].z / carScaleFactor));
        }

        for (uint32_t poly_Idx = 0; poly_Idx < geoBlockHeader->nPolygons; ++poly_Idx) {
            std::string textureName(polygons[poly_Idx].texName, polygons[poly_Idx].texName + 4);
            Texture gl_texture = car_textures[remapped_texture_ids[textureName]];

            texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
            texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
            texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
            texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
            texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);
            texMapStuff.emplace_back(polygons[poly_Idx].texName[0]);

            // TODO: There's another set of indices at index [2], that form barely valid polygons. Middle set [1] are always numbers that match, 0000, 1111, 2222, 3333.
            indices.emplace_back(polygons[poly_Idx].vertex[0][0]);
            indices.emplace_back(polygons[poly_Idx].vertex[0][1]);
            indices.emplace_back(polygons[poly_Idx].vertex[0][2]);
            indices.emplace_back(polygons[poly_Idx].vertex[0][0]);
            indices.emplace_back(polygons[poly_Idx].vertex[0][2]);
            indices.emplace_back(polygons[poly_Idx].vertex[0][3]);

            // TODO: Use Polygon TexMap type to fix texture mapping, use UV factory in trk utils
            std::vector<glm::vec2> transformedUVs = TrackUtils::nfsUvGenerate(NFS_3_PS1, CAR, polygons->texMap[0], gl_texture);
            uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

            glm::vec3 normal = rotationMatrix * calculateQuadNormal(pointToVec(vertices[polygons[poly_Idx].vertex[0][0]]), pointToVec(vertices[polygons[poly_Idx].vertex[0][1]]), pointToVec(vertices[polygons[poly_Idx].vertex[0][2]]), pointToVec(vertices[polygons[poly_Idx].vertex[0][3]]));

            // Use the R/L flag to flip normals
            if(!(polygons[poly_Idx].texMap[0] & 0x4)){
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
        glm::vec3 center = glm::vec3((geoBlockHeader->position[0] / 256.0f) / carScaleFactor, (geoBlockHeader->position[1] / 256.0f) / carScaleFactor, (geoBlockHeader->position[2] / 256.0f) / carScaleFactor);
        car_meshes.emplace_back(CarModel(PS1_PART_NAMES[part_Idx], verts, uvs, texture_indices, texMapStuff, norms, indices, center, specularDamper, specularReflectivity, envReflectivity));

        // Dump GeoBlock data for correlating with geometry/LOD's/Special Cases
        std::cout << "nVerts:    " << geoBlockHeader->nVerts << std::endl;
        std::cout << "unknown1:  " << geoBlockHeader->unknown1 << std::endl;
        std::cout << "nNormals:  " << geoBlockHeader->nNormals << std::endl;
        std::cout << "nPolygons: " << geoBlockHeader->nPolygons << std::endl;

        switch (geoBlockHeader->unknown1) {
            case 1:
                std::cout << "XBlock 1: " << std::endl;
                for (uint32_t i = 0; i < sizeof(xblock_1->unknown) / sizeof(xblock_1->unknown[0]); ++i) {
                    std::cout << (int) xblock_1->unknown[i] << std::endl;
                }
                break;
            case 2:
                std::cout << "XBlock 2: " << std::endl;
                for (uint32_t i = 0; i < sizeof(xblock_2->unknown) / sizeof(xblock_2->unknown[0]); ++i) {
                    std::cout << (int) xblock_2->unknown[i] << std::endl;
                }
                break;
            case 3:
                std::cout << "XBlock 3: " << std::endl;
                for (uint32_t i = 0; i < sizeof(xblock_3->unknown) / sizeof(xblock_3->unknown[0]); ++i) {
                    std::cout << (int) xblock_3->unknown[i] << std::endl;
                }
                break;
            case 4:
                std::cout << "XBlock 4: " << std::endl;
                for (uint32_t i = 0; i < sizeof(xblock_4->unknown) / sizeof(xblock_4->unknown[0]); ++i) {
                    std::cout << (int) xblock_4->unknown[i] << std::endl;
                }
                break;
            case 5:
                std::cout << "XBlock 5: " << std::endl;
                for (uint32_t i = 0; i < sizeof(xblock_5->unknown) / sizeof(xblock_5->unknown[0]); ++i) {
                    std::cout << (int) xblock_5->unknown[i] << std::endl;
                }
                break;
        }
        std::cout << "--------------------------" << std::endl;

        delete geoBlockHeader;
        delete[] normals;
        delete[] vertices;
        delete[] polygons;
        delete xblock_1;
        delete xblock_2;
        delete xblock_3;
        delete xblock_4;
        delete xblock_5;
    }
}

template<typename Platform>
std::shared_ptr<Car> NFS2<Platform>::LoadCar(const std::string &car_base_path) {
    boost::filesystem::path p(car_base_path);
    std::string car_name = p.filename().string();

    stringstream geo_path, psh_path, qfs_path, car_out_path;
    geo_path << car_base_path << ".GEO";
    psh_path << car_base_path << ".PSH";
    qfs_path << car_base_path << ".QFS";

    // For every file in here that's a BMP, load the data into a Texture object. This lets us easily access textures by an ID.
    std::map<unsigned int, Texture> car_textures;
    std::map<std::string, uint32_t> remapped_texture_ids;
    uint32_t remappedTextureID = 0;

    if (std::is_same<Platform, PS1>::value) {
        car_out_path << CAR_PATH << ToString(NFS_3_PS1) << "/" << car_name << "/";
        Utils::ExtractPSH(psh_path.str(), car_out_path.str());
    } else {
        car_out_path << CAR_PATH << ToString(NFS_2) << "/" << car_name << "/";
        Utils::ExtractQFS(qfs_path.str(), car_out_path.str());
    }

    for (boost::filesystem::directory_iterator itr(car_out_path.str()); itr != boost::filesystem::directory_iterator(); ++itr) {
        if (itr->path().filename().string().find("BMP") != std::string::npos && itr->path().filename().string().find("-a") == std::string::npos) {
            // Map texture names, strings, into numbers so I can use them for indexes into the eventual Texture Array
            remapped_texture_ids[itr->path().filename().replace_extension("").string()] = remappedTextureID++;
            if (std::is_same<Platform, PS1>::value) {
                GLubyte *data;
                GLsizei width;
                GLsizei height;
                ASSERT(Utils::LoadBmpCustomAlpha(itr->path().string().c_str(), &data, &width, &height, 0u), "Texture " << itr->path().string() << " did not load succesfully!");
                car_textures[remapped_texture_ids[itr->path().filename().replace_extension("").string()]] = Texture(remapped_texture_ids[itr->path().filename().replace_extension("").string()], data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
            } else {
                bmpread_t bmpAttr; // This will leak.
                ASSERT(bmpread(itr->path().string().c_str(), BMPREAD_ANY_SIZE | BMPREAD_ALPHA, &bmpAttr), "Texture " << itr->path().string() << " did not load succesfully!");
                //bmpAttr = TrackUtils::RemapNFS2CarColours(bmpAttr);
                car_textures[remapped_texture_ids[itr->path().filename().replace_extension("").string()]] = Texture(remapped_texture_ids[itr->path().filename().replace_extension("").string()], bmpAttr.data, static_cast<unsigned int>(bmpAttr.width), static_cast<unsigned int>(bmpAttr.height));
            }
        }
    }

    GLuint texture_array_id = TrackUtils::MakeTextureArray(car_textures, false);

    return std::make_shared<Car>(LoadGEO(geo_path.str(), car_textures, remapped_texture_ids), std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, car_name, texture_array_id);
}

// TRACK
template<typename Platform>
shared_ptr<typename Platform::TRACK> NFS2<Platform>::LoadTrack(const std::string &track_base_path) {
    std::cout << "--- Loading NFS2 Track ---" << std::endl;
    shared_ptr<typename Platform::TRACK> track(new typename Platform::TRACK());

    boost::filesystem::path p(track_base_path);
    track->name = p.filename().string();
    stringstream trk_path, col_path, can_path;

    trk_path << track_base_path << ".TRK";
    col_path << track_base_path << ".COL";

    NFSVer nfs_version = UNKNOWN;

    if (std::is_same<Platform, PC>::value) {
        if (track_base_path.find(ToString(NFS_2_SE)) != std::string::npos) {
            nfs_version = NFS_2_SE;
        } else {
            nfs_version = NFS_2;
        }
        can_path << RESOURCE_PATH << ToString(nfs_version) << "/GAMEDATA/TRACKS/DATA/PC/" << track->name << "00.CAN";
        ASSERT(TrackUtils::LoadCAN(can_path.str(), track->cameraAnimation), "Could not load CAN file (camera animation): " << can_path.str()); // Load camera intro/outro animation data
    } else if (std::is_same<Platform, PS1>::value) {
        nfs_version = NFS_3_PS1;
        std::string ps1_col_path = col_path.str();
        ps1_col_path.replace(ps1_col_path.find("ZZ"), 2, "");
        col_path.str(std::string());
        col_path << ps1_col_path;
    }

    ASSERT(LoadTRK(trk_path.str(), track), "Could not load TRK file: " << trk_path.str()); // Load TRK file to get track block specific data
    ASSERT(LoadCOL(col_path.str(), track), "Could not load COL file: " << col_path.str()); // Load Catalogue file to get global (non trkblock specific) data
    ASSERT(TrackUtils::ExtractTrackTextures(track_base_path, track->name, nfs_version), "Could not extract " << track->name << " texture pack.");

    // Load up the textures
    for (uint32_t tex_Idx = 0; tex_Idx < track->nTextures; tex_Idx++) {
        track->textures[track->polyToQFStexTable[tex_Idx].texNumber] = LoadTexture(track->polyToQFStexTable[tex_Idx], track->name, nfs_version);
    }

    track->textureArrayID = TrackUtils::MakeTextureArray(track->textures, false);
    ParseTRKModels(track);
    track->global_objects = ParseCOLModels(track);

    std::cout << "Track loaded successfully" << std::endl;
    return track;
}

template<typename Platform>
bool NFS2<Platform>::LoadTRK(std::string trk_path, const shared_ptr<typename Platform::TRACK> &track) {
    std::cout << "- Parsing TRK File " << std::endl;
    ifstream trk(trk_path, ios::in | ios::binary);
    // TRK file header data
    unsigned char header[4];
    long unknownHeader[5];

    // Check we're in a valid TRK file
    if (trk.read(((char *) header), sizeof(unsigned char) * 4).gcount() != sizeof(unsigned char) * 4) {
        std::cout << "Couldn't open file/truncated." << std::endl;
        return false;
    }
    // Header should contain TRAC
    if (memcmp(header, "TRAC", sizeof(header)) != 0) {
        std::cout << "Invalid TRK Header." << std::endl;
        return false;
    }

    // Unknown header data
    if (trk.read(((char *) unknownHeader), sizeof(uint32_t) * 5).gcount() != sizeof(uint32_t) * 5) return false;

    // Basic Track data
    trk.read((char *) &track->nSuperBlocks, sizeof(uint32_t));
    trk.read((char *) &track->nBlocks, sizeof(uint32_t));
    track->superblocks = static_cast<typename Platform::SUPERBLOCK *>(calloc(track->nBlocks, sizeof(typename Platform::SUPERBLOCK)));

    // Offsets of Superblocks in TRK file
    uint32_t *superblockOffsets = static_cast<uint32_t *>(calloc(track->nSuperBlocks, sizeof(uint32_t)));
    if (trk.read(((char *) superblockOffsets), track->nSuperBlocks * sizeof(uint32_t)).gcount() != track->nSuperBlocks * sizeof(uint32_t)) {
        free(superblockOffsets);
        return false;
    }

    // Reference coordinates for each block
    track->blockReferenceCoords = static_cast<VERT_HIGHP *>(calloc(track->nBlocks, sizeof(VERT_HIGHP)));
    if (trk.read((char *) track->blockReferenceCoords, track->nBlocks * sizeof(VERT_HIGHP)).gcount() != track->nBlocks * sizeof(VERT_HIGHP)) {
        free(superblockOffsets);
        return false;
    }

    for (uint32_t superBlock_Idx = 0; superBlock_Idx < track->nSuperBlocks; ++superBlock_Idx) {
        std::cout << "SuperBlock " << superBlock_Idx + 1 << " of " << track->nSuperBlocks << std::endl;
        // Get the superblock header
        auto *superblock = &track->superblocks[superBlock_Idx];
        trk.seekg(superblockOffsets[superBlock_Idx], ios_base::beg);
        trk.read((char *) &superblock->superBlockSize, sizeof(uint32_t));
        trk.read((char *) &superblock->nBlocks, sizeof(uint32_t));
        trk.read((char *) &superblock->padding, sizeof(uint32_t));

        if (superblock->nBlocks != 0) {
            // Get the offsets of the child blocks within superblock
            uint32_t *blockOffsets = (uint32_t *) calloc(static_cast<size_t>(superblock->nBlocks), sizeof(uint32_t));
            trk.read((char *) blockOffsets, superblock->nBlocks * sizeof(uint32_t));
            superblock->trackBlocks = static_cast<typename Platform::TRKBLOCK *>(calloc(static_cast<size_t>(superblock->nBlocks), sizeof(typename Platform::TRKBLOCK)));

            for (uint32_t block_Idx = 0; block_Idx < superblock->nBlocks; ++block_Idx) {
                auto *trackblock = &superblock->trackBlocks[block_Idx];
                // Read Header
                trackblock->header = static_cast<TRKBLOCK_HEADER *>(calloc(1, sizeof(TRKBLOCK_HEADER)));
                trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx], ios_base::beg);
                trk.read((char *) trackblock->header, sizeof(TRKBLOCK_HEADER));
                std::cout << "  Block " << block_Idx + 1 << " of " << superblock->nBlocks << " [" << trackblock->header->blockSerial << "]" << std::endl;

                // Sanity Checks
                if ((trackblock->header->blockSize != trackblock->header->blockSizeDup) || (trackblock->header->blockSerial > track->nBlocks)) {
                    std::cout << "   --- Bad Block" << std::endl;
                    free(superblockOffsets);
                    return false;
                }

                // Read 3D Data
                trackblock->vertexTable = static_cast<typename Platform::VERT *>(calloc(static_cast<size_t>(trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert), sizeof(typename Platform::VERT)));
                for (int32_t vert_Idx = 0; vert_Idx < trackblock->header->nStickToNextVerts + trackblock->header->nHighResVert; ++vert_Idx) {
                    trk.read((char *) &trackblock->vertexTable[vert_Idx], sizeof(typename Platform::VERT));
                }

                trackblock->polygonTable = static_cast<typename Platform::POLYGONDATA *>(calloc(static_cast<size_t>(trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly), sizeof(typename Platform::POLYGONDATA)));
                for (int32_t poly_Idx = 0; poly_Idx < (trackblock->header->nLowResPoly + trackblock->header->nMedResPoly + trackblock->header->nHighResPoly); ++poly_Idx) {
                    trk.read((char *) &trackblock->polygonTable[poly_Idx], sizeof(typename Platform::POLYGONDATA));
                }

                // Read Extrablock data
                trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + 64u + trackblock->header->extraBlockTblOffset, ios_base::beg);
                // Get extrablock offsets (relative to beginning of TrackBlock)
                uint32_t *extrablockOffsets = (uint32_t *) calloc(trackblock->header->nExtraBlocks, sizeof(uint32_t));
                trk.read((char *) extrablockOffsets, trackblock->header->nExtraBlocks * sizeof(uint32_t));

                for (uint32_t xblock_Idx = 0; xblock_Idx < trackblock->header->nExtraBlocks; ++xblock_Idx) {
                    trk.seekg(superblockOffsets[superBlock_Idx] + blockOffsets[block_Idx] + extrablockOffsets[xblock_Idx], ios_base::beg);
                    auto *xblockHeader = static_cast<EXTRABLOCK_HEADER *>(calloc(1, sizeof(EXTRABLOCK_HEADER)));
                    trk.read((char *) xblockHeader, sizeof(EXTRABLOCK_HEADER));

                    switch (xblockHeader->XBID) {
                        case 5:
                            trackblock->polyTypes = static_cast<POLY_TYPE *>(calloc(xblockHeader->nRecords, sizeof(POLY_TYPE)));
                            trk.read((char *) trackblock->polyTypes, xblockHeader->nRecords * sizeof(POLY_TYPE));
                            break;
                        case 4:
                            trackblock->nNeighbours = xblockHeader->nRecords;
                            trackblock->blockNeighbours = (uint16_t *) calloc(xblockHeader->nRecords, sizeof(uint16_t));
                            trk.read((char *) trackblock->blockNeighbours, xblockHeader->nRecords * sizeof(uint16_t));
                            break;
                        case 8:
                            trackblock->structures = static_cast<typename Platform::GEOM_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(typename Platform::GEOM_BLOCK)));
                            trackblock->nStructures = xblockHeader->nRecords;
                            for (uint32_t structure_Idx = 0; structure_Idx < trackblock->nStructures; ++structure_Idx) {
                                streamoff padCheck = trk.tellg();
                                trk.read((char *) &trackblock->structures[structure_Idx].recSize, sizeof(uint32_t));
                                trk.read((char *) &trackblock->structures[structure_Idx].nVerts, sizeof(uint16_t));
                                trk.read((char *) &trackblock->structures[structure_Idx].nPoly, sizeof(uint16_t));
                                trackblock->structures[structure_Idx].vertexTable = static_cast<typename Platform::VERT *>(calloc(trackblock->structures[structure_Idx].nVerts, sizeof(typename Platform::VERT)));
                                for (uint32_t vert_Idx = 0; vert_Idx < trackblock->structures[structure_Idx].nVerts; ++vert_Idx) {
                                    trk.read((char *) &trackblock->structures[structure_Idx].vertexTable[vert_Idx], sizeof(typename Platform::VERT));
                                }
                                trackblock->structures[structure_Idx].polygonTable = static_cast<typename Platform::POLYGONDATA *>(calloc(trackblock->structures[structure_Idx].nPoly, sizeof(typename Platform::POLYGONDATA)));
                                for (uint32_t poly_Idx = 0; poly_Idx < trackblock->structures[structure_Idx].nPoly; ++poly_Idx) {
                                    trk.read((char *) &trackblock->structures[structure_Idx].polygonTable[poly_Idx], sizeof(typename Platform::POLYGONDATA));
                                }
                                trk.seekg(trackblock->structures[structure_Idx].recSize - (trk.tellg() - padCheck), ios_base::cur); // Eat possible padding
                            }
                            break;
                        case 7:
                        case 18:
                        case 19:
                            trackblock->nStructureReferences += xblockHeader->nRecords;
                            for (uint32_t structureRef_Idx = 0; structureRef_Idx < trackblock->nStructureReferences; ++structureRef_Idx) {
                                streamoff padCheck = trk.tellg();
                                GEOM_REF_BLOCK structure;
                                trk.read((char *) &structure.recSize, sizeof(uint16_t));
                                trk.read((char *) &structure.recType, sizeof(uint8_t));
                                trk.read((char *) &structure.structureRef, sizeof(uint8_t));
                                // Fixed type
                                if (structure.recType == 1) {
                                    trk.read((char *) &structure.refCoordinates, sizeof(VERT_HIGHP));
                                } else if (structure.recType == 3) { // Animated type
                                    trk.read((char *) &structure.animLength, sizeof(uint16_t));
                                    trk.read((char *) &structure.unknown, sizeof(uint16_t));
                                    structure.animationData = static_cast<ANIM_POS *>(calloc(structure.animLength, sizeof(ANIM_POS)));
                                    for (uint32_t animation_Idx = 0; animation_Idx < structure.animLength; ++animation_Idx) {
                                        trk.read((char *) &structure.animationData[animation_Idx], sizeof(ANIM_POS));
                                    }
                                } else if (structure.recType == 4) {
                                    // 4 Component PSX Vert data? TODO: Restructure to allow the 4th component to be read
                                    trk.read((char *) &structure.refCoordinates, sizeof(VERT_HIGHP));
                                } else {
                                    std::cout << "XBID " << xblockHeader->XBID << " Unknown Structure Reference type: " << (int) structure.recType << " Size: " << (int) structure.recSize << " StructRef: " << (int) structure.structureRef << std::endl;
                                    continue;
                                }
                                trackblock->structureRefData.emplace_back(structure);
                                trk.seekg(structure.recSize - (trk.tellg() - padCheck), ios_base::cur); // Eat possible padding
                            }
                            break;
                            // PS1 Specific XBID, Misc purpose
                            /*case 10: {
                                std::cout << "XBID 10 NStruct: " << xblockHeader->nRecords << std::endl;
                                PS1::TRKBLOCK *ps1TrackBlock = ((PS1::TRKBLOCK *) trackblock);
                                ps1TrackBlock->nUnknownVerts =  xblockHeader->nRecords;
                                uint8_t xbidHeader[8];
                                trk.read((char *) xbidHeader, 8);
                                for(int i = 0; i < 8; ++i){
                                    std::cout << (int) xbidHeader[i] << std::endl;
                                }
                                // TODO: Likely these are not VERTS, and the act of adding the parent block center gives meaning where none is present.
                                ps1TrackBlock->unknownVerts = new PS1::VERT[xblockHeader->nRecords];
                                for (uint32_t record_Idx = 0; record_Idx < xblockHeader->nRecords; ++record_Idx) {
                                    trk.read((char *) &ps1TrackBlock->unknownVerts[record_Idx], sizeof(PS1::VERT));
                                }
                            }
                                break;*/
                        case 6:
                            trackblock->medianData = static_cast<MEDIAN_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(MEDIAN_BLOCK)));
                            trk.read((char *) trackblock->medianData, xblockHeader->nRecords * sizeof(MEDIAN_BLOCK));
                            break;
                        case 13:
                            trackblock->nVroad = xblockHeader->nRecords;
                            trackblock->vroadData = static_cast<typename Platform::VROAD *>(calloc(xblockHeader->nRecords, sizeof(typename Platform::VROAD)));
                            trk.read((char *) trackblock->vroadData, trackblock->nVroad * sizeof(typename Platform::VROAD));
                            break;
                        case 9:
                            trackblock->nLanes = xblockHeader->nRecords;
                            trackblock->laneData = static_cast<LANE_BLOCK *>(calloc(xblockHeader->nRecords, sizeof(LANE_BLOCK)));
                            trk.read((char *) trackblock->laneData, trackblock->nLanes * sizeof(LANE_BLOCK));
                            break;
                        default:
                            std::cout << "Unknown XBID: " << xblockHeader->XBID << " nRecords: " << xblockHeader->nRecords << " RecSize: " << xblockHeader->recSize << std::endl;
                            break;
                    }
                    free(xblockHeader);
                }
                free(extrablockOffsets);
            }
            free(blockOffsets);
        }
    }
    free(superblockOffsets);
    trk.close();
    return true;
}

template<typename Platform>
bool NFS2<Platform>::LoadCOL(std::string col_path, const shared_ptr<typename Platform::TRACK> &track) {
    std::cout << "- Parsing COL File " << std::endl;
    ifstream col(col_path, ios::in | ios::binary);
    // Check we're in a valid TRK file
    unsigned char header[4];
    if (col.read(((char *) header), sizeof(unsigned char) * 4).gcount() != sizeof(unsigned char) * 4) return false;
    if (memcmp(header, "COLL", sizeof(header)) != 0) return false;

    uint32_t version;
    col.read((char *) &version, sizeof(uint32_t));
    if (version != 11) return false;

    streampos colSize;
    col.read((char *) &colSize, sizeof(uint32_t));

    uint32_t nExtraBlocks;
    col.read((char *) &nExtraBlocks, sizeof(uint32_t));

    uint32_t *extraBlockOffsets = (uint32_t *) calloc(nExtraBlocks, sizeof(uint32_t));
    col.read((char *) extraBlockOffsets, nExtraBlocks * sizeof(uint32_t));

    std::cout << "  Version: " << version << "\n  nExtraBlocks: " << nExtraBlocks << "\nParsing COL Extrablocks" << std::endl;

    for (uint32_t xBlock_Idx = 0; xBlock_Idx < nExtraBlocks; ++xBlock_Idx) {
        col.seekg(16 + extraBlockOffsets[xBlock_Idx], ios_base::beg);

        auto *xblockHeader = static_cast<EXTRABLOCK_HEADER *>(calloc(1, sizeof(EXTRABLOCK_HEADER)));
        col.read((char *) xblockHeader, sizeof(EXTRABLOCK_HEADER));

        std::cout << "  XBID " << (int) xblockHeader->XBID << " (XBlock " << xBlock_Idx + 1 << " of " << nExtraBlocks << ")" << std::endl;

        switch (xblockHeader->XBID) {
            case 2: // First xbock always texture table
                track->nTextures = xblockHeader->nRecords;
                track->polyToQFStexTable = static_cast<TEXTURE_BLOCK *>(calloc(track->nTextures, sizeof(TEXTURE_BLOCK)));
                col.read((char *) track->polyToQFStexTable, track->nTextures * sizeof(TEXTURE_BLOCK));
                break;
            case 8: // XBID 8 3D Structure data: This block is only present if nExtraBlocks != 2
                track->nColStructures = xblockHeader->nRecords;
                track->colStructures = static_cast<typename Platform::GEOM_BLOCK *>(calloc(track->nColStructures, sizeof(typename Platform::GEOM_BLOCK)));
                for (uint32_t structure_Idx = 0; structure_Idx < track->nColStructures; ++structure_Idx) {
                    streamoff padCheck = col.tellg();
                    col.read((char *) &track->colStructures[structure_Idx].recSize, sizeof(uint32_t));
                    col.read((char *) &track->colStructures[structure_Idx].nVerts, sizeof(uint16_t));
                    col.read((char *) &track->colStructures[structure_Idx].nPoly, sizeof(uint16_t));
                    track->colStructures[structure_Idx].vertexTable = static_cast<typename Platform::VERT *>(calloc(track->colStructures[structure_Idx].nVerts, sizeof(typename Platform::VERT)));
                    for (uint32_t vert_Idx = 0; vert_Idx < track->colStructures[structure_Idx].nVerts; ++vert_Idx) {
                        col.read((char *) &track->colStructures[structure_Idx].vertexTable[vert_Idx], sizeof(typename Platform::VERT));
                    }
                    track->colStructures[structure_Idx].polygonTable = static_cast<typename Platform::POLYGONDATA *>(calloc(track->colStructures[structure_Idx].nPoly, sizeof(typename Platform::POLYGONDATA)));
                    for (uint32_t poly_Idx = 0; poly_Idx < track->colStructures[structure_Idx].nPoly; ++poly_Idx) {
                        col.read((char *) &track->colStructures[structure_Idx].polygonTable[poly_Idx], sizeof(typename Platform::POLYGONDATA));
                    }
                    col.seekg(track->colStructures[structure_Idx].recSize - (col.tellg() - padCheck), ios_base::cur); // Eat possible padding
                }
                break;
            case 7: // XBID 7 3D Structure Reference: This block is only present if nExtraBlocks != 2
                track->nColStructureReferences = xblockHeader->nRecords;
                for (uint32_t structureRef_Idx = 0; structureRef_Idx < track->nColStructures; ++structureRef_Idx) {
                    streamoff padCheck = col.tellg();
                    GEOM_REF_BLOCK structure;
                    col.read((char *) &structure.recSize, sizeof(uint16_t));
                    col.read((char *) &structure.recType, sizeof(uint8_t));
                    col.read((char *) &structure.structureRef, sizeof(uint8_t));
                    // Fixed type
                    if (structure.recType == 1) {
                        col.read((char *) &structure.refCoordinates, sizeof(VERT_HIGHP));
                    } else if (structure.recType == 3) { // Animated type
                        col.read((char *) &structure.animLength, sizeof(uint16_t));
                        col.read((char *) &structure.unknown, sizeof(uint16_t));
                        structure.animationData = static_cast<ANIM_POS *>(calloc(structure.animLength, sizeof(ANIM_POS)));
                        for (uint32_t animation_Idx = 0; animation_Idx < structure.animLength; ++animation_Idx) {
                            col.read((char *) &structure.animationData[animation_Idx], sizeof(ANIM_POS));
                        }
                    } else if (structure.recType == 4) {
                        // 4 Component PSX Vert data? TODO: Restructure to allow the 4th component to be read
                        col.read((char *) &structure.refCoordinates, sizeof(VERT_HIGHP));
                    } else {
                        std::cout << "XBID " << xblockHeader->XBID << " Unknown COL Structure Reference type: " << (int) structure.recType << " Size: " << (int) structure.recSize << " StructRef: " << (int) structure.structureRef << std::endl;
                    }
                    track->colStructureRefData.emplace_back(structure);
                    col.seekg(structure.recSize - (col.tellg() - padCheck), ios_base::cur); // Eat possible padding
                }
                break;
            case 15:
                track->nCollisionData = xblockHeader->nRecords;
                track->collisionData = static_cast<COLLISION_BLOCK *>(calloc(track->nCollisionData, sizeof(COLLISION_BLOCK)));
                col.read((char *) track->collisionData, track->nCollisionData * sizeof(COLLISION_BLOCK));
                break;
            default:
                break;
        }
        free(xblockHeader);
    }
    ASSERT(col.tellg() == colSize, "Missing Data from the COL file! Read bytes: " << col.tellg() << " Col Reported bytes: " << colSize);
    col.close();
    return true;
}

template<typename Platform>
void NFS2<Platform>::dbgPrintVerts(const std::string &path, const shared_ptr<typename Platform::TRACK> &track) {
    std::ofstream obj_dump;

    if (!(boost::filesystem::exists(path))) {
        boost::filesystem::create_directories(path);
    }

    // Parse out TRKBlock data
    for (uint32_t superBlock_Idx = 0; superBlock_Idx < track->nSuperBlocks; ++superBlock_Idx) {
        auto *superblock = &track->superblocks[superBlock_Idx];
        for (uint32_t block_Idx = 0; block_Idx < superblock->nBlocks; ++block_Idx) {
            auto trkBlock = superblock->trackBlocks[block_Idx];
            VERT_HIGHP blockReferenceCoord;
            // Print clipping rectangle
            //obj_dump << "o Block" << trkBlock.header->blockSerial << "ClippingRect" << std::endl;
            //for(int i = 0; i < 4; i++){
            //    obj_dump << "v " << trkBlock.header->clippingRect[i].x << " " << trkBlock.header->clippingRect[i].z << " " << trkBlock.header->clippingRect[i].y << std::endl;
            //}
            // obj_dump << "f " << 1+(4*trkBlock.header->blockSerial) << " " << 2+(4*trkBlock.header->blockSerial) << " " << 3+(4*trkBlock.header->blockSerial) << " " << 4+(4*trkBlock.header->blockSerial) << std::endl;
            std::ostringstream stringStream;
            stringStream << path << "TrackBlock" << trkBlock.header->blockSerial << ".obj";
            obj_dump.open(stringStream.str());
            obj_dump << "o TrackBlock" << trkBlock.header->blockSerial << std::endl;
            for (int32_t i = 0; i < trkBlock.header->nStickToNextVerts + trkBlock.header->nHighResVert; i++) {
                if (i < trkBlock.header->nStickToNextVerts) {
                    // If in last block go get ref coord of first block, else get ref of next block
                    blockReferenceCoord = (trkBlock.header->blockSerial == track->nBlocks - 1) ? track->blockReferenceCoords[0] : track->blockReferenceCoords[trkBlock.header->blockSerial + 1];
                } else {
                    blockReferenceCoord = track->blockReferenceCoords[trkBlock.header->blockSerial];
                }
                int32_t x = (blockReferenceCoord.x + (256 * trkBlock.vertexTable[i].x));
                int32_t y = (blockReferenceCoord.y + (256 * trkBlock.vertexTable[i].y));
                int32_t z = (blockReferenceCoord.z + (256 * trkBlock.vertexTable[i].z));
                obj_dump << "v " << x / scaleFactor << " " << z / scaleFactor << " " << y / scaleFactor << std::endl;
            }
            for (int32_t poly_Idx = (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly); poly_Idx < (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly + trkBlock.header->nHighResPoly); ++poly_Idx) {
                obj_dump << "f " << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[0] + 1 << " "
                         << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[1] + 1 << " "
                         << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[2] + 1 << " "
                         << (unsigned int) trkBlock.polygonTable[poly_Idx].vertex[3] + 1 << std::endl;
            }
            obj_dump.close();
            for (uint32_t structure_Idx = 0; structure_Idx < trkBlock.nStructures; ++structure_Idx) {
                std::ostringstream stringStream1;
                stringStream1 << path << "SB" << superBlock_Idx << "TB" << block_Idx << "S" << structure_Idx << ".obj";
                obj_dump.open(stringStream1.str());
                VERT_HIGHP *structureReferenceCoordinates = &track->blockReferenceCoords[trkBlock.header->blockSerial];
                // Find the structure reference that matches this structure, else use block default
                for (auto &structure : trkBlock.structureRefData) {
                    // Only check fixed type structure references
                    if (structure.structureRef == structure_Idx) {
                        if (structure.recType == 1 || structure.recType == 4) {
                            structureReferenceCoordinates = &structure.refCoordinates;
                            break;
                        } else if (structure.recType == 3) {
                            // For now, if animated, use position 0 of animation sequence
                            structureReferenceCoordinates = &structure.animationData[0].position;
                            break;
                        }
                    }
                }
                if (structureReferenceCoordinates == &track->blockReferenceCoords[trkBlock.header->blockSerial]) {
                    std::cout << "Couldn't find a reference coordinate for Structure " << structure_Idx << " in SB" << superBlock_Idx << "TB" << block_Idx << std::endl;
                }
                obj_dump << "o Struct" << &trkBlock.structures[structure_Idx] << std::endl;
                for (uint16_t vert_Idx = 0; vert_Idx < trkBlock.structures[structure_Idx].nVerts; ++vert_Idx) {
                    int32_t x = (structureReferenceCoordinates->x + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].x));
                    int32_t y = (structureReferenceCoordinates->y + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].y));
                    int32_t z = (structureReferenceCoordinates->z + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].z));
                    obj_dump << "v " << x / scaleFactor << " " << z / scaleFactor << " " << y / scaleFactor << std::endl;
                }
                for (uint32_t poly_Idx = 0; poly_Idx < trkBlock.structures[structure_Idx].nPoly; ++poly_Idx) {
                    obj_dump << "f " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[0] + 1
                             << " " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[1] + 1
                             << " " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[2] + 1
                             << " " << (unsigned int) trkBlock.structures[structure_Idx].polygonTable[poly_Idx].vertex[3] + 1 << std::endl;
                }
                obj_dump.close();
            }
        }
    }

    // Parse out COL data
    for (uint32_t structure_Idx = 0; structure_Idx < track->nColStructures; ++structure_Idx) {
        std::ostringstream stringStream1;
        stringStream1 << path << "COL" << structure_Idx << ".obj";
        obj_dump.open(stringStream1.str());
        VERT_HIGHP *structureReferenceCoordinates = static_cast<VERT_HIGHP *>(calloc(1, sizeof(VERT_HIGHP)));
        // Find the structure reference that matches this structure, else use block default
        for (uint32_t structRef_Idx = 0; structRef_Idx < track->nColStructureReferences; ++structRef_Idx) {
            // Only check fixed type structure references
            if (track->colStructureRefData[structRef_Idx].structureRef == structure_Idx) {
                if (track->colStructureRefData[structRef_Idx].recType == 1 || track->colStructureRefData[structRef_Idx].recType == 4) {
                    structureReferenceCoordinates = &track->colStructureRefData[structure_Idx].refCoordinates;
                    break;
                } else if (track->colStructureRefData[structRef_Idx].recType == 3) {
                    if (track->colStructureRefData[structure_Idx].animLength != 0) {
                        // For now, if animated, use position 0 of animation sequence
                        structureReferenceCoordinates = &track->colStructureRefData[structure_Idx].animationData[0].position;
                        break;
                    }
                }
            }
            if (structRef_Idx == track->nColStructureReferences - 1)
                std::cout << "Couldn't find a reference coordinate for COL Structure " << structRef_Idx << std::endl;
        }
        obj_dump << "o ColStruct" << &track->colStructures[structure_Idx] << std::endl;
        for (uint16_t vert_Idx = 0; vert_Idx < track->colStructures[structure_Idx].nVerts; ++vert_Idx) {
            int32_t x = (structureReferenceCoordinates->x + (256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].x));
            int32_t y = (structureReferenceCoordinates->y + (256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].y));
            int32_t z = (structureReferenceCoordinates->z + (256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].z));
            obj_dump << "v " << x / scaleFactor << " " << z / scaleFactor << " " << y / scaleFactor << std::endl;
        }
        for (uint32_t poly_Idx = 0; poly_Idx < track->colStructures[structure_Idx].nPoly; ++poly_Idx) {
            obj_dump << "f " << (unsigned int) track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[0] + 1
                     << " " << (unsigned int) track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[1] + 1
                     << " " << (unsigned int) track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[2] + 1
                     << " " << (unsigned int) track->colStructures[structure_Idx].polygonTable[poly_Idx].vertex[3] + 1
                     << std::endl;
        }
        obj_dump.close();
        free(structureReferenceCoordinates);
    }
}


template<typename Platform>
void NFS2<Platform>::ParseTRKModels(const shared_ptr<typename Platform::TRACK> &track) {
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    // Parse out TRKBlock data
    for (uint32_t superBlock_Idx = 0; superBlock_Idx < track->nSuperBlocks; ++superBlock_Idx) {
        auto *superblock = &track->superblocks[superBlock_Idx];
        for (uint32_t block_Idx = 0; block_Idx < superblock->nBlocks; ++block_Idx) {
            // Base Track Geometry
            auto trkBlock = superblock->trackBlocks[block_Idx];
            VERT_HIGHP blockReferenceCoord;

            glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
            TrackBlock current_track_block(trkBlock.header->blockSerial, orientation * glm::vec3(track->blockReferenceCoords[trkBlock.header->blockSerial].x / scaleFactor, track->blockReferenceCoords[trkBlock.header->blockSerial].y / scaleFactor, track->blockReferenceCoords[trkBlock.header->blockSerial].z / scaleFactor));

            if (trkBlock.nStructures != trkBlock.nStructureReferences) {
                std::cerr << "Trk block " << (int) trkBlock.header->blockSerial << " is missing " << trkBlock.nStructures - trkBlock.nStructureReferences << " structure locations!" << std::endl;
            }


            if (std::is_same<Platform, PS1>::value) {
                VERT_HIGHP *refCoord = &track->blockReferenceCoords[trkBlock.header->blockSerial];
                // PS1 Specific Misc XBID 10 Vert debug
                PS1::TRKBLOCK *ps1TrackBlock = ((PS1::TRKBLOCK *) &superblock->trackBlocks[block_Idx]);
                for (uint32_t j = 0; j < ps1TrackBlock->nUnknownVerts; j++) {
                    glm::vec3 light_center = rotationMatrix * glm::vec3((refCoord->x + (256 * ps1TrackBlock->unknownVerts[j].x)) / scaleFactor, (refCoord->y + (256 * ps1TrackBlock->unknownVerts[j].y)) / scaleFactor, (refCoord->z + (256 * ps1TrackBlock->unknownVerts[j].y)) / scaleFactor);
                    current_track_block.lights.emplace_back(Entity(0, j, std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, LIGHT, TrackUtils::MakeLight(light_center, 0)));
                }
            }

            // Structures
            for (uint32_t structure_Idx = 0; structure_Idx < trkBlock.nStructures; ++structure_Idx) {
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
                for (auto &structure : trkBlock.structureRefData) {
                    // Only check fixed type structure references
                    if (structure.structureRef == structure_Idx) {
                        if (structure.recType == 1 || structure.recType == 4) {
                            structureReferenceCoordinates = &structure.refCoordinates;
                            break;
                        } else if (structure.recType == 3) {
                            // For now, if animated, use position 0 of animation sequence
                            structureReferenceCoordinates = &structure.animationData[0].position;
                            break;
                        }
                    }
                }
                if (structureReferenceCoordinates == &track->blockReferenceCoords[trkBlock.header->blockSerial]) {
                    std::cout << "Couldn't find a reference coordinate for Structure " << structure_Idx << " in SB" << superBlock_Idx << "TB" << block_Idx << std::endl;
                }
                for (uint16_t vert_Idx = 0; vert_Idx < trkBlock.structures[structure_Idx].nVerts; ++vert_Idx) {
                    int32_t x = structureReferenceCoordinates->x + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].x);
                    int32_t y = structureReferenceCoordinates->y + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].y);
                    int32_t z = structureReferenceCoordinates->z + (256 * trkBlock.structures[structure_Idx].vertexTable[vert_Idx].z);
                    verts.emplace_back(rotationMatrix * glm::vec3(x / scaleFactor, y / scaleFactor, z / scaleFactor));
                    shading_verts.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
                }
                for (uint32_t poly_Idx = 0; poly_Idx < trkBlock.structures[structure_Idx].nPoly; ++poly_Idx) {
                    // Remap the COL TextureID's using the COL texture block (XBID2)
                    TEXTURE_BLOCK texture_for_block = track->polyToQFStexTable[trkBlock.structures[structure_Idx].polygonTable[poly_Idx].texture];
                    Texture gl_texture = track->textures[texture_for_block.texNumber];
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

                    std::vector<glm::vec2> transformedUVs = TrackUtils::nfsUvGenerate(std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, XOBJ, texture_for_block.alignmentData, gl_texture);
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
                std::stringstream xobj_name;
                xobj_name << "SB" << superBlock_Idx << "TB" << block_Idx << "S" << structure_Idx << ".obj";
                current_track_block.objects.emplace_back(Entity(superBlock_Idx, (trkBlock.header->blockSerial * trkBlock.nStructures) * structure_Idx, std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, XOBJ, Track(verts, norms, uvs, texture_indices, vertex_indices, shading_verts, debug_data, glm::vec3(0, 0, 0))));
            }

            // Mesh Data
            std::vector<unsigned int> vertex_indices;
            std::vector<glm::vec2> uvs;
            std::vector<unsigned int> texture_indices;
            std::vector<glm::vec3> verts;
            std::vector<glm::vec4> trk_block_shading_verts;
            std::vector<glm::vec3> norms;
            std::vector<uint32_t> debug_data;

            for (int32_t i = 0; i < trkBlock.header->nStickToNextVerts + trkBlock.header->nHighResVert; i++) {
                if (i < trkBlock.header->nStickToNextVerts) {
                    // If in last block go get ref coord of first block, else get ref of next block
                    blockReferenceCoord = (trkBlock.header->blockSerial == track->nBlocks - 1) ? track->blockReferenceCoords[0] : track->blockReferenceCoords[trkBlock.header->blockSerial + 1];
                } else {
                    blockReferenceCoord = track->blockReferenceCoords[trkBlock.header->blockSerial];
                }
                int32_t x = (blockReferenceCoord.x + (256 * trkBlock.vertexTable[i].x));
                int32_t y = (blockReferenceCoord.y + (256 * trkBlock.vertexTable[i].y));
                int32_t z = (blockReferenceCoord.z + (256 * trkBlock.vertexTable[i].z));
                verts.emplace_back(rotationMatrix * glm::vec3(x / scaleFactor, y / scaleFactor, z / scaleFactor));
                trk_block_shading_verts.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));

            }
            for (int32_t poly_Idx = (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly); poly_Idx < (trkBlock.header->nLowResPoly + trkBlock.header->nMedResPoly + trkBlock.header->nHighResPoly); ++poly_Idx) {
                // Remap the COL TextureID's using the COL texture block (XBID2)
                TEXTURE_BLOCK texture_for_block = track->polyToQFStexTable[trkBlock.polygonTable[poly_Idx].texture];
                Texture gl_texture = track->textures[texture_for_block.texNumber];
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

                std::vector<glm::vec2> transformedUVs = TrackUtils::nfsUvGenerate(std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, ROAD, texture_for_block.alignmentData, gl_texture);
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
            current_track_block.track.emplace_back(Entity(superBlock_Idx, trkBlock.header->blockSerial, std::is_same<Platform, PS1>::value ? NFS_3_PS1 : NFS_2, ROAD, Track(verts, norms, uvs, texture_indices, vertex_indices, trk_block_shading_verts, debug_data, glm::vec3(0, 0, 0))));

            track->track_blocks.emplace_back(current_track_block);
        }

    }
}

template<typename Platform>
std::vector<Entity> NFS2<Platform>::ParseCOLModels(const shared_ptr<typename Platform::TRACK> &track) {
    glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.

    std::vector<Entity> col_entities;

    // Parse out COL data
    for (uint32_t structure_Idx = 0; structure_Idx < track->nColStructures; ++structure_Idx) {
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> texture_indices;
        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> shading_data;

        auto *structureReferenceCoordinates = static_cast<VERT_HIGHP *>(calloc(1, sizeof(VERT_HIGHP)));
        // Find the structure reference that matches this structure, else use block default
        for (auto &structure : track->colStructureRefData) {
            // Only check fixed type structure references
            if (structure.structureRef == structure_Idx) {
                if (structure.recType == 1 || structure.recType == 4) {
                    structureReferenceCoordinates = &structure.refCoordinates;
                    break;
                } else if (structure.recType == 3) {
                    // For now, if animated, use position 0 of animation sequence
                    structureReferenceCoordinates = &structure.animationData[0].position;
                    break;
                }
            }
        }
        if (!structureReferenceCoordinates->x || !structureReferenceCoordinates->y || !structureReferenceCoordinates->z) {
            std::cerr << "Couldn't find a reference coordinate for Structure " << structure_Idx << " in COL file" << std::endl;
        }
        for (uint16_t vert_Idx = 0; vert_Idx < track->colStructures[structure_Idx].nVerts; ++vert_Idx) {
            int32_t x = ((256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].x));
            int32_t y = ((256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].y));
            int32_t z = ((256 * track->colStructures[structure_Idx].vertexTable[vert_Idx].z));
            verts.emplace_back(rotationMatrix * glm::vec3(x / scaleFactor, z / scaleFactor, y / scaleFactor));
            shading_data.emplace_back(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
        }

        for (uint32_t poly_Idx = 0; poly_Idx < track->colStructures[structure_Idx].nPoly; ++poly_Idx) {
            // Remap the COL TextureID's using the COL texture block (XBID2)
            TEXTURE_BLOCK texture_for_block = track->polyToQFStexTable[track->colStructures[structure_Idx].polygonTable[poly_Idx].texture];
            Texture gl_texture = track->textures[texture_for_block.texNumber];
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
        glm::vec3 position = rotationMatrix * glm::vec3(structureReferenceCoordinates->x / scaleFactor, structureReferenceCoordinates->y / scaleFactor, structureReferenceCoordinates->z / scaleFactor);
        col_entities.emplace_back(Entity(0, structure_Idx, NFS_2, GLOBAL, Track(verts, uvs, texture_indices, indices, shading_data, position)));
        //free(structureReferenceCoordinates);
    }
    return col_entities;
}

template<typename Platform>
Texture
NFS2<Platform>::LoadTexture(TEXTURE_BLOCK track_texture, const std::string &track_name, NFSVer nfs_version) {
    std::stringstream filename;
    uint8_t alphaColour = 0;
    filename << TRACK_PATH << ToString(nfs_version) << "/";

    switch (nfs_version) {
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
    filename << track_name << "/textures/" << setfill('0') << setw(4) << track_texture.texNumber << ".BMP";;

    GLubyte *data;
    GLsizei width;
    GLsizei height;

    ASSERT(Utils::LoadBmpCustomAlpha(filename.str().c_str(), &data, &width, &height, alphaColour), "Texture " << filename.str() << " did not load succesfully!");

    return Texture((unsigned int) track_texture.texNumber, data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}


template class NFS2<PS1>;

template class NFS2<PC>;