#include <argon/argon.hpp>
#include "state.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace argon {


AccountData getGameAccountData() {
	int accountId = GJAccountManager::get()->m_accountID;
	int userId = GameManager::get()->m_playerUserID;
	std::string username = GJAccountManager::get()->m_username;
	std::string gjp = GJAccountManager::get()->m_GJP2;

	return {
		.accountId = accountId,
		.userId = userId,
		.username = std::move(username),
		.gjp2 = std::move(gjp)
	};
}

Result<> startAuth(AuthCallback callback) {
	auto data = getGameAccountData();
	if (data.accountId <= 0 || data.userId <= 0) {
		return Err("Not logged into a Geometry Dash account");
	}

	return startAuthWithAccount(std::move(callback), std::move(data));
}

Result<> startAuthWithAccount(AuthCallback callback, AccountData account) {
	GEODE_UNWRAP_INTO(auto task, startAuthInternal(std::move(account), "message"));

	auto& argon = ArgonState::get();
	argon.pushNewRequest(std::move(callback), std::move(account), std::move(task));

	return Ok();
}

Result<web::WebTask> startAuthInternal(AccountData account, std::string_view preferredMethod) {
	if (account.accountId <= 0 || account.userId <= 0 || account.username.empty()) {
		return Err("Invalid account data");
	}

	// Start stage 1 authentication.

	return Ok(argon::web::startStage1(account, preferredMethod));
}

Result<> setServerUrl(std::string url) {
	if (url.empty()) {
		return Err("Invalid server URL");
	}

	ArgonState::get().setServerUrl(std::move(url));

	return Ok();
}

}
