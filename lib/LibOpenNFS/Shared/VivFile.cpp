#include "VivFile.h"

#include <cstring>
#include <sstream>
#include <filesystem>

namespace LibOpenNFS::Shared {
    bool VivFile::Load(const std::string &vivPath, VivFile &vivFile) {
        // LOG(INFO) << "Loading VIV File located at " << vivPath;
        std::ifstream viv(vivPath, std::ios::in | std::ios::binary);

        bool loadStatus = vivFile._SerializeIn(viv);
        viv.close();

        return loadStatus;
    }

    void VivFile::Save(const std::string &vivPath, VivFile &vivFile) {
        // LOG(INFO) << "Saving CAN File to " << vivPath;
        std::ofstream viv(vivPath, std::ios::out | std::ios::binary);
        vivFile._SerializeOut(viv);
    }

    bool VivFile::Extract(const std::string &outPath, VivFile &vivFile) {
        if (std::filesystem::exists(outPath)) {
            // LOG(INFO) << "VIV has already been extracted, skipping";
            return true;
        } else {
            std::filesystem::create_directories(outPath);
        }

        for (uint8_t fileIdx = 0; fileIdx < vivFile.nFiles; ++fileIdx) {
            VivEntry &curFile{vivFile.files.at(fileIdx)};

            std::stringstream out_file_path;
            out_file_path << outPath << curFile.filename;

            std::ofstream out(out_file_path.str(), std::ios::out | std::ios::binary);
            if (!out.is_open()) {
                // LOG(WARNING) << "Error while creating output file " << fileName;
                return false;
            }
            out.write((char *) curFile.data.data(), curFile.data.size());
            out.close();
        }
        return true;
    }

    bool VivFile::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, vivHeader));

        if (memcmp(vivHeader, "BIGF", sizeof(vivHeader))) {
            // LOG(WARNING) << "Not a valid VIV file (BIGF header missing)";
            return false;
        }

        onfs_check(safe_read(ifstream, vivSize));
        vivSize = _SwapEndian(vivSize);

        onfs_check(safe_read(ifstream, nFiles));
        nFiles = _SwapEndian(nFiles);
        files.resize(nFiles);

        // LOG(INFO) << "VIV contains " << nFiles << " files";
        onfs_check(safe_read(ifstream, startPos));
        startPos = _SwapEndian(startPos);

        std::streampos currentPos = ifstream.tellg();

        for (uint8_t fileIdx = 0; fileIdx < nFiles; ++fileIdx) {
            ifstream.seekg(currentPos, std::ios_base::beg);
            uint32_t filePos = 0, fileSize = 0;
            onfs_check(safe_read(ifstream, filePos));
            filePos = _SwapEndian(filePos);

            onfs_check(safe_read(ifstream, fileSize));
            fileSize = _SwapEndian(fileSize);

            VivEntry &curFile{files.at(fileIdx)};
            int pos = 0;
            char c  = ' ';
            ifstream.read(&c, sizeof(char));
            while (c != '\0') {
                curFile.filename[pos] = tolower(c);
                pos++;
                ifstream.read(&c, sizeof(char));
            }
            curFile.filename[pos] = '\0';

            currentPos = ifstream.tellg();
            ifstream.seekg(filePos, std::ios_base::beg);
            curFile.data.resize(fileSize);
            ifstream.read((char *) curFile.data.data(), fileSize);
            // LOG(INFO) << "File " << fileName << " was written successfully";
        }

        return true;
    }

    void VivFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "VIV Output serialization is not implemented yet");
    }

    uint32_t VivFile::_SwapEndian(uint32_t const x) {
        return (x >> 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x << 24);
    }
} // namespace LibOpenNFS::Shared
