#include "ArgonState.hpp"
#include "Web.hpp"
#include "ArgonStorage.hpp"
#include <Geode/binding/GameManager.hpp>

using enum std::memory_order;

namespace argon {

ArgonState::ArgonState() {
    this->setServerUrl("https://argon.globed.dev");
}

void ArgonState::setServerUrl(std::string url) {
    auto lock = m_serverUrl.lock();

    *lock = std::move(url);

    // Strip trailing slash
    while (!lock->empty() && lock->back() == '/') {
        lock->pop_back();
    }
}

std::string ArgonState::getServerUrl() const {
    return *m_serverUrl.lock();
}

std::string ArgonState::makeUrl(std::string_view suffix) const {
    while (suffix.starts_with('/')) {
        suffix.remove_prefix(1);
    }

    return fmt::format("{}/{}", getServerUrl(), suffix);
}

void ArgonState::setCertVerification(bool state) {
    m_certVerification = state;
}

bool ArgonState::getCertVerification() const {
    return m_certVerification.load();
}

std::lock_guard<std::mutex> ArgonState::acquireConfigLock() {
    auto ptr = m_configLock.load(acquire);

    if (!ptr) {
        this->initConfigLock();
        ptr = m_configLock.load(acquire);
    }

    return std::lock_guard(*ptr);
}

void ArgonState::initConfigLock() {
    if (m_configLock.load(acquire)) return;

    // note: this function is horrible and really has to be thread safe :)

    static const std::string LOCK_KEY = "dankmeme.argon/_config_lock_v2_25ea8834";

    auto gm = GameManager::get();

    auto lockobj = geode::cast::typeinfo_cast<CCMutex*>(gm->getUserObject(LOCK_KEY));
    if (!lockobj) {
        lockobj = CCMutex::create();
        gm->setUserObject(LOCK_KEY, lockobj);
    }

    m_configLock.store(&lockobj->data(), release);
}

bool ArgonState::isConfigLockInitialized() {
    return m_configLock.load(acquire) != nullptr;
}

void ArgonState::handleSuccessfulAuth(AccountData account, std::string authToken, std::string serverIdent, int commentId) {
    arc::spawn([
        account = std::move(account),
        authToken = std::move(authToken),
        serverIdent = std::move(serverIdent),
        commentId
    ](this auto self) -> arc::Future<> {
        // save authtoken
        if (auto err = ArgonStorage::get().storeAuthToken(account, serverIdent, authToken).err()) {
            log::warn("(Argon) failed to save authtoken: {}", *err);
        }

        // don't care if the message deletion fails
        if (commentId != 0) {
            (void) co_await web::deleteGDMessage(account, commentId);
        }
    });
}

}
