#include <symbos.h>

void _Kern_MsgWait(void) { Msg_Respond(_msgpid(), 1, _symmsg); }

