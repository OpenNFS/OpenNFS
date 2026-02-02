#include "MenuText.h"

namespace OpenNFS {
    MenuText::MenuText(NFSVersion _nfsVersion) : nfsVersion(_nfsVersion) {
    }

    bool MenuText::LoadMenuText(LibOpenNFS::NFS3::TextFile &textFile) {
        trackNames= textFile.trackNames;

        return true;
    }

} // namespace OpenNFS