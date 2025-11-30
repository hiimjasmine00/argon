#pragma once

#include <argon/argon.hpp>
#include "web.hpp"
#include "singleton_base.hpp"

#include <asp/sync/Mutex.hpp>
#include <asp/sync/Channel.hpp>
#include <asp/thread/Thread.hpp>
#include <asp/time/SystemTime.hpp>

namespace argon {

struct PendingRequest {
    size_t id;
    AuthCallback _callback;
    AuthProgressCallback progressCallback;
    AccountData account;
    bool forceStrong;
    std::string serverIdent;
    std::string challengeSolution;
    std::string stage2ChosenMethod;
    uint32_t challengeId = 0;
    int userCommentId;
    bool retrying = false;
    bool cancelled = false;
    web::WebListener stage1Listener;
    web::WebListener stage2Listener;
    web::WebListener stage3Listener;
    asp::time::SystemTime startedAuthAt;
    asp::time::SystemTime startedVerificationAt;

    void callback(geode::Result<std::string>&& value);
};

class ArgonState : public SingletonBase<ArgonState> {
public:
    geode::Result<> setServerUrl(std::string url);
    std::string getServerUrl() const;
    std::string makeUrl(std::string_view suffix) const;

    void setCertVerification(bool state);
    bool getCertVerification() const;

    std::lock_guard<std::mutex> acquireConfigLock();
    void initConfigLock();
    bool isConfigLockInitialized();

    std::optional<asp::time::Duration> isInProgress(int accountId);
    void killAuthAttempt(int accountId);

    void progress(PendingRequest* req, AuthProgress progress);
    void pushNewRequest(AuthCallback callback, AuthProgressCallback progress, AccountData account, web::WebTask req, bool forceStrong);
    void pushStage2Request(PendingRequest* preq, web::WebTask req);
    void pushStage3Request(PendingRequest* preq, web::WebTask req);
    void restartStage1(PendingRequest* preq);

protected:
    friend class SingletonBase;

    asp::Mutex<std::string> serverUrl;
    asp::AtomicBool certVerification = true;
    std::atomic<std::mutex*> configLock = nullptr;
    asp::Mutex<std::unordered_set<PendingRequest*>> pendingRequests;
    size_t nextReqId = 1;

    ArgonState();
    ~ArgonState();

    size_t getNextRequestId();
    PendingRequest* getRequestById(size_t id);
    void cleanupRequest(PendingRequest* req);

    void processStage1Response(PendingRequest* req, web::WebResponse* response);
    void processStage2Response(PendingRequest* req, web::WebResponse* response);
    void processStage3Response(PendingRequest* req, web::WebResponse* response);
    void handleStage1Error(PendingRequest* req, std::string error);
    void handleCancellation(PendingRequest* req);
    void handleSuccessfulAuth(PendingRequest* req, std::string authtoken);
    void waitAndRetryStage3(PendingRequest* req, int ms);

    void troubleshootStage2Failure(PendingRequest* req);

public:
    void handleStage2Error(PendingRequest* req, std::string error);
    void handleStage3Error(PendingRequest* req, std::string error);
};

} // namespace argon
