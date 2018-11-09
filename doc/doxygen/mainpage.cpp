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
 * | Library       | Description                                     | Dependencies         |
 * |---------------|-------------------------------------------------|----------------------|
 * | libirccd-core | Common utilities                                | libjson              |
 * | libirccd-ctl  | Classes to connect to irccd instance            | libirccd             |
 * | libirccd-test | Tools to create unit tests                      | libirccd,libirccd-js |
 * | libirccd-js   | Libraries to create Javascript APIs and plugins | libirccd, libduktape |
 * | libirccd      | Everything related to irccd instance            | libirccd-core        |
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
