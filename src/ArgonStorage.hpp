#pragma once
#include "util.hpp"
#include <argon/argon.hpp>

namespace argon {

class ArgonStorage : public SingletonBase<ArgonStorage> {
    friend class SingletonBase;
    ArgonStorage();

public:
    geode::Result<> storeAuthToken(const AccountData& account, std::string_view serverIdent, std::string_view authtoken);
    std::optional<std::string> getAuthToken(const AccountData& account, std::string_view serverUrl);
    bool hasAuthToken(const AccountData& account, std::string_view serverUrl);

    void clearTokens(int accountId);
    void clearAllTokens();

private:
};

}