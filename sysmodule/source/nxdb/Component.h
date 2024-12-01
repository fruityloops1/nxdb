#pragma once

namespace nxdb {

    struct SharedData {
    };

    class Component {
    public:
        virtual ~Component() { }
        virtual void update() { }

    protected:
        static SharedData& getSharedData();
    };

} // namespace nxdb
