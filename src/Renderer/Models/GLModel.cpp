#include "GLModel.h"

GLModel::GLModel(LibOpenNFS::Geometry *geometry) : geometry(geometry) {
}

void GLModel::enable() {
    enabled = true;
}
