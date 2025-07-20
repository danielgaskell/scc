#include <symbos.h>
#include <string.h>

// window controls
_transfer Ctrl_Text _inp_cd_text1 = {0, (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_Text _inp_cd_text2 = {0, (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl_Input _inp_cd_input = {0, 0, 0, 0, 16, 16};
_transfer Ctrl _inp_c_area = {0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000};
_transfer Ctrl _inp_c_text1 = {1, C_TEXT, -1, (unsigned short)&_inp_cd_text1, 5, 5, 150, 8};
_transfer Ctrl _inp_c_text2 = {2, C_TEXT, -1, (unsigned short)&_inp_cd_text2, 5, 13, 150, 8};
_transfer Ctrl _inp_c_input = {3, C_INPUT, -1, (unsigned short)&_inp_cd_input, 5, 23, 150, 12};
_transfer Ctrl _inp_c_but2 = {4, C_BUTTON, -1, (unsigned short)"OK", 32, 39, 46, 12};
_transfer Ctrl _inp_c_but3 = {5, C_BUTTON, -1, (unsigned short)"Cancel", 82, 39, 46, 12};
_transfer Ctrl_Group _inp_ctrls = {6, 0, &_inp_c_area, 0, 0, 4, 5, {0}, 3};

// window data record
_transfer Window _inp_form = {
    WIN_NORMAL,
    WIN_CLOSE | WIN_TITLE | WIN_NOTTASKBAR,
    0,              // PID
    10, 10,         // x, y
    160, 56,        // w, h
    0, 0,           // xscroll, yscroll
    160, 56,        // wfull, hfull
    160, 56,        // wmin, hmin
    160, 56,        // wmax, hmax
    0,              // icon
    0,              // title text
    0,              // no status text
    0,              // no menu
    &_inp_ctrls};  // controls

char _inp_winID = 0;

signed char InputBox(char* title, char* line1, char* line2, char* buffer, unsigned short buflen, void* modalWin) {
    signed char result = -1;
    unsigned short response;
    while (_inp_winID); // multithreading semaphore

    // open form
    _inp_form.x = Screen_Width() / 2 - (_inp_form.w / 2);
    _inp_form.y = Screen_Height() / 2 - (_inp_form.h / 2) - 12;
    if (title)
        _inp_form.title = title;
    else
        _inp_form.title = "Input";
    _inp_cd_text1.text = line1 ? line1 : "";
    _inp_cd_text2.text = line2 ? line2 : "";
    _inp_cd_input.text = buffer;
    _inp_cd_input.scroll = 0;
    _inp_cd_input.cursor = 0;
    _inp_cd_input.selection = 0;
    _inp_cd_input.len = strlen(buffer);
    _inp_cd_input.maxlen = buflen - 1;
    _inp_winID = Win_Open(_symbank, &_inp_form);
    if ((Window*)modalWin) {
        ((Window*)modalWin)->modal = _inp_winID + 1;
        _inp_form.flags |= WIN_MODAL;
    }

    // handle events
	while (1) {
		_symmsg[0] = 0;
		response = Msg_Sleep(_msgpid(), 2, _symmsg);
		if (_symmsg[1] == _inp_winID) {
            if (_symmsg[0] == MSR_DSK_WCLICK) {
                switch (_symmsg[2]) {
                case DSK_ACT_CLOSE: // Alt+F4 or click close
                    goto _done;
                    break;
                case DSK_ACT_CONTENT: // content click
                    switch (_symmsg[8]) {
                    case 3: // input
                        if (_symmsg[3] == DSK_SUB_KEY && _symmsg[4] == '\r') {
                            // Enter key in textbox
                            result = 0;
                            goto _done;
                        }
                        break;
                    case 4: // OK
                        result = 0;
                        goto _done;
                        break;
                    case 5: // Cancel
                        goto _done;
                        break;
                    }
                    break;
                }
            }
		} else if (response & 1) {
		    Msg_Send(2, _msgpid(), _symmsg); // something else, keep it on the queue
		}
	}
_done:

	// close form
	if ((Window*)modalWin) {
        ((Window*)modalWin)->modal = 0;
        _inp_form.flags &= ~WIN_MODAL;
	}
	Win_Close(_inp_winID);
	_inp_winID = 0;
	return result;
}
