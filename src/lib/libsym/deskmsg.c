#include <symbos.h>

void _Desk_Msg(void)  { while(Msg_Send(_msgpid(), 2, _symmsg) == 0); }
void _Desk_Wait(void) { Idle(); Msg_Receive(_msgpid(), 2, _symmsg); }
