//
// Created by Amrik on 03/07/2018.
//

#include "nfs4_loader.h"

using namespace Utils;
using namespace NFS4;

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



    /*for (int i = 0; i < model_names.size(); ++i) {
        std::vector<glm::vec3> vertices;
        glm::vec3 center;
        center /= 10;
        float specularDamper = 0;
        float specularReflectivity = 0;
        float envReflectivity = 0;
        // Body specific shading
        if(i == 0){
            specularDamper = 0.2;
            specularReflectivity = 0.02;
            envReflectivity = 0.4;
        }
        meshes.emplace_back(CarModel(model_names[i], i, vertices, getTexCoords(triOffset + partTriOffsets[i], partTriNumbers[i]), getNormals(normOffset + partVertOffsets[i], partVertNumbers[i]), getIndices(triOffset + partTriOffsets[i], partTriNumbers[i]), center, specularDamper, specularReflectivity, envReflectivity));
        std::cout << "Mesh: " << meshes[i].m_name << " UVs: " << meshes[i].m_uvs.size() << " Verts: "
                  << meshes[i].m_vertices.size() << " Indices: " << meshes[i].m_vertex_indices.size() << " Normals: "
                  << meshes[i].m_normals.size() << std::endl;
    }*/


    fce.close();

    delete fceHeader;
    return true;
}