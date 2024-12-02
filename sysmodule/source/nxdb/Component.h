#pragma once

#include <deque>

namespace nxdb {

    class Component;
    struct SharedData {
        std::deque<Component*> components;

        void handleDeadComponents();
        void registerComponent(Component* component) { components.push_back(component); }
    };

    class Component {
        bool mDelete = false;

    public:
        virtual ~Component() { }
        virtual void update() { }
        virtual void queueForDeletion() { mDelete = true; }

        static SharedData& getSharedData();

        friend struct SharedData;
    };

} // namespace nxdb
