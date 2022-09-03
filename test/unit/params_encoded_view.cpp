//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/url
//

// Test that header file is self-contained.
#include <boost/url/params_encoded_view.hpp>

#include <boost/url/url.hpp>
#include <boost/url/url_view.hpp>
#include <boost/core/ignore_unused.hpp>
#include "test_suite.hpp"

namespace boost {
namespace urls {

/*  Legend

    '#' 0x23
    '%' 0x25
    '&' 0x26
    '=' 0x3d
*/
struct params_encoded_view_test
{
    static
    bool
    is_equal(
        param_pct_view const& p0,
        param_pct_view const& p1)
    {
        return
            p0.key == p1.key &&
            p0.has_value == p1.has_value &&
            (! p0.has_value ||
                p0.value == p1.value);
    }

    static
    void
    check(
        params_encoded_view const& p,
        std::initializer_list<
            param_pct_view> init)
    {
        if(! BOOST_TEST_EQ(
            p.size(), init.size()))
            return;
        auto it0 = p.begin();
        auto it1 = init.begin();
        auto const end = init.end();
        while(it1 != end)
        {
            BOOST_TEST(is_equal(
                *it0, *it1));
            auto tmp = it0++;
            BOOST_TEST_EQ(++tmp, it0);
            ++it1;
        }
        // reverse
        if(init.size() > 0)
        {
            it0 = p.end();
            it1 = init.end();
            do
            {
                auto tmp = it0--;
                BOOST_TEST_EQ(--tmp, it0);
                --it1;
                BOOST_TEST(is_equal(
                    *it0, *it1));
            }
            while(it1 != init.begin());
        }
    }

    static
    void
    check(
        string_view s,
        std::initializer_list<
            param_pct_view> init)
    {
        url u("http://user:pass@www.example.com/path/to/file.txt?k=v#f");
        if(s.empty())
            u.remove_query();
        else
            u.set_encoded_query(
                s.substr(1));
        params_encoded_view p = u.encoded_params();
        if(! BOOST_TEST_EQ(
            p.size(), init.size()))
            return;
        auto it0 = p.begin();
        auto it1 = init.begin();
        auto const end = init.end();
        while(it1 != end)
        {
            if(! BOOST_TEST(is_equal(
                    *it0, *it1)))
                return;
            ++it0;
            ++it1;
        }
        // reconstruct u
        url u2("http://user:pass@www.example.com/path/to/file.txt?k=v#f");
        u2.encoded_params() = init;
        BOOST_TEST_EQ(u2.encoded_query(), u.encoded_query());
        check(u2.encoded_params(), init);
    }

    static
    void
    modify(
        string_view s0,
        string_view s1,
        void(*f)(params_encoded_view&))
     {
        url u("http://user:pass@www.example.com/path/to/file.txt?k=v#f");
        if(! s0.data())
        {
            u.remove_query();
            BOOST_TEST_EQ(u.encoded_query(), "");
        }
        else
        {
            u.set_encoded_query(s0);
            BOOST_TEST_EQ(u.encoded_query(), s0);
        }
        auto p = u.encoded_params();
        f(p);
        if(! s1.data())
        {
            BOOST_TEST(! u.has_query());
            BOOST_TEST_EQ(u.encoded_query(), "");
            BOOST_TEST(u.query() == "");
        }
        else
        {
            BOOST_TEST(u.has_query());
            BOOST_TEST_EQ(u.encoded_query(), s1);
        }
    }

    //--------------------------------------------

    static
    void
    assign(
        params_encoded_view& p,
        std::initializer_list<param_pct_view> init)
    {
        p.assign(
            init.begin(), init.end());
    };

    static
    auto
    append(
        params_encoded_view& p,
        std::initializer_list<param_pct_view> init) ->
            params_encoded_view::iterator
    {
        return p.append(
            init.begin(), init.end());
    };

    static
    auto
    insert(
        params_encoded_view& p,
        params_encoded_view::iterator before,
        std::initializer_list<param_pct_view> init) ->
            params_encoded_view::iterator
    {
        return p.insert(before,
            init.begin(), init.end());
    };

    static
    auto
    replace(
        params_encoded_view& p,
        params_encoded_view::iterator from,
        params_encoded_view::iterator to,
        std::initializer_list<param_pct_view> init) ->
            params_encoded_view::iterator
    {
        return p.replace(from, to,
            init.begin(), init.end());
    };

    //--------------------------------------------

    void
    testSpecial()
    {
        BOOST_STATIC_ASSERT(
            ! std::is_default_constructible<
                params_encoded_view>::value);

        // params_encoded_view(params_encoded_view)
        {
            url u;
            params_encoded_view p0 = u.encoded_params();
            BOOST_TEST_EQ(&p0.url(), &u);
            params_encoded_view p1(p0);
            BOOST_TEST_EQ(
                &p0.url(), &p1.url());
            check(p0, {});
            check(p1, {});
        }

        // operator=(params_encoded_view)
        {
            url u0, u1;
            params_encoded_view p0 = u0.encoded_params();
            params_encoded_view p1 = u1.encoded_params();
            p1 = p0;
            BOOST_TEST_EQ(&p0.url(), &p1.url());
            check(p0, {});
            check(p1, {});
        }
    }

    void
    testObservers()
    {
        // url()
        {
            url u;
            BOOST_TEST_EQ(&u.encoded_params().url(), &u);
        }

        // empty()
        {
            {
                url u;
                BOOST_TEST(u.encoded_params().empty());
                check(u.encoded_params(), {});
            }
            {
                url u("?");
                BOOST_TEST(! u.encoded_params().empty());
                check(u.encoded_params(), {{""}});
            }
            {
                url u("?k=v");
                BOOST_TEST(! u.encoded_params().empty());
                check(u.encoded_params(), {{"k", "v"}});
            }
        }

        // size()
        {
            {
                url u;
                BOOST_TEST_EQ(
                    u.encoded_params().size(), 0);
            }
            {
                url u("?");
                BOOST_TEST_EQ(
                    u.encoded_params().size(), 1);
            }
            {
                url u("?k=v&x=y");
                BOOST_TEST_EQ(
                    u.encoded_params().size(), 2);
            }
            {
                url u("?k0=0&k1=1&k2=&k3&k4=4444#f");
                BOOST_TEST_EQ(
                    u.encoded_params().size(), 5);
                check(u.encoded_params(), {
                    { "k0", "0" },
                    { "k1", "1" },
                    { "k2", "" },
                    { "k3" },
                    { "k4", "4444" }});
            }
        }

        // begin()
        {
            {
                url u;
                BOOST_TEST_EQ(
                    u.encoded_params().begin(),
                    u.encoded_params().begin());
            }
            {
                url u("?");
                BOOST_TEST_NE(
                    u.encoded_params().begin(),
                    u.encoded_params().end());
            }
        }

        // end()
        {
            {
                url u;
                BOOST_TEST_EQ(
                    u.encoded_params().end(),
                    u.encoded_params().end());
            }
            {
                url u("?");
                BOOST_TEST_NE(
                    u.encoded_params().end(),
                    u.encoded_params().begin());
            }
        }

        {
            url u0("?x=1&y=2&x=3&z=4");
            url u1("?%78=1&%79=2&%78=3&%7a=4");
            params_encoded_view p0 = u0.encoded_params();
            params_encoded_view p1 = u1.encoded_params();

            // contains
            BOOST_TEST(p0.contains("x"));
            BOOST_TEST(p1.contains("x"));
            BOOST_TEST(! p0.contains("X"));
            BOOST_TEST(! p1.contains("X"));
            BOOST_TEST(
                p0.contains("X", ignore_case));
            BOOST_TEST(
                p1.contains("X", ignore_case));

            // count
            BOOST_TEST_EQ(p0.count("x"), 2);
            BOOST_TEST_EQ(p0.count("X"), 0);
            BOOST_TEST_EQ(p1.count("%78"), 2);
            BOOST_TEST_EQ(p1.count("%58"), 0);
            BOOST_TEST_EQ(
                p0.count("x", ignore_case), 2);
            BOOST_TEST_EQ(
                p1.count("%58", ignore_case), 2);

            // find
            BOOST_TEST_EQ(
                p0.find("x"), p0.begin());
            BOOST_TEST_EQ(
                p1.find("x"), p1.begin());
            BOOST_TEST_EQ(
                p0.find("X", ignore_case),
                p0.begin());
            BOOST_TEST_EQ(
                p1.find("X", ignore_case),
                p1.begin());

            // find(from)
            BOOST_TEST_EQ(
                p0.find(std::next(p0.begin()), "x"),
                std::next(p0.begin(), 2));
            BOOST_TEST_EQ(
                p1.find(std::next(p1.begin()), "x"),
                std::next(p1.begin(), 2));
            BOOST_TEST_EQ(
                p0.find(std::next(p0.begin()),
                    "X", ignore_case),
                std::next(p0.begin(), 2));
            BOOST_TEST_EQ(
                p1.find(std::next(p1.begin()),
                    "X", ignore_case),
                std::next(p1.begin(), 2));
        }

        // (various)
        {
            url u(
                "?a=1&%62=2&c=3&c=4"
                "&c=5&d=6&e=7&d=8&f=9#f");
            params_encoded_view p = u.encoded_params();
            BOOST_TEST_EQ(p.count("a"), 1);
            BOOST_TEST_EQ(p.count("b"), 1);
            BOOST_TEST_EQ(p.count("c"), 3);
            BOOST_TEST_EQ(p.count("d"), 2);
            BOOST_TEST_EQ(p.count("e"), 1);
            BOOST_TEST_EQ(p.count("f"), 1);

            BOOST_TEST_EQ(p.count("g"), 0);
            BOOST_TEST_EQ(p.count("A"), 0);
            BOOST_TEST_EQ(p.count("B"), 0);
            BOOST_TEST_EQ(p.count("C"), 0);
            BOOST_TEST_EQ(p.count("D"), 0);
            BOOST_TEST_EQ(p.count("E"), 0);
            BOOST_TEST_EQ(p.count("F"), 0);
            BOOST_TEST_EQ(p.count("G"), 0);

            BOOST_TEST_EQ(
                p.count("A", ignore_case), 1);
            BOOST_TEST_EQ(
                p.count("B", ignore_case), 1);
            BOOST_TEST_EQ(
                p.count("C", ignore_case), 3);
            BOOST_TEST_EQ(
                p.count("D", ignore_case), 2);
            BOOST_TEST_EQ(
                p.count("E", ignore_case), 1);
            BOOST_TEST_EQ(
                p.count("F", ignore_case), 1);
            BOOST_TEST_EQ(
                p.count("G", ignore_case), 0);

            BOOST_TEST(p.contains("a"));
            BOOST_TEST(p.contains("b"));
            BOOST_TEST(p.contains("c"));
            BOOST_TEST(p.contains("d"));
            BOOST_TEST(p.contains("e"));
            BOOST_TEST(p.contains("f"));
            BOOST_TEST(! p.contains("g"));

            BOOST_TEST(! p.contains("A"));
            BOOST_TEST(! p.contains("B"));
            BOOST_TEST(! p.contains("C"));
            BOOST_TEST(! p.contains("D"));
            BOOST_TEST(! p.contains("E"));
            BOOST_TEST(! p.contains("F"));
            BOOST_TEST(! p.contains("G"));

            BOOST_TEST(
                p.contains("A", ignore_case));
            BOOST_TEST(
                p.contains("B", ignore_case));
            BOOST_TEST(
                p.contains("C", ignore_case));
            BOOST_TEST(
                p.contains("D", ignore_case));
            BOOST_TEST(
                p.contains("E", ignore_case));
            BOOST_TEST(
                p.contains("F", ignore_case));
            BOOST_TEST(
                ! p.contains("G", ignore_case));
        }
    }

    void
    testModifiers()
    {
        using std::next;

        //----------------------------------------

        //
        // operator=(initializer_list)
        //

        modify(
            {},
            {},
            [](params_encoded_view& p)
            {
                p = {};
            });

        modify(
            {},
            "y",
            [](params_encoded_view& p)
            {
                p = {{ "y" }};
            });

        modify(
            {},
            "y=",
            [](params_encoded_view& p)
            {
                p = {{ "y", "" }};
            });

        modify(
            {},
            "y=g",
            [](params_encoded_view& p)
            {
                p = {{ "y", "g" }};
            });

        modify(
            {},
            "y=g&",
            [](params_encoded_view& p)
            {
                p = {{ "y", "g" }, {}};
            });

        modify(
            {},
            "y=g&z",
            [](params_encoded_view& p)
            {
                p = {{ "y", "g" }, { "z" }};
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            {},
            [](params_encoded_view& p)
            {
                p = {};
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y",
            [](params_encoded_view& p)
            {
                p = {{ "y" }};
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=",
            [](params_encoded_view& p)
            {
                p = {{ "y", "" }};
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g",
            [](params_encoded_view& p)
            {
                p = {{ "y", "g" }};
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&",
            [](params_encoded_view& p)
            {
                p = {{ "y", "g" }, {}};
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&z",
            [](params_encoded_view& p)
            {
                p = {{ "y", "g" }, { "z" }};
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "%23&%26==&%3d=%26",
            [](params_encoded_view& p)
            {
                // encodings
                p = {{ "#" }, { "&", "=" }, { "=", "&" }};
            });
#endif

        //----------------------------------------

        //
        // clear
        //

        modify(
            "",
            {},
            [](params_encoded_view& p)
            {
                p.clear();
                BOOST_TEST(! p.url().has_query());
            });

        modify(
            "key",
            {},
            [](params_encoded_view& p)
            {
                p.clear();
                BOOST_TEST(! p.url().has_query());
            });

        modify(
            "key=",
            {},
            [](params_encoded_view& p)
            {
                p.clear();
                BOOST_TEST(! p.url().has_query());
            });

        modify(
            "key=value",
            {},
            [](params_encoded_view& p)
            {
                p.clear();
                BOOST_TEST(! p.url().has_query());
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            {},
            [](params_encoded_view& p)
            {
                p.clear();
                BOOST_TEST(! p.url().has_query());
            });

        //----------------------------------------

        //
        // assign(initializer_list)
        //

        modify(
            {},
            {},
            [](params_encoded_view& p)
            {
                p.assign({});
            });

        modify(
            {},
            "y",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y" }});
            });

        modify(
            {},
            "y=",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "" }});
            });

        modify(
            {},
            "y=g",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "g" }});
            });

        modify(
            {},
            "y=g&",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "g" }, {} });
            });

        modify(
            {},
            "y=g&z",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "g" }, { "z" } });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            {},
            [](params_encoded_view& p)
            {
                p.assign({});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y" }});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "" }});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "g" }});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "g" }, {} });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&z",
            [](params_encoded_view& p)
            {
                p.assign(
                    {{ "y", "g" }, { "z" } });
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "%23&%26==&%3d=%26",
            [](params_encoded_view& p)
            {
                // encodings
                p.assign(
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //
        // assign(FwdIt, FwdIt)
        //

        modify(
            {},
            {},
            [](params_encoded_view& p)
            {
                p.assign({});
            });

        modify(
            {},
            "y",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y" }});
            });

        modify(
            {},
            "y=",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "" }});
            });

        modify(
            {},
            "y=g",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "g" }});
            });

        modify(
            {},
            "y=g&",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "g" }, {} });
            });

        modify(
            {},
            "y=g&z",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "g" }, { "z" } });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            {},
            [](params_encoded_view& p)
            {
                assign(p, {});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y" }});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "" }});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "g" }});
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "g" }, {} });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&z",
            [](params_encoded_view& p)
            {
                assign(p,
                    {{ "y", "g" }, { "z" } });
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "%23&%26==&%3d=%26",
            [](params_encoded_view& p)
            {
                // encodings
                assign(p,
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //----------------------------------------

        //
        // append(param_pct_view)
        //

        modify(
            {},
            "y",
            [](params_encoded_view& p)
            {
                auto it = p.append(
                    { "y" });
                BOOST_TEST(is_equal(
                    *it, { "y" }));
            });

        modify(
            {},
            "y=",
            [](params_encoded_view& p)
            {
                auto it = p.append(
                    { "y", "" });
                BOOST_TEST(is_equal(
                    *it, { "y", "" }));
            });

        modify(
            {},
            "y=g",
            [](params_encoded_view& p)
            {
                auto it = p.append(
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "",
            "&y",
            [](params_encoded_view& p)
            {
                auto it = p.append(
                    { "y" });
                BOOST_TEST(is_equal(
                    *it, { "y" }));
            });

        modify(
            "",
            "&y=",
            [](params_encoded_view& p)
            {
                auto it = p.append(
                    { "y", "" });
                BOOST_TEST(is_equal(
                    *it, { "y", "" }));
            });

        modify(
            "",
            "&y=g",
            [](params_encoded_view& p)
            {
                auto it = p.append(
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "",
            "&key=value",
            [](params_encoded_view& p)
            {
                // should not go through
                // initializer_list overload
                auto it = p.append({
                    string_view("key"),
                    string_view("value")});
                BOOST_TEST(is_equal(
                    *it, { "key", "value" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&%23",
            [](params_encoded_view& p)
            {
                // encodings
                p.append({ "#" });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&%26==",
            [](params_encoded_view& p)
            {
                // encodings
                p.append({ "&", "=" });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&%3d=%26",
            [](params_encoded_view& p)
            {
                // encodings
                p.append({ "=", "&" });
            });
#endif

        //
        // append(FwdIt, FwdIt)
        //

        modify(
            {},
            "y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = append(p,
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "",
            "&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = append(p,
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "t",
            "t&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = append(p,
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&%23&%26==&%3d=%26",
            [](params_encoded_view& p)
            {
                // encodings
                append(p,
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //
        // append_list(initializer_list)
        //

        modify(
            {},
            "y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = p.append_list(
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "",
            "&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = p.append_list(
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "t",
            "t&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = p.append_list(
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&%23&%26==&%3d=%26",
            [](params_encoded_view& p)
            {
                // encodings
                p.append_list(
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //----------------------------------------

        //
        // insert(iterator, param_pct_view)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&k0=0&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert(
                    std::next(p.begin(), 0),
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&k0=0&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert(
                    std::next(p.begin(), 0),
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&y=g&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert(
                    std::next(p.begin(), 1),
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&y=g&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert(
                    std::next(p.begin(), 2),
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&y=g&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert(
                    std::next(p.begin(), 3),
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&y=g&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert(
                    std::next(p.begin(), 4),
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&y=g",
            [](params_encoded_view& p)
            {
                auto it = p.insert(
                    std::next(p.begin(), 5),
                    { "y", "g" });
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&key=value&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // should not go through
                // initializer_list overload
                auto it = p.insert(
                    std::next(p.begin(), 2),
                    {string_view("key"),
                        string_view("value")});
                BOOST_TEST(is_equal(
                    *it, { "key", "value" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&%23&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                p.insert(std::next(p.begin(), 2),
                    { "#" });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&%26==&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                p.insert(std::next(p.begin(), 2),
                    { "&", "=" });
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&%3d=%26&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                p.insert(std::next(p.begin(), 2),
                    { "=", "&" });
            });
#endif

        //
        // insert(iterator, FwdIt, FwdIt)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&z=q&k0=0&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = insert(p,
                    std::next(p.begin(), 0),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&y=g&z=q&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = insert(p,
                    std::next(p.begin(), 1),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&y=g&z=q&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = insert(p,
                    std::next(p.begin(), 2),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&y=g&z=q&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = insert(p,
                    std::next(p.begin(), 3),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&y=g&z=q&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = insert(p,
                    std::next(p.begin(), 4),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = insert(p,
                    std::next(p.begin(), 5),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&%23&%26==&%3d=%26&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                insert(p, std::next(p.begin(), 2),
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //
        // insert_list(iterator, initializer_list)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&z=q&k0=0&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert_list(
                    std::next(p.begin(), 0),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&y=g&z=q&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert_list(
                    std::next(p.begin(), 1),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&y=g&z=q&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert_list(
                    std::next(p.begin(), 2),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&y=g&z=q&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert_list(
                    std::next(p.begin(), 3),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&y=g&z=q&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.insert_list(
                    std::next(p.begin(), 4),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = p.insert_list(
                    std::next(p.begin(), 5),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(is_equal(
                    *it, { "y", "g" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&%23&%26==&%3d=%26&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                p.insert_list(std::next(p.begin(), 2),
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //----------------------------------------
        
        //
        // erase(iterator)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 0));
                BOOST_TEST(is_equal(
                    *it, { "k1", "1" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 1));
                BOOST_TEST(is_equal(
                    *it, { "k2", "" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 2));
                BOOST_TEST(is_equal(
                    *it, { "k3" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 3));
                BOOST_TEST(is_equal(
                    *it, { "k4", "4444" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 4));
                BOOST_TEST_EQ(it, p.end());
            });

        //
        // erase(iterator, iterator)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 0),
                    std::next(p.begin(), 2));
                BOOST_TEST(is_equal(
                    *it, { "k2", "" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 1),
                    std::next(p.begin(), 3));
                BOOST_TEST(is_equal(
                    *it, { "k3" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 2),
                    std::next(p.begin(), 4));
                BOOST_TEST(is_equal(
                    *it, { "k4", "4444" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=",
            [](params_encoded_view& p)
            {
                auto it = p.erase(
                    std::next(p.begin(), 3),
                    std::next(p.begin(), 5));
                BOOST_TEST_EQ(it, p.end());
            });

        //
        // erase(string_view, ignore_case_param)
        //

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("K0");
                BOOST_TEST_EQ(n, 0);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k1=1&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("k0");
                BOOST_TEST_EQ(n, 2);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k1=1&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("K0", ignore_case);
                BOOST_TEST_EQ(n, 2);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k0=2&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("k1");
                BOOST_TEST_EQ(n, 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k0=2&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("K1", ignore_case);
                BOOST_TEST_EQ(n, 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("k2");
                BOOST_TEST_EQ(n, 0);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&k3=4&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("K2");
                BOOST_TEST_EQ(n, 2);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&k3=4&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("k2", ignore_case);
                BOOST_TEST_EQ(n, 2);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=3&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto n = p.erase("k3");
                BOOST_TEST_EQ(n, 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5",
            [](params_encoded_view& p)
            {
                auto n = p.erase("k4");
                BOOST_TEST_EQ(n, 1);
            });

        //----------------------------------------

        //
        // replace(iterator, param_pct_view)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k0"), { "y", "g" });
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&y=g&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k1"), { "y", "g" });
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&y=g&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k2"), { "y", "g" });
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&y=g&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k3"), { "y", "g" });
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&y=g",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k4"), { "y", "g" });
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&%23&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                p.replace(p.find("k1"),
                    { "#" });
            });
#endif

        //
        // replace(iterator, iterator, initializer_list)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&z=q&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k0"), p.find("k3"),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&y=g&z=q&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k1"), p.find("k4"),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = p.replace(
                    p.find("k2"), p.end(),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            {},
            [](params_encoded_view& p)
            {
                // clear
                auto it = p.replace(
                    p.find("k0"), p.end(), {});
                BOOST_TEST_EQ(it, p.begin());
                BOOST_TEST_EQ(it, p.end());
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&%23&%26==&%3d=%26&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                p.replace(
                    p.find("k1"), p.find("k3"),
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //
        // replace(iterator, iterator, FwdIt, FwdIt)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "y=g&z=q&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = replace(p,
                    p.find("k0"), p.find("k3"),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&y=g&z=q&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = replace(p,
                    p.find("k1"), p.find("k4"),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&y=g&z=q",
            [](params_encoded_view& p)
            {
                auto it = replace(p,
                    p.find("k2"), p.end(),
                    {{ "y", "g" }, { "z", "q" }});
                BOOST_TEST(
                    is_equal(*it, { "y", "g" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            {},
            [](params_encoded_view& p)
            {
                // clear
                auto it = replace(p,
                    p.find("k0"), p.end(), {});
                BOOST_TEST_EQ(it, p.begin());
                BOOST_TEST_EQ(it, p.end());
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&%23&%26==&%3d=%26&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                replace(p,
                    p.find("k1"), p.find("k3"),
                    {{ "#" }, { "&", "=" }, { "=", "&" }});
            });
#endif

        //----------------------------------------

        //
        // unset(iterator)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.unset(next(p.begin(), 0));
                BOOST_TEST(is_equal(*it, { "k0" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.unset(next(p.begin(), 1));
                BOOST_TEST(is_equal(*it, { "k1" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.unset(next(p.begin(), 2));
                BOOST_TEST(is_equal(*it, { "k2" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.unset(next(p.begin(), 3));
                BOOST_TEST(is_equal(*it, { "k3" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4",
            [](params_encoded_view& p)
            {
                auto it = p.unset(next(p.begin(), 4));
                BOOST_TEST(is_equal(*it, { "k4" }));
            });

        //
        // set(iterator, string_view)
        //

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=42&k1=1&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.set(next(p.begin(), 0), "42");
                BOOST_TEST(is_equal(*it, { "k0", "42" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=42&k2=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.set(next(p.begin(), 1), "42");
                BOOST_TEST(is_equal(*it, { "k1", "42" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=42&k3&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.set(next(p.begin(), 2), "42");
                BOOST_TEST(is_equal(*it, { "k2", "42" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3=42&k4=4444",
            [](params_encoded_view& p)
            {
                auto it = p.set(next(p.begin(), 3), "42");
                BOOST_TEST(is_equal(*it, { "k3", "42" }));
            });

        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=&k3&k4=42",
            [](params_encoded_view& p)
            {
                auto it = p.set(next(p.begin(), 4), "42");
                BOOST_TEST(is_equal(*it, { "k4", "42" }));
            });

#if 0
        modify(
            "k0=0&k1=1&k2=&k3&k4=4444",
            "k0=0&k1=1&k2=%23%25%26=&k3&k4=4444",
            [](params_encoded_view& p)
            {
                // encodings
                p.set(next(p.begin(), 2), "#%&=");
            });
#endif

        //
        // set(string_view, string_view, ignore_case_param)
        //

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=x&k1=1&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto it = p.set("k0", "x");
                BOOST_TEST(is_equal(*it, { "k0", "x" }));
                BOOST_TEST_EQ(p.count("k0"), 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=x&k0=2&K2=3&k3=4&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto it = p.set("k1", "x");
                BOOST_TEST(is_equal(*it, { "k1", "x" }));
                BOOST_TEST_EQ(p.count("k1"), 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6&k2=x",
            [](params_encoded_view& p)
            {
                auto it = p.set("k2", "x");
                BOOST_TEST(is_equal(*it, { "k2", "x" }));
                BOOST_TEST_EQ(p.count("k2"), 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=x&k3=4&k4=6",
            [](params_encoded_view& p)
            {
                auto it = p.set("k2", "x", ignore_case);
                BOOST_TEST(is_equal(*it, { "K2", "x" }));
                BOOST_TEST_EQ(p.count("k2", ignore_case), 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=3&k3=x&K2=5&k4=6",
            [](params_encoded_view& p)
            {
                auto it = p.set("k3", "x");
                BOOST_TEST(is_equal(*it, { "k3", "x" }));
                BOOST_TEST_EQ(p.count("k3"), 1);
            });

        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=x",
            [](params_encoded_view& p)
            {
                auto it = p.set("k4", "x");
                BOOST_TEST(is_equal(*it, { "k4", "x" }));
                BOOST_TEST_EQ(p.count("k4"), 1);
            });

#if 0
        modify(
            "k0=0&k1=1&k0=2&K2=3&k3=4&K2=5&k4=6",
            "k0=0&k1=1&k0=2&K2=%23%25%26=&k3=4&k4=6",
            [](params_encoded_view& p)
            {
                // encodings
                p.set("k2", "#%&=", ignore_case);
            });
#endif
    }

    void
    testIterator()
    {
        using T = params_encoded_view::iterator;

        // iterator()
        {
            T t0;
            T t1;
            BOOST_TEST_EQ(t0, t1);
        }

        // operator==()
        {
            url u;
            BOOST_TEST_EQ(
                u.encoded_params().begin(),
                u.encoded_params().begin());
        }

        // operator!=()
        {
            url u("?");
            BOOST_TEST_NE(
                u.encoded_params().begin(),
                u.encoded_params().end());
        }

        // value_type outlives reference
        {
            params_encoded_view::value_type v;
            {
                url u("/?a=1&bb=22&ccc=333&dddd=4444#f");
                params_encoded_view ps = u.encoded_params();
                params_encoded_view::reference r = *ps.begin();
                v = params_encoded_view::value_type(r);
            }
            BOOST_TEST_EQ(v.key, "a");
            BOOST_TEST_EQ(v.value, "1");
            BOOST_TEST_EQ(v.has_value, true);
        }
    }

    void
    testRange()
    {
        check( "", {} );
        check( "?", { {} } );
        check( "?&", { {}, {} } );
        check( "?key", { { "key" } } );
        check( "?key=", { { "key", "" } } );
        check( "?key=value", { { "key", "value" } } );
        check( "?first=John&last=Doe", { { "first", "John" }, { "last", "Doe" } } );
        check( "?key=value&", { { "key", "value" }, {} } );
        check( "?&key=value", { {}, { "key", "value" } } );
    }

    void
    run()
    {
        testSpecial();
        testObservers();
        testModifiers();
        testIterator();
        testRange();
    }
};

TEST_SUITE(
    params_encoded_view_test,
    "boost.url.params_encoded_view");

} // urls
} // boost
