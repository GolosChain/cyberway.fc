#undef  BOOST_STACKTRACE_LINK
#undef  BOOST_STACKTRACE_DYN_LINK
#undef  BOOST_STACKTRACE_USE_BACKTRACE
#define BOOST_STACKTRACE_USE_ADDR2LINE

#include <fc/stacktrace.hpp>

#include <iostream>

namespace fc {
    string to_detail_string(const stacktrace& strace) { try {
        namespace bs = boost::stacktrace;
        using to_string_type = bs::detail::to_string_impl_base<bs::detail::to_string_using_addr2line>;
        return fc::detail::to_string_impl<to_string_type>(strace);
    } catch (...) {
        return {};
    } }

} // namespace fc