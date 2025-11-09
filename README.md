# Argon

API for authenticating Geometry Dash accounts. Made by creators of Globed, and is completely free and open-source, including the [server](https://github.com/GlobedGD/argon-server). Can be selfhosted (for example for a GDPS), but we provide our server at https://argon.globed.dev and this library uses it by default.

Benefits compared to some of the other auth APIs (Globed, DashAuth, GDAuth):

* Does not send your GJP or password to any 3rd party server
* Tokens are stored globally, meaning that authentication is done only once even if the user has multiple mods installed that use Argon
* Integrated in your mod, user does not need to install any dependencies
* Smooth UX for switching accounts (will not try to use or overwrite the token of the other account)
* Our official instance is whitelisted by RobTop, which means authentication is faster and more reliable against IP blocks
* Challenges don't entirely rely on the IP address, but are still secured in other ways, preventing errors if the user has a weird ISP
* Ability to retrieve user's username without making requests to the GD server (with just their token)
* Automatic troubleshooter for figuring out the cause of auth issues (for example invalid session or too many sent messages)

## Usage (client-side)

First, add Argon to the `CMakeLists.txt` of your mod:

```cmake
CPMAddPackage("gh:GlobedGD/argon@1.2.1")
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

If you prefer to use tasks over callbacks, task API is available since 1.2.0:

```cpp
$on_mod(Loaded) {
    $async() {
        auto res = co_await argon::startAuth();

        if (res.isOk()) {
            auto token = std::move(res).unwrap();
            log::debug("Token: {}", token);
        } else {
            log::warn("Auth failed: {}", res.unwrapErr());
        }
    };
}
```

If running into token validation issues, tokens can be cleared to attempt authentication again:

```cpp
argon::clearToken();
```

If you want to use a custom server instead of the one provided by us, the URL can be set like so:

```cpp
// this unwrap is safe as long as *no* auth requests are currently running
argon::setServerUrl("http://localhost:4341").unwrap();
```

Few more functions are provided for managing tokens and for ensuring thread safety, you can find out about the rest of the functionality by reading the docstrings in `include/argon/argon.hpp` header.

## Usage (server-side)

The base url for our Argon server is https://argon.globed.dev

After the user has generated an authtoken, they should send it to your server, which in turn will send the token to the Argon server for validation. Server-side API has its own [documentation](https://github.com/GlobedGD/argon-server/blob/main/docs/server-api.md) which describes in detail how to use it. You should read these docs, but here's a quick start in Python that you can adapt to other languages:

```py
import requests

BASE_URL = "https://argon.globed.dev/v1"

def validate(account_id: int, token: str):
    # This function will validate the account ID and token that your user sent to your server.

    r = requests.get(f"{BASE_URL}/validation/check?account_id={account_id}&authtoken={token}")

    # make sure to check the status code is 200, otherwise return an error to the user!
    if r.status_code != 200:
        raise ValueError(f"Error from argon (code {r.status_code}): {r.text}")

    # check actual token validity
    resp = r.json()
    if not resp["valid"]:
        raise ValueError(f"Invalid token: {resp["cause"]}")

    # if resp["valid"] is true, this means the token is valid and we can give user access :)
    return

def validate_strong(account_id: int, user_id: int, username: str, token: str):
    # This function will validate the account ID, user ID and username that the user sent to you.
    # It's strongly advised that you read the notes section before using the strong endpoint,
    # because it has some caveats!
    # https://github.com/GlobedGD/argon-server/blob/main/docs/server-api.md#get-v1validationcheck_strong

    r = requests.get(
        f"{BASE_URL}/validation/check_strong?account_id={account_id}&user_id={user_id}&username={username}&authtoken={token}"
    )

    # make sure to check the status code
    if r.status_code != 200:
        raise ValueError(f"Error from argon (code {r.status_code}): {r.text}")

    # check actual token validity
    resp = r.json()

    strong_valid = resp["valid"]
    weak_valid = resp["valid_weak"]

    # if valid_weak is false, then the token is completely invalid or the user is impersonating
    if not weak_valid:
        raise ValueError(f"Invalid token: {resp["cause"]}")

    # if valid is false but valid_weak is true, the most likely reason is an invalid client-side username.
    # tell the user to refresh login in GD account settings.
    if not strong_valid:
        raise ValueError("Mismatched username, please refresh login in account settings")

    # if both valid and valid_weak are true, this means the username validation passed successfully :)
    # you can retrieve the user's *actual* username from the response as well:

    username = resp["username"]

    return
```
