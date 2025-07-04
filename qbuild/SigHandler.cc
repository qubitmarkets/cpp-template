// Copyright (c) 2025 Qubit Markets Pte. Ltd.

#include "SigHandler.h"
#include "CaptureBacktrace.h"
#include "ansi_colors.h"
#include <memory.h>
#include <signal.h>
#include <stdio.h>  // stderr
#include <csignal>
#include <exception>  // std::set_terminate

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

__attribute__((used)) static const int __qbuild_SIG_HANDLER_VERSION__ = 0x00010000;

#define ENUM(Name, IntType, ...)                   \
    struct Name {                                  \
        enum Enum : IntType { __VA_ARGS__, _MAX }; \
        static constexpr IntType MAX = Enum::_MAX; \
    };

namespace {

SigHandlerCallbacks g_sig_handler_callbacks{};

const char* g_signal_names[] = {
    "NULL",
    "SIGHUP",     // 1
    "SIGINT",     // 2
    "SIGQUIT",    // 3
    "SIGILL",     // 4
    "SIGTRAP",    // 5
    "SIGABRT",    // 6
    "SIGBUS",     // 7
    "SIGFPE",     // 8
    "SIGKILL",    // 9
    "SIGUSR1",    // 10
    "SIGSEGV",    // 11
    "SIGUSR2",    // 12
    "SIGPIPE",    // 13
    "SIGALRM",    // 14
    "SIGTERM",    // 15
    "SIGSTKFLT",  // 16
    "SIGCHLD",    // 17
    "SIGCONT",    // 18
    "SIGSTOP",    // 19
    "SIGTSTP",    // 20
    "SIGTTIN",    // 21
    "SIGTTOU",    // 22
    "SIGURG",     // 23
    "SIGXCPU",    // 24
    "SIGXFSZ",    // 25
    "SIGVTALRM",  // 26
    "SIGPROF",    // 27
    "SIGWINCH",   // 28
    "SIGIO",      // 29
    "SIGPWR",     // 30
    "SIGSYS",     // 31
};

const char* get_signame(int sig) {
    if (sig < 1 || sig > 31) {
        return "(invalid)";
    }
    return g_signal_names[sig];
}

extern "C" void sig_handler(int sig, siginfo_t* siginfo, void* context) {
    bool urgent_signal = !(sig == SIGINT || sig == SIGUSR1 || sig == SIGHUP);
    bool is_handled = false;
    for (auto& cb : g_sig_handler_callbacks.on_start_sighandler) {
        is_handled |= cb(sig);
    }
    if (is_handled) {
        // If any callback handled the signal, we can return early
        for (auto& cb : g_sig_handler_callbacks.on_exit_sighandler) {
            cb(sig);
        }
        return;
    }
    if (!(sig == SIGUSR1 || sig == SIGINT)) {
        eprintf("\nCaught signal: %d %s\n", sig, get_signame(sig));
    }
    {
        CaptureBacktrace bt;
        bt.capture(1);
        bt.print();
    }
    if (sig == SIGINT || sig == SIGHUP) {
        // Terminate the process
        eprintf("\nCaught signal: %d %s, exiting...\n", sig, get_signame(sig));
        for (auto& cb : g_sig_handler_callbacks.on_exit_process) {
            cb(sig);
        }
        // Only call the on_exit_process callbacks once in the application lifetime
        g_sig_handler_callbacks.on_exit_process.clear();
        ::exit(128 + sig);
    }
    if (!(sig == SIGUSR1 || sig == SIGUSR2)) {
        // Remove handler, let the system crash
        struct sigaction action;
        ::memset(&action, 0, sizeof(action));
        action.sa_handler = SIG_DFL;
        ::sigaction(sig, &action, nullptr);
    }
    for (auto& cb : g_sig_handler_callbacks.on_exit_sighandler) {
        cb(sig);
    }
}

void unhandled_exception_handler() {
    eprintf(COLOR_RED "The application crashed" COLOR_NONE "\n");
    auto msg = "unhandled exception thrown";
    eprintf("Message:  " COLOR_CYAN "%s" COLOR_NONE "\n", msg);

    CaptureBacktrace cap;
    cap.capture(1);
    cap.print();

    exit(1);
}

}  // namespace

void SigHandlerCallbacks::default_exit_process(int sig) {
    ::exit(128 + sig);
}

SigHandlerCallbacks& SigHandler::get_callbacks() {
    return g_sig_handler_callbacks;
}

void SigHandler::install() {
    auto* env = ::getenv("SIGHANDLER");
    if (env && strcmp(env, "0") == 0) {
        eprintf("SIGHANDLER=0, not installing signal handler\n");
        return;
    }
    std::set_terminate(unhandled_exception_handler);

    int signals[] = {SIGUSR1, SIGINT, SIGHUP, SIGSEGV, SIGABRT, SIGUSR2};
    for (auto sig : signals) {
        struct sigaction oldact;
        struct sigaction action;
        ::memset(&action, 0, sizeof(action));
        action.sa_sigaction = &sig_handler;
        if (::sigaction(sig, &action, &oldact) == -1) {
            eprintf("Unable to set signal handler");
        }
    }
}

void SigHandler::register_sighandler(Callback<bool(int sig)> cb) {
    g_sig_handler_callbacks.on_start_sighandler.push_back(cb);
}
void SigHandler::register_sighandler_exit(Callback<void(int sig)> cb) {
    g_sig_handler_callbacks.on_exit_sighandler.push_back(cb);
}
void SigHandler::register_on_exit_process(Callback<void(int sig)> cb) {
    g_sig_handler_callbacks.on_exit_process.push_back(cb);
}

// Always call on_exit_process callbacks
SigHandlerCallbacks::~SigHandlerCallbacks() {
    for (auto& cb : on_exit_process) {
        cb(0);  // Call with 0 to indicate normal exit
    }
    // Only call the on_exit_process callbacks once in the application lifetime
    g_sig_handler_callbacks.on_exit_process.clear();
}
