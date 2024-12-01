#include "Component.h"

namespace nxdb {

    static SharedData sSharedData;

    SharedData& Component::getSharedData() {
        return sSharedData;
    }

} // namespace nxdb
