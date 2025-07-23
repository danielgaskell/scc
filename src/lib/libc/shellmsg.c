#include <symbos.h>

unsigned char _shellexit;

void _Shell_MsgWait(void) {
    unsigned char response = _symmsg[0] + 128;
    while (Msg_Send(_sympid, _shellpid, _symmsg) == 0);
    for (;;) {
        if (Msg_Sleep(_sympid, _shellpid, _symmsg) & 1) {
            if (_symmsg[0] == response)
                break;
            if (_symmsg[0] == 0) {
                if (_shellexit == 0)
                    exit(0);
                _shellexit = 2;
                _shellerr = ERR_NOSHELL;
                _symmsg[3] = ERR_NOSHELL - 16;
                return;
            }
        }
    }
    if (_symmsg[3] > 1)
        _shellerr = _symmsg[3] + 16;
    else
        _shellerr = 0;
}
