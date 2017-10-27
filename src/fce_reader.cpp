//
//  fce_reader.cpp
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/10/2017.
//

#include "fce_reader.h"

void convertFCE(const char *fce_path, const char* out_path){
    FCE_Reader fce_reader(fce_path);
    fce_reader.writeObj(out_path);
}

FCE_Reader::FCE_Reader(const char *path) {
    std::cout << "Loading FCE File" << std::endl;
    read(path);
}

unsigned int FCE_Reader::endian_swap(unsigned int x)
{
    int swapped;
    
    swapped = (x>>24) |
    ((x<<8) & 0x00FF0000) |
    ((x>>8) & 0x0000FF00) |
    (x<<24);
    
    return static_cast<unsigned int>(swapped);
}

unsigned int FCE_Reader::readInt32LE( FILE *file )
{
    int data;
    
    data = fgetc( file );
    data = data << 8 | fgetc( file );
    data = data << 8 | fgetc( file );
    data = data << 8 | fgetc( file );
    
    return endian_swap(static_cast<unsigned int>(data));
}

std::vector<glm::vec3> FCE_Reader::getVertices(int offset, unsigned int length){
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

    if (previousOffset != offset){globalIndex+=12;}
    fseek(fce_file, 252+globalIndex, SEEK_SET);//retrieve global coords
    fread(globalBuffer, 4,3,fce_file);
    previousOffset = offset;
    
    fseek(fce_file, offset, SEEK_SET);

    /*Read Verts in*/
    for(vertIdx = 0; vertIdx <= length; vertIdx++){
        fread(buffer, 4, 3,fce_file);
        glm::vec3 temp_vertex = glm::vec3(0.0f, 0.0f, 0.0f);

        /* Read X, Y, Z into vertices array*/
        for(int vertAxesIdx = 0; vertAxesIdx < 3; vertAxesIdx++){
            temp_vertex[vertAxesIdx] = buffer[vertAxesIdx]+globalBuffer[vertAxesIdx];
        }
        vertices.push_back(temp_vertex);
    }

    return vertices;
}

std::vector<glm::vec2> FCE_Reader::getTextCoords(int offset, unsigned int length){
    float texBuffer[6];
    std::vector<glm::vec2> uvs;
    uvs.reserve(length);

    fseek(fce_file, offset, SEEK_SET);

    /*Read Triangles in*/
    for(unsigned int vertIdx = 0; vertIdx <= length; vertIdx++){
        fread(texBuffer, 4, 6, fce_file);
        /* Read V1 UV, V2 UV, V3 UV */
        for(int uvIdx = 0; uvIdx < 3; uvIdx++){
            glm::vec2 temp_uv = glm::vec2(texBuffer[uvIdx], texBuffer[uvIdx + 3]);
            uvs.push_back(temp_uv);
        }
        fseek(fce_file, static_cast<long>(offset + 56 * vertIdx), SEEK_SET);
    }

    return uvs;
}

std::vector<glm::vec3> FCE_Reader::getNormals(int offset, unsigned int length){
   float normalLength = 0;
    float normBuffer[3];
    std::vector<glm::vec3> normals;
    normals.reserve(length);

    fseek(fce_file, offset, SEEK_SET);

    /*Read Normals in, and normalize dem normals!*/
    for(int normIdx = 0; normIdx <= length; normIdx++){
        fread(normBuffer, 4,3,fce_file);
        normalLength = static_cast<float>(sqrt(pow(normBuffer[0], 2) + pow(normBuffer[1], 2) + pow(normBuffer[2], 2)));
        //TODO: Drop the hungarian typing?
        glm::vec3 temp_normals = glm::vec3(normBuffer[0], normBuffer[1], normBuffer[2]);
        temp_normals /= normalLength;
        normals.push_back(temp_normals);
    }

    return normals;
}

std::vector<glm::vec3> FCE_Reader::getIndices(int offset, unsigned int length, int prevNumFaces){
    int indexBuffer[3];
    std::vector<glm::vec3> indices;
    indices.reserve(length);

    fseek(fce_file, offset, SEEK_SET);
    /*Read Triangles in*/
    for(int triIdx = 0; triIdx <= length; triIdx++){
        indexBuffer[0] = readInt32LE(fce_file)+1;
        indexBuffer[1] = readInt32LE(fce_file)+1;
        indexBuffer[2] = readInt32LE(fce_file)+1;

        glm::vec3 temp_indices = glm::vec3(indexBuffer[0],indexBuffer[1],indexBuffer[2]);

        temp_indices += prevNumFaces;

        if (triIdx != 0) indices.push_back(temp_indices);
        fseek(fce_file, offset + 56*triIdx, SEEK_SET);
    }

    return indices;
}

void FCE_Reader::read(const char *fce_path){
        //TODO: Map U/V coords.
        fce_file = fopen(fce_path, "r");

        if (!fce_file) {
            printf("Error while opening %s\n", fce_path);
            exit(2);
        }
        /*Retrieve Vertices, normal, triangle offsets from DATA_START*/
        fseek(fce_file, 16, SEEK_SET);
        int vertOffset = DATA_START + readInt32LE(fce_file);

        fseek(fce_file, 20, SEEK_SET);
        int normOffset = DATA_START + readInt32LE(fce_file);

        fseek(fce_file, 24, SEEK_SET);
        int triOffset = DATA_START + readInt32LE(fce_file) + 4;

        /*Retrieve number of colours*/
        fseek(fce_file, 2044, SEEK_SET);
        int numPriColours = readInt32LE(fce_file);

        fseek(fce_file, 2304, SEEK_SET);
        int numSecColours = readInt32LE(fce_file);

        /*Retrieve number of meshes*/
        fseek(fce_file, 248, SEEK_SET);
        unsigned int numParts = readInt32LE(fce_file);

        /*Get part names, store to numParts+1 x 64 array*/
        fseek(fce_file, 3588, SEEK_SET);

        meshes.reserve(numParts);

        for (int j = 0; j < numParts; j++) {
            std::string partName;
            int c = 0;
            for (int k = 0; k < 64; k++) {
                if (c == '.') { break; }
                c = fgetc(fce_file);
                if(c != '\0') partName += c;
            }
            std::cout << "Parsing Part: " << partName << std::endl;
            meshes.emplace_back(NFS3_Mesh(partName));
        }

        /*Retrieve part by part data, Vert/Tri*/
        fseek(fce_file, 1276, SEEK_SET);//Get part vertex amount
        unsigned int partVertNumbers[numParts + 1];
        for (int i = 0; i < numParts; i++) {
            partVertNumbers[i] = readInt32LE(fce_file);
        }
        fseek(fce_file, 1020, SEEK_SET);//Get part vert offsets
        unsigned int partVertOffsets[numParts + 1];
        for (int i = 0; i < numParts; i++) {
            partVertOffsets[i] = readInt32LE(fce_file) * 12;
        }
        fseek(fce_file, 1788, SEEK_SET);//Get part Tri amount
        unsigned int partTriNumbers[numParts + 1];
        for (int i = 0; i < numParts; i++) {
            partTriNumbers[i] = readInt32LE(fce_file);
        }
        fseek(fce_file, 1532, SEEK_SET);//Get part tri offsets
        unsigned int partTriOffsets[numParts + 1];
        for (int i = 0; i < numParts; i++) {
            partTriOffsets[i] = readInt32LE(fce_file) * 56;
        }

        /*Write all part data to output obj file*/
        unsigned int cumulativeVerts = 0;
        int i = 0;

        for(auto myPart : meshes){
            if (i != 0) { cumulativeVerts += partVertNumbers[i - 1] + 1;}
            meshes[i].setVertices(getVertices(vertOffset + partVertOffsets[i], partVertNumbers[i]));
            meshes[i].setUVs(getTextCoords(triOffset + partTriOffsets[i] + 28, partTriNumbers[i]));
            meshes[i].setNormals(getNormals(normOffset + partVertOffsets[i], partVertNumbers[i]));
            meshes[i].setIndices(getIndices(triOffset + partTriOffsets[i], partTriNumbers[i], cumulativeVerts));
            i++;
        }

    fclose(fce_file);
}

void FCE_Reader::writeObj(std::string path){
    std::cout << "Writing Meshes to " << path << std::endl;
    std::ofstream obj_dump;
    obj_dump.open("Model.obj");
    for(NFS3_Mesh mesh : meshes){
        /* Print Part name*/
        obj_dump << "o " << mesh.getName() << std::endl;
        //Dump Vertices
        for(auto vertex : mesh.getVertices()){
            obj_dump << "v " << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
        }
        //Dump UVs
        //for(auto uv : mesh.getUVs()){
        //    obj_dump << "vt " << uv[0] << " " << uv[1] << std::endl;
        //}
        //Dump Indices
        for(auto vert_index : mesh.getIndices()){
            obj_dump << "f " << vert_index[0] << " " << vert_index[1] << " " << vert_index[2] << std::endl;
        }
    }
    obj_dump.close();
}

std::vector<NFS3_Mesh> FCE_Reader::getMeshes() {
    return meshes;
}



