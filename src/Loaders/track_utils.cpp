//
// Created by Amrik Sadhra on 19/07/2018.
//

#include "track_utils.h"
#include "nfs2_loader.h"

namespace TrackUtils {
    std::map<unsigned int, GLuint> GenTextures(std::map<unsigned int, Texture> textures) {
        std::map<unsigned int, GLuint> gl_id_map;

        for (auto it = textures.begin(); it != textures.end(); ++it) {
            Texture texture = it->second;
            GLuint textureID;
            glGenTextures(1, &textureID);
            gl_id_map[it->first] = textureID;
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // TODO: Use Filtering for Textures with no alpha component
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) texture.texture_data);
        }

        return gl_id_map;
    }

    std::vector<unsigned int> RemapTextureIDs(const std::set<unsigned int> &minimal_texture_ids_set, std::vector<unsigned int> &texture_indices) {
        // Get ordered list of unique texture id's present in block
        std::vector<unsigned int> texture_ids;
        texture_ids.assign(minimal_texture_ids_set.begin(), minimal_texture_ids_set.end());
        // Remap polygon texture ID's to correspond to ordered texture ID's
        std::map<int, int> ordered_mapping;
        for (int t = 0; t < texture_ids.size(); ++t) {
            ordered_mapping[texture_ids[t]] = t;
        }
        for (auto &texture_index : texture_indices) {
            texture_index = static_cast<unsigned int>(ordered_mapping.find(texture_index)->second);
        }
        return texture_ids;
    }

    Light MakeLight(glm::vec3 light_position, uint32_t light_type) {
        // Use Data from NFSHS NFS3 Tracks TR.INI
        switch (light_type) {
            case 0:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00);
            case 1:
                return Light(light_position, glm::vec4(185, 255, 255, 255), light_type, 0, 0, 0, 4.50);
            case 2:
                return Light(light_position, glm::vec4(255, 255, 255, 210), light_type, 0, 0, 0, 5.00);
            case 3:
                return Light(light_position, glm::vec4(255, 128, 229, 240), light_type, 0, 0, 0, 4.50);
            case 4:
                return Light(light_position, glm::vec4(255, 217, 196, 94), light_type, 0, 0, 0, 5.00);
            case 5:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 6, 0, 5.00);
            case 6:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 5, 27, 5.00);
            case 7:
                return Light(light_position, glm::vec4(255, 255, 0, 0), light_type, 1, 6, 0, 3.13);
            case 8:
                return Light(light_position, glm::vec4(255, 163, 177, 190), light_type, 0, 0, 0, 3.75);
            case 9:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 0, 0, 0, 3.13);
            case 10:
                return Light(light_position, glm::vec4(186, 223, 22, 22), light_type, 0, 0, 0, 2.50);
            case 11:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 0, 0, 0, 2.50);
            case 12:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 1, 6, 0, 3.13);
            case 13:
                return Light(light_position, glm::vec4(255, 224, 224, 39), light_type, 0, 0, 0, 3.75);
            case 14:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00);
            case 15:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00);
            case 16:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00);
            case 17:
                return Light(light_position, glm::vec4(185, 255, 255, 255), light_type, 0, 0, 0, 4.50);
            case 18:
                return Light(light_position, glm::vec4(255, 255, 255, 210), light_type, 0, 0, 0, 5.00);
            case 19:
                return Light(light_position, glm::vec4(255, 128, 229, 240), light_type, 0, 0, 0, 4.50);
            case 20:
                return Light(light_position, glm::vec4(255, 217, 196, 94), light_type, 0, 0, 0, 5.00);
            case 21:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 6, 0, 5.00);
            case 22:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 1, 5, 27, 5.00);
            case 23:
                return Light(light_position, glm::vec4(255, 255, 0, 0), light_type, 1, 6, 0, 3.13);
            case 24:
                return Light(light_position, glm::vec4(255, 163, 177, 190), light_type, 0, 0, 0, 3.75);
            case 25:
                return Light(light_position, glm::vec4(255, 223, 22, 22), light_type, 0, 0, 0, 3.13);
            case 26:
                return Light(light_position, glm::vec4(186, 223, 22, 22), light_type, 0, 0, 0, 2.50);
            case 27:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 0, 0, 0, 2.50);
            case 28:
                return Light(light_position, glm::vec4(255, 30, 149, 227), light_type, 1, 6, 0, 3.13);
            case 29:
                return Light(light_position, glm::vec4(255, 224, 224, 39), light_type, 0, 0, 0, 3.75);
            case 30:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00);
            case 31:
                return Light(light_position, glm::vec4(255, 222, 234, 235), light_type, 0, 0, 0, 5.00);
            default:
                return Light(light_position, glm::vec4(255, 255, 255, 255), light_type, 0, 0, 0, 5.00);
        }
    }

    bool ExtractTrackTextures(const std::string &track_path, const ::std::string track_name, NFSVer nfs_version) {
        std::stringstream output_dir, tex_archive_path;
        std::string psh_path = track_path;
        std::string full_track_path = track_path + "/" + track_name;
        output_dir << TRACK_PATH;

        switch (nfs_version) {
            case NFS_2:
                output_dir << "NFS2/";
                tex_archive_path << full_track_path << "0.qfs";
                break;
            case NFS_2_SE:
                output_dir << "NFS2_SE/";
                tex_archive_path << full_track_path << "0M.qfs";
                break;
            case NFS_3:
                output_dir << "NFS3/";
                tex_archive_path << full_track_path << "0.qfs";
                break;
            case NFS_3_PS1:
                psh_path.replace(psh_path.find("ZZ"), 2, "");
                output_dir << "NFS3_PS1/";
                tex_archive_path << psh_path << "0.PSH";
                break;
            case NFS_4:
                output_dir << "NFS4/";
                tex_archive_path << full_track_path << "/TR0.qfs";
                break;
            case UNKNOWN:
            default:
                output_dir << "UNKNOWN/";
                break;
        }
        output_dir << track_name;

        if (boost::filesystem::exists(output_dir.str())) {
            return true;
        } else {
            boost::filesystem::create_directories(output_dir.str());
        }

        std::cout << "Extracting track textures" << std::endl;

        if (nfs_version == NFS_3_PS1) {
            output_dir << "/textures/";
            return Utils::ExtractPSH(tex_archive_path.str(), output_dir.str());
        } else if (nfs_version == NFS_3) {
            std::stringstream sky_fsh_path;
            sky_fsh_path << full_track_path.substr(0, full_track_path.find_last_of('/')) << "/sky.fsh";
            if (boost::filesystem::exists(sky_fsh_path.str())) {
                std::stringstream sky_textures_path;
                sky_textures_path << output_dir.str() << "/sky_textures/";
                std::cout << sky_fsh_path.str() << std::endl;
                ASSERT(Utils::ExtractQFS(sky_fsh_path.str(), sky_textures_path.str()), "Unable to extract sky textures from sky.fsh");
            }
        }

        output_dir << "/textures/";
        return (Utils::ExtractQFS(tex_archive_path.str(), output_dir.str()));
    }

    glm::vec3 parseRGBString(const std::string &rgb_string) {
        std::stringstream tempComponent;
        uint8_t commaCount = 0;
        glm::vec3 rgbValue;

        for (int char_Idx = 0; char_Idx < rgb_string.length(); ++char_Idx) {
            if (rgb_string[char_Idx] == ',') {
                switch (commaCount) {
                    case 0:
                        rgbValue.x = stoi(tempComponent.str());
                        break;
                    case 1:
                        rgbValue.y = stoi(tempComponent.str());
                        break;
                    case 2:
                        rgbValue.z = stoi(tempComponent.str());
                        break;
                }
                tempComponent.str("");
                if(++commaCount >= 3) break;
            }  else {
                tempComponent << rgb_string[char_Idx];
            }
        }

        return rgbValue;
    }
}
