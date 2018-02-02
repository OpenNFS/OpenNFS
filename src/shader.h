#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <GL/glew.h>

using namespace std;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

