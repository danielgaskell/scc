#include <symbos.h>
#include <graphics.h>

_transfer char icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};

char ballimg[138];
char countbyte = 0;

_data char canvas[128*96/2 + 24];

_transfer Ctrl c_area = {0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000};
_transfer Ctrl c_canvas = {1, C_IMAGE_EXT, -1, (unsigned short)canvas, 0, 0, 128, 96};

_transfer Ctrl_Group ctrls = {2, 0, &c_area};

_transfer Window form1 = {
    WIN_NORMAL,
    WIN_CLOSE | WIN_TITLE | WIN_ICON,
    0,          // PID
    10, 10,     // x, y
    128, 96,    // w, h
    0, 0,       // xscroll, yscroll
    128, 96,    // wfull, hfull
    128, 96,    // wmin, hmin
    128, 96,    // wmax, hmax
    icon,       // icon
    "Ball Demo",// title text
    0,          // no status text
    0,          // no menu
    &ctrls};    // controls

char winID;

int test2(int x) {
    register y = 0;
    ++y;
}

int test(int x) {
    register y = 0;
    ++y;
}

int main(int argc, char *argv[]) {
    int x, y, i, dx, dy;
    unsigned char redraw_x, redraw_y, redraw_w, redraw_h;

    Gfx_Init(canvas, 128, 96);
    Gfx_Select(canvas);

    // draw checkerboard background
    i = 0;
    for (y = 0; y < 96; y += 16) {
        for (x = 0; x < 128; x += 16) {
            Gfx_BoxF(x, y, x+15, y+15, i);
            ++i;
            if (i > 15)
                i = 0;
        }
        i += 2;
    }

    // set up ball
    if (Gfx_Load("ball.sgx", ballimg)) {
        MsgBox("Could not find BALL.SGX", "", "", COLOR_BLACK, BUTTON_OK, 0, 0);
        exit(1);
    }
    x = 0;
    y = 0;
    dx = 8;
    dy = 0;
    test(x);
    Gfx_Put(ballimg, x, y, PUT_XOR);

    // set up frame counter
    Counter_Add(_symbank, &countbyte, _sympid, 3); // increments every 3/50th second (about 16 FPS)

    // open window
	winID = Win_Open(_symbank, &form1);

    // main loop
	while (1) {
        // handle messages
		_symmsg[0] = 0;
		Msg_Receive(_sympid, -1, _symmsg);
		if (_symmsg[0] == MSR_DSK_WCLICK && _symmsg[2] == DSK_ACT_CLOSE) {
            // Alt+F4 or click close
            Counter_Delete(_symbank, &countbyte);
            exit();
		}

        // redraw ball (at most every 3/50th second)
        if (countbyte) {
            // move and draw ball
            Gfx_Put(ballimg, x, y, PUT_XOR); // effectively erases old position
            if ((x + dx) < 0 || (x + dx) >= (128 - 16))
                dx = -dx; // bounce left/right
            if ((y + dy) >= (96 - 16)) {
                dy = -(dy * 9 / 10); // bounce bottom
                dx = (dx * 9 / 10);
            } else {
                dy += 1; // gravity
            }
            x += dx;
            y += dy;
            Gfx_Put(ballimg, x, y, PUT_XOR); // draw in new position

            // request a redraw of just the relevant part of the canvas
            redraw_w = abs(dx) + 16;
            redraw_h = abs(dy) + 17; // +1 to account for gravity change in dy
            if (dx > 0) { redraw_x = x - dx; } else { redraw_x = x; }
            if (dy > 0) { redraw_y = y - dy; } else { redraw_y = y; }
            Win_Redraw_Area(winID, 1, 0, redraw_x, redraw_y, redraw_w, redraw_h);

            // reset counter
            countbyte = 0;
        }
	}
}
