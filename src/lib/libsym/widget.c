#include <symbos.h>

unsigned char _widget_win;
unsigned char _widget_col;
unsigned short _widget_w;
unsigned short _widget_h;
unsigned char _widget_bank;
char* _widget_coladdr;

void Widget_Init(Ctrl_Collection* col, char* msg, unsigned char send_pid) {
    unsigned char i;
    Ctrl_Group* group = col->controls;
    Ctrl* ptr;

    // get data
    _widget_win = msg[1];
    _widget_col = msg[2];
    _widget_w = _widget_sizes[msg[3]][0];
    _widget_h = _widget_sizes[msg[3]][1];

    // _sympid goes in high byte of each control value
    group->pid = _sympid;
    ptr = group->first;
    for (i = 0; i < group->controls; ++i) {
        ptr->value = (ptr->value & 0xFF) | (_sympid << 8);
        ++ptr;
    }

    // send back response
    _msemaon();
    _symmsg[0] = MSR_DSK_EXTDSK;
    _symmsg[1] = FNC_DXT_WDGOKY;
    *(Ctrl_Collection**)&_symmsg[2] = col;
    _symmsg[4] = _symbank;
    Msg_Send(_sympid, send_pid, _symmsg);
    _msemaoff();

    // get address of collection record
    Win_Addr(_widget_win, &_widget_bank, (Window**)&_widget_coladdr);
    _widget_coladdr = (char*)Bank_ReadWord(_widget_bank, _widget_coladdr + 36); // control group address
    _widget_coladdr = (char*)Bank_ReadWord(_widget_bank, _widget_coladdr + 2); // control data records
    _widget_coladdr += (_widget_col * 16); // collection control record
}

unsigned short Widget_X(void) {
    return Bank_ReadWord(_widget_bank, _widget_coladdr + 6);
}

unsigned short Widget_Y(void) {
    return Bank_ReadWord(_widget_bank, _widget_coladdr + 8);
}
