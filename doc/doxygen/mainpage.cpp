/**
 * \mainpage
 *
 * Welcome to the irccd C++ API.
 *
 * ## Introduction
 *
 * The irccd libraries let you create your own native plugins but also your own
 * bot from scratch if you want.
 *
 * ## Libraries
 *
 * The irccd libraries are split and described as following:
 *
 * | Library         | Description                                     | Dependencies                 |
 * |-----------------|-------------------------------------------------|------------------------------|
 * | libirccd        | Common utilities                                | libjson                      |
 * | libirccd-ctl    | Classes to connect to irccd instance            | libirccd-daemon              |
 * | libirccd-test   | Tools to create unit tests                      | libirccd-daemon, libirccd-js |
 * | libirccd-js     | Libraries to create Javascript APIs and plugins | libirccd-daemon, libduktape  |
 * | libirccd-daemon | Everything related to irccd instance            | libirccd                     |
 *
 * There is also external libraries shipped with irccd:
 *
 * | Library       | Description          | Link        |
 * |---------------|----------------------|-------------|
 * | libduktape    | Javascript engine    | [duktape][] |
 * | libjson       | Niels Lohmanns' JSON | [json][]    |
 *
 * [duktape]: http://duktape.org
 * [json]: https://github.com/nlohmann/json
 */