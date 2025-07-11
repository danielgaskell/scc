#include <network.h>
#include "network.h"

void Net_SkipMsg(signed char handle) {
    unsigned char count = 0;
    _nsemaon();
    while (count < 64) { // 64 iterations without throwing away a message = buffer is empty of relevant messages (slow but simple way to robustly check this)
        if (!(Msg_Receive(_msgpid(), _netpid, _netmsg) & 1))
            // no messages left, break
            break;
        if ((_netmsg[0] == NET_TCPEVT || _netmsg[0] == NET_UDPEVT) && _netmsg[3] == handle) {
            // relevant message, throw it away
            count = 0;
        } else {
            // not relevant, put message back on queue
            Msg_Send(_netpid, _msgpid(), _netmsg);
            ++count;
        }
    }
    _nsemaoff();
}
