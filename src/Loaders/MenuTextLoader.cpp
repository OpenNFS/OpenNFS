#include "MenuTextLoader.h"

#include <NFS3/NFS3Loader.h>
#include "../Config.h"

namespace OpenNFS {
    std::shared_ptr<MenuText> MenuTextLoader::LoadMenuText(NFSVersion nfsVersion) {
        MenuText menuText(nfsVersion);
        std::stringstream basePath;
        basePath << RESOURCE_PATH << magic_enum::enum_name(nfsVersion);

        switch (nfsVersion) {
            case NFSVersion::NFS_3:
                {
                    basePath << NFS_3_TEXT_PATH;
                    LibOpenNFS::NFS3::TextFile textFile = LibOpenNFS::NFS3::Loader::LoadMenuText(basePath.str());
                    menuText.LoadMenuText(textFile);
                }
                break;
        }

        return std::make_shared<MenuText>(menuText);
    }
} // namespace OpenNFS
