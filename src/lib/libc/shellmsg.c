#include <symbos.h>

void _Shell_MsgWait(void) {
    unsigned char response = _symmsg[0] + 128;
    while (Msg_Send(_sympid, _shellpid, _symmsg) == 0);
    while (_symmsg[0] != response) {
        Idle();
        Msg_Receive(_sympid, _shellpid, _symmsg);
    }
    if (_symmsg[3] > 1)
        _shellerr = _symmsg[3] + 16;
    else
        _shellerr = 0;
}
