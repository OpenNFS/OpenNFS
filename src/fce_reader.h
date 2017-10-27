//
//  fce_reader.h
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/10/2017.
//

#pragma once

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <glm/vec2.hpp>
#include <vector>
#include <cmath>
#include <glm/vec3.hpp>
#include "NFS3_Mesh.h"

#define DATA_START 7940

void convertFCE(const char *fce_path, const char* out_path);

class FCE_Reader {
public:
    explicit FCE_Reader(const char *path);
    void writeObj(std::string path);
    std::vector<NFS3_Mesh> getMeshes();
private:
    void read(const char *fce_path); //Add exception handling, true false return?
    unsigned int endian_swap(unsigned int x);
    unsigned int readInt32LE( FILE *file );
    std::vector<glm::vec3> getVertices(int offset, unsigned int length);
    std::vector<glm::vec2> getTextCoords(int offset, unsigned int length);
    std::vector<glm::vec3> getNormals(int offset, unsigned int length);
    std::vector<glm::vec3> getIndices(int offset, unsigned int length, int prevNumFaces);
    /* Globals*/
    FILE *fce_file;
    std::vector<NFS3_Mesh> meshes;
};
