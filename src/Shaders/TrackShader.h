//
// Created by Amrik on 01/03/2018.
//

#ifndef OPENNFS3_TRACKSHADER_H
#define OPENNFS3_TRACKSHADER_H


#include "BaseShader.h"
#include "../Scene/Model.h"
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <map>

class TrackShader : public BaseShader {
public:
    TrackShader();
    void bindTrackTextures(Model track_block, map<short, GLuint> gl_id_map);
    void loadMVPMatrix(glm::mat4 matrix);
    void loadSkyColor(glm::vec3 color);

    typedef BaseShader super;
protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint MatrixID;
    GLint ColorID;
    GLint TrackTexturesID;
};


#endif //OPENNFS3_TRACKSHADER_H
