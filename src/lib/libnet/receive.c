#include <symbos.h>
#include <network.h>
#include "network.h"

signed char TCP_ReceiveToEnd(unsigned char handle, unsigned char bank, char* addr, unsigned short maxlen) {
    unsigned short get_bytes;
    char ch, fd, got_bytes, stopping;
    char* ptr = addr;
    char* ptrend = addr + maxlen - 1;
    TCP_Trans trans_obj;
    NetStat net_stat;
    #ifdef _NETDEBUG
    char textbuf[32];
    #endif

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
    got_bytes = 0;
    stopping = 0;
    while (!stopping && !Net_Wait(NET_TCPEVT)) {
        if (_netmsg[3] == handle) {
            TCP_Event(_netmsg, &net_stat);
            _nsemaoff();
            if (net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSED) {
                // TCP_CLOSED implies "read until exhausted, then close" - but if it
                // arrives during a TCP_Receive cycle, we exhaust first and *then*
                // encounter TCP_CLOSED with an outdated bytesrec. Some devices
                // misbehave when asked to read from an empty RX, so trap this.
                if (got_bytes)
                    break;
                stopping = 1;
            }
            if (net_stat.bytesrec) {
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

                    #ifdef _NETDEBUG
                    strcpy(textbuf, " -> ");
                    itoa(trans_obj.transferred, textbuf+strlen(textbuf), 10); strcat(textbuf, ",");
                    itoa(trans_obj.remaining, textbuf+strlen(textbuf), 10); strcat(textbuf, "\r\n");
                    Shell_Print(textbuf);
                    #endif

                    if (_tcp_abort) {
                        _neterr = ERR_TRUNCATED;
                        goto _fail;
                    }
                    got_bytes = 1;
                }
            }
            if (_tcp_abort) {
                _neterr = ERR_TRUNCATED;
                goto _fail;
            }
            _nsemaon();
        } else {
            // not relevant to this socket, put back on queue
            #ifdef NETDEBUG
            msg_print("Requeuing: ");
            #endif
            Msg_Send(_netpid, _msgpid(), _netmsg);
        }
    }
    _nsemaoff();
    _packsemaoff();
    if (fd < 8)
        File_Close(fd);
    return TCP_Close(handle);

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
