#include <stdlib.h>
#include <string.h>
#include <symbos.h>
#include <network.h>
#include "network.h"

int HTTP_GET(char* url, char* dest, unsigned short maxlen, char* headers, unsigned char bodyonly) {
    unsigned long ip;
    unsigned long counter;
    signed char socket;
    char buf[8];
    int result;
    int port;
    char* path;
    char* ptr;
    char* ptrend;
    TCP_Trans trans_obj;
    NetStat net_stat;

    // setup
    --maxlen; // to leave room for zero-terminator
    *dest = 0;
    result = Net_SplitURL(url, _nethost, &path, &port);
    if (result != PROTO_HTTP) {
        _neterr = ERR_BADDOMAIN;
        goto _fail;
    }

    // resolve IP
    ip = DNS_Resolve(_symbank, url);
    if (ip == 0)
        return -1;

    // open connection
    socket = TCP_OpenClient(ip, -1, 80);
    if (socket == -1)
        return -1;
    _symmsg[0] = 0;
    counter = Sys_Counter() + _nettimeout;
    for (;;) {
        Msg_Receive(_sympid, _netpid, _symmsg);
        if (_symmsg[0] == 159) {
            TCP_Event(&net_stat);
            if (net_stat.status == TCP_OPENED)
                break;
            else if (net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSED || Sys_Counter() > counter)
                goto _fail;
        }
        if (Sys_Counter() > counter) {
            _neterr = ERR_TIMEOUT;
            goto _fail;
        }
    }

    // send message
    strcpy(_netmsg, "GET /");
    if (path)
        strcat(_netmsg, path);
    strcat(_netmsg, " HTTP/1.1\r\n");
    strcat(_netmsg, "Host: ");
    strcat(_netmsg, url);
    strcat(_netmsg, "\r\n");
    strcat(_netmsg, _useragent);
    if (headers)
        strcat(_netmsg, headers);
    strcat(_netmsg, "Connection: close\r\n\r\n");
    result = TCP_Send(socket, _symbank, _netmsg, strlen(_netmsg));
    if (result == -1)
        goto _fail;

    // wait for response and copy to buffer
    _symmsg[0] = 0;
    ptr = dest;
    ptrend = dest + maxlen;
    counter = Sys_Counter() + _nettimeout;
    for (;;) {
        Msg_Receive(_sympid, _netpid, _symmsg);
        if (_symmsg[0] == 159) {
            TCP_Event(&net_stat);
            if (net_stat.datarec) {
                result = TCP_Receive(socket, _symbank, ptr, ptrend - ptr, &trans_obj);
                ptr += trans_obj.transferred;
            }
            if (ptr >= ptrend || net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSED)
                break;
        } else if (Sys_Counter() > counter) {
            _neterr = ERR_TIMEOUT;
            goto _fail;
        }
    }
    *ptr = 0;

    // skip headers
    if (memcmp(dest, "HTTP/", 5) == 0) {
        // get response code
        result = atoi(dest + 8);

        if (bodyonly) {
            // return message body only
            ptr = strstr(dest, "\r\n\r\n");
            if (ptr)
                strcpy(dest, ptr + 4); // message body
            else
                *dest = 0; // no body
        }
    } else {
        result = 0;
    }

    // close connection
    result = TCP_Close(socket);
    if (result == -1)
        goto _fail;
    return result;

_fail:
    result = _neterr;
    TCP_Close(socket);
    _neterr = result;
    return -1;
}
