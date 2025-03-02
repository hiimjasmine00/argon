#pragma once
#include "state.hpp"
#include <Geode/utils/web.hpp>

namespace argon {
    void stage2Start(PendingRequest* req, int id, int challenge);

    void stage2StartMessage(PendingRequest* req, int id, int challenge);
    void stage2StartComment(PendingRequest* req, int id, int challenge);
}
