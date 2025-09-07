#include "web.hpp"

#include "state.hpp"

#include <Geode/binding/GJMoreGamesLayer.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

namespace argon::web {

// TODO: reverse engineer the "encryption" and replace this all with geode base64

static std::string base64Encode(const gd::string& data) {
    // trust me i did not want to use this but oh well
    std::string ret = cocos2d::ZipUtils::base64URLEncode(data);
    ret.resize(strlen(ret.c_str()));

    return ret;
}

static std::string base64EncodeEnc(const gd::string& data, gd::string key) {
    std::string ret = ZipUtils::base64EncodeEnc(data, key);
    ret.resize(strlen(ret.c_str()));

    return ret;
}

static const char* platformString() {
#ifdef GEODE_IS_MACOS
# ifdef GEODE_IS_ARM_MAC
    return "MacOS arm64";
# else
    return "MacOS x86_64";
# endif
#elif defined GEODE_IS_WINDOWS
    return "Windows";
#elif defined GEODE_IS_ANDROID64
    return "Android64";
#elif defined GEODE_IS_ANDROID32
    return "Android32";
#elif defined GEODE_IS_IOS
    return "iOS";
#else
    return "Unknown";
#endif
}

std::string getUserAgent() {
    return fmt::format("argon/v{} ({}, Geode {}, GD {})",
            ARGON_VERSION,
            platformString(),
            Loader::get()->getVersion(),
            Loader::get()->getGameVersion());
}

WebTask startStage1(const AccountData& account, std::string_view preferredMethod, bool forceStrong) {
    auto& argon = ArgonState::get();
    std::string_view serverUrl = argon.getServerUrl();

    auto payload = matjson::makeObject({
        {"accountId", account.accountId},
        {"userId", account.userId},
        {"username", account.username},
        {"forceStrong", forceStrong},
        {"reqMod", Mod::get()->getID()},
        {"preferred", preferredMethod}
    });

    auto req = web::WebRequest()
        .userAgent(getUserAgent())
        .certVerification(argon.getCertVerification())
        .timeout(std::chrono::seconds(10))
        .bodyJSON(payload)
        .post(fmt::format("{}/v1/challenge/start", serverUrl));

    return std::move(req);
}

WebTask restartStage1(const AccountData& account, std::string_view preferredMethod, bool forceStrong) {
    auto& argon = ArgonState::get();
    std::string_view serverUrl = argon.getServerUrl();

    auto payload = matjson::makeObject({
        {"accountId", account.accountId},
        {"userId", account.userId},
        {"username", account.username},
        {"forceStrong", forceStrong},
        {"reqMod", Mod::get()->getID()},
        {"preferred", preferredMethod}
    });

    auto req = web::WebRequest()
        .userAgent(getUserAgent())
        .certVerification(argon.getCertVerification())
        .timeout(std::chrono::seconds(10))
        .bodyJSON(payload)
        .post(fmt::format("{}/v1/challenge/restart", serverUrl));

    return std::move(req);
}

WebTask startStage2Message(const AccountData& account, std::string_view serverUrl, int id, std::string_view solution) {
    std::string text = fmt::format("#ARGON# {}", solution);

    auto payload = fmt::format(
        "accountID={}&gjp2={}&gameVersion=22&binaryVersion=45"
        "&secret=Wmfd2893gb7&toAccountID={}&subject={}&body={}",
        account.accountId, account.gjp2, id, base64Encode(text),
        base64EncodeEnc("This is a message sent to verify your account, it can be safely deleted.", "14251")
    );

    // Upload a message to the GD bot account
    auto req = web::WebRequest()
        .certVerification(getCertVerification())
        .timeout(std::chrono::seconds(20))
        .bodyString(payload)
        .userAgent("")
        .post(fmt::format("{}/uploadGJMessage20.php", serverUrl));

    return std::move(req);
}

WebTask startStage2Comment(const AccountData& account, std::string_view serverUrl, int id, std::string_view solution) {
    return {}; // TODO
}

WebTask stage2MessageCleanup(const AccountData& account, int id, std::string_view serverUrl) {
    auto payload = fmt::format(
        "accountID={}&gjp2={}&gameVersion=22&binaryVersion=45"
        "&secret=Wmfd2893gb7&isSender=1&messageID={}",
        account.accountId, account.gjp2, id
    );

    // delete the message
    auto req = web::WebRequest()
        .certVerification(getCertVerification())
        .timeout(std::chrono::seconds(20))
        .bodyString(payload)
        .userAgent("")
        .post(fmt::format("{}/deleteGJMessages20.php", serverUrl));

    return std::move(req);

}

WebTask stage2CommentCleanup(const AccountData& account, int id, std::string_view serverUrl) {
    return {}; // TODO
}

WebTask startStage3(const AccountData& account, uint32_t challengeId, std::string_view solution) {
    auto& argon = ArgonState::get();
    std::string_view serverUrl = argon.getServerUrl();

    auto payload = matjson::makeObject({
        {"challengeId", challengeId},
        {"accountId", account.accountId},
        {"solution", solution}
    });

    auto req = web::WebRequest()
        .userAgent(getUserAgent())
        .certVerification(argon.getCertVerification())
        .timeout(std::chrono::seconds(10))
        .bodyJSON(payload)
        .post(fmt::format("{}/v1/challenge/verify", serverUrl));

    return std::move(req);
}

WebTask pollStage3(const AccountData& account, uint32_t challengeId, std::string_view solution) {
    auto& argon = ArgonState::get();
    std::string_view serverUrl = argon.getServerUrl();

    auto payload = matjson::makeObject({
        {"challengeId", challengeId},
        {"accountId", account.accountId},
        {"solution", solution}
    });

    auto req = web::WebRequest()
        .userAgent(getUserAgent())
        .certVerification(argon.getCertVerification())
        .timeout(std::chrono::seconds(10))
        .bodyJSON(payload)
        .post(fmt::format("{}/v1/challenge/verifypoll", serverUrl));

    return std::move(req);
}

}
