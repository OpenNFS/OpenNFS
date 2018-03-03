//
//  nfs_loader.cpp
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/10/2017.
//


#include <tiny_obj_loader.h>
#include "nfs_loader.h"


void convertFCE(const char *fce_path, const char *out_path) {
    NFS_Loader fce_reader(fce_path);
    fce_reader.writeObj(out_path);
}

unsigned int endian_swap(unsigned int x) {
    int swapped;

    swapped = (x >> 24) |
              ((x << 8) & 0x00FF0000) |
              ((x >> 8) & 0x0000FF00) |
              (x << 24);

    return static_cast<unsigned int>(swapped);
}

unsigned int readInt32(FILE *file, bool littleEndian) {
    int data = fgetc(file);
    data = data << 8 | fgetc(file);
    data = data << 8 | fgetc(file);
    data = data << 8 | fgetc(file);

    return littleEndian ? endian_swap(static_cast<unsigned int>(data)) : data;
}

void extractViv(const char *viv_path) {
    FILE *vivfile, *outfile;  // file stream
    int numberOfFiles; // number of files in viv file
    char filename[100];
    int filepos, filesize;
    char buf[4];
    int c;
    int pos;
    long currentpos;
    int a, b;

    vivfile = fopen(viv_path, "rb");

    if (!vivfile) {
        printf("Error while opening %s\n", viv_path);
        exit(2);
    }

    for (a = 0; a < 4; a++) {
        buf[a] = fgetc(vivfile);
    }

    if (memcmp(buf, "BIGF", 4)) {
        printf("%s if not a viv file\n", viv_path);
        fclose(vivfile);
        exit(3);
    }

    readInt32(vivfile, false); // size of viv file

    numberOfFiles = readInt32(vivfile, false);
    printf("Number of files: %d\n", numberOfFiles);

    int startPos = readInt32(vivfile, false); // start position of files

    currentpos = ftell(vivfile);

    for (a = 0; a < numberOfFiles; a++) {
        if (fseek(vivfile, currentpos, SEEK_SET)) {
            printf("Error while seeking in file %s\n", viv_path);
            fclose(vivfile);
            exit(4);
        }

        filepos = readInt32(vivfile, false);
        filesize = readInt32(vivfile, false);

        pos = 0;
        c = fgetc(vivfile);
        while (c != '\0') {
            filename[pos] = (char) c;
            pos++;
            c = fgetc(vivfile);
        }
        filename[pos] = '\0';

        currentpos = ftell(vivfile);

        if ((outfile = fopen(filename, "wb")) == NULL) {
            printf("Error while opening file %s\n", filename);
            fclose(vivfile);
            exit(4);
        }

        if (fseek(vivfile, filepos, SEEK_SET)) {
            printf("Error while seeking in file %s\n", viv_path);
            fclose(vivfile);
            exit(3);
        }

        for (b = 0; b < filesize; b++) {
            c = fgetc(vivfile);
            fputc(c, outfile);
        }

        fclose(outfile);

        printf("File %s written successfully\n", filename);
    }

    fclose(vivfile);
}


NFS_Loader::NFS_Loader(const char *viv_path) {
    std::cout << "Loading FCE File" << std::endl;
    extractViv(viv_path);
    readFCE("car.fce");
}


bool NFS_Loader::loadObj(std::string obj_path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, obj_path.c_str())) {
        std::cout << err << std::endl;
        return false;
    }
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        std::vector<glm::vec3> verts = std::vector<glm::vec3>();
        std::vector<glm::vec3> norms = std::vector<glm::vec3>();
        std::vector<glm::vec2> uvs = std::vector<glm::vec2>();
        std::vector<unsigned int> indices = std::vector<unsigned int>();
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                indices.emplace_back((const unsigned int &) idx.vertex_index);

                verts.emplace_back(glm::vec3(attrib.vertices[3 * idx.vertex_index + 0] * 0.1,
                                             attrib.vertices[3 * idx.vertex_index + 1] * 0.1,
                                             attrib.vertices[3 * idx.vertex_index + 2] * 0.1));
                norms.emplace_back(
                        glm::vec3(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1],
                                  attrib.normals[3 * idx.normal_index + 2]));
                uvs.emplace_back(glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0],
                                           1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]));
            }
            index_offset += fv;
            // per-face material
            shapes[s].mesh.material_ids[f];
        }
        Car obj_mesh = Car(shapes[s].name + "_obj", s, verts, uvs, norms, indices, glm::vec3(0, 0, 0));
        meshes.emplace_back(obj_mesh);
    }
    return true;
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
        glm::vec3 temp_offset_vertex;
        /* Read X, Y, Z into vertices array*/
        for (int vertAxesIdx = 0; vertAxesIdx < 3; vertAxesIdx++) {
            temp_vertex[vertAxesIdx] = (buffer[vertAxesIdx]+ globalBuffer[vertAxesIdx])/10;
            temp_offset_vertex[vertAxesIdx] = globalBuffer[vertAxesIdx]/10;
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
        //normalLength = static_cast<float>(sqrt(pow(normBuffer[0], 2) + pow(normBuffer[1], 2) + pow(normBuffer[2], 2)));
        //TODO: Drop the hungarian typing?
        glm::vec3 temp_normals = glm::vec3(normBuffer[0], normBuffer[1], normBuffer[2]);
        //temp_normals /= normalLength;
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

void NFS_Loader::readFCE(const char *fce_path) {
    fce_file = fopen(fce_path, "rb");

    if (!fce_file) {
        printf("Error while opening %s\n", fce_path);
        exit(2);
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
    unsigned int partVertNumbers[numParts];
    for (int i = 0; i < numParts; i++) {
        partVertNumbers[i] = readInt32(fce_file, true);
    }
    fseek(fce_file, P1stVerticesOffset, SEEK_SET);//Get part vert offsets
    unsigned int partVertOffsets[numParts];
    for (int i = 0; i < numParts; i++) {
        partVertOffsets[i] = readInt32(fce_file, true) * tVectorSize;
    }
    fseek(fce_file, PNumTrianglesOffset, SEEK_SET);//Get part Tri amount
    unsigned int partTriNumbers[numParts];
    for (int i = 0; i < numParts; i++) {
        partTriNumbers[i] = readInt32(fce_file, true);
    }
    fseek(fce_file, P1stTriangleOffset, SEEK_SET);//Get part tri offsets
    unsigned int partTriOffsets[numParts];
    for (int i = 0; i < numParts; i++) {
        partTriOffsets[i] = readInt32(fce_file, true) * tTriangleSize;
    }

    for (int i = 0; i < model_names.size(); ++i) {
        std::vector<glm::vec3> vertices;
        glm::vec3 center = getVertices(i, vertOffset + partVertOffsets[i], partVertNumbers[i], vertices);
        meshes.emplace_back(Car(model_names[i], i,
                                  vertices,
                                  getTexCoords(triOffset + partTriOffsets[i], partTriNumbers[i]),
                                  getNormals(normOffset + partVertOffsets[i], partVertNumbers[i]),
                                  getIndices(triOffset + partTriOffsets[i], partTriNumbers[i]), center));
        std::cout << "Mesh: " << meshes[i].m_name << " UVs: " << meshes[i].m_uvs.size() << " Verts: "
                  << meshes[i].m_vertices.size() << " Indices: " << meshes[i].m_vertex_indices.size() << " Normals: "
                  << meshes[i].m_normals.size() << std::endl;
    }

    fclose(fce_file);
}

void NFS_Loader::writeObj(std::string path) {
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

std::vector<Car> NFS_Loader::getMeshes() {
    return meshes;
}




