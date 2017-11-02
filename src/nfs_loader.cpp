//
//  nfs_loader.cpp
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/10/2017.
//

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
    int data;

    data = fgetc(file);
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
            filename[pos] = c;
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
            //Bug here, extra characters advancing the file pointer by too much
            //In cases where there is a 0A 0A, fputc will write 0D 0A
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

std::vector<glm::vec3> NFS_Loader::getVertices(int offset, unsigned int length) {
    int vertIdx = 0;

    std::vector<glm::vec3> vertices;
    vertices.reserve(length);

    /*X,Y,Z raw coords stored into float buffer by fread*/
    float buffer[3];

    /*Buffer to store global coordinates. All X,Y,Z coords currently local. To Global, + global coordinates to local*/
    float globalBuffer[3];

    /*HACK: Needed so that can Identify new part. If new part, get new Global coordinates*/
    static int globalIndex;
    static int previousOffset;

    if (previousOffset != offset) { globalIndex += tVectorSize; }
    fseek(fce_file, 252 + globalIndex, SEEK_SET);//retrieve global coords
    fread(globalBuffer, 4, 3, fce_file);
    previousOffset = offset;

    fseek(fce_file, offset, SEEK_SET);

    /*Read Verts in*/
    for (vertIdx = 0; vertIdx <= length; vertIdx++) {
        fread(buffer, 4, 3, fce_file);
        glm::vec3 temp_vertex = glm::vec3(0.0f, 0.0f, 0.0f);

        /* Read X, Y, Z into vertices array*/
        for (int vertAxesIdx = 0; vertAxesIdx < 3; vertAxesIdx++) {
            temp_vertex[vertAxesIdx] = buffer[vertAxesIdx] + globalBuffer[vertAxesIdx];
        }
        vertices.push_back(temp_vertex);
    }

    return vertices;
}

std::vector<glm::vec2> NFS_Loader::getTexCoords(int offset, unsigned int numTriangles) {
    float texBuffer[6];
    std::vector<glm::vec2> uvs;
    uvs.reserve(numTriangles);

    fseek(fce_file, offset + 0x20, SEEK_SET);

    /*Read Triangles in*/
    for (unsigned int triIdx = 0; triIdx < numTriangles; triIdx++) {
        fread(texBuffer, 4, 6, fce_file);
        /* Read V1 UV, V2 UV, V3 UV */
        for (int uvIdx = 0; uvIdx < 3; uvIdx++) {
            glm::vec2 temp_uv = glm::vec2(texBuffer[uvIdx], texBuffer[uvIdx + 3]);
            uvs.push_back(temp_uv);
        }
        fseek(fce_file, 0x20, SEEK_CUR);
        //fseek(fce_file, static_cast<long>(offset + 0x20 + tTriangleSize * triIdx), SEEK_SET);
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
    for (int normIdx = 0; normIdx <= length; normIdx++) {
        fread(normBuffer, 4, 3, fce_file);
        normalLength = static_cast<float>(sqrt(pow(normBuffer[0], 2) + pow(normBuffer[1], 2) + pow(normBuffer[2], 2)));
        //TODO: Drop the hungarian typing?
        glm::vec3 temp_normals = glm::vec3(normBuffer[0], normBuffer[1], normBuffer[2]);
        temp_normals /= normalLength;
        normals.push_back(temp_normals);
    }

    return normals;
}

std::vector<unsigned int> NFS_Loader::getIndices(int offset, unsigned int length, int prevNumFaces) {
    int indexBuffer[3];
    std::vector<unsigned int> indices;
    indices.reserve(length);

    fseek(fce_file, offset, SEEK_SET);
    /*Read Triangles in*/
    for (int triIdx = 0; triIdx <= length; triIdx++) {
        for (int indexIdx = 0; indexIdx < 3; indexIdx++) {
            unsigned int index = readInt32(fce_file, true) + prevNumFaces;
            if (triIdx != 0) {
                indices.push_back(index);
            }
        }
        fseek(fce_file, offset + tTriangleSize * triIdx, SEEK_SET);
    }

    return indices;
}

void NFS_Loader::readFCE(const char *fce_path) {
    fce_file = fopen(fce_path, "rb");

    if (!fce_file) {
        printf("Error while opening %s\n", fce_path);
        exit(2);
    }
    /*Retrieve Vertices, normal, triangle offsets from DATA_START*/
    fseek(fce_file, VertTblOffset, SEEK_SET);
    int vertOffset = DATA_START + readInt32(fce_file, true);

    fseek(fce_file, NormTblOffset, SEEK_SET);
    int normOffset = DATA_START + readInt32(fce_file, true);

    fseek(fce_file, TriaTblOffset, SEEK_SET);
    int triOffset = DATA_START + readInt32(fce_file, true) + 4;

    /*Retrieve number of colours*/
    fseek(fce_file, NumPriColoursOffset, SEEK_SET);
    int numPriColours = readInt32(fce_file, true);

    fseek(fce_file, NumSecColoursOffset, SEEK_SET);
    int numSecColours = readInt32(fce_file, true);

    /*Retrieve number of meshes*/
    fseek(fce_file, NumPartsOffset, SEEK_SET);
    unsigned int numParts = readInt32(fce_file, true);

    /*Get part names, and use to instantiate NFS3 Meshes */
    fseek(fce_file, PartNamesOffset, SEEK_SET);
    meshes.reserve(numParts);

    for (int j = 0; j < numParts; j++) {
        std::string partName;
        int c = 0;
        for (int k = 0; k < 64; k++) {
            if (c == '.') { break; }
            c = fgetc(fce_file);
            if (c != '\0') partName += c;
        }
        std::cout << "Parsing Part: " << partName << std::endl;
        meshes.emplace_back(NFS3_Mesh(partName));
    }

    /*Retrieve part by part data, Vert/Tri*/
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

    /*Write all part data to output obj file*/
    unsigned int cumulativeVerts = 0;
    int i = 0;

    for (NFS3_Mesh myPart : meshes) {
        if (i != 0) { cumulativeVerts += partVertNumbers[i - 1] + 1; }
        meshes[i].setVertices(getVertices(vertOffset + partVertOffsets[i], partVertNumbers[i]));
        meshes[i].setUVs(getTexCoords(triOffset + partTriOffsets[i], partTriNumbers[i]));
        meshes[i].setNormals(getNormals(normOffset + partVertOffsets[i], partVertNumbers[i]));
        meshes[i].setIndices(getIndices(triOffset + partTriOffsets[i], partTriNumbers[i], cumulativeVerts));
        i++;
    }

    fclose(fce_file);
}

void NFS_Loader::writeObj(std::string path) {
    std::cout << "Writing Meshes to " << path << std::endl;
    std::ofstream obj_dump;
    obj_dump.open("Model.obj");

    for (NFS3_Mesh mesh : meshes) {
        /* Print Part name*/
        obj_dump << "o " << mesh.getName() << std::endl;
        //Dump Vertices
        for (auto vertex : mesh.getVertices()) {
            obj_dump << "v " << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
        }
        //Dump UVs
        for (auto uv : mesh.getUVs()) {
            obj_dump << "vt " << uv[0] << " " << uv[1] << std::endl;
        }
        //Dump Indices
        for (auto vert_index : mesh.getIndices()) {
            //    obj_dump << "f " << vert_index[0] << " " << vert_index[1] << " " << vert_index[2] << std::endl;
        }
    }
    obj_dump.close();
}

std::vector<NFS3_Mesh> NFS_Loader::getMeshes() {
    return meshes;
}



