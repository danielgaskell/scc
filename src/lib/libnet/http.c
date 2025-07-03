#include <stdlib.h>
#include <string.h>
#include <symbos.h>
#include <network.h>
#include "network.h"

int _http_request(char type, char* url, char* dest, unsigned short maxlen, char* headers, char* body, unsigned short bodylen, unsigned char bodyonly) {
    char ip[4];
    unsigned short counter;
    unsigned short packetlen;
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
    if (DNS_Resolve(_symbank, url, ip))
        return -1;

    // open connection
    socket = TCP_OpenClient(ip, -1, 80);
    if (socket == -1)
        return -1;

    // send message
    strcpy(_netpacket, type ? "POST /" : "GET /" );
    if (path)
        strcat(_netpacket, path);
    strcat(_netpacket, " HTTP/1.1\r\n");
    strcat(_netpacket, "Host: ");
    strcat(_netpacket, url);
    strcat(_netpacket, "\r\n");
    strcat(_netpacket, _useragent);
    if (headers)
        strcat(_netpacket, headers);
    strcat(_netpacket, "Connection: close\r\n\r\n");
    if (body) {
        strcat(_netpacket, "Content-Length: ");
        uitoa(bodylen, _netpacket + strlen(_netpacket), 10);
        strcat(_netpacket, "\r\n\r\n");
    }
    result = TCP_Send(socket, _symbank, _netpacket, strlen(_netpacket));
    if (result == -1)
        goto _fail;
    if (body) {
        while (bodylen) {
            packetlen = bodylen > sizeof(_netpacket) ? sizeof(_netpacket) : bodylen;
            memcpy(_netpacket, body, packetlen);
            result = TCP_Send(socket, _symbank, _netpacket, packetlen);
            if (result == -1)
                goto _fail;
            body += packetlen;
            bodylen -= packetlen;
        }
    }

    // wait for response and copy to buffer
    _netmsg[0] = 0;
    ptr = dest;
    ptrend = dest + maxlen;
    counter = Sys_Counter16() + _nettimeout;
    for (;;) {
        Msg_Receive(_msgpid(), _netpid, _netmsg);
        if (_netmsg[0] == 159) {
            TCP_Event(_netmsg, &net_stat);
            if (net_stat.datarec) {
                result = TCP_Receive(socket, _symbank, ptr, ptrend - ptr, &trans_obj);
                ptr += trans_obj.transferred;
            }
            if (ptr >= ptrend || net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSED)
                break;
        } else if (Sys_Counter16() > counter) {
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
    result = TCP_Disconnect(socket);
    if (result == -1)
        goto _fail;
    return result;

_fail:
    result = _neterr;
    TCP_Disconnect(socket);
    _neterr = result;
    return -1;
}

int HTTP_GET(char* url, char* dest, unsigned short maxlen, char* headers, unsigned char bodyonly) { return _http_request(0, url, dest, maxlen, headers, 0, 0, bodyonly); }
int HTTP_POST(char* url, char* dest, unsigned short maxlen, char* headers, char* body, unsigned short bodylen, unsigned char bodyonly) { return _http_request(1, url, dest, maxlen, headers, body, bodylen, bodyonly); }
