// Compile with: cc widget.c -w 0,64,48

#include <symbos.h>

// control extended data records
_transfer Ctrl_Text cd_text1 = {"Hello world", (COLOR_YELLOW << 2) | COLOR_BLACK, ALIGN_LEFT};

// control data records
_transfer Ctrl c_area = {0, C_AREA, -1, COLOR_BLACK, 0, 0, 10000, 10000};
_transfer Ctrl c_text1 = {1, C_TEXT, -1, (unsigned short)&cd_text1, 4, 4, 100, 8};

// control group and collection
_transfer Ctrl_Group ctrls = {2, 0, &c_area};
_transfer Ctrl_Collection ctrls_col = {&ctrls, 10000, 10000, 0, 0, CSCROLL_NONE};

int main(int argc, char *argv[]) {
    unsigned short resp;
    unsigned char sender_pid;

	// event loop
	while (1) {
		resp = Msg_Sleep(_sympid, -1, _symmsg);
		if (resp & 1) { // check if we actually received a message
            sender_pid = resp >> 8; // get sender PID from high byte of resp

            if (_symmsg[0] == 0) {
                // message 0 = request to close
                exit(0);

            } else if (_symmsg[0] == MSR_DSK_WCLICK || _symmsg[0] == MSC_WDG_CLICK) {
                // normal window interaction
                switch (_symmsg[2]) {
                    case DSK_ACT_CLOSE:
                        // request to close
                        exit(0);

                    case DSK_ACT_CONTENT:
                        // user interacted with widget controls
                        // ...
                        break;
                }

            } else if (_symmsg[0] == MSC_WDG_SIZE) {
                // widget was loaded or resized
                Widget_Init(&ctrls_col, _symmsg, sender_pid);

            } else if (_symmsg[0] == MSC_WDG_PROP) {
                // user asked to open properties window
                // ...
            }
		}
	}
}
