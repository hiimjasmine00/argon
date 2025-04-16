#pragma once

#include <argon/argon.hpp>
#include <chrono>
#include "web.hpp"
#include "singleton_base.hpp"

namespace argon {

struct PendingRequest {
    size_t id;
    AuthCallback callback;
    AuthProgressCallback progressCallback;
    AccountData account;
    bool forceStrong;
    std::string serverIdent;
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
    std::lock_guard<std::mutex> lockServerUrl();

    void progress(PendingRequest* req, AuthProgress progress);
    void pushNewRequest(AuthCallback callback, AuthProgressCallback progress, AccountData account, web::WebTask req, bool forceStrong);
    void pushStage2Request(PendingRequest* preq, web::WebTask req);
    void pushStage3Request(PendingRequest* preq, web::WebTask req);
    void restartStage1(PendingRequest* preq);

protected:
    friend class SingletonBase;

    std::mutex serverUrlMtx;
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
