#include <symbos.h>

void _Desk_Msg(void)  { while(Msg_Send(_threadpid(), 2, _symmsg) == 0); }
