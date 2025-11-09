#pragma once

#include <argon/argon.hpp>
#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>

namespace argon::web {
    using WebTask = geode::utils::web::WebTask;
    using WebRequest = geode::utils::web::WebRequest;
    using WebResponse = geode::utils::web::WebResponse;
    using WebProgress = geode::utils::web::WebProgress;
    using WebListener = geode::EventListener<geode::utils::web::WebTask>;

    std::string getBaseServerUrl();
    std::string getUserAgent();

    WebTask startStage1(const AccountData& account, std::string_view preferredMethod, bool forceStrong = false);
    WebTask restartStage1(const AccountData& account, std::string_view preferredMethod, bool forceStrong = false);
    WebTask startStage2Message(const AccountData& account, std::string_view serverUrl, int id, std::string_view solution);
    WebTask startStage2Comment(const AccountData& account, std::string_view serverUrl, int id, std::string_view solution);
    WebTask startMessageLimitCheck(const AccountData& account, std::string_view serverUrl);

    WebTask stage2MessageCleanup(const AccountData& account, int id, std::string_view serverUrl);
    WebTask stage2CommentCleanup(const AccountData& account, int id, std::string_view serverUrl);

    WebTask startStage3(const AccountData& account, uint32_t challengeId, std::string_view solution);
    WebTask pollStage3(const AccountData& account, uint32_t challengeId, std::string_view solution);
}