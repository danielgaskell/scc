#include <symbos.h>
#include <string.h>

extern signed char _timer_wake;
unsigned char _wake_pid;
unsigned short _wake_cycles;
unsigned short _wake_counter;
_transfer char _wake_env[64];

void _wake_thread(void) {
    for (;;) {
        --_wake_counter;
        if (_wake_counter == 0) {
            _wake_counter = _wake_cycles;
            Msg_Send(_wake_pid, _wake_pid, _wake_env);
        }
        Idle();
    }
}

signed char Timer_Wake(unsigned char pid, unsigned char msgid, unsigned short cycles) {
    if (_timer_wake != -1)
        return -1;
    memset(_wake_env, 0, sizeof(_wake_env));
    _wake_env[0] = msgid;
    _wake_pid = pid;
    _wake_cycles = cycles;
    _wake_counter = cycles;
    *((unsigned short*)(_wake_env + sizeof(_wake_env) - 3)) = (unsigned short)_wake_thread;
    _timer_wake = Timer_Add(_symbank, _wake_env + sizeof(_wake_env) - 15);
    return _timer_wake;
}
