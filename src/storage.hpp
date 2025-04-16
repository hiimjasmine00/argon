#pragma once

#include "singleton_base.hpp"
#include "state.hpp"

namespace argon {
    class ArgonStorage : public SingletonBase<ArgonStorage> {
        friend class SingletonBase;
        ArgonStorage();

    public:
        geode::Result<> storeAuthToken(PendingRequest* req, std::string_view authtoken);

    private:
    };
} // namespace argon
