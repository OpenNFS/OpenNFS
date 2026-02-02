#pragma once

#include <string>
#include <vector>

#include <Common/NFSVersion.h>
#include <NFS3/TEXT/TextFile.h>

namespace OpenNFS {
    class MenuText {
      public:
        MenuText(NFSVersion _nfsVersion);

        bool LoadMenuText(LibOpenNFS::NFS3::TextFile &textFile);

        NFSVersion nfsVersion;
        std::vector<std::string> trackNames; 
    };
} // namespace OpenNFS
