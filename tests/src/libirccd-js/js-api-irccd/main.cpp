/*
 * main.cpp -- test Irccd API
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define BOOST_TEST_MODULE "Irccd Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/test/js_fixture.hpp>

using namespace irccd::js;
using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(irccd_js_api_suite, js_fixture)

BOOST_AUTO_TEST_CASE(version)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "major = Irccd.version.major;"
        "minor = Irccd.version.minor;"
        "patch = Irccd.version.patch;"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "major"));
    BOOST_TEST(IRCCD_VERSION_MAJOR == duk_get_int(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "minor"));
    BOOST_TEST(IRCCD_VERSION_MINOR == duk_get_int(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "patch"));
    BOOST_TEST(IRCCD_VERSION_PATCH == duk_get_int(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(from_javascript)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "try {"
        "  throw new Irccd.SystemError(1, 'test');"
        "} catch (e) {"
        "  errno = e.errno;"
        "  name = e.name;"
        "  message = e.message;"
        "  v1 = (e instanceof Error);"
        "  v2 = (e instanceof Irccd.SystemError);"
        "}"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "errno"));
    BOOST_TEST(1 == duk_get_int(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "name"));
    BOOST_TEST("SystemError" == duk_get_string(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "message"));
    BOOST_TEST("test" == duk_get_string(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "v1"));
    BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "v2"));
    BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(from_native)
{
    duk_push_c_function(plugin_->get_context(), [] (duk_context *ctx) -> duk_ret_t {
        duk::raise(ctx, std::system_error(make_error_code(std::errc::invalid_argument)));

        return 0;
    }, 0);

    duk_put_global_string(plugin_->get_context(), "f");

    const auto ret = duk_peval_string(plugin_->get_context(),
        "try {"
        "  f();"
        "} catch (e) {"
        "  errno = e.errno;"
        "  name = e.name;"
        "  v1 = (e instanceof Error);"
        "  v2 = (e instanceof Irccd.SystemError);"
        "}"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "errno"));
    BOOST_TEST(EINVAL == duk_get_int(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "name"));
    BOOST_TEST("SystemError" == duk_get_string(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "v1"));
    BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "v2"));
    BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
