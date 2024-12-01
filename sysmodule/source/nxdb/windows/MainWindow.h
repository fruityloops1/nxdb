#pragma once

#include "../Component.h"
#include "../Process.h"

namespace nxdb {

    class MainWindow : public Component {
        int mNumProcesses = 0;
        Process mProcessList[sMaxPids];
        int mFramesSinceLastProcessListUpdate = 361;

    public:
        void update() override;
    };

} // namespace nxdb
