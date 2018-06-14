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
#include <vector>
#include <cmath>
#include <sstream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "../Scene/CarModel.h"
#include "../Util/Utils.h"
#include "../Config.h"

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

void ConvertFCE(const std::string &fce_path, const std::string &obj_out_path);

class NFS_Loader {
public:
    explicit NFS_Loader(const std::string &car_base_path, std::string *car_name);
    void writeObj(const std::string &path);
    bool loadObj(std::string obj_path);
    std::vector<CarModel> getMeshes();
private:
    bool readFCE(const std::string fce_path);
    glm::vec3 getVertices(int partNumber, int offset, unsigned int length, std::vector<glm::vec3> &vertices);
    std::vector<glm::vec2> getTexCoords(int offset, unsigned int numTriangles);
    std::vector<glm::vec3> getNormals(int offset, unsigned int length);
    std::vector<unsigned int> getIndices(int offset, unsigned int length);
    /* Globals*/
    FILE *fce_file;
    std::vector<CarModel> meshes;
};