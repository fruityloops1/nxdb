#include "Component.h"

namespace nxdb {

    static SharedData sSharedData;

    SharedData& Component::getSharedData() {
        return sSharedData;
    }

    void SharedData::handleDeadComponents() {
        for (int i = 0; i < components.size(); i++) {
            auto& component = components[i];

            if (component->mDelete) {
                delete component;
                components.erase(components.begin() + i);
            }
        }
    }

} // namespace nxdb
