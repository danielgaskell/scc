#include <symbos.h>
#include <network.h>
#include "network.h"

_transfer char _serv_icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                                 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};

_data char _ip_buf_public[34];
_data char _ip_buf_local[34];
_data char _code_buf[7];

_transfer List_Column _serv_l1_col1 = {ALIGN_LEFT | LTYPE_TEXT, 0, 63, "Nickname"};
_transfer List_Column _serv_l1_col2 = {ALIGN_LEFT | LTYPE_TEXT, 0, 63, "IP Address"};

_transfer List_Row2Col _serv_cd_lr1 = {1, "Prevtenet", "192.168.0.1"};
_transfer List_Row2Col _serv_cd_lr2 = {2, "Zipper", "10.0.0.1"};

_transfer List _serv_l1 = {2, 0, &_serv_cd_lr1, 0, 2, 1, &_serv_l1_col1, 0, LIST_MULTI | LIST_SCROLL};

_transfer Ctrl_TFrame _serv_cd_tf1 = {"IP Address", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl_Text _serv_cd_text1 = {_ip_buf_public, (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_Text _serv_cd_text2 = {_ip_buf_local, (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_TFrame _serv_cd_tf2 = {"Session Code", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl_Text _serv_cd_text3 = {_code_buf, (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_TFrame _serv_cd_tf3 = {"Connected Clients", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl _serv_c_area = {0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000};
_transfer Ctrl _serv_c_tf1 = {1, C_TFRAME, -1, (unsigned short)&_serv_cd_tf1, 2, 3, 146, 37};
_transfer Ctrl _serv_c_text1 = {2, C_TEXT, -1, (unsigned short)&_serv_cd_text1, 11, 13, 64, 8};
_transfer Ctrl _serv_c_text2 = {3, C_TEXT, -1, (unsigned short)&_serv_cd_text2, 11, 23, 64, 8};
_transfer Ctrl _serv_c_tf2 = {4, C_TFRAME, -1, (unsigned short)&_serv_cd_tf2, 2, 40, 146, 28};
_transfer Ctrl _serv_c_text3 = {5, C_TEXT, -1, (unsigned short)&_serv_cd_text3, 11, 51, 64, 8};
_transfer Ctrl _serv_c_but1 = {6, C_BUTTON, -1, (unsigned short)"Reset", 92, 48, 48, 12};
_transfer Ctrl _serv_c_tf3 = {7, C_TFRAME, -1, (unsigned short)&_serv_cd_tf3, 2, 68, 146, 67};
_transfer Ctrl _serv_c_l1 = {8, C_LISTFULL, -1, (unsigned short)&_serv_l1, 8, 77, 134, 52};
_transfer Ctrl _serv_c_but2 = {9, C_BUTTON, -1, (unsigned short)"Disconnect", 5, 136, 68, 12};
_transfer Ctrl _serv_c_but3 = {10, C_BUTTON, -1, (unsigned short)"Open Server", 77, 136, 68, 12};
_transfer Ctrl_Group _serv_ctrls = {11, 0, &_serv_c_area};

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

unsigned char Net_ServerWin(Window* modal_win) {
    char winID;

    // set up form
    _serv_form.x = Screen_Width() / 2 - (_serv_form.w / 2);
    _serv_form.y = Screen_Height() / 2 - (_serv_form.h / 2) - 12;
    strcpy(_ip_buf_public, "Remote clients: Loading...      ");
    strcpy(_ip_buf_local, "Local clients: Loading...      ");
    strcpy(_code_buf, "A27BC9"); // FIXME randomize

    // open form
    winID = Win_Open(_symbank, &_serv_form);
    if (modal_win) {
        modal_win->modal = winID + 1;
        _serv_form.flags |= WIN_MODAL;
    }

    // handle events
	while (1) {
		_symmsg[0] = 0;
		Msg_Sleep(_sympid, -1, _symmsg);
		if (_symmsg[0] == MSR_DSK_WCLICK && _symmsg[1] == winID) {
            switch (_symmsg[2]) {
                case DSK_ACT_CLOSE: // Alt+F4 or click close
                    goto _done;
            }
		}
	}
_done:

	// close form
	if (modal_win) {
        modal_win->modal = 0;
        _serv_form.flags &= ~WIN_MODAL;
	}
	Win_Close(winID);
}
