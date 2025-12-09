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
        std::string serverUrl;
    };

    enum class AuthProgress {
        RequestedChallenge,
        SolvingChallenge,
        VerifyingChallenge,

        RetryingRequest,
        RetryingSolve,
        RetryingVerify,
    };

    // Converts the `AuthProgress` enum to a human readable string,
    // e.g. "Requesting challenge", "Solving challenge"
    std::string authProgressToString(AuthProgress progress);

    using AuthCallback = std::function<void(geode::Result<std::string>)>;
    using AuthProgressCallback = std::function<void(AuthProgress)>;
    using AuthLoginTask = geode::Task<geode::Result<std::string>, AuthProgress>;

    // Collects the account data of the currently logged in user. Call only on main thread.
    AccountData getGameAccountData();

    // Returns whether the user is signed into a Geometry Dash account. Call only on main thread.
    bool signedIn();

    // Set the URL of the used Argon server, thread-safe.
    geode::Result<> setServerUrl(std::string url);

    // Get the URL of the used Argon server, thread-safe.
    std::string getServerUrl();

    // Enable or disable SSL certificate verification, by default is enabled.
    void setCertVerification(bool state);

    // Get whether certificate verification is enabled
    bool getCertVerification();

    // Initializes the config lock structure for interoperability between other mods using Argon.
    // Should be called from the main thread once the game has at least reached the loading screen
    // (what matters is that GameManager::init has been run.)
    void initConfigLock();

    // Returns whether the config lock has been initialized
    bool isConfigLockInitialized();

    /* Starting auth */

    // Start authentication with the current user account.
    // `callback` is called with the authtoken once auth is complete, or with an error if one happens. It should always be called.
    // `progress` is an optional progress callback that will be called whenever the auth process reaches the next stage.
    //
    // **NOTE**: this function is not thread-safe, see README for details.
    geode::Result<> startAuth(AuthCallback callback, AuthProgressCallback progress = {}, bool forceStrong = false);

    // Start authentication with given user credentials.
    // `callback` is called with the authtoken once auth is complete, or with an error if one happens. It should always be called.
    // `progress` is an optional progress callback that will be called whenever the auth process reaches the next stage.
    //
    // This function can be safely called from any thread, and it won't block the thread.
    // **NOTE**: not thread-safe if the config lock wasn't initialized yet, see README for details.
    geode::Result<> startAuthWithAccount(AccountData account, AuthCallback callback, AuthProgressCallback progress = {}, bool forceStrong = false);

    // Wrapper around startAuthWithAccount that returns an awaitable Task
    // This function can be safely called from any thread, and it won't block the thread.
    // **NOTE**: not thread-safe if the config lock wasn't initialized yet, see README for details.
    AuthLoginTask startAuthWithAccount(AccountData account, bool forceStrong = false);

    // Wrapper around startAuth that returns an awaitable Task
    // **NOTE**: this function is not thread-safe, see README for details.
    AuthLoginTask startAuth(bool forceStrong = false);

    /* Managing tokens */

    // Clears all authtokens from the storage that use the same server URL as the current selected.
    // Don't use this unless there's a good reason to. Thread-safe.
    void clearAllTokens();

    // Clears all authtokens from the storage for the currently used GD account.
    // Only tokens generated with the same server URL are deleted. Not thread-safe, for thread safety use `(int)` overload.
    void clearToken();

    // Clears all authtokens from the storage for this account.
    // Only tokens generated with the same server URL are deleted. Thread-safe.
    void clearToken(int accountId);

    // Clears all authtokens from the storage for this account.
    // Only tokens generated with the same server URL are deleted. Thread-safe.
    void clearToken(const AccountData& account);

    // Checks if there's an authtoken stored for the currently used GD account.
    // Not thread-safe, for thread safety use `(const AccountData&)` overload.
    bool hasToken();

    // Checks if there's an authtoken stored for this account, thread-safe.
    // If this returns true, all auth functions will likely immediately return success.
    bool hasToken(const AccountData& account);
}
