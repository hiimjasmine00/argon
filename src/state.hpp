#pragma once

#include <argon/argon.hpp>
#include <chrono>
#include "web.hpp"

namespace argon {

template <typename Derived>
class SingletonBase {
public:
    // no copy
    SingletonBase(const SingletonBase&) = delete;
    SingletonBase& operator=(const SingletonBase&) = delete;
    // no move
    SingletonBase(SingletonBase&&) = delete;
    SingletonBase& operator=(SingletonBase&&) = delete;

    static Derived& get() {
        static Derived instance;

        return instance;
    }

protected:
    SingletonBase() {}
};

struct PendingRequest {
    size_t id;
    AuthCallback callback;
    AuthProgressCallback progressCallback;
    AccountData account;
    std::string challengeSolution;
    std::string stage2ChosenMethod;
    bool retrying = false;
    web::WebListener stage1Listener;
    web::WebListener stage2Listener;
    web::WebListener stage3Listener;
    std::chrono::system_clock::time_point startedVerificationAt;
};

class ArgonState : public SingletonBase<ArgonState> {
public:
    geode::Result<> setServerUrl(std::string url);
    std::string_view getServerUrl() const;

    void progress(PendingRequest* req, AuthProgress progress);
    void pushNewRequest(AuthCallback callback, AuthProgressCallback progress, AccountData account, web::WebTask req);
    void pushStage2Request(PendingRequest* preq, web::WebTask req);
    void pushStage3Request(PendingRequest* preq, web::WebTask req);
    void restartStage1(PendingRequest* preq);

protected:
    friend class SingletonBase;

    std::string serverUrl;
    std::unordered_set<PendingRequest*> pendingRequests;
    size_t nextReqId = 1;

    ArgonState();

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

public:
    void handleStage2Error(PendingRequest* req, std::string error);
    void handleStage3Error(PendingRequest* req, std::string error);
};

} // namespace argon
