#include <stdlib.h>
#include <string.h>
#include <symbos.h>
#include <network.h>
#include <ctype.h>
#include "network.h"

char _http_proxy_ip[4] = {0, 0, 0, 0};
int _http_proxy_port = 1234;
signed char _http_progress;
unsigned char _http_interrupt;
unsigned char _http_semaphore = 0;
char* _match_http = "HTTP/1.1 ";
char* _match_len = "\r\nCONTENT-LENGTH:";
char* _match_crlf = "\r\n\r\n";

int _http_request(char type, char* url, char* dest, unsigned short maxlen, char* headers, char* body, unsigned short bodylen, unsigned char keep_headers) {
    char ip[4];
    char numbuf[12];
    unsigned short counter;
    unsigned short packetlen;
    unsigned short buffer_left;
    unsigned short content_len;
    unsigned short content_received;
    signed char socket;
    int result;
    int port;
    unsigned short i;
    char* path;
    char* ptr;
    char* ptr2;
    char fd;
    char ch, in_body, num_accum, num_type, progress_http, progress_len, progress_crlf;
    char proxy = _http_proxy_ip[0] | _http_proxy_ip[1] | _http_proxy_ip[2] | _http_proxy_ip[3];
    TCP_Trans trans_obj;
    NetStat net_stat;

    // semaphore
    while (_http_semaphore);
    _http_semaphore = 1;

    // setup
    fd = 8;
    _http_interrupt = 0;
    _http_progress = HTTP_LOOKUP;
    if (maxlen) {
        --maxlen; // to leave room for zero-terminator
        *dest = 0;
    } else {
        fd = File_New(_symbank, dest, 0);
        if (fd > 7) {
            _neterr = ERR_NETFILE;
            _http_semaphore = 0;
            return -1;
        }
    }
    result = Net_SplitURL(url, _nethost, &path, &port);
    if (result != PROTO_HTTP) {
        _neterr = ERR_BADDOMAIN;
        _http_semaphore = 0;
        return -1;
    }

    // resolve IP
    if (proxy) {
        ip[0] = _http_proxy_ip[0];
        ip[1] = _http_proxy_ip[1];
        ip[2] = _http_proxy_ip[2];
        ip[3] = _http_proxy_ip[3];
        port = _http_proxy_port;
    } else if (DNS_Resolve(_symbank, _nethost, ip)) {
        _http_semaphore = 0;
        return -1;
    }
    _http_progress = HTTP_CONNECTING;

    // open connection
    socket = TCP_OpenClient(ip, -1, 80);
    if (socket == -1) {
        _http_semaphore = 0;
        return -1;
    }
    _http_progress = HTTP_SENDING;

    // send message
    strcpy(_netpacket, type ? "POST /" : "GET /" );
    if (proxy)
        strcat(_netpacket, url);
    else if (path)
        strcat(_netpacket, path);
    strcat(_netpacket, " HTTP/1.1\r\n");
    strcat(_netpacket, "Host: ");
    strcat(_netpacket, _nethost);
    strcat(_netpacket, "\r\n");
    strcat(_netpacket, _useragent);
    if (headers)
        strcat(_netpacket, headers);
    strcat(_netpacket, "Connection: close\r\n\r\n"); // ideally, remote will close after completion; but proxies do not do this, so we still need to track content-length manually.
    if (body) {
        strcat(_netpacket, "Content-Length: ");
        uitoa(bodylen, _netpacket + strlen(_netpacket), 10);
        strcat(_netpacket, "\r\n\r\n");
    }
    result = TCP_Send(socket, _symbank, _netpacket, strlen(_netpacket));
    if (result == -1)
        goto _fail;
    if (body) {
        if (TCP_Send(socket, _symbank, body, bodylen))
            goto _fail;
    }
    _http_progress = HTTP_WAITING;

    // process response (using a confusing state machine)
    result = 0;
    in_body = 0;
    num_accum = 0;
    progress_http = 0;
    progress_len = 0;
    progress_crlf = 0;
    ptr = _netpacket;
    buffer_left = maxlen;
    content_len = 0;
    content_received = 0;
    counter = Sys_Counter16() + _nettimeout;
    for (;;) {
        _netmsg[0] = 0;
        Msg_Receive(_msgpid(), _netpid, _netmsg);
        if (_netmsg[0] == NET_TCPEVT) {
            TCP_Event(_netmsg, &net_stat);
            if (net_stat.datarec) {
                for (;;) {
                    if (TCP_Receive(socket, _symbank, _netpacket, sizeof(_netpacket), &trans_obj))
                        goto _fail;
                    packetlen = trans_obj.transferred;

                    // parse relevant parts of headers
                    if (!in_body) {
                        *(_netpacket + packetlen) = 0;
                        ptr2 = _netpacket;
                        while (ch = toupper(*ptr2++)) {
                            // accumulate numbers
                            if (num_accum > 0 && num_accum < sizeof(numbuf)) {
                                if (ch >= '0' && ch <= '9') {
                                    numbuf[num_accum++] = ch;
                                } else if (!(ch == ' ' && num_accum == 1)) { // allows leading whitespace
                                    numbuf[num_accum] = 0;
                                    if (num_accum > 1) {
                                        if (num_type)
                                            content_len = atoi(numbuf + 1); // FIXME handle lengths longer than 16-bit (without using LONG)
                                        else
                                            result = atoi(numbuf + 1);
                                    }
                                    num_accum = 0;
                                }
                            }
                            // look for HTTP/1.1
                            if (ch == _match_http[progress_http]) {
                                if (!_match_http[++progress_http]) {
                                    num_accum = 1;
                                    num_type = 0;
                                }
                            }
                            // look for Content-Length:
                            if (ch == _match_len[progress_len]) {
                                if (!_match_len[++progress_len]) {
                                    num_accum = 1;
                                    num_type = 1;
                                }
                            } else {
                                progress_len = 0;
                            }
                            // look for start of body
                            if (ch == _match_crlf[progress_crlf]) {
                                if (!_match_crlf[++progress_crlf]) {
                                    in_body = 1;
                                    i = ptr2 - ptr;
                                    if (!keep_headers) {
                                        // if only saving body, jump pointer ahead to start of body
                                        packetlen -= i;
                                        ptr = ptr2;
                                    } else {
                                        // otherwise, we need to include this round of headers in the expected content_len
                                        content_len += i;
                                    }
                                    break;
                                }
                            } else {
                                progress_crlf = 0;
                            }
                        }
                    }

                    // output
                    if (keep_headers || in_body) {
                        if (maxlen) {
                            if (packetlen > buffer_left)
                                packetlen = buffer_left;
                            memcpy(dest, ptr, packetlen);
                            dest += packetlen;
                            buffer_left -= packetlen;
                            *dest = 0;
                            if (!buffer_left)
                                goto _done;
                        } else {
                            if (!File_Write(fd, _symbank, ptr, packetlen)) {
                                _neterr = ERR_NETFILE;
                                goto _fail;
                            }
                        }
                        if (content_len > 100)
                            _http_progress = content_received / (content_len / 100);
                        if (in_body) {
                            content_len -= packetlen;
                            content_received += packetlen;
                            if (content_len <= 0)
                                goto _done;
                        }
                    }
                    ptr = _netpacket;
                    counter = Sys_Counter16() + _nettimeout; // reset timeout after packet receipt
                    if (!trans_obj.remaining)
                        break;
                    if (_http_interrupt)
                        goto _done;
                }
            }
            if (net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSED)
                break;
        } else {
            Msg_Send(_netpid, _msgpid(), _netmsg); // something else, put back on queue
        }
        if (Sys_Counter16() > counter) {
            _neterr = ERR_TIMEOUT;
            goto _fail;
        }
        if (_http_interrupt)
            break;
    }

_done:
    // close connection
    _http_progress = HTTP_DONE;
    if (TCP_Disconnect(socket))
        goto _fail;
    if (fd < 8)
        File_Close(fd);
    _http_semaphore = 0;
    return result;

_fail:
    _http_progress = HTTP_DONE;
    result = _neterr;
    TCP_Disconnect(socket);
    if (fd < 8)
        File_Close(fd);
    _neterr = result;
    _http_semaphore = 0;
    return -1;
}

int HTTP_GET(char* url, char* dest, unsigned short maxlen, char* headers, unsigned char keep_headers) { return _http_request(0, url, dest, maxlen, headers, 0, 0, keep_headers); }
int HTTP_POST(char* url, char* dest, unsigned short maxlen, char* headers, char* body, unsigned short bodylen, unsigned char keep_headers) { return _http_request(1, url, dest, maxlen, headers, body, bodylen, keep_headers); }
