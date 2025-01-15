#include <symbos.h>

// text buffer
char textbuf[256];

// window 8x8 icon
_transfer char icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};

// controls
_transfer Ctrl c_area = {0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000};
_transfer Ctrl c_button1 = {1, C_BUTTON, -1, (unsigned short)"Music", 10, 10, 64, 12};
_transfer Ctrl c_button2 = {2, C_BUTTON, -1, (unsigned short)"Effect", 84, 10, 64, 12};

// control group
_transfer Ctrl_Group ctrls = {3, 0, &c_area};

// window data record
_transfer Window form1 = {
    WIN_NORMAL,
    WIN_CLOSE | WIN_TITLE | WIN_ICON,
    0,            // PID
    50, 50,       // x, y
    158, 32,      // w, h
    0, 0,         // xscroll, yscroll
    158, 32,      // wfull, hfull
    158, 32,      // wmin, hmin
    158, 32,      // wmax, hmax
    icon,         // icon
    "Sound Demo", // title text
    0,            // no status text
    0,            // no menu
    &ctrls};      // controls

char winID;

int main(int argc, char *argv[]) {
    unsigned char fid;

	// initialize sound daemon
	if (Sound_Init())
        MsgBox("Sound daemon is not running.", "", "", COLOR_BLACK, BUTTON_OK, 0, 0);

    // load the startup music
    Dir_PathAdd(0, "STARTUP.SPM", textbuf); // filename -> absolute path relative to location of .exe
    fid = File_Open(_symbank, textbuf);
    if (fid <= 7) { // >7 = error condition (see docs)
        Music_Load(fid, SOUND_PSG);
        File_Close(fid);
    } else {
        MsgBox("Unable to open STARTUP.SPM.", "", "", COLOR_BLACK, BUTTON_OK, 0, 0);
    }

    // open window
	winID = Win_Open(_symbank, &form1);

    // main event loop
	while (1) {
		_symmsg[0] = 0;
		Msg_Sleep(_sympid, -1, _symmsg);
		if (_symmsg[0] == MSR_DSK_WCLICK) {
			switch (_symmsg[2]) {
				case DSK_ACT_CLOSE: // Alt+F4 or click close
					exit();

                case DSK_ACT_CONTENT: // control clicked
                    if (_symmsg[8] == 1)        // control ID 1 clicked (button1), start playing track 1
                        Music_Start(0);
                    else if (_symmsg[8] == 2)   // control ID 2 clicked (button1), start playing a system effect (handle 0)
                        Effect_Play(0, FX_SLIDE1, 255, FX_ANY, PAN_MIDDLE, 0);
                    break;
			}
		}
	}
}
