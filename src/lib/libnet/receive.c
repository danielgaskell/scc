#include <symbos.h>
#include <network.h>
#include "network.h"

signed char TCP_ReceiveToEnd(unsigned char handle, unsigned char bank, char* addr, unsigned short maxlen) {
    unsigned short get_bytes;
    char ch, fd, progress = 0;
    char* ptr = addr;
    char* ptrend = addr + maxlen - 1;
    TCP_Trans trans_obj;
    NetStat net_stat;

    // setup
    fd = 8;
    _tcp_abort = 0;
    *(((unsigned short*)&_tcp_progress)) = 0;
    *(((unsigned short*)&_tcp_progress) + 1) = 0;
    if (!maxlen) {
        fd = File_New(bank, addr, 0);
        if (fd > 7) {
            _neterr = ERR_NETFILE;
            return -1;
        }
        ptr = _netpacket;
        ptrend = _netpacket + sizeof(_netpacket);
    }

    // receive
    _nsemaon();
    _packsemaon();
    while (!Net_Wait(NET_TCPEVT)) {
        if (_netmsg[3] == handle) {
            TCP_Event(_netmsg, &net_stat);
            if (net_stat.bytesrec) {
                _nsemaoff();
                get_bytes = net_stat.bytesrec;
                while (get_bytes) {
                    if (TCP_Receive(handle, bank, ptr, ptrend - ptr, &trans_obj))
                        goto _fail;
                    if (maxlen) {
                        ptr += trans_obj.transferred;
                        *ptr = 0;
                        if (ptr >= ptrend) {
                            _neterr = ERR_TOOLARGE;
                            goto _fail;
                        }
                    } else {
                        if (!File_Write(fd, _symbank, _netpacket, trans_obj.transferred)) {
                            _neterr = ERR_NETFILE;
                            goto _fail;
                        }
                    }
                    _safeadd((char*)&_tcp_progress, trans_obj.transferred);
                    get_bytes = trans_obj.remaining;
                    if (_tcp_abort) {
                        _neterr = ERR_TRUNCATED;
                        goto _fail;
                    }
                }
                _nsemaon();
            }
            if (net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSED) {
                _nsemaoff();
                _packsemaoff();
                if (fd < 8)
                    File_Close(fd);
                return TCP_Close(handle);
            }
            if (_tcp_abort) {
                _neterr = ERR_TRUNCATED;
                break;
            }
        } else {
            Msg_Send(_netpid, _msgpid(), _netmsg); // not relevant to this socket, put back on queue
        }
    }

_fail:
    ch = _neterr;
    _nsemaoff();
    _packsemaoff();
    TCP_Disconnect(handle);
    if (fd < 8)
        File_Close(fd);
    _neterr = ch;
    return -1;
}
