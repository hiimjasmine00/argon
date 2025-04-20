# Argon

API for authenticating Geometry Dash accounts. Made by creators of Globed, and is completely free and open-source, including the [server](https://github.com/GlobedGD/argon-server). Can be selfhosted (for example for a GDPS), but we provide our server at https://argon.globed.dev and this library uses it by default.

Benefits compared to some of the other auth APIs (Globed, DashAuth, GDAuth):

* Does not send your GJP or password to any 3rd party server
* Tokens are stored globally, meaning that authentication is done only once even if the user has multiple mods installed that use Argon
* Smooth UX for switching accounts (will not try to use or overwrite the token of the other account)
* Our official instance is whitelisted by RobTop, which means authentication is faster and more reliable against IP blocks
* Challenges don't entirely rely on the IP address, but are still secured in other ways, preventing errors if the user has a weird ISP
* Ability to retrieve user's username without making requests to the GD server (with just their token)
* (TODO, not done yet) Automatic troubleshooter for figuring out the cause of auth issues (for example invalid session or too many sent messages)

## Usage (client-side)

First, add Argon to the `CMakeLists.txt` of your mod:

```cmake
CPMAddPackage("gh:GlobedGD/argon@1.0.0")
target_link_libraries(${PROJECT_NAME} argon)
```

Complete example of how to perform authentication:

```cpp
#include <argon/argon.hpp>
#include <Geode/Geode.hpp>

using namespace geode::prelude;


$on_mod(Loaded) {
    // Do note the authentication is asynchronous, do NOT pass anything in the lambda captures to these callbacks,
    // unless you are certain that object will continue to exist.

    auto res = argon::startAuth([](Result<std::string> res) {
        // This callback is called on the main thread once the authentication completes.
        // Note that if the authtoken is already stored in user's cache,
        // this function will be called immediately!

        if (!res) {
            log::warn("Auth failed: {}", res.unwrapErr());
            return;
        }

        auto token = std::move(res).unwrap();

        // Now you have an authtoken that you can use to verify the user!
        // Send this to your mod's server, which should verify it with the Argon server to ensure it is valid.

        log::debug("Token: {}", token);
    }, [](argon::AuthProgress progress) {
        // This callback is called whenever the client moves to the next step of authentication

        log::info("Auth progress: {}", argon::authProgressToString(progress));
    });

    if (!res) {
        log::warn("Failed to start auth attempt: {}", res.unwrapErr());
    }
}
```

If running into token validation issues, tokens can be cleared to attempt authentication again:

```cpp
argon::clearToken();
```

If using a custom server, the URL can be set like so:

```cpp
// this unwrap is safe as long as *no* auth requests are currently running
argon::setServerUrl("http://localhost:4341").unwrap();
```

Few more functions are provided for managing tokens and for ensuring thread safety, you can find out about the rest of the functionality by reading the docstrings in `include/argon/argon.hpp` header.

## Usage (server-side)

After the user has generated an authtoken, they should send it to your server, which in turn will send the token to the Argon server for validation. Server-side API has its own [documentation](https://github.com/GlobedGD/argon-server/blob/main/docs/server-api.md) which describes in detail how to use it.
