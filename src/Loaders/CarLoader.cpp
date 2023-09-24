#include "CarLoader.h"

#include <tiny_obj_loader.h>
#include <NFS2/NFS2Loader.h>
#include <NFS3/NFS3Loader.h>
/*#include "NFS4/PC/NFS4Loader.h"
#include "NFS4/PS1/NFS4PS1Loader.h"
#include "NFS5/NFS5Loader.h"*/

std::shared_ptr<Car> CarLoader::LoadCar(NFSVersion nfsVersion, const std::string &carName) {
    std::stringstream carPath;
    carPath << RESOURCE_PATH << get_string(nfsVersion);

    switch (nfsVersion) {
    case NFSVersion::NFS_2:
        carPath << NFS_2_CAR_PATH << carName;
        return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carPath.str(), nfsVersion);
    case NFSVersion::NFS_2_SE:
        carPath << NFS_2_SE_CAR_PATH << carName;
        return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carPath.str(), nfsVersion);
    case NFSVersion::NFS_2_PS1:
        carPath << "/" << carName;
        return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PC>::LoadCar(carPath.str(), nfsVersion);
    case NFSVersion::NFS_3:
        carPath << NFS_3_CAR_PATH << carName;
        return LibOpenNFS::NFS3::Loader::LoadCar(carPath.str());
    case NFSVersion::NFS_3_PS1:
        carPath << "/" << carName;
        return LibOpenNFS::NFS2::Loader<LibOpenNFS::NFS2::PS1>::LoadCar(carPath.str(), nfsVersion);
    /*case NFS_4:
        carPath << NFS_4_CAR_PATH << carName;
        return NFS4::LoadCar(carPath.str(), nfsVersion);
        break;
    case NFS_4_PS1:
        carPath << "/" << carName << ".VIV";
        return NFS4PS1::LoadCar(carPath.str());
        break;
    case MCO:
        carPath << MCO_CAR_PATH << carName;
        return NFS4::LoadCar(carPath.str(), nfsVersion);
        break;
    case NFS_5:
        carPath << NFS_5_CAR_PATH << carName;
        return NFS5::LoadCar(carPath.str());
        break;*/
    default:
        ASSERT(false, "Unknown car type!");
    }
}

std::vector<CarModel> CarLoader::LoadOBJ(std::string obj_path) {
    std::vector<CarModel> meshes;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string warn;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_path.c_str(), nullptr, true, true)) {
        LOG(WARNING) << err;
        return meshes;
    }
    // Loop over shapes
    for (auto & shape : shapes) {
        std::vector<glm::vec3> verts      = std::vector<glm::vec3>();
        std::vector<glm::vec3> norms      = std::vector<glm::vec3>();
        std::vector<glm::vec2> uvs        = std::vector<glm::vec2>();
        std::vector<unsigned int> indices = std::vector<unsigned int>();
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                indices.emplace_back((const unsigned int &) idx.vertex_index);

                verts.emplace_back(
                  glm::vec3(attrib.vertices[3 * idx.vertex_index + 0] * 0.1, attrib.vertices[3 * idx.vertex_index + 1] * 0.1, attrib.vertices[3 * idx.vertex_index + 2] * 0.1));
                norms.emplace_back(glm::vec3(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2]));
                uvs.emplace_back(glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0], 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]));
            }
            index_offset += fv;
            // per-face material
            shape.mesh.material_ids[f];
        }
        CarModel obj_mesh = CarModel(shape.name + "_obj", verts, uvs, norms, indices, glm::vec3(0, 0, 0), 0.01f, 0.0f, 0.5f);
        meshes.emplace_back(obj_mesh);
    }
    return meshes;
}