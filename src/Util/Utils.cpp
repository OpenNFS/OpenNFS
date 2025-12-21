#include "Utils.h"

#include <random>
#include <Common/TextureUtils.h>

#include "ImageLoader.h"

using namespace std::filesystem;

namespace OpenNFS::Utils {
    glm::mat4 rotAroundPoint(float const rad, glm::vec3 const &point, glm::vec3 const &axis) {
        return glm::translate(glm::mat4(1), point) * glm::rotate(glm::mat4(1), rad, axis) * glm::translate(glm::mat4(1), -point);
    }

    glm::vec3 bulletToGlm(const btVector3 &v) {
        return {v.getX(), v.getY(), v.getZ()};
    }

    btVector3 glmToBullet(const glm::vec3 &v) {
        return {v.x, v.y, v.z};
    }

    glm::quat bulletToGlm(const btQuaternion &q) {
        return {q.getW(), q.getX(), q.getY(), q.getZ()};
    }

    btQuaternion glmToBullet(const glm::quat &q) {
        return {q.x, q.y, q.z, q.w};
    }

    btMatrix3x3 glmToBullet(const glm::mat3 &m) {
        return {m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]};
    }

    // btTransform does not contain a full 4x4 matrix, so this transform is lossy.
    // Affine transformations are OK but perspective transformations are not.
    btTransform glmToBullet(const glm::mat4 &m) {
        glm::mat3 const m3{m};
        return btTransform(glmToBullet(m3), glmToBullet(glm::vec3(m[3][0], m[3][1], m[3][2])));
    }

    glm::mat4 bulletToGlm(const btTransform &t) {
        auto m {glm::mat4()};
        const btMatrix3x3 &basis {t.getBasis()};
        // rotation
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                m[c][r] = basis[r][c];
            }
        }
        // traslation
        const btVector3 &origin {t.getOrigin()};
        m[3][0] = origin.getX();
        m[3][1] = origin.getY();
        m[3][2] = origin.getZ();
        // unit scale
        m[0][3] = 0.0f;
        m[1][3] = 0.0f;
        m[2][3] = 0.0f;
        m[3][3] = 1.0f;
        return m;
    }

    float RandomFloat(float const min, float const max) {
        static std::mt19937 mt(std::random_device{}());
        std::uniform_real_distribution<double> fdis(min, max);

        return static_cast<float>(fdis(mt));
    }

    DimensionData GenDimensions(const std::vector<glm::vec3> &vertices) {
        DimensionData modelDimensions = {};
        modelDimensions.maxVertex = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        modelDimensions.minVertex = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);

        for (const auto &vertex: vertices) {
            modelDimensions.minVertex = glm::min(modelDimensions.minVertex, vertex);
            modelDimensions.maxVertex = glm::max(modelDimensions.maxVertex, vertex);
        }

        return modelDimensions;
    }

    btTransform MakeTransform(glm::vec3 const position, glm::quat const orientation) {
        btTransform transform;
        transform.setOrigin(glmToBullet(position));
        transform.setRotation(glmToBullet(orientation));

        return transform;
    }

    std::vector<NfsAssetList> PopulateAssets() {
        std::vector<NfsAssetList> installedNFS;

        path basePath(RESOURCE_PATH);
        bool hasMisc {false};
        bool hasUI {false};

        for (directory_iterator itr(basePath); itr != directory_iterator(); ++itr) {
            NfsAssetList currentNFS;
            currentNFS.tag = NFSVersion::UNKNOWN;

            if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_2_SE)) {
                currentNFS.tag = NFSVersion::NFS_2_SE;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_2_SE_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                CHECK_F(exists(trackBasePath), "NFS 2 Special Edition track folder: %s is missing",
                        trackBasePath.c_str());

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".trk") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_2_SE_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                CHECK_F(exists(carBasePath), "NFS 2 Special Edition car folder: %s is missing", carBasePath.c_str());

                // TODO: Work out where NFS2 SE Cars are stored
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_2)) {
                currentNFS.tag = NFSVersion::NFS_2;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_2_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                CHECK_F(exists(trackBasePath), "NFS 2 track folder: %s is missing", trackBasePath.c_str());

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".trk") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_2_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                CHECK_F(exists(carBasePath), "NFS 2 car folder: %s is missing", carBasePath.c_str());

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".geo") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_2_PS1)) {
                currentNFS.tag = NFSVersion::NFS_2_PS1;

                for (directory_iterator trackItr(itr->path().string()); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".trk") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                for (directory_iterator carItr(itr->path().string()); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".geo") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_3_PS1)) {
                currentNFS.tag = NFSVersion::NFS_3_PS1;

                for (directory_iterator trackItr(itr->path().string()); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".trk") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                for (directory_iterator carItr(itr->path().string()); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".geo") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_3)) {
                currentNFS.tag = NFSVersion::NFS_3;

                std::string sfxPath = itr->path().string() + "/gamedata/render/pc/sfx.fsh";
                CHECK_F(exists(sfxPath), "NFS 3 SFX Resource: %s is missing", sfxPath.c_str());
                CHECK_F(LibOpenNFS::TextureUtils::ExtractQFS(sfxPath, RESOURCE_PATH + "sfx/"),
                        "Unable to extract SFX textures from %s", sfxPath.c_str());

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_3_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                CHECK_F(exists(trackBasePath), "NFS 3 Hot Pursuit track folder: %s is missing", trackBasePath.c_str());

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_3_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                CHECK_F(exists(carBasePath), "NFS 3 Hot Pursuit car folder: %s is missing", carBasePath.c_str());

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find("traffic") == std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().string());
                    }
                }

                carBasePathStream << "traffic/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back("traffic/" + carItr->path().filename().string());
                }

                carBasePathStream << "pursuit/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find("pursuit") == std::string::npos) {
                        currentNFS.cars.emplace_back("traffic/pursuit/" + carItr->path().filename().string());
                    }
                }
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_4_PS1)) {
                currentNFS.tag = NFSVersion::NFS_4_PS1;

                for (directory_iterator dirItr(itr->path().string()); dirItr != directory_iterator(); ++dirItr) {
                    if (dirItr->path().filename().string().find("zzz") == 0 && dirItr->path().filename().string().
                        find(".viv") != std::string::npos) {
                        currentNFS.cars.emplace_back(dirItr->path().filename().replace_extension("").string());
                    } else if (dirItr->path().filename().string().find("ztr") == 0 && dirItr->path().filename().string()
                               .find(".grp") != std::string::npos) {
                        currentNFS.tracks.emplace_back(dirItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_4)) {
                currentNFS.tag = NFSVersion::NFS_4;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_4_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                CHECK_F(exists(trackBasePath), "NFS 4 High Stakes track folder: %s is missing", trackBasePath.c_str());

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_4_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                CHECK_F(exists(carBasePath), "NFS 4 High Stakes car folder: %s is missing", carBasePath.c_str());

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find("traffic") == std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().string());
                    }
                }

                carBasePathStream << "traffic/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    if ((carItr->path().filename().string().find("choppers") == std::string::npos) &&
                        (carItr->path().filename().string().find("pursuit") == std::string::npos)) {
                        currentNFS.cars.emplace_back("traffic/" + carItr->path().filename().string());
                    }
                }

                carBasePathStream << "choppers/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back("traffic/choppers/" + carItr->path().filename().string());
                }

                carBasePathStream.str(std::string());
                carBasePathStream << itr->path().string() << NFS_4_CAR_PATH << "traffic/"
                        << "pursuit/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back("traffic/pursuit/" + carItr->path().filename().string());
                }
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::MCO)) {
                currentNFS.tag = NFSVersion::MCO;

                std::string trackBasePath = itr->path().string() + MCO_TRACK_PATH;
                CHECK_F(exists(trackBasePath), "Motor City Online track folder: %s is missing", trackBasePath.c_str());

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::string carBasePath = itr->path().string() + MCO_CAR_PATH;
                CHECK_F(exists(carBasePath), "Motor City Online car folder: %s is missing", carBasePath.c_str());
                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                }
            } else if (itr->path().filename().string() == magic_enum::enum_name(NFSVersion::NFS_5)) {
                currentNFS.tag = NFSVersion::NFS_5;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_5_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                CHECK_F(exists(trackBasePath), "NFS 5 track folder: %s is missing", trackBasePath.c_str());

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".crp") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_5_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                CHECK_F(exists(carBasePath), "NFS 5 car folder: %s is missing", carBasePath.c_str());

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".crp") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == "misc") {
                hasMisc = true;
                continue;
            } else if (itr->path().filename().string() == "ui") {
                hasUI = true;
                continue;
            } else if (itr->path().filename().string() == "asset") {
                continue;
            } else {
                LOG(WARNING) << "Unknown folder in resources directory: " << itr->path().filename().string();
                continue;
            }
            installedNFS.emplace_back(currentNFS);
        }

        CHECK_F(hasMisc, "Missing \'misc\' folder in resources directory");
        CHECK_F(hasUI, "Missing \'ui\' folder in resources directory");
        CHECK_F(!installedNFS.empty(), "No Need for Speed games detected in resources directory");

        for (auto const &nfs: installedNFS) {
            LOG(INFO) << "Detected: " << magic_enum::enum_name(nfs.tag);
        }
        return installedNFS;
    }

    static bool FilePathSortByDepthReverse(const std::filesystem::path &a, const std::filesystem::path &b) {
        return (a.string().size() > b.string().size());
    }

    void RenameAssetsToLowercase() {
        LOG(INFO) << "Renaming all available NFS resource files to lowercase for cross-platform ONFS resource load";

        // Get paths of all available NFS versions
        std::vector<path> baseNfsPaths;
        for (directory_iterator itr(RESOURCE_PATH); itr != directory_iterator(); ++itr) {
            // Yucky way of iterating Enum
            for (uint8_t uNfsIdx = 0; uNfsIdx < 11; ++uNfsIdx) {
                auto version = (NFSVersion) uNfsIdx;
                if (itr->path().filename().string() == magic_enum::enum_name(version)) {
                    baseNfsPaths.push_back(itr->path());
                }
            }
        }

        // Perform the renames, store the paths to avoid modifying paths whilst being iterated on
        std::vector<path> originalPaths;
        std::vector<path> lowercasePaths;
        for (auto &baseNfsPath: baseNfsPaths) {
            for (recursive_directory_iterator iter(baseNfsPath), end; iter != end; ++iter) {
                // Convert the filename to lowercase using transform() function and ::tolower in STL, then add it back to path for full, relative path
                path originalPath = iter->path();
                std::string lowerNfsFileName = originalPath.filename().string();
                std::ranges::transform(lowerNfsFileName, lowerNfsFileName.begin(), ::tolower);
                path lowerNfsFilePath(originalPath.parent_path().string() + "/" + lowerNfsFileName);

                // Store the original path and lowercase path for later rename
                if (originalPath != lowerNfsFilePath) {
                    originalPaths.push_back(originalPath);
                    lowercasePaths.push_back(lowerNfsFilePath);
                }
            }
        }
        // Sort the paths by depth, deepest first, so the file references remain valid at base as we traverse and rename
        std::ranges::sort(originalPaths, FilePathSortByDepthReverse);
        std::ranges::sort(lowercasePaths, FilePathSortByDepthReverse);

        // Perform the renaming
        for (uint32_t uNfsFileIdx = 0; uNfsFileIdx < originalPaths.size(); ++uNfsFileIdx) {
            rename(originalPaths[uNfsFileIdx], lowercasePaths[uNfsFileIdx]);
            LOG(INFO) << "Renaming " << originalPaths[uNfsFileIdx].string() << " to " << lowercasePaths[uNfsFileIdx].
string();
        }

        LOG(INFO) << "Renaming complete on " << originalPaths.size() << " files";
    }
} // namespace OpenNFS::Utils
