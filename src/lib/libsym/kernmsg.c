#include <symbos.h>

void _Kern_MsgWait(void) { Msg_Respond(_threadpid(), 1, _symmsg); }

