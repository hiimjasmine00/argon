#pragma once

#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>
#include <string>
#include <functional>

namespace argon {
    struct AccountData {
        int accountId;
        int userId;
        std::string username;
        std::string gjp2;
    };

    class AuthVerdict {};

    using AuthCallback = std::function<void(geode::Result<AuthVerdict>)>;

    AccountData getGameAccountData();

    geode::Result<> startAuth(AuthCallback callback);
    geode::Result<> startAuthWithAccount(AuthCallback callback, AccountData account);

    geode::Result<> setServerUrl(std::string url);

    geode::Result<geode::utils::web::WebTask> startAuthInternal(AccountData account, std::string_view preferredMethod);
}