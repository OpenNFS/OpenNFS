//
// Created by Amrik on 01/03/2018.
//

#ifndef OPENNFS3_TRACKSHADER_H
#define OPENNFS3_TRACKSHADER_H


#include "BaseShader.h"
#include "../Scene/Track.h"
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <map>

class TrackShader : public BaseShader {
public:
    TrackShader();
    void bindTrackTextures(Track track_block, map<short, GLuint> gl_id_map);
    void loadMVPMatrix(glm::mat4 matrix);

protected:
    void bindAttributes() override;
    void getAllUniformLocations() override;
    void customCleanup() override;
    GLint MatrixID;
    GLint ColorID;
    GLint TrackTexturesID;

    typedef BaseShader super;
};


#endif //OPENNFS3_TRACKSHADER_H
