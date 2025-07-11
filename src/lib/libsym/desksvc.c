#include <symbos.h>
#include "deskmsg.h"

void _Desk_Service(char id) {
    unsigned short response;
    _symmsg[0] = 48;
    _symmsg[1] = id;
    _Desk_Msg();
    for (;;) {
        response = Msg_Sleep(_msgpid(), 2, _symmsg);
        if (_symmsg[0] == 163 || _symmsg[1] == id)
            return;
        if (response & 1)
            Msg_Send(2, _msgpid(), _symmsg); // something else, keep it on the queue
    }
}
