#include <symbos.h>
#include <network.h>
#include <string.h>
#include "network.h"

_data char _chatnull[2] = {0, 0};
_data char _chatbuf[2048];
char* _chatbufptr;
Ctrl* _chatctrl;

_data char _chatinput[129];
char _clientcolors[8] = {COLOR_BLUE, COLOR_ORANGE, COLOR_RED, COLOR_LGREEN, COLOR_MAGENTA, COLOR_GREEN, COLOR_GRAY, COLOR_LRED};

_data char _chat_sendimg[82] = {
    0x06, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x18,
    0x88, 0x88, 0x88, 0x88, 0x51, 0x18, 0x66, 0x66, 0x66, 0x66, 0x51, 0x18, 0x66, 0x11, 0x11, 0x66, 0x51,
    0x18, 0x61, 0x88, 0x88, 0x16, 0x51, 0x18, 0x61, 0x88, 0x88, 0x16, 0x51, 0x18, 0x61, 0x88, 0x88, 0x16,
    0x51, 0x18, 0x66, 0x11, 0x18, 0x16, 0x51, 0x18, 0x66, 0x66, 0x61, 0x16, 0x51, 0x18, 0x66, 0x66, 0x66,
    0x66, 0x51, 0x15, 0x55, 0x55, 0x55, 0x55, 0x51, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

_transfer Ctrl_Input cd_chatinput = {_chatinput, 0, 0, 0, 0, 128};
_transfer Ctrl_Text_Ctrl cd_chatlines[48] = {
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0},
    {_chatnull, 128, (char*)-1, (COLOR_BLACK << 4) | COLOR_WHITE, 0}};

_transfer Ctrl c_chatarea = {0, C_AREA, -1, COLOR_WHITE, 0, 0, 10000, 10000};
_transfer Ctrl c_chatlines[48] = {
    {1, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[0], 1, 1, 10000, 8},
    {2, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[1], 1, 9, 10000, 8},
    {3, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[2], 1, 17, 10000, 8},
    {4, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[3], 1, 25, 10000, 8},
    {5, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[4], 1, 33, 10000, 8},
    {6, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[5], 1, 41, 10000, 8},
    {7, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[6], 1, 49, 10000, 8},
    {8, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[7], 1, 57, 10000, 8},
    {9, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[8], 1, 65, 10000, 8},
    {10, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[9], 1, 73, 10000, 8},
    {11, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[10], 1, 81, 10000, 8},
    {12, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[11], 1, 89, 10000, 8},
    {13, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[12], 1, 97, 10000, 8},
    {14, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[13], 1, 105, 10000, 8},
    {15, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[14], 1, 113, 10000, 8},
    {16, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[15], 1, 121, 10000, 8},
    {17, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[16], 1, 129, 10000, 8},
    {18, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[17], 1, 137, 10000, 8},
    {19, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[18], 1, 145, 10000, 8},
    {20, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[19], 1, 153, 10000, 8},
    {21, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[20], 1, 161, 10000, 8},
    {22, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[21], 1, 169, 10000, 8},
    {23, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[22], 1, 177, 10000, 8},
    {24, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[23], 1, 185, 10000, 8},
    {25, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[24], 1, 193, 10000, 8},
    {26, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[25], 1, 201, 10000, 8},
    {27, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[26], 1, 209, 10000, 8},
    {28, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[27], 1, 217, 10000, 8},
    {29, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[28], 1, 225, 10000, 8},
    {30, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[29], 1, 233, 10000, 8},
    {31, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[30], 1, 241, 10000, 8},
    {32, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[31], 1, 249, 10000, 8},
    {33, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[32], 1, 257, 10000, 8},
    {34, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[33], 1, 265, 10000, 8},
    {35, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[34], 1, 273, 10000, 8},
    {36, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[35], 1, 281, 10000, 8},
    {37, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[36], 1, 289, 10000, 8},
    {38, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[37], 1, 297, 10000, 8},
    {39, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[38], 1, 305, 10000, 8},
    {40, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[39], 1, 313, 10000, 8},
    {41, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[40], 1, 321, 10000, 8},
    {42, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[41], 1, 329, 10000, 8},
    {43, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[42], 1, 337, 10000, 8},
    {44, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[43], 1, 345, 10000, 8},
    {45, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[44], 1, 353, 10000, 8},
    {46, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[45], 1, 361, 10000, 8},
    {47, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[46], 1, 369, 10000, 8},
    {48, C_TEXT_CTRL, -1, (unsigned short)&cd_chatlines[47], 1, 377, 10000, 8}};
_transfer Ctrl c_chatinput = {49, C_INPUT, -1, (unsigned short)&cd_chatinput, 1, 386, 186, 12};
_transfer Ctrl c_chatsend = {50, C_IMAGE_EXT, -1, (unsigned short)_chat_sendimg, 187, 386, 12, 12};

_transfer Ctrl_Group cg_chat = {51, 0, &c_chatarea};
_transfer Ctrl_Collection cc_chatbox = {&cg_chat, 1000, 398, 0, 0, CSCROLL_V};

void Chat_Init(void* box_ctrl) {
    char i;

    // basically Gfx_Prep_Raw, but without the dependency on libgfx
    *((unsigned short*)&_chat_sendimg[3]) = (unsigned short)_chat_sendimg + 10;
    *((unsigned short*)&_chat_sendimg[5]) = (unsigned short)_chat_sendimg + 9;

    // configure box
    _chatctrl = box_ctrl;
    _chatctrl->param = (unsigned short)&cc_chatbox;
    cc_chatbox.yscroll = cc_chatbox.hfull - _chatctrl->h + 1;
    c_chatinput.w = _chatctrl->w - 22;
    c_chatsend.x = _chatctrl->w - 21;

    // clear lines/buffers
    for (i = 0; i < 48; ++i)
        cd_chatlines[i].text = (unsigned short)_chatnull;
    _chatbufptr = _chatbuf;
    _chatbuf[0] = 0;
    _chatinput[0] = 0;
    cd_chatinput.scroll = 0;
    cd_chatinput.cursor = 0;
    cd_chatinput.selection = 0;
    cd_chatinput.len = 0;
}

unsigned char _wraplen(char* str, int max_width) {
    unsigned char i = 0;
    unsigned char last_space = 0;
    int cur_width = 0;
    while (*str) {
        if (cur_width > max_width) {
            if (last_space == 0)
                return i;
            else
                return last_space;
        }
        if (*str == ' ')
            last_space = i;
        else if (*str == '-')
            last_space = i+1;
        cur_width += Text_Width(_symbank, str, 1);
        ++str;
        ++i;
    }
    return i;
}

void Chat_Msg(signed char client, char* msg) {
    char i, first = 1;
    char* start;
    int usedwidth;
    int len = strlen(msg) + 21;
    if (_chatbufptr > _chatbuf + sizeof(_chatbuf) - len)
        _chatbufptr = _chatbuf;

    // FIXME clear out lines we'll overwrite

    // write new line(s)
    while (*msg) {
        for (i = 0; i < 47; ++i)
            cd_chatlines[i].text = cd_chatlines[i+1].text;
        start = _chatbufptr;
        usedwidth = 0;
        if (first) {
            if (client >= 0) {
                *_chatbufptr++ = 1;
                *_chatbufptr++ = (_clientcolors[client & 7] << 4) | COLOR_WHITE;
                strcpy(_chatbufptr, &_clientnames[client][0]);
                _chatbufptr += strlen(_chatbufptr);
                *_chatbufptr++ = 1;
                *_chatbufptr++ = (COLOR_BLACK << 4) | COLOR_WHITE;
                *_chatbufptr++ = ':';
                *_chatbufptr++ = ' ';
                usedwidth = Text_Width(_symbank, &_clientnames[client][0], -1) + 6;
            }
            first = 0;
        }
        len = _wraplen(msg, _chatctrl->w - usedwidth - 8);
        memcpy(_chatbufptr, msg, len);
        _chatbufptr += len;
        *_chatbufptr++ = 0;
        msg += len;
        cd_chatlines[47].text = start;
    }
}
