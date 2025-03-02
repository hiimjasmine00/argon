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

    auto task = argon::web::startStage2Message(preq->account, id, solution);

    argon.pushStage2Request(preq, std::move(task));
}

void stage2StartComment(PendingRequest* preq, int id, std::string_view solution) {
    auto& argon = ArgonState::get();

    auto task = argon::web::startStage2Comment(preq->account, id, solution);

    argon.pushStage2Request(preq, std::move(task));
}

void stage3Start(PendingRequest* preq) {
    auto& argon = ArgonState::get();

    auto task = argon::web::startStage3(preq->account, preq->challengeSolution);

    argon.pushStage3Request(preq, std::move(task));
}

} // namespace argon
