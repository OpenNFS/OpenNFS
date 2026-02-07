#include "MenuText.h"

#include <iostream>
#include "../Util/Logger.h"

namespace OpenNFS {
    MenuText::MenuText(NFSVersion _nfsVersion) : nfsVersion(_nfsVersion) {
    }

    bool MenuText::LoadMenuText(LibOpenNFS::NFS3::TextFile &textFile) {
        trackNames = textFile.trackNames;

        return true;
    }

    bool MenuText::LoadMenuText(LibOpenNFS::NFS4::TextFile &textFile) {
        trackNames = textFile.trackNames;

        return true;
    }

} // namespace OpenNFS