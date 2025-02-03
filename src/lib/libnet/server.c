#include <symbos.h>
#include <network.h>
#include "network.h"

_transfer char _serv_icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                                 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};

_transfer char _servenv[256];
_transfer char _servmsg[14];

unsigned char _clients = 0;
unsigned char _serv_winID;
unsigned long _serv_ip;
unsigned short _serv_port;
signed char _serv_socket = -1;
unsigned char _servpid = 0;
unsigned char _servopen = 0;
char* _codechars = "0123456789ABCDEF";

_data char _ip_buf[16];
_data char _code_buf[9] = {0};
_data char _port_buf[6] = {'5', '3', '0', '0', '0', 0};

_transfer List_Column _serv_col1 = {ALIGN_LEFT | LTYPE_TEXT, 0, 63, "Nickname"};
_transfer List_Column _serv_col2 = {ALIGN_LEFT | LTYPE_TEXT, 0, 63, "IP Address"};
_transfer List_Row2Col _serv_cd_lrows[32];
_transfer List _serv_list = {0, 0, &_serv_cd_lrows, 0, 2, 1, &_serv_col1, 0, LIST_MULTI | LIST_SCROLL};

_transfer Ctrl_TFrame _serv_cd_tf1 = {"IP Address", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl_Text _serv_cd_text1 = {_ip_buf, (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_Text _serv_cd_text2 = {"Port:", (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_Input _serv_cd_port = {_port_buf, 0, 5, 0, 5, 5};
_transfer Ctrl_Text _serv_cd_text3 = {"Port must be open/forwarded.", (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_TFrame _serv_cd_tf2 = {"Session Code", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl_Text _serv_cd_text4 = {_code_buf, (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_TFrame _serv_cd_tf3 = {"Connected Clients", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl _serv_c_area = {0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000};
_transfer Ctrl _serv_c_tf1 = {1, C_TFRAME, -1, (unsigned short)&_serv_cd_tf1, 2, 3, 146, 37};
_transfer Ctrl _serv_c_text1 = {2, C_TEXT, -1, (unsigned short)&_serv_cd_text1, 11, 13, 70, 8};
_transfer Ctrl _serv_c_text2 = {3, C_TEXT, -1, (unsigned short)&_serv_cd_text2, 83, 13, 64, 8};
_transfer Ctrl _serv_c_port = {4, C_INPUT, -1, (unsigned short)&_serv_cd_port, 104, 11, 36, 12};
_transfer Ctrl _serv_c_text3 = {5, C_TEXT, -1, (unsigned short)&_serv_cd_text3, 11, 24, 128, 8};
_transfer Ctrl _serv_c_tf2 = {6, C_TFRAME, -1, (unsigned short)&_serv_cd_tf2, 2, 40, 146, 28};
_transfer Ctrl _serv_c_text4 = {7, C_TEXT, -1, (unsigned short)&_serv_cd_text4, 11, 51, 64, 8};
_transfer Ctrl _serv_c_but1 = {8, C_BUTTON, -1, (unsigned short)"Reset", 92, 48, 48, 12};
_transfer Ctrl _serv_c_tf3 = {9, C_TFRAME, -1, (unsigned short)&_serv_cd_tf3, 2, 68, 146, 67};
_transfer Ctrl _serv_c_list = {10, C_LISTFULL, -1, (unsigned short)&_serv_list, 8, 77, 134, 52};
_transfer Ctrl _serv_c_but2 = {11, C_BUTTON, -1, (unsigned short)"Open Server", 5, 136, 68, 12};
_transfer Ctrl _serv_c_but3 = {12, C_BUTTON, -1, (unsigned short)"Close", 77, 136, 68, 12};
_transfer Ctrl_Group _serv_ctrls = {13, 0, &_serv_c_area};

// window data record
_transfer Window _serv_form = {
    WIN_NORMAL,
    WIN_CLOSE | WIN_TITLE | WIN_ICON,
    0,              // PID
    10, 10,         // x, y
    150, 152,       // w, h
    0, 0,           // xscroll, yscroll
    150, 152,       // wfull, hfull
    150, 152,       // wmin, hmin
    150, 152,       // wmax, hmax
    _serv_icon,     // icon
    "Server",       // title text
    0,              // no status text
    0,              // no menu
    &_serv_ctrls};  // controls

void _set_but2(char* str) {
    _serv_c_but2.param = (unsigned short)str;
    Win_Redraw_Ext(_serv_winID, 11, 0);
}

void _serv_thread(void) {
    // open server
    _set_but2("Opening...");
    _serv_port = atoi(_port_buf);
    _serv_socket = UDP_Open(1, _serv_port, _symbank);
    if (_serv_socket == -1) {
        Net_ErrMsg(&_serv_form);
        goto _thread_done;
    }
    _set_but2("Disconnect");

    // main event loop
    for (;;) {
        _servmsg[0] = 0;
        Msg_Sleep(_servpid, -1, _servmsg);
        switch (_servmsg[0]) {
        case 255: // shut yourself down
            goto _thread_done;
        }
    }

_thread_done:
    if (_serv_socket != -1)
        UDP_Close(_serv_socket);
    _set_but2("Open Server");
    _servpid = 0;
    thread_quit(_servenv);
}

unsigned char _ask_reset(void) {
    unsigned char i;
    if (_clients) {
        i = MsgBox("This will close all open", "client connections. Proceed?", "", COLOR_BLACK, BUTTON_YN | TITLE_CONFIRM, 0, &_serv_form);
        if (i != MSGBOX_YES)
            return 1;
    }
    return 0;
}

void _disconnect(void) {
    if (_servpid) {
        _clients = 0;
        _serv_list.lines = 0;
        _msemaon();
        _symmsg[0] = 255; // shut yourself down
        Msg_Send(_msgpid(), _servpid, _symmsg);
        _msemaoff();
    }
}

void _connect(void) {
    if (!_servpid)
        _servpid = thread_start(_serv_thread, _servenv, sizeof(_servenv));
}

void _serv_reset(void) {
    char i;
    if (_ask_reset())
        return;
    _disconnect();
    srand(Sys_Counter());
    for (i = 0; i < 6; ++i)
        _code_buf[i] = _codechars[rand() & 0x0F];
    _code_buf[6] = ' ';
    _code_buf[7] = ' ';
    _code_buf[8] = 0;
    Win_Redraw_Ext(_serv_winID, 7, 0);
}

unsigned char Net_ServerWin(void* modalWin, unsigned char clients, unsigned short port, unsigned short bufsize) {
    unsigned char i;

    // set up form
    _serv_form.x = Screen_Width() / 2 - (_serv_form.w / 2);
    _serv_form.y = Screen_Height() / 2 - (_serv_form.h / 2) - 12;
    if (port) {
        // ltoa() is not thread-safe by definition, so we do nasty stuff with itoa()
        itoa(port / 10, _port_buf, 10);
        i = strlen(_port_buf);
        _port_buf[i++] = '0' + (port % 10);
        _port_buf[i] = 0;
        _serv_cd_port.cursor = i;
        _serv_cd_port.len = i;
    }
    strcpy(_ip_buf, "Loading IP...");
    if (!_code_buf[0])
        _serv_reset();

    // open form
    _serv_winID = Win_Open(_symbank, &_serv_form);
    if ((Window*)modalWin) {
        ((Window*)modalWin)->modal = _serv_winID + 1;
        _serv_form.flags |= WIN_MODAL;
    }

    // update IP
    _serv_ip = Net_PublicIP();
    if (_serv_ip)
        iptoa(_serv_ip, _ip_buf);
    else
        strcpy(_ip_buf, "(lookup failed)");
    Win_Redraw_Ext(_serv_winID, 2, 0);

    // handle events
	while (1) {
		_symmsg[0] = 0;
		Msg_Sleep(_msgpid(), -1, _symmsg);
		if (_symmsg[0] == MSR_DSK_WCLICK && _symmsg[1] == _serv_winID) {
            switch (_symmsg[2]) {
            case DSK_ACT_CLOSE: // Alt+F4 or click close
                goto _done;
                break;
            case DSK_ACT_CONTENT: // content click
                switch (_symmsg[8]) {
                case 8: // Reset
                    _serv_reset();
                    break;
                case 11: // Open/Disconnect
                    if (_servpid) {
                        if (!_ask_reset())
                            _disconnect();
                    } else {
                        _connect();
                    }
                    break;
                case 12: // Close
                    goto _done;
                    break;
                }
                break;
            }
		}
	}
_done:

	// close form
	if ((Window*)modalWin) {
        ((Window*)modalWin)->modal = 0;
        _serv_form.flags &= ~WIN_MODAL;
	}
	Win_Close(_serv_winID);
}
