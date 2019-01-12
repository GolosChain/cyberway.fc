#pragma once

#include <fc/string.hpp>
#include <fc/filesystem.hpp>

#include <boost/stacktrace.hpp>

namespace fc {
    using boost::stacktrace::stacktrace;

    string to_string(const stacktrace&);
    string to_detail_string(const stacktrace&);

    void install_btrace_signal_handler(const path&);

    string get_btrace(const path&);
    string get_btrace();

    bool has_btrace(const path&);
    bool has_btrace();

    void remove_btrace(const path&);
    void remove_btrace();

    namespace detail {
        template <typename ToStringImpl>
        string to_string_impl(const stacktrace& strace) {
            if (!strace) return {};

            auto& frames = strace.as_vector();

            string res;
            res.reserve(64 * strace.size());

            ToStringImpl impl;

            std::size_t i = 0;
            for (auto itr = frames.begin(), etr = frames.end(); etr != itr; ++i, ++itr) {
                if (i < 10) {
                    res += ' ';
                }
                res += fc::to_string(i);
                res += '#';
                res += ' ';
                res += impl(itr->address());
                res += '\n';
            }

            return res;
        }
    } // namespace detail
} // namespace fc