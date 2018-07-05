//
// Created by Amrik on 03/07/2018.
//

#include "nfs4_loader.h"


NFS4_Loader::NFS4_Loader(const std::string &car_base_path, std::string *car_name) {
    boost::filesystem::path p(car_base_path);
    *car_name = p.filename().string();

    std::stringstream viv_path, car_out_path, fce_path;
    viv_path << car_base_path << "/car.viv";
    car_out_path << CAR_PATH << *car_name << "/";
    fce_path << CAR_PATH << *car_name << "/car.fce";

    ASSERT(ExtractVIV(viv_path.str(), car_out_path.str()), "Unable to extract " << viv_path.str() << " to " << car_out_path.str());
    ASSERT(LoadFCE(fce_path.str()), "Unable to load " << fce_path.str());
}

bool NFS4_Loader::LoadFCE(const std::string fce_path) {
    std::cout << "- Parsing FCE File: "  << fce_path <<  std::endl;
    ifstream fce(fce_path, ios::in | ios::binary);

    auto *fceHeader = new FCE::HEADER();
    fce.read((char*) fceHeader, sizeof(FCE::HEADER));

    for(int part_Idx = 0; part_Idx < fceHeader->nParts; ++part_Idx){
        float specularDamper = 0.2;
        float specularReflectivity = 0.02;
        float envReflectivity = 0.4;

        std::vector<uint32_t> indices;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::string part_name(fceHeader->partNames[part_Idx]);
        glm::vec3 center(fceHeader->partCoords[part_Idx].x, fceHeader->partCoords[part_Idx].y, fceHeader->partCoords[part_Idx].z);
        center /= 10;

        auto *partVertices = new FCE::VECTOR[fceHeader->partNumVertices[part_Idx]];
        auto *partNormals = new FCE::VECTOR[fceHeader->partNumVertices[part_Idx]];
        auto *partTriangles = new FCE::TRIANGLE[fceHeader->partNumTriangles[part_Idx]];

        fce.seekg(sizeof(FCE::HEADER)+fceHeader->vertTblOffset+(fceHeader->partFirstVertIndices[part_Idx]*sizeof(FCE::VECTOR)), ios_base::beg);
        fce.read((char*) partVertices, fceHeader->partNumVertices[part_Idx]*sizeof(FCE::VECTOR));
        for(int vert_Idx = 0; vert_Idx < fceHeader->partNumVertices[part_Idx]; ++vert_Idx){
            vertices.emplace_back(glm::vec3(partVertices[vert_Idx].x, partVertices[vert_Idx].y, partVertices[vert_Idx].z));
        }

        fce.seekg(sizeof(FCE::HEADER)+fceHeader->normTblOffset+(fceHeader->partFirstVertIndices[part_Idx]*sizeof(FCE::VECTOR)), ios_base::beg);
        fce.read((char*) partNormals, fceHeader->partNumVertices[part_Idx]*sizeof(FCE::VECTOR));
        for(int normal_Idx = 0; normal_Idx < fceHeader->partNumVertices[part_Idx]; ++normal_Idx){
            normals.emplace_back(glm::vec3(partNormals[normal_Idx].x, partNormals[normal_Idx].y, partNormals[normal_Idx].z));
        }

        fce.seekg(sizeof(FCE::HEADER)+fceHeader->triTblOffset+(fceHeader->partFirstTriIndices[part_Idx]*sizeof(FCE::TRIANGLE)), ios_base::beg);
        fce.read((char*) partTriangles, fceHeader->partNumTriangles[part_Idx]*sizeof(FCE::TRIANGLE));
        for(int tri_Idx = 0; tri_Idx < fceHeader->partNumTriangles[part_Idx]; ++tri_Idx){
            indices.emplace_back(partTriangles[tri_Idx].vertex[0]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[1]);
            indices.emplace_back(partTriangles[tri_Idx].vertex[2]);
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[0], partTriangles[tri_Idx].uvTable[3]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[1], partTriangles[tri_Idx].uvTable[4]));
            uvs.emplace_back(glm::vec2(partTriangles[tri_Idx].uvTable[2], partTriangles[tri_Idx].uvTable[5]));
        }

        meshes.emplace_back(CarModel(part_name, part_Idx, vertices, uvs, normals, indices, center, specularDamper, specularReflectivity, envReflectivity));
        std::cout << "Mesh: " << meshes[part_Idx].m_name << " UVs: " << meshes[part_Idx].m_uvs.size() << " Verts: "
                  << meshes[part_Idx].m_vertices.size() << " Indices: " << meshes[part_Idx].m_vertex_indices.size() << " Normals: "
                  << meshes[part_Idx].m_normals.size() << std::endl;
        meshes[part_Idx].enable();

        delete[] partNormals;
        delete[] partVertices;
        delete[] partTriangles;
    }

    fce.close();

    delete fceHeader;
    return true;
}