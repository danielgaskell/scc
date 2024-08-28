#include <symbos.h>

// window 8x8 icon
_transfer char icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};

// extended data record for c_text1
_transfer Ctrl_Text cd_text1 = {
	"Hello world!",                    // text
	(COLOR_BLACK << 2) | COLOR_ORANGE, // color
	ALIGN_LEFT};                       // flags

// two controls, immediately after each other
_transfer Ctrl c_area = {
	0, C_AREA, -1,             // control ID, type, bank
	COLOR_ORANGE,              // param (color)
	0, 0,                      // x, y
	10000, 10000};             // width, height
_transfer Ctrl c_text1 = {
	0, C_TEXT, -1,             // control ID, type, bank
	(unsigned short)&cd_text1, // param (extended data record)
	20, 10,                    // x, y
	100, 8};                   // width, height

// control group
_transfer Ctrl_Group ctrls = {
	2, 0,                      // number of controls, process ID
	&c_area};                  // address of first control

// window data record
_transfer Window form1 = {
    WIN_NORMAL,
    WIN_CLOSE | WIN_TITLE | WIN_ICON,
    0,          // PID
    10, 10,     // x, y
    178, 110,   // w, h
    0, 0,       // xscroll, yscroll
    178, 110,   // wfull, hfull
    178, 110,   // wmin, hmin
    178, 110,   // wmax, hmax
    icon,       // icon
    "Form 1",   // title text
    0,          // no status text
    0,          // no menu
    &ctrls};    // controls

char winID;

int main(int argc, char *argv[]) {
	winID = Win_Open(_symbank, &form1);

	while (1) {
		// handle events
		_symmsg[0] = 0;
		Msg_Sleep(_sympid, -1, _symmsg);
		if (_symmsg[0] == MSR_DSK_WCLICK) {
			switch (_symmsg[2]) {
				case DSK_ACT_CLOSE: // Alt+F4 or click close
					exit();
				// more event cases go here...
			}
		}
	}
}
