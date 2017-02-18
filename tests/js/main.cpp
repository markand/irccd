/*
 * main.cpp -- test duktape.hpp extensions
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <gtest/gtest.h>

#include <duktape.hpp>
#include <fs.hpp>

namespace irccd {

class Test : public testing::Test {
protected:
    UniqueContext m_ctx;
};

/*
 * dukx_peval_file
 * ------------------------------------------------------------------
 */

TEST_F(Test, no_file)
{
    ASSERT_THROW(dukx_peval_file(m_ctx, "nonexistent"), Exception);

    try {
        dukx_peval_file(m_ctx, "nonexistent");
    } catch (const Exception& ex) {
        ASSERT_EQ("Error", ex.name);
        ASSERT_EQ("nonexistent", ex.fileName);
    }
}

TEST_F(Test, syntax_error)
{
    ASSERT_THROW(dukx_peval_file(m_ctx, SOURCEDIR "/syntax-error.js"), Exception);

    try {
        dukx_peval_file(m_ctx, SOURCEDIR "/syntax-error.js");
    } catch (const Exception& ex) {
        ASSERT_EQ("SyntaxError", ex.name);
        ASSERT_EQ("syntax-error.js", fs::baseName(ex.fileName));
        ASSERT_EQ(6, ex.lineNumber);
        ASSERT_EQ("empty expression not allowed (line 6)", ex.message);
    }
}

} // !irccd

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
