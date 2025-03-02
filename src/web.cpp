#include "web.hpp"

#include "state.hpp"
#include "challenge.hpp"

#include <Geode/loader/Loader.hpp>
#include "external/ServerAPIEvents.hpp"

using namespace geode::prelude;

namespace argon::web {

static std::string base64Encode(const gd::string& data) {
    // trust me i did not want to use this but oh well
    return cocos2d::ZipUtils::base64URLEncode(data);
}

static std::string getBaseServerUrl() {
    if (Loader::get()->isModLoaded("km7dev.server_api")) {
        auto url = ServerAPIEvents::getCurrentServer().url;
        if (!url.empty() && url != "NONE_REGISTERED") {
            while (url.ends_with('/')) {
                url.pop_back();
            }

            return url;
        }
    }

    // This was taken from the impostor mod :)

    // The addresses are pointing to "https://www.boomlings.com/database/getGJLevels21.php"
    // in the main game executable
    char* originalUrl = nullptr;
#ifdef GEODE_IS_WINDOWS
    static_assert(GEODE_COMP_GD_VERSION == 22074, "Unsupported GD version");
    originalUrl = (char*)(base::get() + 0x53ea48);
#elif defined(GEODE_IS_ARM_MAC)
    static_assert(GEODE_COMP_GD_VERSION == 22074, "Unsupported GD version");
    originalUrl = (char*)(base::get() + 0x7749fb);
#elif defined(GEODE_IS_INTEL_MAC)
    static_assert(GEODE_COMP_GD_VERSION == 22074, "Unsupported GD version");
    originalUrl = (char*)(base::get() + 0x8516bf);
#elif defined(GEODE_IS_ANDROID64)
    static_assert(GEODE_COMP_GD_VERSION == 22074, "Unsupported GD version");
    originalUrl = (char*)(base::get() + 0xEA2988);
#elif defined(GEODE_IS_ANDROID32)
    static_assert(GEODE_COMP_GD_VERSION == 22074, "Unsupported GD version");
    originalUrl = (char*)(base::get() + 0x952E9E);
#else
    static_assert(false, "Unsupported platform");
#endif

    std::string ret = originalUrl;
    if(ret.size() > 34) ret = ret.substr(0, 34);

    while (ret.ends_with('/')) {
        ret.pop_back();
    }

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
    return fmt::format("argon/{} ({}, Geode {}, GD {})",
			ARGON_VERSION,
			platformString(),
			Loader::get()->getVersion(),
			Loader::get()->getGameVersion());
}

WebTask startStage1(const AccountData& account, std::string_view preferredMethod) {
	auto& argon = ArgonState::get();
	std::string_view serverUrl = argon.getServerUrl();

	auto payload = matjson::makeObject({
		{"accountId", account.accountId},
		{"userId", account.userId},
		{"username", account.username},
		{"reqMod", Mod::get()->getID()},
		{"preferred", preferredMethod}
	});

	auto req = web::WebRequest()
		.userAgent(getUserAgent())
		.timeout(std::chrono::seconds(10))
		.bodyJSON(payload)
		.post(fmt::format("{}/v1/challenge/start", serverUrl));

    return std::move(req);
}

WebTask restartStage1(const AccountData& account, std::string_view preferredMethod) {
	auto& argon = ArgonState::get();
	std::string_view serverUrl = argon.getServerUrl();

	auto payload = matjson::makeObject({
		{"accountId", account.accountId},
		{"userId", account.userId},
		{"username", account.username},
		{"reqMod", Mod::get()->getID()},
		{"preferred", preferredMethod}
	});

	auto req = web::WebRequest()
		.userAgent(getUserAgent())
		.timeout(std::chrono::seconds(10))
		.bodyJSON(payload)
		.post(fmt::format("{}/v1/challenge/restart", serverUrl));

    return std::move(req);
}

WebTask startStage2Message(const AccountData& account, int id, int challenge) {
    std::string text = fmt::format("#ARGON# {}", solveChallenge(challenge));

    auto payload = fmt::format(
        "accountID={}&gjp2={}&gameVersion=22&binaryVersion=45"
        "&secret=Wmfd2893gb7&toAccountID={}&subject={}&body={}",
        account.accountId, account.gjp2, id, base64Encode(text), base64Encode("This is a verification message, it can be safely deleted.")
    );

    // Upload a message to the GD bot account
    auto req = web::WebRequest()
        .timeout(std::chrono::seconds(20))
        .bodyString(payload)
        .userAgent("")
        .header("Content-Type", "application/x-www-form-urlencoded")
        .post(fmt::format("{}/uploadGJMessage20.php", getBaseServerUrl()));

    return std::move(req);
}

WebTask startStage2Comment(const AccountData& accData, int id, int challenge) {
    return {}; // TODO
}

}