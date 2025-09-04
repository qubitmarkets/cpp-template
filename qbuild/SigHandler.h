// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#pragma once

#include "Callback.h"
#include <vector>

/// Signal Handler for Linux
///
/// Note: SIGUSR1 is used to provide information (eg. current stack trace) of what the running process is doing
///       You can bind ^T to SIGUSR1 using the etc/tmux-signal script if you use tmux
///

struct SigHandlerCallbacks {
    ~SigHandlerCallbacks();

    // Called just before the process terminates
    std::vector<Callback<void(int)>> on_exit_process;

    // Called when a signal is received
    // Returns true if the signal should be ignored, false to print the stack trace and exit
    // e.g. logger_flush();
    std::vector<Callback<bool(int)>> on_start_sighandler;
    // Called when the signal handler is exited
    std::vector<Callback<void(int)>> on_exit_sighandler;

    // Exits the process with code 128+sig
    static void default_exit_process(int sig);
};

struct SigHandler {
    static void install();
    static void uninstall();
    // Called when a signal is received
    // Returns true if the signal should be ignored, false to print the stack trace and exit
    static void register_sighandler(Callback<bool(int sig)> cb);
    // Called when the signal handler is exited
    static void register_sighandler_exit(Callback<void(int sig)> cb);
    static void register_on_exit_process(Callback<void(int sig)> cb);
};
