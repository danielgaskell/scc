#include <symbos.h>

extern signed char _timer_wake;
unsigned char _wake_pid;
unsigned char _wake_msg;
unsigned short _wake_cycles;
unsigned short _wake_counter;
_transfer char _wake_env[32];

void _wake_thread(void) {
    for (;;) {
        --_wake_counter;
        if (!_wake_counter) {
            _wake_env[0] = _wake_msg;
            Msg_Send(_msgpid(), _wake_pid, _wake_env);
            _wake_counter = _wake_cycles;
        }
        Idle();
    }
}

signed char Timer_Wake(unsigned char pid, unsigned char msgid, unsigned short cycles) {
    if (_timer_wake != -1)
        return -1;
    _wake_pid = pid;
    _wake_msg = msgid;
    _wake_cycles = cycles;
    _wake_counter = cycles;
    _timer_wake = thread_start(_wake_thread, _wake_env, sizeof(_wake_env));
    return _timer_wake;
}
