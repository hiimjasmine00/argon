# 1.1.4

* Fix `argon::setCertVerification(false)` not actually doing anything :p
* Improve error messages (again!)

# 1.1.3

* Improve error messages further
* Add a way to disable SSL certificate verification for web requests

# 1.1.2

* Slightly improve error messages
* Fix issues on Amazon Store version of GD

# 1.1.1

* Make library usable without precompiled headers

# 1.1.0

* Now return an error if a mod tries to perform auth multiple times at once with the same account
* Bump asp, hopefully should not cause compile issues on MSVC now
* Use a different internal mutex structure

# 1.0.1

Improve iOS support
