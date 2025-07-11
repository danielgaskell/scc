#include <symbos.h>

void _Desk_Msg(void)  { while(Msg_Send(_msgpid(), 2, _symmsg) == 0); }
