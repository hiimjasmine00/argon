#pragma once

#include "singleton_base.hpp"
#include "state.hpp"

namespace argon {

class ArgonStorage : public SingletonBase<ArgonStorage> {
    friend class SingletonBase;
    ArgonStorage();

public:
    geode::Result<> storeAuthToken(PendingRequest* req, std::string_view authtoken);
    std::optional<std::string> getAuthToken(const AccountData& account, std::string_view serverUrl);

    void clearTokens(int accountId);
    void clearAllTokens();

private:
};

} // namespace argon
