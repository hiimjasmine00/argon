#include "state.hpp"

#include "stage2.hpp"

#include <matjson/reflect.hpp>

using namespace geode::prelude;

namespace argon {

struct Stage1ResponseData {
    std::string method;
    int id;
    int challenge;
};

ArgonState::ArgonState() {
    this->setServerUrl("https://argon.dankmeme.dev");
}

void ArgonState::setServerUrl(std::string url) {
    serverUrl = std::move(url);

    // Strip trailing slash
    if (!serverUrl.empty() && serverUrl.back() == '/') {
        serverUrl.pop_back();
    }
}

std::string_view ArgonState::getServerUrl() const {
    return serverUrl;
}

void ArgonState::pushNewRequest(AuthCallback callback, AccountData account, web::WebTask req) {
    size_t id = this->getNextRequestId();

    auto preq = new PendingRequest {
        .id = id,
        .callback = std::move(callback),
        .account = std::move(account),
    };

    preq->stage1Listener.bind([preq](web::WebTask::Event* e) {
        auto& argon = ArgonState::get();

        if (web::WebResponse* value = e->getValue()) {
            argon.processStage1Response(preq, value);
        } else if (web::WebProgress* progress = e->getProgress()) {
            // idk we ignore progress for now
        } else if (e->isCancelled()) {
            argon.handleStage1Error(preq, "Request cancelled");
        }
    });

    preq->stage1Listener.setFilter(std::move(req));

    this->pendingRequests.insert(preq);
}

void ArgonState::pushStage2Request(PendingRequest* preq, geode::utils::web::WebTask req) {
    preq->stage2Listener.bind([preq](web::WebTask::Event* e) {
        auto& argon = ArgonState::get();

        if (web::WebResponse* value = e->getValue()) {
            argon.processStage2Response(preq, value);
        } else if (web::WebProgress* progress = e->getProgress()) {
            // idk we ignore progress for now
        } else if (e->isCancelled()) {
            argon.handleStage2Error(preq, "Request cancelled");
        }
    });

    preq->stage2Listener.setFilter(std::move(req));
}

PendingRequest* ArgonState::getRequestById(size_t id) {
    for (auto req : this->pendingRequests) {
        if (req->id == id) {
            return req;
        }
    }

    return nullptr;
}

void ArgonState::cleanupRequest(PendingRequest* req) {
    this->pendingRequests.erase(req);
    delete req;
}

void ArgonState::processStage1Response(PendingRequest* req, web::WebResponse* response) {
    auto res = response->json();

    if (!res) {
        log::warn("(Argon) Stage 1 request failed with code {}, server did not send a JSON, dumping server response.", response->code());
        log::warn("{}", response->string().unwrapOrDefault());
        this->handleStage1Error(req, fmt::format("Unknown server error ({})", response->code()));
        return;
    }

    auto obj = std::move(res).unwrap();
    bool success = obj["success"].asBool().unwrapOr(false);

    if (!success) {
        std::string error = obj["error"].asString().unwrapOr("Malformed server response (no error message)");
        this->handleStage1Error(req, std::move(error));
        return;
    }

    auto datares = obj["data"].as<Stage1ResponseData>();
    if (!datares) {
        this->handleStage1Error(req, "Malformed server response (no 'data' key)");
        return;
    }

    auto data = std::move(datares).unwrap();

    // Start stage 2.
    req->stage2ChosenMethod = data.method;
    argon::stage2Start(req, data.id, data.challenge);
}

void ArgonState::processStage2Response(PendingRequest* req, geode::utils::web::WebResponse* response) {
    auto res = response->string().unwrapOrDefault();
    if (res.empty()) {
        this->handleStage2Error(req, "Server did not send a response");
        return;
    }

    if (!response->ok()) {
        log::warn("(Argon) Stage 2 request failed with code {}, dumping server response.", response->code());
        log::warn("{}", res);
        this->handleStage2Error(req, fmt::format("Server responded with code {}", response->code()));
        return;
    }

    if (res == "-1") {
        this->handleStage2Error(req, "Stage 2 failed (generic error)");
        return;
    }

    // we can assume that sending the message succeeded now, begin stage 3 by asking the server if auth succeeded

}

void ArgonState::restartStage1(PendingRequest* preq) {
    auto task = argon::web::restartStage1(preq->account, preq->stage2ChosenMethod);
    preq->stage1Listener.setFilter(std::move(task));
}

void ArgonState::handleStage1Error(PendingRequest* req, std::string error) {
    req->callback(Err(std::move(error)));
    this->cleanupRequest(req);
}

void ArgonState::handleStage2Error(PendingRequest* req, std::string error) {
    // If we can, we should try another authentication method, before completely failing.
    if (!req->retrying) {
        if (req->stage2ChosenMethod == "message") {
            req->stage2ChosenMethod = "comment";
        } else if (req->stage2ChosenMethod == "comment") {
            req->stage2ChosenMethod = "message";
        }

        log::warn("(Argon) Stage 2 failed, retrying authentication with method \"{}\"", req->stage2ChosenMethod);
        log::warn("(Argon) Fail reason: {}", error);

        req->retrying = true;
        this->restartStage1(req);
        return;
    }

    // otherwise, just fail
    req->callback(Err(std::move(error)));
    this->cleanupRequest(req);
}

size_t ArgonState::getNextRequestId() {
    return nextReqId++;
}

} // namespace argon