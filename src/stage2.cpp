#include "stage2.hpp"

#include "web.hpp"

using namespace geode::prelude;

namespace argon {

void stage2Start(PendingRequest* req, int id, int challenge) {
    auto& argon = ArgonState::get();

    if (req->stage2ChosenMethod == "message") {
        stage2StartMessage(req, id, challenge);
    } else if (req->stage2ChosenMethod == "comment") {
        stage2StartComment(req, id, challenge);
    } else {
        argon.handleStage2Error(req, fmt::format("Challenge method \"{}\" unsupported by this client", req->stage2ChosenMethod));
    }
}

void stage2StartMessage(PendingRequest* preq, int id, int challenge) {
    auto& argon = ArgonState::get();

    auto task = argon::web::startStage2Message(preq->account, id, challenge);

    argon.pushStage2Request(preq, std::move(task));
}

void stage2StartComment(PendingRequest* preq, int id, int challenge) {
    auto& argon = ArgonState::get();

    auto task = argon::web::startStage2Comment(preq->account, id, challenge);

    argon.pushStage2Request(preq, std::move(task));
}

} // namespace argon
