#include <symbos.h>

unsigned char Msg_Wait(char rec_pid, char send_pid, char* msg, char id) {
    unsigned short response;
    msg[0] = 0;
    for (;;) {
        response = Msg_Sleep(rec_pid, send_pid, msg);
        if (msg[0] == id)
            return response >> 8;
        if (response & 1)
            Msg_Send(send_pid, rec_pid, msg); // something else, keep it on the queue
    }
}

void Msg_Respond(char rec_pid, char send_pid, char* msg) {
    char id = msg[0] + 128;
    while (Msg_Send(rec_pid, send_pid, msg) == 0);
    Msg_Wait(rec_pid, send_pid, msg, id);
}
