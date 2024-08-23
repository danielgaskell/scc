#include <symbos.h>
#include <string.h>

char winID;

_transfer char display[65];
_transfer char bits_str[36];
_transfer char check1 = 1;
_transfer char radiog1 = 2;
_transfer char radiog2 = 1;
_transfer char radiog1_coord[4] = {-1, -1, -1, -1};
_transfer char radiog2_coord[4] = {-1, -1, -1, -1};

// control data
_transfer CTRL_TEXT cd_display = { display, COLOR_BLACK*4 | COLOR_YELLOW, ALIGN_RIGHT };
_transfer CTRL_TEXT cd_bits = { bits_str, COLOR_BLACK*4 | COLOR_ORANGE, ALIGN_RIGHT };
_transfer CTRL_RADIO cd_radio1 = { &radiog1, "Hex", COLOR_BLACK*4 | COLOR_ORANGE, 1, radiog1_coord };
_transfer CTRL_RADIO cd_radio2 = { &radiog1, "Dec", COLOR_BLACK*4 | COLOR_ORANGE, 2, radiog1_coord };
_transfer CTRL_RADIO cd_radio3 = { &radiog1, "Oct", COLOR_BLACK*4 | COLOR_ORANGE, 3, radiog1_coord };
_transfer CTRL_RADIO cd_radio4 = { &radiog1, "Bin", COLOR_BLACK*4 | COLOR_ORANGE, 4, radiog1_coord };
_transfer CTRL_RADIO cd_radio5 = { &radiog2, "Long", COLOR_BLACK*4 | COLOR_ORANGE, 1, radiog2_coord };
_transfer CTRL_RADIO cd_radio6 = { &radiog2, "Word", COLOR_BLACK*4 | COLOR_ORANGE, 2, radiog2_coord };
_transfer CTRL_RADIO cd_radio7 = { &radiog2, "Byte", COLOR_BLACK*4 | COLOR_ORANGE, 3, radiog2_coord };
_transfer CTRL_CHECK cd_check1 = { &check1, "Signed", COLOR_BLACK*4 | COLOR_ORANGE };

// controls
_transfer CTRL c_area = { 0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000 };
_transfer CTRL c_outline = { 1, C_FRAME, -1, AREA_FILL | COLOR_YELLOW*16 | COLOR_BLACK*4 | COLOR_BLACK, 2, 2, 174, 12 };
_transfer CTRL c_display = { 2, C_TEXT, -1, (unsigned short)&cd_display, 4, 4, 170, 8 };
_transfer CTRL c_bits = { 3, C_TEXT, -1, (unsigned short)&cd_bits, 4, 16, 170, 8 };
_transfer CTRL c_frame1 = { 4, C_FRAME, -1, COLOR_BLACK*4 | COLOR_BLACK, 2, 26, 37, 46 };
_transfer CTRL c_radio1 = { 5, C_RADIO, -1, (unsigned short)&cd_radio1, 6, 30, 28, 8 };
_transfer CTRL c_radio2 = { 6, C_RADIO, -1, (unsigned short)&cd_radio2, 6, 40, 28, 8 };
_transfer CTRL c_radio3 = { 7, C_RADIO, -1, (unsigned short)&cd_radio3, 6, 50, 28, 8 };
_transfer CTRL c_radio4 = { 8, C_RADIO, -1, (unsigned short)&cd_radio4, 6, 60, 28, 8 };
_transfer CTRL c_frame2 = { 9, C_FRAME, -1, COLOR_BLACK*4 | COLOR_BLACK, 2, 74, 37, 34 };
_transfer CTRL c_radio5 = { 10, C_RADIO, -1, (unsigned short)&cd_radio5, 6, 77, 28, 8 };
_transfer CTRL c_radio6 = { 11, C_RADIO, -1, (unsigned short)&cd_radio6, 6, 87, 28, 8 };
_transfer CTRL c_radio7 = { 12, C_RADIO, -1, (unsigned short)&cd_radio7, 6, 97, 28, 8 };
_transfer CTRL c_button1 = { 13, C_BUTTON, -1, (unsigned short)"Abs", 42, 26, 20, 12 };
_transfer CTRL c_button2 = { 14, C_BUTTON, -1, (unsigned short)"LRo", 42, 40, 20, 12 };
_transfer CTRL c_button3 = { 15, C_BUTTON, -1, (unsigned short)"LSh", 42, 54, 20, 12 };
_transfer CTRL c_button4 = { 16, C_BUTTON, -1, (unsigned short)"And", 42, 68, 20, 12 };
_transfer CTRL c_button5 = { 17, C_BUTTON, -1, (unsigned short)"Not", 42, 82, 20, 12 };
_transfer CTRL c_button6 = { 18, C_BUTTON, -1, (unsigned short)"Mod", 64, 26, 20, 12 };
_transfer CTRL c_button7 = { 19, C_BUTTON, -1, (unsigned short)"RRo", 64, 40, 20, 12 };
_transfer CTRL c_button8 = { 20, C_BUTTON, -1, (unsigned short)"RSh", 64, 54, 20, 12 };
_transfer CTRL c_button9 = { 21, C_BUTTON, -1, (unsigned short)"Or", 64, 68, 20, 12 };
_transfer CTRL c_button10 = { 22, C_BUTTON, -1, (unsigned short)"Xor", 64, 82, 20, 12 };
_transfer CTRL c_check1 = { 22, C_CHECK, -1, (unsigned short)&cd_check1, 45, 98, 32, 12 };
_transfer CTRL c_button11 = { 23, C_BUTTON, -1, (unsigned short)"A", 88, 26, 16, 12 };
_transfer CTRL c_button12 = { 24, C_BUTTON, -1, (unsigned short)"B", 88, 40, 16, 12 };
_transfer CTRL c_button13 = { 25, C_BUTTON, -1, (unsigned short)"C", 88, 54, 16, 12 };
_transfer CTRL c_button14 = { 26, C_BUTTON, -1, (unsigned short)"D", 88, 68, 16, 12 };
_transfer CTRL c_button15 = { 27, C_BUTTON, -1, (unsigned short)"E", 88, 82, 16, 12 };
_transfer CTRL c_button16 = { 28, C_BUTTON, -1, (unsigned short)"F", 88, 96, 16, 12 };
_transfer CTRL c_button17 = { 29, C_BUTTON, -1, (unsigned short)"MC", 106, 26, 16, 12 };
_transfer CTRL c_button18 = { 30, C_BUTTON, -1, (unsigned short)"<-", 106, 40, 16, 12 };
_transfer CTRL c_button19 = { 31, C_BUTTON, -1, (unsigned short)"7", 106, 54, 16, 12 };
_transfer CTRL c_button20 = { 32, C_BUTTON, -1, (unsigned short)"4", 106, 68, 16, 12 };
_transfer CTRL c_button21 = { 33, C_BUTTON, -1, (unsigned short)"1", 106, 82, 16, 12 };
_transfer CTRL c_button22 = { 34, C_BUTTON, -1, (unsigned short)"0", 106, 96, 34, 12 };
_transfer CTRL c_button23 = { 35, C_BUTTON, -1, (unsigned short)"MR", 124, 26, 16, 12 };
_transfer CTRL c_button24 = { 36, C_BUTTON, -1, (unsigned short)"CE", 124, 40, 16, 12 };
_transfer CTRL c_button25 = { 37, C_BUTTON, -1, (unsigned short)"8", 124, 54, 16, 12 };
_transfer CTRL c_button26 = { 38, C_BUTTON, -1, (unsigned short)"9", 124, 68, 16, 12 };
_transfer CTRL c_button27 = { 39, C_BUTTON, -1, (unsigned short)"2", 124, 82, 16, 12 };
_transfer CTRL c_button28 = { 40, C_BUTTON, -1, (unsigned short)"MS", 142, 26, 16, 12 };
_transfer CTRL c_button29 = { 41, C_BUTTON, -1, (unsigned short)"C", 142, 40, 16, 12 };
_transfer CTRL c_button30 = { 42, C_BUTTON, -1, (unsigned short)"9", 142, 54, 16, 12 };
_transfer CTRL c_button31 = { 43, C_BUTTON, -1, (unsigned short)"6", 142, 68, 16, 12 };
_transfer CTRL c_button32 = { 44, C_BUTTON, -1, (unsigned short)"3", 142, 82, 16, 12 };
_transfer CTRL c_button34 = { 45, C_BUTTON, -1, (unsigned short)"Sq", 160, 26, 16, 12 };
_transfer CTRL c_button35 = { 46, C_BUTTON, -1, (unsigned short)"/", 160, 40, 16, 12 };
_transfer CTRL c_button36 = { 47, C_BUTTON, -1, (unsigned short)"*", 160, 54, 16, 12 };
_transfer CTRL c_button37 = { 48, C_BUTTON, -1, (unsigned short)"-", 160, 68, 16, 12 };
_transfer CTRL c_button38 = { 49, C_BUTTON, -1, (unsigned short)"+", 160, 82, 16, 12 };
_transfer CTRL c_button39 = { 50, C_BUTTON, -1, (unsigned short)"=", 142, 96, 34, 12 };

// control group
_transfer CTRL_GROUP ctrls = { 52, 0, &c_area };

// window
_transfer WINDOW form = {
    WIN_NORMAL,
    WIN_CLOSE | WIN_TITLE | WIN_NOTTASKBAR,
    0,          // PID
    10, 10,     // x, y
    178, 110,   // w, h
    0, 0,       // xscroll, yscroll
    178, 110,   // wfull, hfull
    178, 110,   // wmin, hmin
    178, 110,   // wmax, hmax
    0,          // icon
    "Programmer's Calculator",
    0,          // status text
    0,          // menu
    &ctrls};    // controls

int main(int argc, char *argv[]) {
    strcpy(display, "0");
    strcpy(bits_str, "00000000 00000000 00000000 00000000");

    winID = Win_Open(_symbank, &form);

    while (1) {
		// handle events
		_symmsg[0] = 0;
		Msg_Sleep(_sympid, -1, _symmsg);
        if (_symmsg[0] == MSR_DSK_WCLICK) {
            switch (_symmsg[2]) {
                case DSK_ACT_CLOSE: // Alt+F4 or Click Close
                    return 0;
                    break;
            }
        }
	}
}
