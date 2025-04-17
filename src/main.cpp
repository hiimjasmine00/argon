#include <argon/argon.hpp>
#include "state.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace argon {

std::string authProgressToString(AuthProgress progress) {
    switch (progress) {
        case AuthProgress::RequestedChallenge:
            return "Requested challenge";
        case AuthProgress::SolvingChallenge:
            return "Solving challenge";
        case AuthProgress::VerifyingChallenge:
            return "Verifying solution";
        case AuthProgress::RetryingRequest:
            return "Requested challenge (retry)";
        case AuthProgress::RetryingSolve:
            return "Solving challenge (retry)";
        case AuthProgress::RetryingVerify:
            return "Verifying solution (retry)";
        default:
            return "Unknown";
    }
}

AccountData getGameAccountData() {
    int accountId = GJAccountManager::get()->m_accountID;
    int userId = GameManager::get()->m_playerUserID;
    std::string username = GJAccountManager::get()->m_username;
    std::string gjp = GJAccountManager::get()->m_GJP2;

    return {
        .accountId = accountId,
        .userId = userId,
        .username = std::move(username),
        .gjp2 = std::move(gjp)
    };
}

Result<> startAuth(AuthCallback callback, AuthProgressCallback progress, bool forceStrong) {
    auto data = getGameAccountData();
    if (data.accountId <= 0 || data.userId <= 0) {
        return Err("Not logged into a Geometry Dash account");
    }

    return startAuthWithAccount(std::move(data), std::move(callback), std::move(progress), forceStrong);
}

Result<> startAuthWithAccount(AccountData account, AuthCallback callback, AuthProgressCallback progress, bool forceStrong) {
    auto& argon = ArgonState::get();
    auto _ = argon.lockServerUrl();

    GEODE_UNWRAP_INTO(auto task, startAuthInternal(account, "message", forceStrong));

    argon.pushNewRequest(
        std::move(callback),
        std::move(progress),
        std::move(account),
        std::move(task),
        forceStrong
    );

    return Ok();
}

Result<web::WebTask> startAuthInternal(const AccountData& account, std::string_view preferredMethod, bool forceStrong) {
    if (account.accountId <= 0 || account.userId <= 0 || account.username.empty()) {
        return Err("Invalid account data");
    }

    // Start stage 1 authentication.

    return Ok(argon::web::startStage1(account, preferredMethod, forceStrong));
}

Result<> setServerUrl(std::string url) {
    if (url.empty()) {
        return Err("Invalid server URL");
    }

    return ArgonState::get().setServerUrl(std::move(url));
}

}
