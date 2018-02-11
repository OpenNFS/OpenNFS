//
//  nfs_loader.h
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/10/2017.
//

#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <glm/vec2.hpp>
#include <vector>
#include <cmath>
#include <glm/vec3.hpp>
#include "Model.h"
#include <tinyobjloader/tiny_obj_loader.h>

#define DATA_START 0x1F04
#define VertTblOffset 0x0010
#define NormTblOffset 0x0014
#define TriaTblOffset 0x0018
#define NumPriColoursOffset 0x7FC
#define NumSecColoursOffset 0x0900
#define NumPartsOffset 0x00F8
#define PartNamesOffset 0x0E04
#define PNumVerticesOffset 0x04FC
#define P1stVerticesOffset 0x03FC
#define PNumTrianglesOffset 0x06FC
#define P1stTriangleOffset 0x05FC
#define tVectorSize 0x0C
#define tTriangleSize 0x38

void convertFCE(const char *fce_path, const char* out_path);
void extractViv(const char *viv_path);
unsigned int endian_swap(unsigned int x);

class NFS_Loader {
public:
    explicit NFS_Loader(const char *viv_path);
    void writeObj(std::string path);
    bool loadObj(std::string obj_path);
    std::vector<Model> getMeshes();
private:
    void readFCE(const char *fce_path); //Add exception handling, true false return?

    std::vector<glm::vec3> getVertices(int partNumber, int offset, unsigned int length);
    std::vector<glm::vec2> getTexCoords(int offset, unsigned int numTriangles);
    std::vector<glm::vec3> getNormals(int offset, unsigned int length);
    std::vector<unsigned int> getIndices(int offset, unsigned int length);
    /* Globals*/
    FILE *fce_file;
    std::vector<Model> meshes;
};