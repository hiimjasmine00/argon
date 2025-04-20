#include "stages.hpp"

#include "web.hpp"
#include "challenge.hpp"

using namespace geode::prelude;

namespace argon {

void stage2Start(PendingRequest* req, int id, int challenge) {
    auto& argon = ArgonState::get();
    auto solution = solveChallenge(challenge);

    req->challengeSolution = std::move(solution);
    if (req->stage2ChosenMethod == "message") {
        stage2StartMessage(req, id, req->challengeSolution);
    } else if (req->stage2ChosenMethod == "comment") {
        stage2StartComment(req, id, req->challengeSolution);
    } else {
        argon.handleStage2Error(req, fmt::format("Challenge method \"{}\" unsupported by this client", req->stage2ChosenMethod));
    }
}

void stage2StartMessage(PendingRequest* preq, int id, std::string_view solution) {
    auto& argon = ArgonState::get();

    auto task = argon::web::startStage2Message(preq->account, preq->account.serverUrl, id, solution);

    argon.pushStage2Request(preq, std::move(task));
}

void stage2StartComment(PendingRequest* preq, int id, std::string_view solution) {
    auto& argon = ArgonState::get();

    auto task = argon::web::startStage2Comment(preq->account, preq->account.serverUrl, id, solution);

    argon.pushStage2Request(preq, std::move(task));
}

void stage2Cleanup(PendingRequest* req) {
    req->stage2ChosenMethod == "message" ? stage2MessageCleanup(req) : stage2CommentCleanup(req);
}

void stage2MessageCleanup(PendingRequest* req) {
    auto& argon = ArgonState::get();

    argon::web::stage2MessageCleanup(req->account, req->userCommentId, req->account.serverUrl)
        .listen([](web::WebResponse* resp) {
            auto respstr = resp->string().unwrapOrDefault();
            if (!resp->ok() || respstr != "1") {
                log::warn("(Argon) Failed to cleanup message (code {}): {}", resp->code(), resp->string().unwrapOrDefault());
            }
        });
}

void stage2CommentCleanup(PendingRequest* req) {
    auto& argon = ArgonState::get();

    argon::web::stage2CommentCleanup(req->account, req->userCommentId, req->account.serverUrl)
        .listen([](web::WebResponse* resp) {
            auto respstr = resp->string().unwrapOrDefault();
            if (!resp->ok() || respstr != "1") {
                log::warn("(Argon) Failed to cleanup comment (code {}): {}", resp->code(), resp->string().unwrapOrDefault());
            }
        });
}

void stage3Start(PendingRequest* preq) {
    auto& argon = ArgonState::get();

    auto task = argon::web::startStage3(preq->account, preq->challengeId, preq->challengeSolution);

    argon.pushStage3Request(preq, std::move(task));
}

} // namespace argon
