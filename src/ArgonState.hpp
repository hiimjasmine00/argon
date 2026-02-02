#pragma once
#include <argon/argon.hpp>
#include "util.hpp"

#include <asp/sync/Mutex.hpp>
#include <asp/time/SystemTime.hpp>
#include <atomic>

namespace argon {

class ArgonState : public SingletonBase<ArgonState> {
public:
    void setServerUrl(std::string url);
    std::string getServerUrl() const;
    std::string makeUrl(std::string_view suffix) const;

    void setCertVerification(bool state);
    bool getCertVerification() const;

    std::lock_guard<std::mutex> acquireConfigLock();
    void initConfigLock();
    bool isConfigLockInitialized();

    void handleSuccessfulAuth(AccountData account, std::string authToken, std::string serverIdent, int commentId);

protected:
    friend class SingletonBase;

    asp::Mutex<std::string> m_serverUrl;
    std::atomic<bool> m_certVerification{true};
    std::atomic<std::mutex*> m_configLock = nullptr;

    ArgonState();
};

}