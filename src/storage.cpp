#include "storage.hpp"

#include <Geode/loader/Dirs.hpp>
#include <matjson.hpp>
#include <asp/fs.hpp>

using namespace geode::prelude;

static auto storagePath = geode::dirs::getModsSaveDir() / ".dankmeme.argon-data.json";

namespace argon {

ArgonStorage::ArgonStorage() {}

static matjson::Value makeNewConfigFile() {
    return matjson::makeObject({
        {"_ver", 0},
        {"tokens", matjson::Value::array()},
    });
}

static Result<matjson::Value> parseConfigFile(const std::string& data) {
    matjson::Value out;

    auto res = matjson::parse(data);
    if (!res) {
        return Err("parse error: {}", res.unwrapErr());
    }

    out = std::move(res).unwrap();

    // validation
    if (
        !out["_ver"].isNumber()
        || !out["tokens"].isArray()
    ) {
        return Err("invalid structure");
    }

    return Ok(std::move(out));
}

static matjson::Value loadOrCreateConfig() {
    matjson::Value data;

    if (asp::fs::isFile(storagePath)) {
        auto res = asp::fs::readToString(storagePath);
        if (!res) {
            log::warn("(Argon) failed to read argon data file: {}", res.unwrapErr().message());
            data = makeNewConfigFile();
        } else {
            auto res2 = parseConfigFile(res.unwrap());
            if (!res2) {
                log::warn("(Argon) failed to read config file, resetting: {}", res2.unwrapErr());
                data = makeNewConfigFile();
            } else {
                data = std::move(res2).unwrap();
            }
        }
    } else {
        data = makeNewConfigFile();
    }

    data["_ver"] = data["_ver"].asUInt().unwrapOr(0) + 1;

    return data;
}

Result<> ArgonStorage::storeAuthToken(PendingRequest* req, std::string_view authtoken) {
    auto _lock = ArgonState::get().acquireConfigLock();

    auto data = loadOrCreateConfig();

    // find if theres any token that has the same data, replace it instead of adding a new entry
    bool insertedToken = false;

    // parseConfigFile already verified for us that data["tokens"] will be valid
    auto& arr = data["tokens"].asArray().unwrap();

    auto serverUrl = ArgonState::get().getServerUrl();

    for (auto& value : arr) {
        std::string url = value["url"].asString().unwrapOrDefault();
        int accountId = value["accid"].asInt().unwrapOrDefault();
        int userId = value["userid"].asInt().unwrapOrDefault();

        if (url != serverUrl
            || accountId != req->account.accountId
            || userId != req->account.userId
        ) {
            continue;
        }

        // if the same url and account ID, this leaves ident, username and the token fields to be arbitrary
        // and indeed, we will ignore their current values and just replace them
        value["name"] = req->account.username;
        value["ident"] = req->serverIdent;
        value["token"] = authtoken;

        insertedToken = true;
        break;
    }

    if (!insertedToken) {
        arr.push_back(matjson::makeObject({
            {"url", serverUrl},
            {"accid", req->account.accountId},
            {"userid", req->account.userId},
            {"name", req->account.username},
            {"ident", req->serverIdent},
            {"token", authtoken},
        }));
    }

    auto res = asp::fs::write(storagePath, data.dump(matjson::NO_INDENTATION));
    if (!res) {
        return Err(fmt::format("failed to save argon data file: {}", res.unwrapErr().message()));
    }

    return Ok();
}

std::optional<std::string> ArgonStorage::getAuthToken(const AccountData& account, std::string_view serverUrl) {
    auto _lock = ArgonState::get().acquireConfigLock();

    auto data = loadOrCreateConfig();

    // parseConfigFile already verified for us that data["tokens"] will be valid
    auto& arr = data["tokens"].asArray().unwrap();

    for (auto& value : arr) {
        int accountId = value["accid"].asInt().unwrapOrDefault();
        int userId = value["userid"].asInt().unwrapOrDefault();

        if (accountId != account.accountId || userId != account.userId) {
            continue;
        }

        std::string url = value["url"].asString().unwrapOrDefault();

        if (url != serverUrl) {
            continue;
        }

        std::string username = value["name"].asString().unwrapOrDefault();

        if (username != account.username) {
            continue;
        }

        // std::string ident = value["ident"].asString().unwrapOrDefault();
        std::string token = value["token"].asString().unwrapOrDefault();

        return std::make_optional(std::move(token));
    }

    return std::nullopt;
}

bool ArgonStorage::hasAuthToken(const AccountData& account, std::string_view serverUrl) {
    return this->getAuthToken(account, serverUrl).has_value();
}

void ArgonStorage::clearTokens(int accountId) {
    auto _lock = ArgonState::get().acquireConfigLock();

    auto data = loadOrCreateConfig();

    // parseConfigFile already verified for us that data["tokens"] will be valid
    auto& arr = data["tokens"].asArray().unwrap();

    for (int i = arr.size() - 1; i >= 0; i--) {
        auto& val = arr[i];

        if (val["accid"].asInt().unwrapOrDefault() == accountId) {
            arr.erase(arr.begin() + i);
        }
    }

    auto res = asp::fs::write(storagePath, data.dump(matjson::NO_INDENTATION));
    if (!res) {
        log::warn("(Argon) failed to save argon data file: {}", res.unwrapErr().message());
    }
}

void ArgonStorage::clearAllTokens() {
    auto _lock = ArgonState::get().acquireConfigLock();

    auto data = loadOrCreateConfig();
    data["tokens"] = matjson::Value::array();

    auto res = asp::fs::write(storagePath, data.dump(matjson::NO_INDENTATION));
    if (!res) {
        log::warn("(Argon) failed to save argon data file: {}", res.unwrapErr().message());
    }
}

} // namespace argon
