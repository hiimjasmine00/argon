#include "storage.hpp"

using namespace geode::prelude;

namespace argon {

ArgonStorage::ArgonStorage() {}

Result<> ArgonStorage::storeAuthToken(PendingRequest* req, std::string_view authtoken) {
    // TODO
    return Ok();
}

} // namespace argon
