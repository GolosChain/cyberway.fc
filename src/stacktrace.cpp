#undef BOOST_STACKTRACE_LINK
#undef BOOST_STACKTRACE_DYN_LINK
#undef BOOST_STACKTRACE_USE_BACKTRACE
#undef BOOST_STACKTRACE_USE_ADDR2LINE

#include <signal.h>

#include <fstream>
#include <iostream>

#include <fc/stacktrace.hpp>
#include <fc/filesystem.hpp>

namespace fc {
    string to_string(const stacktrace& strace) { try {
        namespace bs = boost::stacktrace;
        using to_string_type = bs::detail::to_string_impl_base<bs::detail::to_string_using_nothing>;
        return fc::detail::to_string_impl<to_string_type>(strace);
    } catch (...) {
        return {};
    } }

    namespace {
        static path dumpname_ = "./backtrace.dump";

        void stacktrace_signal_handler_(int signum) {
            ::signal(signum, SIG_DFL);
            boost::stacktrace::safe_dump_to(dumpname_.string().c_str());
            ::raise(SIGABRT);
        }
    }

    bool has_btrace(const path& filename) {
        return (exists(filename));
    }

    bool has_btrace() {
        return exists(dumpname_);
    }

    string get_btrace(const path& filename) { try {
        if (!has_btrace(filename)) return {};

        // there is a btrace
        auto stream = std::ifstream(filename.string());
        auto btrace = stacktrace::from_dump(stream);

        // cleaning up
        stream.close();
        return to_detail_string(btrace);
    } catch (...) {
        return {};
    } }

    string get_btrace() {
        return get_btrace(dumpname_);
    }

    void remove_btrace(const path& filename) {
        if (has_btrace(filename)) {
            remove(filename);
        }
    }

    void remove_btrace() {
        remove_btrace(dumpname_);
    }

    void install_btrace_signal_handler(const path& filename) {
        static bool installed = false;
        if (installed) return;
        installed = true;

        dumpname_ = filename;
        if (has_btrace()) {
            std::cerr
                << std::endl
                << "Previous run crashed:" << std::endl
                << get_btrace() << std::endl
                << std::endl;

            remove_btrace();
        }

        ::signal(SIGSEGV, &stacktrace_signal_handler_);
        ::signal(SIGABRT, &stacktrace_signal_handler_);
    }
}