#pragma once

#include <string>
#include <memory>
#include <Common/NFSVersion.h>

#include "../Menu/MenuText.h"


namespace OpenNFS {
    class MenuTextLoader {
      public:
        static std::shared_ptr<MenuText> LoadMenuText(NFSVersion nfsVersion);
    };
} // namespace OpenNFS
