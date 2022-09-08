//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/url
//

#ifndef BOOST_URL_IMPL_URL_VIEW_IPP
#define BOOST_URL_IMPL_URL_VIEW_IPP

#include <boost/url/url_view.hpp>
#include <boost/url/detail/except.hpp>

namespace boost {
namespace urls {

namespace detail {

url_view
url_impl::
construct() const noexcept
{
    return url_view(*this);
}

} // detail

//------------------------------------------------

url_view::
~url_view()
{
}

url_view::
url_view() noexcept = default;

url_view::
url_view(string_view s)
    : url_view(parse_uri_reference(s
        ).value(BOOST_URL_POS))
{
}

url_view::
url_view(
    url_view_base const& u) noexcept
    : url_view_base(u.u_)
{
}

url_view&
url_view::
operator=(
    url_view_base const& u) noexcept
{
    u_ = u.u_;
    return *this;
}

} // urls
} // boost

#endif
