#include "ShaderSet.h"

#ifdef _WIN32
#include <sys/stat.h>
#else
// Not Windows? Assume unix-like.
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <fstream>
#include <string>

static uint64_t GetShaderFileTimestamp(char const *filename) {
    uint64_t timestamp = 0;

#ifdef _WIN32
    struct __stat64 stFileInfo;
    if (_stat64(filename, &stFileInfo) == 0) {
        timestamp = stFileInfo.st_mtime;
    }
#else
    struct stat fileStat{};

    if (stat(filename, &fileStat) == -1) {
        perror(filename);
        return 0;
    }

#ifdef __APPLE__
    timestamp = fileStat.st_mtimespec.tv_sec;
#else
    timestamp = fileStat.st_mtime;
#endif
#endif

    return timestamp;
}

static std::string ShaderStringFromFile(char const *filename) {
    std::ifstream fs(filename);
    if (!fs) {
        return "";
    }

    std::string s(std::istreambuf_iterator<char>{fs}, std::istreambuf_iterator<char>{});

    return s;
}

ShaderSet::~ShaderSet() {
    for (std::pair<ShaderNameTypePair const, Shader> &shader : mShaders) {
        glDeleteShader(shader.second.Handle);
    }

    for (std::pair<std::vector<ShaderNameTypePair const *> const, Program> &program : mPrograms) {
        glDeleteProgram(program.second.InternalHandle);
    }
}

void ShaderSet::SetVersion(std::string const &version) {
    mVersion = version;
}

void ShaderSet::SetPreamble(std::string const &preamble) {
    mPreamble = preamble;
}

GLuint *ShaderSet::AddProgram(std::vector<std::pair<std::string, GLenum>> const &typedShaders) {
    std::vector<ShaderNameTypePair const *> shaderNameTypes;

    // find references to existing shaders, and create ones that didn't exist previously.
    for (std::pair<std::string, GLenum> const &shaderNameType : typedShaders) {
        ShaderNameTypePair tmpShaderNameType;
        std::tie(tmpShaderNameType.Name, tmpShaderNameType.Type) = shaderNameType;

        // test that the file can be opened (to catch typos or missing file bugs)
        {
            std::ifstream ifs(tmpShaderNameType.Name);
            if (!ifs) {
                LOGF(WARNING, "Failed to open shader %s\n", tmpShaderNameType.Name.c_str());
            }
        }

        auto const foundShader = mShaders.emplace(std::move(tmpShaderNameType), Shader{}).first;
        if (!foundShader->second.Handle) {
            foundShader->second.Handle = glCreateShader(shaderNameType.second);
            // Mask the hash to 16 bits because some implementations are limited to that number of bits.
            // The sign bit is masked out, since some shader compilers treat the #line as signed, and others treat it unsigned.
            foundShader->second.HashName = (int32_t)std::hash<std::string>()(shaderNameType.first) & 0x7FFF;
        }
        shaderNameTypes.push_back(&foundShader->first);
    }

    // ensure the programs have a canonical order
    std::ranges::sort(shaderNameTypes);
    shaderNameTypes.erase(std::ranges::unique(shaderNameTypes).begin(), end(shaderNameTypes));

    // find the program associated to these shaders (or create it if missing)
    auto const foundProgram = mPrograms.emplace(shaderNameTypes, Program{}).first;
    if (!foundProgram->second.InternalHandle) {
        // public handle is 0 until the program has linked without error
        foundProgram->second.PublicHandle = 0;

        foundProgram->second.InternalHandle = glCreateProgram();
        for (ShaderNameTypePair const *shader : shaderNameTypes) {
            glAttachShader(foundProgram->second.InternalHandle, mShaders[*shader].Handle);
        }
    }

    return &foundProgram->second.PublicHandle;
}

void ShaderSet::UpdatePrograms() {
    // find all shaders with updated timestamps
    std::set<std::pair<ShaderNameTypePair const, Shader> *> updatedShaders;
    for (std::pair<ShaderNameTypePair const, Shader> &shader : mShaders) {
        uint64_t timestamp = GetShaderFileTimestamp(shader.first.Name.c_str());
        if (timestamp > shader.second.Timestamp) {
            shader.second.Timestamp = timestamp;
            updatedShaders.insert(&shader);
        }
    }

    // recompile all updated shaders
    for (std::pair<ShaderNameTypePair const, Shader> *shader : updatedShaders) {
        // the #line prefix ensures error messages have the right line number for their file
        // the #line directive also allows specifying a "file name" number, which makes it possible to identify which file the error came
        // from.
        std::string version = "#version " + mVersion + "\n";

        std::string defines;
        switch (shader->first.Type) {
        case GL_VERTEX_SHADER:
            defines += "#define VERTEX_SHADER\n";
            break;
        case GL_FRAGMENT_SHADER:
            defines += "#define FRAGMENT_SHADER\n";
            break;
        case GL_GEOMETRY_SHADER:
            defines += "#define GEOMETRY_SHADER\n";
            break;
        case GL_TESS_CONTROL_SHADER:
            defines += "#define TESS_CONTROL_SHADER\n";
            break;
        case GL_TESS_EVALUATION_SHADER:
            defines += "#define TESS_EVALUATION_SHADER\n";
            break;
        case GL_COMPUTE_SHADER:
            defines += "#define COMPUTE_SHADER\n";
            break;
        default:
            LOG(FATAL) << "Unknown Shader Type: " << shader->first.Type;
            break;
        }

        std::string preamble_hash = std::to_string((int32_t)std::hash<std::string>()("preamble") & 0x7FFF);
        std::string preamble = "#line 1 " + preamble_hash + "\n" + mPreamble + "\n";

        std::string source_hash = std::to_string(shader->second.HashName);
        std::string source = "#line 1 " + source_hash + "\n" + ShaderStringFromFile(shader->first.Name.c_str()) + "\n";

        char const *strings[] = {version.c_str(), defines.c_str(), preamble.c_str(), source.c_str()};
        GLint lengths[] = {(GLint)version.length(), (GLint)defines.length(), (GLint)preamble.length(), (GLint)source.length()};

        glShaderSource(shader->second.Handle, std::size(strings), strings, lengths);
        glCompileShader(shader->second.Handle);

        GLint status;
        glGetShaderiv(shader->second.Handle, GL_COMPILE_STATUS, &status);
        if (!status) {
            GLint logLength;
            glGetShaderiv(shader->second.Handle, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength + 1);
            glGetShaderInfoLog(shader->second.Handle, logLength, nullptr, log.data());

            std::string log_s = log.data();

            // replace all filename hashes in the error messages with actual filenames
            for (size_t found_preamble; (found_preamble = log_s.find(preamble_hash)) != std::string::npos;) {
                log_s.replace(found_preamble, preamble_hash.size(), "preamble");
            }
            for (size_t found_source; (found_source = log_s.find(source_hash)) != std::string::npos;) {
                log_s.replace(found_source, source_hash.size(), shader->first.Name);
            }

            LOGF(WARNING, "Error compiling %s:\n%s\n", shader->first.Name.c_str(), log_s.c_str());
        }
    }

    // relink all programs that had their shaders updated and have all their shaders compiling successfully
    for (std::pair<std::vector<ShaderNameTypePair const *> const, Program> &program : mPrograms) {
        bool programNeedsRelink = false;
        for (ShaderNameTypePair const *programShader : program.first) {
            for (std::pair<ShaderNameTypePair const, Shader> *shader : updatedShaders) {
                if (&shader->first == programShader) {
                    programNeedsRelink = true;
                    break;
                }
            }

            if (programNeedsRelink)
                break;
        }

        // Don't attempt to link shaders that didn't compile successfully
        bool canRelink = true;
        if (programNeedsRelink) {
            for (ShaderNameTypePair const *programShader : program.first) {
                GLint status;
                glGetShaderiv(mShaders[*programShader].Handle, GL_COMPILE_STATUS, &status);
                if (!status) {
                    canRelink = false;
                    break;
                }
            }
        }

        if (programNeedsRelink && canRelink) {
            glLinkProgram(program.second.InternalHandle);

            GLint logLength;
            glGetProgramiv(program.second.InternalHandle, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength + 1);
            glGetProgramInfoLog(program.second.InternalHandle, logLength, nullptr, log.data());

            std::string log_s = log.data();

            // replace all filename hashes in the error messages with actual filenames
            std::string preamble_hash = std::to_string((int32_t)std::hash<std::string>()("preamble"));
            for (size_t found_preamble; (found_preamble = log_s.find(preamble_hash)) != std::string::npos;) {
                log_s.replace(found_preamble, preamble_hash.size(), "preamble");
            }
            for (ShaderNameTypePair const *shaderInProgram : program.first) {
                std::string source_hash = std::to_string(mShaders[*shaderInProgram].HashName);
                for (size_t found_source; (found_source = log_s.find(source_hash)) != std::string::npos;) {
                    log_s.replace(found_source, source_hash.size(), shaderInProgram->Name);
                }
            }

            GLint status;
            glGetProgramiv(program.second.InternalHandle, GL_LINK_STATUS, &status);

            constexpr size_t MAX_LOG_LENGTH = 512;
            char linkerLogBuffer[MAX_LOG_LENGTH];
            snprintf(linkerLogBuffer, MAX_LOG_LENGTH, "");

            for (ShaderNameTypePair const *shader : program.first) {
                if (shader != program.first.front()) {
                    snprintf(linkerLogBuffer + strlen(linkerLogBuffer), MAX_LOG_LENGTH, ", ");
                }
                snprintf(linkerLogBuffer + strlen(linkerLogBuffer), MAX_LOG_LENGTH, "%s", shader->Name.c_str());
            }
            LOGF(DEBUG, "%s program (%s)", status ? "Successfully linked" : "Error linking", linkerLogBuffer);

            if (!status) {
                program.second.PublicHandle = 0;
            } else {
                program.second.PublicHandle = program.second.InternalHandle;
            }
        }
    }
}

void ShaderSet::SetPreambleFile(std::string const &preambleFilename) {
    SetPreamble(ShaderStringFromFile(preambleFilename.c_str()));
}

GLuint *ShaderSet::AddProgramFromExts(std::vector<std::string> const &shaders) {
    std::vector<std::pair<std::string, GLenum>> typedShaders;
    for (std::string const &shader : shaders) {
        size_t const extLoc{shader.find_last_of('.')};
        if (extLoc == std::string::npos) {
            return nullptr;
        }

        GLenum shaderType;

        std::string ext = shader.substr(extLoc + 1);
        if (ext == "vert")
            shaderType = GL_VERTEX_SHADER;
        else if (ext == "frag")
            shaderType = GL_FRAGMENT_SHADER;
        else if (ext == "geom")
            shaderType = GL_GEOMETRY_SHADER;
        else if (ext == "tesc")
            shaderType = GL_TESS_CONTROL_SHADER;
        else if (ext == "tese")
            shaderType = GL_TESS_EVALUATION_SHADER;
        else if (ext == "comp")
            shaderType = GL_COMPUTE_SHADER;
        else
            return nullptr;

        typedShaders.emplace_back(shader, shaderType);
    }

    return AddProgram(typedShaders);
}

GLuint *ShaderSet::AddProgramFromCombinedFile(std::string const &filename, std::vector<GLenum> const &shaderTypes) {
    std::vector<std::pair<std::string, GLenum>> typedShaders;

    for (auto type : shaderTypes)
        typedShaders.emplace_back(filename, type);

    return AddProgram(typedShaders);
}