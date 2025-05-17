#pragma once

/// Signal Handler for Linux
///
/// Note: SIGUSR1 is used to provide information (eg. current stack trace) of what the running process is doing
///       You can bind ^T to SIGUSR1 using the etc/tmux-signal script if you use tmux
///

struct SigHandlerCallbacks {
    // Called just before the process terminates
    void (*on_exit_process)(int sig) = [](int) {};
    // Called when a signal is received
    // e.g. logger_flush();
    void (*on_start_sighandler)(int sig) = [](int) {};
    // Called when the signal handler is exited
    void (*on_exit_sighandler)(int sig) = [](int) {};

    // Exits the process with code 128+sig
    static void default_exit_process(int sig);
};

struct SigHandler {
    static void install();
    static SigHandlerCallbacks& get_callbacks();
};
