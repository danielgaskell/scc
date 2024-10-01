#include <symbos.h>
#include <graphics.h>

_transfer char icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};

char imgbuf1[512];
char imgbuf2[512];

_data char canvas[128*64/2 + 24];

_transfer Ctrl c_area = {0, C_AREA, -1, COLOR_ORANGE, 0, 0, 10000, 10000};
_transfer Ctrl c_canvas = {1, C_IMAGE_EXT, -1, (unsigned short)canvas, 10, 10, 128, 64};

_transfer Ctrl_Group ctrls = {2, 0, &c_area};

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
    Gfx_Init(canvas, 128, 64);
    Gfx_Select(canvas);

    // draw some pixel primitives
    Gfx_Pixel(10, 10, COLOR_RED);
    Gfx_Line(5, 5, 26, 14, COLOR_BLACK);
    Gfx_Circle(20, 20, 30, COLOR_BLUE);

    // plot an image
    Gfx_Load("calc16.sgx", imgbuf1); // note: automatically downgrades color depth on 4-color displays
    Gfx_Put(imgbuf1, 16, 16, PUT_SET);

    // open window
	winID = Win_Open(_symbank, &form1);

    // copy part of canvas and plot it elsewhere
    Gfx_Get(imgbuf2, 16, 16, 8, 8);
    Gfx_Put(imgbuf2, 16, 48, PUT_SET);

    // refresh just the area of the canvas that changed
    Win_Redraw_Area(winID, 1, 0, 16, 48, 8, 8);

    // main event loop
	while (1) {
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
