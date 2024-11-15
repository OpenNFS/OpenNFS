#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <utility>
#include <vector>
#include <cstdlib>
#include <string>
#include <Entities/Geometry.h>
#include <GL/glew.h>

class GLModel {
public:
    GLModel(LibOpenNFS::Geometry *geometry);
    GLModel() = default;

    void enable();

    virtual bool genBuffers() = 0;
    virtual void update()     = 0;
    virtual void destroy()    = 0;
    virtual void render()     = 0;

    /*--------- Model State --------*/
    bool enabled = false;
    // Rendering
    LibOpenNFS::Geometry *geometry;

protected:
    GLuint VertexArrayID;
};