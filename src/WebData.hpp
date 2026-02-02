#pragma once
#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>
#include <matjson/std.hpp>
#include <asp/data/Cow.hpp>
#include <string>
#include <stdint.h>

using namespace asp::data;
using namespace geode::prelude;
using geode::utils::web::WebRequest;
using geode::utils::web::WebResponse;

namespace argon::web {

struct Stage1ResponseData {
    std::string method;
    int id;
    uint32_t challengeId;
    int challenge;
    std::string ident;
};

static CowString truncate(std::string_view s, size_t maxSize = 128) {
    if (s.size() >= maxSize) {
        // genius
        std::string out{s.substr(0, maxSize)};
        out.push_back('.');
        out.push_back('.');
        out.push_back('.');

        return CowString::fromOwned(std::move(out));
    } else {
        return CowString::fromBorrowed(s);
    }
}


static std::string wrapError(WebResponse& response, std::string_view what) {
    auto str = response.string().unwrapOrDefault();

    log::warn("(Argon) {} failed (code {})", what, response.code());
    log::warn("Response: '{}'", str);
    log::warn("Curl error message: '{}'", response.errorMessage());

    if (response.code() == -1) {
        // curl error, request did not even reach the server
        std::string fullmsg = std::move(str);
        auto emsg = response.errorMessage();
        if (!emsg.empty()) {
            if (fullmsg.empty()) {
                fullmsg = emsg;
            } else {
                fullmsg += fmt::format(" ({})", emsg);
            }
        }

        if (fullmsg.empty()) {
            fullmsg = "(unknown error, response and error buffer are empty)";
        }

        auto msg = truncate(fullmsg);

        return fmt::format("Request error ({}): {}", what, msg.toBorrowed());
    } else {
        // server error

        auto resp = str;
        if (resp.empty()) {
            resp = "(no response body)";
        }

        auto msg = truncate(resp);
        return fmt::format("Server error ({}, code {}): {}", what, response.code(), msg.toBorrowed());
    }
}

template <typename T>
geode::Result<T> extractData(geode::utils::web::WebResponse& resp) {
    GEODE_UNWRAP_INTO(auto json, resp.json());

    bool success = json["success"].asBool().unwrapOr(false);
    if (!success) {
        auto error = json["error"].asString().unwrapOr("Malformed server response (no error message)");
        return Err(wrapError(resp, std::move(error)));
    }

    return json["data"].as<T>();
}

}

template<>
struct matjson::Serialize<argon::web::Stage1ResponseData> {
    static geode::Result<argon::web::Stage1ResponseData> fromJson(const Value& value) {
        auto method = value["method"].asString();
        auto id = value["id"].as<int>();
        auto challengeId = value["challengeId"].as<uint32_t>();
        auto challenge = value["challenge"].as<int>();
        auto ident = value["ident"].asString();

        if (!method || !id || !challengeId || !challenge || !ident) {
            return geode::Err("Malformed Stage1ResponseData: missing required fields");
        }

        return geode::Ok(argon::web::Stage1ResponseData {
            .method = std::move(method).unwrap(),
            .id = std::move(id).unwrap(),
            .challengeId = std::move(challengeId).unwrap(),
            .challenge = std::move(challenge).unwrap(),
            .ident = std::move(ident).unwrap()
        });
    }
};
