#pragma once

#include <argon/argon.hpp>
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
    AccountData account;
    std::string stage2ChosenMethod;
    bool retrying = false;
    web::WebListener stage1Listener;
    web::WebListener stage2Listener;
};

class ArgonState : public SingletonBase<ArgonState> {
public:
    void setServerUrl(std::string url);
    std::string_view getServerUrl() const;

    void pushNewRequest(AuthCallback callback, AccountData account, web::WebTask req);
    void pushStage2Request(PendingRequest* preq, web::WebTask req);
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
    void handleStage1Error(PendingRequest* req, std::string error);

public:
    void handleStage2Error(PendingRequest* req, std::string error);
};

} // namespace argon
