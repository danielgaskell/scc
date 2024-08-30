#include <symbos.h>
#include <string.h>

char winID = -1; // main window ID (returned by Win_Open())

long value = 0;
long last_value = 0;
long mem_value = 0;
char old_base = 10;
char need_param = 0;
char op = 0;

_transfer char display[36];    // string buffer for the number display
_transfer char bits_str[36];   // string buffer for the bits display
_transfer char check1 = 1;     // global variable for checkbox state
_transfer char base = 10;      // global variable for radio group 1 state
_transfer char bindigits = 32; // global variable for radio group 2 state
_transfer char radiog1_coord[4] = {-1, -1, -1, -1}; // radio group 1 coordinates (required by Ctrl_Radio)
_transfer char radiog2_coord[4] = {-1, -1, -1, -1}; // radio group 2 coordinates (required by Ctrl_Radio)

// control data
_transfer Ctrl_Text cd_display = { display, (COLOR_BLACK << 2) | COLOR_YELLOW | TEXT_FILL, ALIGN_RIGHT };
_transfer Ctrl_Text cd_bits = { bits_str, (COLOR_BLACK << 2) | COLOR_ORANGE | TEXT_FILL, ALIGN_RIGHT };
_transfer Ctrl_Radio cd_radio1 = { &base, "Hex", COLOR_BLACK*4 | COLOR_ORANGE, 16, radiog1_coord };
_transfer Ctrl_Radio cd_radio2 = { &base, "Dec", COLOR_BLACK*4 | COLOR_ORANGE, 10, radiog1_coord };
_transfer Ctrl_Radio cd_radio3 = { &base, "Oct", COLOR_BLACK*4 | COLOR_ORANGE, 8, radiog1_coord };
_transfer Ctrl_Radio cd_radio4 = { &base, "Bin", COLOR_BLACK*4 | COLOR_ORANGE, 2, radiog1_coord };
_transfer Ctrl_Radio cd_radio5 = { &bindigits, "Long", COLOR_BLACK*4 | COLOR_ORANGE, 32, radiog2_coord };
_transfer Ctrl_Radio cd_radio6 = { &bindigits, "Word", COLOR_BLACK*4 | COLOR_ORANGE, 16, radiog2_coord };
_transfer Ctrl_Radio cd_radio7 = { &bindigits, "Byte", COLOR_BLACK*4 | COLOR_ORANGE, 8, radiog2_coord };
_transfer Ctrl_Check cd_check1 = { &check1, "Signed", COLOR_BLACK*4 | COLOR_ORANGE };
  /* A trick: notice how the "value" properties of the radio buttons are set
     directly to the values we need - base, binary digits - so we can just read
     them from the global variables registered to store the radio button state. */

// controls
_transfer Ctrl c_area = { 0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000 };
_transfer Ctrl c_bitsback = { 1, C_AREA, -1, COLOR_ORANGE, 4, 16, 170, 8 };
_transfer Ctrl c_outline = { 2, C_FRAME, -1, AREA_FILL | COLOR_YELLOW*16 | COLOR_BLACK*4 | COLOR_BLACK, 2, 2, 174, 12 };
_transfer Ctrl c_display = { 3, C_TEXT, -1, (unsigned short)&cd_display, 4, 4, 170, 8 };
_transfer Ctrl c_bits = { 4, C_TEXT, -1, (unsigned short)&cd_bits, 4, 16, 170, 8 };
_transfer Ctrl c_frame1 = { 5, C_FRAME, -1, COLOR_BLACK*4 | COLOR_BLACK, 2, 26, 37, 46 };
_transfer Ctrl c_radio1 = { 6, C_RADIO, -1, (unsigned short)&cd_radio1, 6, 30, 28, 8 };
_transfer Ctrl c_radio2 = { 7, C_RADIO, -1, (unsigned short)&cd_radio2, 6, 40, 28, 8 };
_transfer Ctrl c_radio3 = { 8, C_RADIO, -1, (unsigned short)&cd_radio3, 6, 50, 28, 8 };
_transfer Ctrl c_radio4 = { 9, C_RADIO, -1, (unsigned short)&cd_radio4, 6, 60, 28, 8 };
_transfer Ctrl c_frame2 = { 10, C_FRAME, -1, COLOR_BLACK*4 | COLOR_BLACK, 2, 74, 37, 34 };
_transfer Ctrl c_radio5 = { 11, C_RADIO, -1, (unsigned short)&cd_radio5, 6, 77, 28, 8 };
_transfer Ctrl c_radio6 = { 12, C_RADIO, -1, (unsigned short)&cd_radio6, 6, 87, 28, 8 };
_transfer Ctrl c_radio7 = { 13, C_RADIO, -1, (unsigned short)&cd_radio7, 6, 97, 28, 8 };
_transfer Ctrl c_button1 = { 14, C_BUTTON, -1, (unsigned short)"Abs", 42, 26, 20, 12 };
_transfer Ctrl c_button2 = { 15, C_BUTTON, -1, (unsigned short)"LRo", 42, 40, 20, 12 };
_transfer Ctrl c_button3 = { 16, C_BUTTON, -1, (unsigned short)"LSh", 42, 54, 20, 12 };
_transfer Ctrl c_button4 = { 17, C_BUTTON, -1, (unsigned short)"And", 42, 68, 20, 12 };
_transfer Ctrl c_button5 = { 18, C_BUTTON, -1, (unsigned short)"Not", 42, 82, 20, 12 };
_transfer Ctrl c_button6 = { 19, C_BUTTON, -1, (unsigned short)"Mod", 64, 26, 20, 12 };
_transfer Ctrl c_button7 = { 20, C_BUTTON, -1, (unsigned short)"RRo", 64, 40, 20, 12 };
_transfer Ctrl c_button8 = { 21, C_BUTTON, -1, (unsigned short)"RSh", 64, 54, 20, 12 };
_transfer Ctrl c_button9 = { 22, C_BUTTON, -1, (unsigned short)"Or", 64, 68, 20, 12 };
_transfer Ctrl c_button10 = { 23, C_BUTTON, -1, (unsigned short)"Xor", 64, 82, 20, 12 };
_transfer Ctrl c_check1 = { 24, C_CHECK, -1, (unsigned short)&cd_check1, 45, 98, 32, 12 };
_transfer Ctrl c_button11 = { 25, C_BUTTON, -1, (unsigned short)"A", 88, 26, 16, 12 };
_transfer Ctrl c_button12 = { 26, C_BUTTON, -1, (unsigned short)"B", 88, 40, 16, 12 };
_transfer Ctrl c_button13 = { 27, C_BUTTON, -1, (unsigned short)"C", 88, 54, 16, 12 };
_transfer Ctrl c_button14 = { 28, C_BUTTON, -1, (unsigned short)"D", 88, 68, 16, 12 };
_transfer Ctrl c_button15 = { 29, C_BUTTON, -1, (unsigned short)"E", 88, 82, 16, 12 };
_transfer Ctrl c_button16 = { 30, C_BUTTON, -1, (unsigned short)"F", 88, 96, 16, 12 };
_transfer Ctrl c_button17 = { 31, C_BUTTON, -1, (unsigned short)"MC", 106, 26, 16, 12 };
_transfer Ctrl c_button18 = { 32, C_BUTTON, -1, (unsigned short)"<-", 106, 40, 16, 12 };
_transfer Ctrl c_button19 = { 33, C_BUTTON, -1, (unsigned short)"7", 106, 54, 16, 12 };
_transfer Ctrl c_button20 = { 34, C_BUTTON, -1, (unsigned short)"4", 106, 68, 16, 12 };
_transfer Ctrl c_button21 = { 35, C_BUTTON, -1, (unsigned short)"1", 106, 82, 16, 12 };
_transfer Ctrl c_button22 = { 36, C_BUTTON, -1, (unsigned short)"0", 106, 96, 34, 12 };
_transfer Ctrl c_button23 = { 37, C_BUTTON, -1, (unsigned short)"MR", 124, 26, 16, 12 };
_transfer Ctrl c_button24 = { 38, C_BUTTON, -1, (unsigned short)"CE", 124, 40, 16, 12 };
_transfer Ctrl c_button25 = { 39, C_BUTTON, -1, (unsigned short)"8", 124, 54, 16, 12 };
_transfer Ctrl c_button26 = { 40, C_BUTTON, -1, (unsigned short)"5", 124, 68, 16, 12 };
_transfer Ctrl c_button27 = { 41, C_BUTTON, -1, (unsigned short)"2", 124, 82, 16, 12 };
_transfer Ctrl c_button28 = { 42, C_BUTTON, -1, (unsigned short)"MS", 142, 26, 16, 12 };
_transfer Ctrl c_button29 = { 43, C_BUTTON, -1, (unsigned short)"C", 142, 40, 16, 12 };
_transfer Ctrl c_button30 = { 44, C_BUTTON, -1, (unsigned short)"9", 142, 54, 16, 12 };
_transfer Ctrl c_button31 = { 45, C_BUTTON, -1, (unsigned short)"6", 142, 68, 16, 12 };
_transfer Ctrl c_button32 = { 46, C_BUTTON, -1, (unsigned short)"3", 142, 82, 16, 12 };
_transfer Ctrl c_button34 = { 47, C_BUTTON, -1, (unsigned short)"Sq", 160, 26, 16, 12 };
_transfer Ctrl c_button35 = { 48, C_BUTTON, -1, (unsigned short)"/", 160, 40, 16, 12 };
_transfer Ctrl c_button36 = { 49, C_BUTTON, -1, (unsigned short)"*", 160, 54, 16, 12 };
_transfer Ctrl c_button37 = { 50, C_BUTTON, -1, (unsigned short)"-", 160, 68, 16, 12 };
_transfer Ctrl c_button38 = { 51, C_BUTTON, -1, (unsigned short)"+", 160, 82, 16, 12 };
_transfer Ctrl c_button39 = { 52, C_BUTTON, -1, (unsigned short)"=", 142, 96, 34, 12 };

// main control group
_transfer Ctrl_Group ctrls = { 53, 0, &c_area };

// small icon (8x8 4-color SGX format)
_transfer char icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};

// window
_transfer Window form = {
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
    "Programmer's Calculator",
    0,          // status text
    0,          // menu
    &ctrls};    // controls

// convert an ASCII character to a digit (including hex characters)
char char_to_digit(char ch) {
    if (ch <= '9')
        return ch - '0';
    else
        return ch - 'A' + 10;
}

// extract value from display, respecting working base
void display2value(char working_base) {
    signed char i, sign;
    unsigned long placemult;

    // get value, respecting working base
    i = strlen(display) - 1;
    sign = 1;
    value = 0;
    placemult = 1;
    while (i >= 0) {
        if (display[i] == '-') {
            sign = -1;
        } else {
            value += (char_to_digit(display[i]) * placemult);
            placemult *= working_base;
        }
        --i;
    }
    value *= sign;
}

// update display with value, in the current base
void value2display(void) {
    char* ptr = display;

    // call the appropriate function to generate the number
    if (check1)
        ltoa(value, display, base);
    else
        ultoa(value, display, base);

    // convert result to uppercase
    while (*ptr) {
        if (*ptr >= 'a' && *ptr <= 'z')
            *ptr -= ('a' - 'A');
        ++ptr;
    }
}

// update bit display from value
void update_bits(void) {
    signed char i;
    char t;
    long working;

    working = value;
    t = 0;
    i = bindigits + (bindigits / 8) - 1;
    bits_str[i+1] = 0;
    for (; i >= 0; --i) {
        ++t;
        if (t == 9) {
            // insert a space every 8 bits
            bits_str[i] = ' ';
            t = 0;
        } else {
            // normal bit, emit 0 or 1
            if (working & 0x01)
                bits_str[i] = '1';
            else
                bits_str[i] = '0';
            working = working >> 1;
        }
    }
}

// redraw display and bits
void redraw_display(void) {
    Win_Redraw(winID, -3, 2); // redraw the 3 controls starting with ID 2 (i.e., the displays and the main display frame) if requested
}

// integer sqrt (faster and smaller than the float version in math.h)
unsigned long sqrti(unsigned long s) {
    unsigned long x0, x1;
	if (s <= 1)
		return s;
	x0 = s / 2;
	x1 = (x0 + s / x0) / 2;
    while (x1 < x0) {
		x0 = x1;
		x1 = (x0 + s / x0) / 2;
	}
	return x0;
}

// integer rotate left
unsigned long lro(unsigned long val, char by) {
    unsigned long mask;
    if (bindigits == 32)
        mask = 0xFFFFFFFFL;
    else if (bindigits == 16)
        mask = 0xFFFF;
    else
        mask = 0xFF;
    while (by--)
        val = ((val << 1) | (val >> (bindigits - 1))) & mask;
    return val;
}

// integer rotate right
unsigned long rro(unsigned long val, char by) {
    unsigned long mask;
    if (bindigits == 32)
        mask = 0xFFFFFFFFL;
    else if (bindigits == 16)
        mask = 0xFFFF;
    else
        mask = 0xFF;
    while (by--)
        val = ((val >> 1) | (val << (bindigits - 1))) & mask;
    return val;
}

// execute the last operator
void do_op(void) {
    display2value(base);

    // no operator waiting: just set left term
    if (op == 0) {
        last_value = value;
        //MsgBox("1", "", "", COLOR_BLACK, BUTTON_OK, 0);
        return;
    }

    // binary operator waiting: execute it
    switch (op) {
    case 15: value = lro(last_value, value); break;
    case 16: value = last_value << value; break;
    case 17: value = last_value & value; break;
    case 19: value = last_value % value; break;
    case 20: value = rro(last_value, value); break;
    case 21: value = last_value >> value; break;
    case 22: value = last_value | value; break;
    case 23: value = last_value ^ value; break;
    case 48: value = last_value / value; break;
    case 49: value = last_value * value; break;
    case 50: value = last_value - value; break;
    case 51: value = last_value + value; break;
    }
    //MsgBox("2", "", "", COLOR_BLACK, BUTTON_OK, 0);

    // refresh everything
    value2display();
    update_bits();
    redraw_display();
    last_value = value;
    op = 0;
}

// do a unary operator
void do_unary(char uop) {
    display2value(base);
    switch (uop) {
    case 14: value = abs(value); break;
    case 18: value = ~value; break;
    case 47: value = sqrti(abs(value)); break;
    }
    value2display();
    update_bits();
    redraw_display();
}

// handle a keypress reported by the event loop
void handle_key(char key) {
    char tmp[2];
    char i;

    // handle number keys
    if (key >= 'a' && key <= 'f') // convert lowercase to uppercase
        key -= ('a' - 'A');
    if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'F')) {
        // sanity check digits
        if (base < 16 && key >= 'A') // reject A-F if base < 16
            return;
        if (base < 10 && key >= '8') // reject 8-9 if base < 10
            return;
        if (base < 8 && key >= '2')  // reject 2-7 if base < 8
            return;

        // start a new number if needed
        if (need_param) {
            strcpy(display, "0");
            need_param = 0;
        }

        // update display
        if (display[0] == '0') {
            display[0] = key;
        } else {
            tmp[0] = key;
            tmp[1] = 0;
            strcat(display, tmp);
        }
        display2value(base);
        update_bits();
        redraw_display();
        return;
    }

    // handle other keys
    switch (key) {
    case '/': do_op(); op = 48; need_param = 1; break;
    case '*': do_op(); op = 49; need_param = 1; break;
    case '-': do_op(); op = 50; need_param = 1; break;
    case '+': do_op(); op = 51; need_param = 1; break;
    case '=': do_op(); break;

    case KEY_DEL:
        strcpy(display, "0");
        handle_key('0');
        break;

    case KEY_BACK:
        i = strlen(display);
        if (i == 1)
            display[0] = '0';
        else
            display[i-1] = 0;
        display2value(base);
        update_bits();
        redraw_display();
        break;
    }
}

// handle a click reported by the event loop
void handle_click(char control) {
    switch (control) {
    // radio button group 1, change base
    case 6: case 7: case 8: case 9:
        display2value(old_base);
        value2display();
        redraw_display();
        old_base = base;
        break;

    // radio button group 2, change data type
    case 11: case 12: case 13:
        display2value(base);
        if (bindigits == 16) {
            if (check1)
                value = (signed short)value;
            else
                value = (unsigned short)value;
        } else if (bindigits == 8) {
            if (check1)
                value = (signed char)value;
            else
                value = (unsigned char)value;
        }
        value2display();
        update_bits();
        Win_Redraw(winID, 1, 0); // redraw the bits background, to erase excess bits if present
        redraw_display();
        break;

    // sign checkbox
    case 24:
        display2value(base);
        if (check1 == 1) { // when converting to signed, propagate the sign bit
            if (bindigits == 16 && (value & 0x8000))
                value |= 0xFFFF0000L;
            else if (bindigits == 8 && (value & 0x80))
                value |= 0xFFFFFF00L;
        } else { // when converting to unsigned, truncate to avoid excess high bits
            if (bindigits == 16)
                value &= 0xFFFF;
            else if (bindigits == 8)
                value &= 0xFF;
        }
        value2display();
        redraw_display();
        break;

    // number buttons
    case 25: handle_key('A'); break;
    case 26: handle_key('B'); break;
    case 27: handle_key('C'); break;
    case 28: handle_key('D'); break;
    case 29: handle_key('E'); break;
    case 30: handle_key('F'); break;
    case 33: handle_key('7'); break;
    case 34: handle_key('4'); break;
    case 35: handle_key('1'); break;
    case 36: handle_key('0'); break;
    case 39: handle_key('8'); break;
    case 40: handle_key('5'); break;
    case 41: handle_key('2'); break;
    case 44: handle_key('9'); break;
    case 45: handle_key('6'); break;
    case 46: handle_key('3'); break;
    case 52: handle_key('='); break;

    // binary operators
    case 15: case 16: case 17: case 19: case 20: case 21:
    case 22: case 23: case 48: case 49: case 50: case 51:
        do_op();
        op = control;
        need_param = 1;
        break;

    // unary operators
    case 14: do_unary(14); break; // Abs
    case 18: do_unary(18); break; // Not
    case 47: do_unary(47); break; // Sqrt

    // other buttons
    case 32: // <-
        handle_key(KEY_BACK);
        break;
    case 38: // CE
        last_value = 0;
        op = 0;
        strcpy(display, "0");
        handle_key('0');
        break;
    case 43: // clear
        handle_key(KEY_DEL);
        break;
    case 42: // MS
        mem_value = value;
        break;
    case 37: // MR
        value = mem_value;
        value2display();
        update_bits();
        redraw_display();
        break;
    case 31: // MC
        mem_value = 0;
        break;
    }
}

// main entry point / loop
int main(int argc, char *argv[]) {
    // initial setup
    strcpy(display, "0");
    update_bits();

    // open the window
    winID = Win_Open(_symbank, &form);

    // main event loop
    while (1) {
		_symmsg[0] = 0;
		Msg_Sleep(_sympid, -1, _symmsg); // wait for a message

        if (_symmsg[0] == MSR_DSK_WCLICK) { // main "control was used" message
            switch (_symmsg[2]) {
            case DSK_ACT_CONTENT: // normal control interaction
                switch (_symmsg[3]) {
                case DSK_SUB_MLCLICK: // left mouse click
                    handle_click(_symmsg[8]);
                    break;
                case DSK_SUB_MLCLICK: // keypress on a control
                    handle_key(_symmsg[4]);
                    break;
                }
                break;

            case DSK_ACT_KEY:     // keypress without modifying a control
                handle_key(_symmsg[4]);
                break;

            case DSK_ACT_CLOSE:   // Alt+F4 or clicked Close button
                return 0;
                break;
            }
        }
	}
}
