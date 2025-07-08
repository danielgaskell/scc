#include <string.h>
#include <symbos.h>
#include <network.h>
#include "network.h"

int _ftp_response;

int FTP_Response(unsigned char handle, char* addr, unsigned short maxlen) {
    unsigned short get_bytes;
    char ch;
    char progress = 2; // starts looking for number first
    char* ptr = addr;
    char* ptrend = addr + maxlen - 1;
    char* ptr_check = addr;
    char* num_loc = addr;
    TCP_Trans trans_obj;
    NetStat net_stat;

    _nsemaon();
    while (!Net_Wait(NET_TCPEVT)) {
        if (_netmsg[3] == handle) {
            TCP_Event(_netmsg, &net_stat);
            if (net_stat.bytesrec) {
                // read response
                _nsemaoff();
                get_bytes = net_stat.bytesrec;
                while (get_bytes) {
                    if (TCP_Receive(handle, _symbank, ptr, ptrend - ptr, &trans_obj))
                        return -1;
                    ptr += trans_obj.transferred;
                    if (ptr >= ptrend) {
                        _neterr = ERR_TOOLARGE;
                        return -1;
                    }
                    get_bytes = trans_obj.remaining;
                }
                *ptr = 0;

                // check for terminating response code (format \r\n### Text\r\n)
                while (ch = *ptr_check) {
                    switch (progress) {
                    case 0: if (ch == '\r') progress = 1; break;
                    case 1: if (ch == '\n') {progress = 2; num_loc = ptr_check + 1;} else progress = 0; break;
                    case 2: case 3: case 4: if (ch >= '0' && ch <= '9') ++progress; else progress = 0; break;
                    case 5: if (ch == ' ') progress = 6; else progress = 0; break;
                    case 6: if (ch == '\r') progress = 7; break;
                    case 7:
                        if (ch == '\n') {
                            _ftp_response = atoi(num_loc);
                            return _ftp_response;
                        } else {
                            progress = 0;
                        }
                        break;
                    }
                    ++ptr_check;
                }
                _nsemaon();
            }
            if (net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSED)
                break;
        } else {
            Msg_Send(_netpid, _msgpid(), _netmsg); // not relevant to this socket, put back on queue
        }
    }
    _nsemaoff();
    TCP_Close(handle);
    _neterr = ERR_CONNECT;
    return -1;
}

int FTP_Command(unsigned char handle, char* cmd, char* addr, unsigned short maxlen) {
    if (TCP_Send(handle, _symbank, cmd, strlen(cmd)))
        return -1;
    return FTP_Response(handle, addr, maxlen);
}

signed char FTP_Open(char* ip, int rport, char* username, char* password) {
    signed char handle;
    int result;

    _packsemaon();
    handle = TCP_OpenClient(ip, -1, rport);
    if (handle == -1) {
        _packsemaoff();
        return -1;
    }
    result = FTP_Response(handle, _netpacket, sizeof(_netpacket));
    if (result == 230) {
        // no username required, we're done
        _packsemaoff();
        return handle;
    } else if (result == 220) {
        // username required, provide it
        strcpy(_netpacket, "USER ");
        strcat(_netpacket, username);
        strcat(_netpacket, "\r\n");
        result = FTP_Command(handle, _netpacket, _netpacket, sizeof(_netpacket));
        if (result == 230) {
            // no password required, we're done
            _packsemaoff();
            return handle;
        } else if (result == 331) {
            // password required, provide it
            strcpy(_netpacket, "PASS ");
            strcat(_netpacket, password);
            strcat(_netpacket, "\r\n");
            if (FTP_Command(handle, _netpacket, _netpacket, sizeof(_netpacket)) == 230) {
                _packsemaoff();
                return handle;
            } else {
                goto _fail;
            }
        } else {
            // something else, fail
            goto _fail;
        }
    }
_fail:
    _packsemaoff();
    _neterr = ERR_CONNECT;
    return -1;
}

signed char FTP_GetPassive(unsigned char handle, char* ip, unsigned short* port) {
    int result;
    char i;
    char temp_ip[6];
    char* ptr;

    _packsemaon();
    if (FTP_Command(handle, "PASV\r\n", _netpacket, sizeof(_netpacket)) != 227)
        goto _fail;
    if (!(ptr = strchr(_netpacket, '(')))
        goto _fail;
    for (i = 0; i < 6; ++i) {
        temp_ip[i] = atoi(++ptr);
        if (i != 5 && !(ptr = strchr(ptr, ',')))
            goto _fail;
    }
    _packsemaoff();
    ip[0] = temp_ip[0];
    ip[1] = temp_ip[1];
    ip[2] = temp_ip[2];
    ip[3] = temp_ip[3];
    *port = temp_ip[4]*256 + temp_ip[5];
    return 0;

_fail:
    _packsemaoff();
    _neterr = ERR_RESPONSE;
    return -1;
}

signed char _ftp_updown(unsigned char handle, char* filename, unsigned char bank, char* addr, unsigned short maxlen, unsigned char mode, unsigned char request) {
    char fd;
    char ip[4];
    unsigned short port;
    unsigned short len;
    signed char passive;
    _packsemaon();
    if (FTP_Command(handle, mode ? "TYPE I\r\n" : "TYPE A\r\n", _netpacket, sizeof(_netpacket)) != 200) {
        _packsemaoff();
        return -1;
    }
    _packsemaoff();
    if (FTP_GetPassive(handle, ip, &port))
        return -1;
    _packsemaon();
    if (request) {
        strcpy(_netpacket, request == 2 ? "STOR " : "RETR ");
        strcat(_netpacket, filename);
        strcat(_netpacket, "\r\n");
    } else {
        strcpy(_netpacket, "LIST\r\n");
    }
    if (FTP_Command(handle, _netpacket, _netpacket, sizeof(_netpacket)) != 150) {
        _packsemaoff();
        return -1;
    }
    _packsemaoff();
    passive = TCP_OpenClient(ip, -1, port);
    if (passive == -1)
        return -1;
    if (request < 2) {
        // download/list
        if (TCP_ReceiveToEnd(passive, bank, addr, maxlen))
            return -1;
    } else {
        // upload
        if (maxlen) {
            // from buffer
            if (TCP_Send(passive, bank, addr, maxlen)) {
                TCP_Disconnect(passive);
                return -1;
            }
        } else {
            // from file
            fd = File_Open(bank, addr);
            if (fd < 8) {
                _packsemaon();
                len = 1;
                while (len) {
                    len = File_Read(fd, _symbank, _netpacket, sizeof(_netpacket));
                    if (len) {
                        if (TCP_Send(passive, _symbank, _netpacket, len)) {
                            _packsemaoff();
                            TCP_Disconnect(passive);
                            File_Close(fd);
                            return -1;
                        }
                    }
                }
                _packsemaoff();
                File_Close(fd);
            } else {
                _neterr = ERR_NETFILE;
                return -1;
            }
        }
        if (TCP_Disconnect(passive))
            return -1;
    }
    _packsemaon();
    if (FTP_Response(handle, _netpacket, sizeof(_netpacket)) != 226) {
        _packsemaoff();
        return -1;
    }
    _packsemaoff();
    return 0;
}

signed char FTP_Upload(unsigned char handle, char* filename, unsigned char bank, char* addr, unsigned short maxlen, unsigned char mode) {
    return _ftp_updown(handle, filename, bank, addr, maxlen, mode, 2);
}

signed char FTP_Download(unsigned char handle, char* filename, unsigned char bank, char* addr, unsigned short maxlen, unsigned char mode) {
    return _ftp_updown(handle, filename, bank, addr, maxlen, mode, 1);
}

signed char FTP_Listing(unsigned char handle, unsigned char bank, char* addr, unsigned short maxlen) {
    return _ftp_updown(handle, 0, bank, addr, maxlen, FTP_ASCII, 0);
}

signed char FTP_ChDir(unsigned char handle, char* path) {
    int result;
    _packsemaon();
    strcpy(_netpacket, "CWD ");
    strcat(_netpacket, path);
    strcat(_netpacket, "\r\n");
    result = FTP_Command(handle, _netpacket, _netpacket, sizeof(_netpacket));
    _packsemaoff();
    return (result == 250) ? 0 : -1;
}

signed char FTP_Disconnect(unsigned char handle) {
    TCP_Send(handle, _symbank, "QUIT\r\n", 6);
    return TCP_Disconnect(handle);
}
