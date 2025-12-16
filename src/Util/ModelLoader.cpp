#include "ModelLoader.h"
#include <iostream>
#include "Logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace OpenNFS {
    bool ModelLoader::LoadObj(const std::string& path, std::vector<ModelData>& models, float scale) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        std::string warn;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr, true, true);

        if (!warn.empty()) {
            LOG(WARNING) << warn;
        }

        if (!err.empty()) {
            LOG(WARNING) << err;
        }

        if (!ret) {
            LOG(WARNING) << "Failed to load/parse .obj file: " << path;
            return false;
        }

        for (const auto& shape : shapes) {
            ModelData modelData;
            size_t index_offset = 0;

            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];

                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                    modelData.m_indices.push_back(idx.vertex_index);

                    modelData.m_vertices.emplace_back(
                        attrib.vertices[3 * idx.vertex_index + 0] * scale,
                        attrib.vertices[3 * idx.vertex_index + 1] * scale,
                        attrib.vertices[3 * idx.vertex_index + 2] * scale
                    );

                    // Backfill normals if missing
                    if (idx.normal_index >= 0) {
                        modelData.m_normals.emplace_back(
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]
                        );
                    } else {
                        modelData.m_normals.emplace_back(0.0f, 0.0f, 0.0f);
                    }

                    // Backfill UVs if missing
                    if (idx.texcoord_index >= 0) {
                        modelData.m_uvs.emplace_back(
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]
                        );
                    } else {
                        modelData.m_uvs.emplace_back(0.0f, 0.0f);
                    }
                }
                index_offset += fv;
            }
            models.push_back(modelData);
        }

        return true;
    }
}
