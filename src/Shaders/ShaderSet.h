#pragma once

#include <GL/glew.h>

#include <vector>
#include <utility>
#include <map>
#include <set>
#include <string>

#include "../Util/Logger.h"

class ShaderSet {
    // typedefs for readability
    using ShaderHandle  = GLuint;
    using ProgramHandle = GLuint;

    // filename and shader type
    struct ShaderNameTypePair {
        std::string Name;
        GLenum Type;
        bool operator<(const ShaderNameTypePair& rhs) const {
            return std::tie(Name, Type) < std::tie(rhs.Name, rhs.Type);
        }
    };

    // Shader in the ShaderSet system
    struct Shader {
        ShaderHandle Handle;
        // Timestamp of the last update of the shader
        uint64_t Timestamp;
        // Hash of the name of the shader. This is used to recover the shader name from the GLSL compiler error messages.
        // It's not a perfect solution, but it's a miracle when it doesn't work.
        int32_t HashName;
    };

    // Program in the ShaderSet system.
    struct Program {
        // The handle exposed externally ("public") and the most recent (succeeding/failed) linked program ("internal")
        // the public handle becomes 0 when a linking failure happens, until the linking error gets fixed.
        ProgramHandle PublicHandle;
        ProgramHandle InternalHandle;
    };

    // the version in the version string that gets prepended to each shader
    std::string mVersion;
    // the preamble which gets prepended to each shader (for eg. shared binding conventions)
    std::string mPreamble;
    // maps shader name/types to handles, in order to reuse shared shaders.
    std::map<ShaderNameTypePair, Shader> mShaders;
    // allows looking up the program that represents a linked set of shaders
    std::map<std::vector<const ShaderNameTypePair*>, Program> mPrograms;

public:
    ShaderSet() = default;

    // Destructor releases all owned shaders
    ~ShaderSet();

    // The version string to prepend to all shaders
    // Separated from the preamble because #version doesn't compile in C++
    void SetVersion(const std::string& version);

    // A string that gets prepended to every shader that gets compiled
    // Useful for compile-time constant #defines (like attrib locations)
    void SetPreamble(const std::string& preamble);

    // Convenience for reading the preamble from a file
    // The preamble is NOT auto-reloaded.
    void SetPreambleFile(const std::string& preambleFilename);

    // list of (file name, shader type) pairs
    // eg: AddProgram({ {"foo.vert", GL_VERTEX_SHADER}, {"bar.frag", GL_FRAGMENT_SHADER} });
    // To be const-correct, this should maybe return "const GLuint*". I'm trusting you not to write to that pointer.
    GLuint* AddProgram(const std::vector<std::pair<std::string, GLenum>>& typedShaders);

    // Polls the timestamps of all the shaders and recompiles/relinks them if they changed
    void UpdatePrograms();

    // Convenience to add shaders based on extension file naming conventions
    // vertex shader: .vert
    // fragment shader: .frag
    // geometry shader: .geom
    // tessellation control shader: .tesc
    // tessellation evaluation shader: .tese
    // compute shader: .comp
    // eg: AddProgramFromExts({"foo.vert", "bar.frag"});
    // To be const-correct, this should maybe return "const GLuint*". I'm trusting you not to write to that pointer.
    GLuint* AddProgramFromExts(const std::vector<std::string>& shaders);

    // Convenience to add a single file that contains many shader stages.
    // Similar to what is explained here: https://software.intel.com/en-us/blogs/2012/03/26/using-ifdef-in-opengl-es-20-shaders
    // eg: AddProgramFromCombinedFile("shader.glsl", { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER });
    //
    // The shader will be compiled many times with a different #define based on which shader stage it's being used for, similarly to the
    // Intel article above. The defines are as follows: vertex shader: VERTEX_SHADER fragment shader: FRAGMENT_SHADER geometry shader:
    // GEOMETRY_SHADER tessellation control shader: TESS_CONTROL_SHADER tessellation evaluation shader: TESS_EVALUATION_SHADER compute
    // shader: COMPUTE_SHADER
    //
    // Note: These defines are not unique to the AddProgramFromCombinedFile API. The defines are also set with any other AddProgram*() API.
    // Note: You may use the defines from inside the preamble. (ie. the preamble is inserted after those defines.)
    //
    // Example combined file shader:
    //     #ifdef VERTEX_SHADER
    //     void main() { /* your vertex shader main */ }
    //     #endif
    //
    //     #ifdef FRAGMENT_SHADER
    //     void main() { /* your fragment shader main */ }
    //     #endif
    GLuint* AddProgramFromCombinedFile(const std::string& filename, const std::vector<GLenum>& shaderTypes);
};