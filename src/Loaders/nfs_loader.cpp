//
//  nfs_loader.cpp
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/10/2017.
//

#include "nfs_loader.h"

using namespace Utils;

void ConvertFCE(const std::string &fce_path, const std::string &obj_out_path) {
    std::string car_name;
    NFS_Loader fce_reader(fce_path, &car_name);
    fce_reader.writeObj(obj_out_path);
}

NFS_Loader::NFS_Loader(const std::string &car_base_path, std::string *car_name) {
    boost::filesystem::path p(car_base_path);
    *car_name = p.filename().string();

    std::stringstream viv_path, car_out_path, fce_path;
    viv_path << car_base_path << "/car.viv";
    car_out_path << CAR_PATH << car_name << "/";
    fce_path << CAR_PATH << car_name << "/car.fce";

    ASSERT(ExtractVIV(viv_path.str(), car_out_path.str()), "Unable to extract " << viv_path.str() << " to " << car_out_path.str());
    ASSERT(readFCE(fce_path.str()), "Unable to load " << fce_path.str());
}

glm::vec3 NFS_Loader::getVertices(int partNumber, int offset, unsigned int length, std::vector<glm::vec3> &vertices) {
    int vertIdx = 0;
    vertices.reserve(length);

    /*X,Y,Z raw coords stored into float buffer by fread*/
    float buffer[3];

    /*Buffer to store global coordinates. All X,Y,Z coords currently local. To Global, + global coordinates to local*/
    float globalBuffer[3];

    /* If new part, get new Global coordinates*/
    fseek(fce_file, 252 + (partNumber * tVectorSize), SEEK_SET);
    fread(globalBuffer, 4, 3, fce_file);
    glm::vec3 center = glm::vec3(globalBuffer[0], globalBuffer[1], globalBuffer[2]);

    fseek(fce_file, offset, SEEK_SET);

    /*Read Verts in*/
    for (vertIdx = 0; vertIdx < length; vertIdx++) {
        fread(buffer, 4, 3, fce_file);
        glm::vec3 temp_vertex;
        /* Read X, Y, Z into vertices array*/
        for (int vertAxesIdx = 0; vertAxesIdx < 3; vertAxesIdx++) {
            temp_vertex[vertAxesIdx] = (buffer[vertAxesIdx])/10;
        }
        vertices.emplace_back(temp_vertex);
    }

    return center;
}

std::vector<glm::vec2> NFS_Loader::getTexCoords(int offset, unsigned int numTriangles) {
    float texBuffer[7];
    std::vector<glm::vec2> uvs;
    uvs.reserve(numTriangles);

    fseek(fce_file, offset + 0x18, SEEK_SET);

    /*Read Triangles in*/
    for (unsigned int triIdx = 0; triIdx < numTriangles; triIdx++) {
        fread(texBuffer, 0x04, 7, fce_file);
        /* Read V1 UV, V2 UV, V3 UV */
        for (int uvIdx = 1; uvIdx < 4; uvIdx++) {
            glm::vec2 temp_uv = glm::vec2(texBuffer[uvIdx], texBuffer[uvIdx + 3]);
            uvs.emplace_back(temp_uv);
        }
        fseek(fce_file, tTriangleSize - ((7 * 0x04)), SEEK_CUR);
    }

    return uvs;
}

std::vector<glm::vec3> NFS_Loader::getNormals(int offset, unsigned int length) {
    float normalLength = 0;
    float normBuffer[3];
    std::vector<glm::vec3> normals;
    normals.reserve(length);

    fseek(fce_file, offset, SEEK_SET);

    /*Read Normals in, and normalize dem normals!*/
    for (int normIdx = 0; normIdx < length; normIdx++) {
        fread(normBuffer, 4, 3, fce_file);
        glm::vec3 temp_normals = glm::vec3(normBuffer[0], normBuffer[1], normBuffer[2]);
        normals.emplace_back(temp_normals);
    }

    return normals;
}

std::vector<unsigned int> NFS_Loader::getIndices(int offset, unsigned int length) {
    std::vector<unsigned int> indices;
    indices.reserve(length);

    fseek(fce_file, offset, SEEK_SET);
    /*Read Triangles in*/
    for (int triIdx = 0; triIdx < length; triIdx++) {
        for (int indexIdx = 0; indexIdx < 3; indexIdx++) {
            unsigned int index = readInt32(fce_file, true);
            indices.emplace_back(index);
        }
        fseek(fce_file, tTriangleSize - (3 * 0x04), SEEK_CUR);
    }

    return indices;
}

bool NFS_Loader::readFCE(const std::string fce_path) {
    std::cout << "Loading FCE File: "  << fce_path <<  std::endl;
    fce_file = fopen(fce_path.c_str(), "rb");

    if (!fce_file) {
        printf("Error while opening %s\n", fce_path.c_str());
        return false;
    }

    fseek(fce_file, 0x0008, SEEK_SET);
    int expectedVerticesCount = readInt32(fce_file, true);

    //Retrieve Vertices, normal, triangle offsets from DATA_START
    fseek(fce_file, VertTblOffset, SEEK_SET);
    int vertOffset = DATA_START + readInt32(fce_file, true);

    fseek(fce_file, NormTblOffset, SEEK_SET);
    int normOffset = DATA_START + readInt32(fce_file, true);

    fseek(fce_file, TriaTblOffset, SEEK_SET);
    int triOffset = DATA_START + readInt32(fce_file, true) + 0x4;

    //Retrieve number of colours
    fseek(fce_file, NumPriColoursOffset, SEEK_SET);
    int numPriColours = readInt32(fce_file, true);

    fseek(fce_file, NumSecColoursOffset, SEEK_SET);
    int numSecColours = readInt32(fce_file, true);

    //Retrieve number of meshes
    fseek(fce_file, NumPartsOffset, SEEK_SET);
    unsigned int numParts = readInt32(fce_file, true);

    //Get part names, and use to instantiate NFS3 Meshes
    fseek(fce_file, PartNamesOffset, SEEK_SET);
    meshes.reserve(numParts);
    std::vector<std::string> model_names;

    for (int j = 0; j < numParts; j++) {
        std::string partName;
        int c = 0;
        for (int k = 0; k < 64; k++) {
            if (c == '.') { break; }
            c = fgetc(fce_file);
            if (c != '\0') partName += c;
        }
        model_names.emplace_back(partName);
    }

    //Retrieve part by part data, Vert/Tri
    fseek(fce_file, PNumVerticesOffset, SEEK_SET);//Get part vertex amount
    unsigned int *partVertNumbers = new unsigned int[numParts];
    for (int i = 0; i < numParts; i++) {
        partVertNumbers[i] = readInt32(fce_file, true);
    }
    fseek(fce_file, P1stVerticesOffset, SEEK_SET);//Get part vert offsets
    unsigned int *partVertOffsets = new unsigned int[numParts];
    for (int i = 0; i < numParts; i++) {
        partVertOffsets[i] = readInt32(fce_file, true) * tVectorSize;
    }
    fseek(fce_file, PNumTrianglesOffset, SEEK_SET);//Get part Tri amount
    unsigned int *partTriNumbers = new unsigned int[numParts];
    for (int i = 0; i < numParts; i++) {
        partTriNumbers[i] = readInt32(fce_file, true);
    }
    fseek(fce_file, P1stTriangleOffset, SEEK_SET);//Get part tri offsets
    unsigned int *partTriOffsets = new unsigned int[numParts];
    for (int i = 0; i < numParts; i++) {
        partTriOffsets[i] = readInt32(fce_file, true) * tTriangleSize;
    }

    for (int i = 0; i < model_names.size(); ++i) {
        std::vector<glm::vec3> vertices;
        glm::vec3 center = getVertices(i, vertOffset + partVertOffsets[i], partVertNumbers[i], vertices);
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
        meshes.emplace_back(CarModel(model_names[i], i,
                                  vertices,
                                  getTexCoords(triOffset + partTriOffsets[i], partTriNumbers[i]),
                                  getNormals(normOffset + partVertOffsets[i], partVertNumbers[i]),
                                  getIndices(triOffset + partTriOffsets[i], partTriNumbers[i]), center, specularDamper, specularReflectivity, envReflectivity));
        std::cout << "Mesh: " << meshes[i].m_name << " UVs: " << meshes[i].m_uvs.size() << " Verts: "
                  << meshes[i].m_vertices.size() << " Indices: " << meshes[i].m_vertex_indices.size() << " Normals: "
                  << meshes[i].m_normals.size() << std::endl;
    }

	delete partTriNumbers;
	delete partTriOffsets;
	delete partVertNumbers;
	delete partVertOffsets;

    fclose(fce_file);
    return true;
}

void NFS_Loader::writeObj(const std::string &path) {
    std::cout << "Writing Meshes to " << path << std::endl;

    std::ofstream obj_dump;
    obj_dump.open(path);

    for (Model &mesh : meshes) {
        /* Print Part name*/
        obj_dump << "o " << mesh.m_name << std::endl;
        //Dump Vertices
        for (auto vertex : mesh.m_vertices) {
            obj_dump << "v " << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
        }
        //Dump UVs
        for (auto uv : mesh.m_uvs) {
            obj_dump << "vt " << uv[0] << " " << uv[1] << std::endl;
        }
        //Dump Indices
        for (auto vert_index : mesh.m_vertex_indices) {
            obj_dump << "f " << vert_index << std::endl;
        }
    }
    obj_dump.close();
}

std::vector<CarModel> NFS_Loader::getMeshes() {
    return meshes;
}




