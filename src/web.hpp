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

    std::string getUserAgent();

    WebTask startStage1(const AccountData& accData, std::string_view preferredMethod);
    WebTask restartStage1(const AccountData& accData, std::string_view preferredMethod);
    WebTask startStage2Message(const AccountData& accData, int id, std::string_view solution);
    WebTask startStage2Comment(const AccountData& accData, int id, std::string_view solution);

    WebTask startStage3(const AccountData& accData, std::string_view solution);
    WebTask pollStage3(const AccountData& accData);
}