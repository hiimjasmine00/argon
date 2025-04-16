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

    enum class AuthProgress {
        RequestedChallenge,
        SolvingChallenge,
        VerifyingChallenge,

        RetryingRequest,
        RetryingSolve,
        RetryingVerify,
    };

    std::string authProgressToString(AuthProgress progress);

    using AuthCallback = std::function<void(geode::Result<std::string>)>;
    using AuthProgressCallback = std::function<void(AuthProgress)>;

    AccountData getGameAccountData();

    geode::Result<> startAuth(AuthCallback callback, AuthProgressCallback progress = {}, bool forceStrong = false);
    geode::Result<> startAuthWithAccount(AccountData account, AuthCallback callback, AuthProgressCallback progress = {}, bool forceStrong = false);

    geode::Result<> setServerUrl(std::string url);

    geode::Result<geode::utils::web::WebTask> startAuthInternal(AccountData account, std::string_view preferredMethod, bool forceStrong);
}