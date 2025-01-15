# SymbOS programming

## Contents

* [Console applications](#console-applications)
* [Windowed applications](#windowed-applications)
	* [Memory segments](#memory-segments)
	* [Windows](#windows)
	* [Controls](#controls)
	* [Desktop commands](#desktop-commands)
	* [Handling events](#handling-events)
* [Control reference](#control-reference)
* [Event reference](#event-reference)
* [Advanced topics](#advanced-topics)
	* [Menus](#menus)
	* [Toolbars](#toolbars)
	* [Resizing calculations](#resizing-calculations)
	* [Modal windows](#modal-windows)
	
See also: [System call reference](syscall1.md).

## Console applications

For the most part, console applications meant to run in SymShell can be written in normal C style using the functions in `stdio.h` (`printf()`, `fgets()`, etc.). No additional headers are necessary:

```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Hello world!\n");
}
```

The only subtlety is that SymbOS expects console applications to have the file extension `.com` rather than `.exe`. We can rename the executable file after compilation, or tell `cc` to use a custom output name:

```bash
cc -o condemo.com condemo.c
```

While SCC's `stdio` functions are meant to work relatively seamlessly in SymShell, they are somewhat slow and bulky and are not entirely optimized for SymShell's display model. When writing code from scratch for SymbOS, consider designing it around the [system shell functions](syscall2.md#shell-functions) (`Shell_StringOut()`, `Shell_CharIn()`, etc.) rather than the standard `stdio` functions.

## Windowed applications

Writing windowed applications in SCC is not inherently difficult, but requires a thorough knowledge of SymbOS's desktop model. SCC provides the necessary headers and data types to interact with the SymbOS desktop manager, but there are no shortcuts here; writing a windowed application requires careful coding, testing, and regularly cross-referencing the documentation to ensure that every byte is where it needs to be. The most important data structures and functions are summarized here, but you may also find it useful to consult the [SymbOS developer documentation](https://symbos.org/download.htm).

This section has a lot to take in, but don't worry! To keep you from getting overwhelmed, a good place to start is to try compiling `windemo.c` (in the `sample` folder). Then, read through this section with the goal of learning what each line in `windemo.c` actually does. Once you understand it, you can use the code in `windemo.c` as a skeleton for building your own graphical application, referring to this documentation as needed to see how SymbOS handles the controls and events you want to add. Once you understand the basic concepts of segments, data structures, and messaging, writing windowed applications isn't actually that hard.

(If you want a more Visual Basic-like experience with a graphical form editor, try the useful---if quirky---[Quigs](https://symbos.org/quigs.htm) language.)

The first thing `windemo.c` does is include the header file `symbos.h`, which defines many of the features referenced in the following sections:

```c
#include <symbos.h>
```

### Memory segments

A SymbOS executable is divided into three memory segments. For console apps written with normal libc functionality, it is not generally necessary to think about this, but many SymbOS system calls make important distinctions between memory segments:

1. **Code**: Contains code and globals (unless manually placed into a different segment---see below). Can contain up to 64KB of code and data.
2. **Data**: Stores data that must not cross a 16KB page boundary. This is required by many system functions, particularly those dealing with text or image data, so SCC places string literals in this segment by default. Can contain up to 16KB of data.
3. **Transfer**: Stores data that must be relocated to the upper 16KB of address space so it can be accessed by (mainly) the desktop manager. Most data structures relating to desktop windows and controls should be placed here. Can contain up to 16KB of data.

Globals are placed in the **code** segment by default, but we can manually assign a global to the **data** or **transfer** segments by adding `_data` or `_transfer` to its declaration:

```c
_transfer char imgdata[256];
```

Many system calls also make important distinctions between memory *banks*. Because the Z80 can only address 64KB of memory at a time, extended memory (up to 1MB) is divided into multiple "banks" of 64KB each. We do not generally need to worry about this because all three segments of the executable will be loaded into the same bank. However, there are two exceptions:

1. Some system calls will need to know which bank an address is in. In most cases, this is just the application's main bank (stored in the `_symbank` global).
2. If we need to handle more than 64KB of data in our application, it is possible to [reserve and indirectly address](#memory-management) memory in other banks.

### Windows

SymbOS defines windows and controls using carefully structured data, which must typically be located in the **transfer** segment. To make it easier to define and reference these data structures, `symbos.h` defines a set of struct types that give convenient member names to the different components. Defining a window consists of carefully defining and arranging the necessary structs in the **transfer** segment.

The format of a window's primary data structure is as follows (see also `symbos.h`):

```c
typedef struct {
    unsigned char state;    // (see below)
    unsigned short flags;   // (see below)
    unsigned char pid;      // process ID of owner (set by Win_Open)
    unsigned short x;       // window X position
    unsigned short y;       // window Y position
    unsigned short w;       // window width
    unsigned short h;       // window height
    unsigned short xscroll; // X offset of window content
    unsigned short yscroll; // Y offset of window content
    unsigned short wfull;   // full width of window content
    unsigned short hfull;   // full height of window content
    unsigned short wmin;    // minimum width of window
    unsigned short hmin;    // minimum height of window
    unsigned short wmax;    // maximum width of window
    unsigned short hmax;    // maximum height of window
    char* icon;             // address of the 8x8 window icon, in SGX format
    char* title;            // address of the title text
    char* status;           // address of the status text
    void* menu;             // address of the Menu struct (see Menus)
    void* controls;         // address of the control group struct
    void* toolbar;          // address of the control group struct for the toolbar
    unsigned short toolheight; // toolbar height, in pixels
    char reserved1[9];
    unsigned char modal;    // modal window ID + 1
    char reserved2[140];
} Window;
```

Some commentary on these elements is useful.

`state` is one of the following: `WIN_CLOSED`, `WIN_NORMAL`, `WIN_MAXIMIZED`, or `WIN_MINIMIZED`.

`flags` is an OR'd bitmask which may contain one or more of the following flags:

* `WIN_ICON` = show window icon
* `WIN_RESIZABLE` = window can be resized
* `WIN_CLOSE` = show close button
* `WIN_TOOLBAR` = show toolbar
* `WIN_TITLE` = show titlebar
* `WIN_MENU` = show menubar
* `WIN_STATUS` = show statusbar
* `WIN_ADJUSTX` = disable horizontal slider in resizable windows (i.e., the program will adjust its own X scroll, not the system)
* `WIN_ADJUSTY` = disable vertical slider in resizable windows (i.e., the program will adjust its own Y scroll, not the system)
* `WIN_NOTTASKBAR` = do not display in taskbar
* `WIN_NOTMOVEABLE` = window cannot be moved
* `WIN_MODAL` = modal window ([see below](#modal-windows))

For windows that have the `WIN_RESIZABLE` flag, there is an important distinction between the size of the *window* (given by `.w` and `.h`) and the size of its *content* (given by `.wfull` and `.hfull`). The window will have scroll bars at the edges, and if the size of the window is smaller than the size of the content, the user will be able to use these scroll bars to move around and view the whole content of the window. This can be used to accomplish a variety of interesting techniques by adjusting the content of the window in response to the user scrolling or resizing the window; see [resizing calculations](#resizing-calculations) for some discussion of this.

The window icon is an 8x8 4-color SGX image. The `.icon` member can be set to 0 if there is no icon, but this will also prevent the window from being displayed correctly in the taskbar. Images can be converted to SGX format using software such as [MSX Viewer 5](https://marmsx.msxall.com/msxvw/msxvw5/index_en.php) (classic version), and their raw data examined using a hex editor. A simple default icon is:

```c
_transfer char icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};
```

The content of the window data structure can be defined using a static initializer. Note that SCC does not currently support named initializers such as `.x = 10`, so we will instead need to list all necessary elements in order, being careful not to omit any intervening elements. Here's an example window definition:

```c
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
```

(The line `&ctrls` will be explained in the next section.)

### Controls

Window controls (buttons, text, etc.) are defined using `Ctrl` structs, which must themselves be linked to an overarching `Ctrl_Group` struct. The definition of a `Ctrl_Group` struct is:

```c
typedef struct {
    unsigned char controls;   // total number of controls in the group
    unsigned char pid;        // process ID of owner (set by Win_Open for main group)
    void* first;              // address of first control's data
    void* calcrule;           // address of first resizing calculation (see below), or 0 for none
    unsigned short unused1;
    unsigned char retctrl;    // control ID to "click" on pressing Return
    unsigned char escctrl;    // control ID to "click" on pressing Escape
    char reserved1[4];
    unsigned char focusctrl;  // control ID to select on window focus
    unsigned char reserved2;
} Ctrl_Group;
```

Each control is an instance of the struct `Ctrl`, which has the definition:

```c
typedef struct {
    unsigned short value; // control ID or return value
    unsigned char type;   // control type
    char bank;            // bank of extended data record (-1 for default)
    unsigned short param; // parameter, or address of extended data record
    unsigned short x;     // X location relative to window content
    unsigned short y;     // Y location relative to window content
    unsigned short w;     // width in pixels
    unsigned short h;     // height in pixels
    unsigned short unused;
} Ctrl;
```

The controls in a control group must be defined in order immediately after each other, with the encompassing `Ctrl_Group.first` pointing to the address of the first control. (This will also be the order in which they are drawn onscreen.)

The member `.param` requires some explanation. For the simplest type of control, it is simply a 16-bit number containing various values and flags required by the control. For instance, an important control type is `C_AREA`, which fills the control's area with a solid color; in this case `.param` is the color code (e.g., `COLOR_BLACK`, `COLOR_RED`, `COLOR_ORANGE` or `COLOR_YELLOW` in the default Amstrad CPC 4-color mode). This control is very important because SymbOS will not fill the window background automatically! Starting the main control group with a `C_AREA` control ensures that the window background is filled in.

For more complicated control types, `.param` holds the address of the control's extended data record. (Note that we will need to cast this address to `(unsigned short)` or a compatible type for SCC to accept it, since SCC does not normally allow assigning a pointer directly to a scalar variable.) Each control type has a different extended data record format. For example, to declare a text label, we would use `Ctrl_Text`:

```c
typedef struct {
    char* text;          // address of text string
    unsigned char color; // (foreground << 2) | background
    unsigned char flags; // ALIGN_LEFT, ALIGN_CENTER, or ALIGN_RIGHT
} Ctrl_Text;
```

Combining this into a set of controls for our example window:

```c
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
```

### Desktop commands

To display a window, we need to tell the desktop manager to open it. `symbos.h` provides a number of helper functions for making system calls (see the [System Call Reference](syscall1.md#window-management)); the first one we need is `Win_Open()`:

```c
char Win_Open(char bank, void* addr);
```

This opens the window defined at address `addr` in memory bank `bank`, and returns a window ID that will be needed to identify the window in subsequent commands. Usually, this is the main bank the application is running in, which can be found in the global variable `_symbank`. So, to open the window we defined earlier:

```c
char winID;
winID = Win_Open(_symbank, &form1);
```

Another important call is `Win_Close()`, which closes an open window (windows will also be closed automatically on app exit):

```c
void Win_Close(char winID);
```

The last really important call is `Win_Redraw()`, which redraws a portion of the window content. SymbOS will not automatically redraw the window after we change something (for instance, changing the text of a button), so we need to do it manually:

```c
void Win_Redraw(unsigned char winID, signed char what, unsigned char first);
```

Here, `what` can be either (1) -1, meaning "redraw entire window content"; (2) the control ID of a single control to redraw; or (3) a negative number (from -2 to -16) indicating how many controls should be redrawn (from 2 to 16), in which case `first` indicates the control ID of the first control to redraw. (There are separate commands for redrawing the menu, toolbar, titlebar, and statusbar; see the [System Call Reference](syscalls.md#window-management).)

### Handling events

SymbOS apps communicate with the desktop manager via interprocess messages. `symbos.h` provides helper functions that handle most of the necessary messages automatically (as with `Win_Open()` above). However, we still need to understand how messaging works internally in order to set up the main loop that will receive event notifications from the desktop manager.

Messages are passed via a small buffer which must be placed in the **transfer** segment. The standard message buffer in SCC is `_symmsg`, which is suitable for most tasks:

```c
char _symmsg[14];
```

The core functions for sending and receiving messages are:

```c
unsigned char Msg_Send(char rec_pid, char send_pid, char* msg);
unsigned short Msg_Receive(char rec_pid, char send_pid, char* msg);
unsigned short Msg_Sleep(char rec_pid, char send_pid, char* msg);
```

* `Msg_Send()` sends the message in the buffer `*msg` to the process ID `send_pid`. 
* `Msg_Receive()` checks whether the process ID `send_pid` has sent us a message, and if so, places it in the buffer `*msg`.
* `Msg_Sleep()` is similar to `Msg_Receive()`, but if there is no message, it will sleep, releasing CPU time back to SymbOS until such a message arrives.

In each case `rec_pid` is the "inbound" process ID that should receive the response (that is, our own process ID, which can be found in the global variable `_sympid`). The process ID of the desktop manager is always 2, but to receive messages from any process, we can use -1.

After opening our starting window, we need to go into a loop that:

1. Calls `Msg_Sleep()` to wait for a message;
2. Checks the message type in the first byte, `msg[0]`; and
3. Takes any necessary action.

Message type codes are [documented below](#event-reference). The most immediately useful message is `MSR_DSK_WCLICK`, which is sent for most interactions with a window or form. The return message then has the format:

* `msg[0]`: `MSR_DSK_WCLICK`
* `msg[1]`: Window ID
* `msg[2]`: Action type, one of:
	* `DSK_ACT_CLOSE`: Close button has been clicked, or the user has typed Alt+F4.
	* `DSK_ACT_MENU`: A menu option has been clicked, with:
		* `msg[8]` = Value of the clicked menu entry
	* `DSK_ACT_CONTENT` = A control has been clicked or modified, with:
		* `msg[3]`: Sub-action, one of:
			* `DSK_SUB_MLCLICK`: Left mouse button clicked
			* `DSK_SUB_MRCLICK`: Right mouse button clicked
			* `DSK_SUB_MDCLICK`: Left mouse button double clicked
			* `DSK_SUB_MMCLICK`: Middle mouse button clicked
			* `DSK_SUB_KEY`: Key pressed, with key ASCII value in `msg[4]`
		* `*(int*)&msg[4]` = Mouse X position relative to window content
		* `*(int*)&msg[6]` = Mouse y position relative to window content
		* `msg[8]` = control ID
	* `DSK_ACT_TOOLBAR`: Equivalent to `DSK_ACT_CONTENT`, but for controls in the toolbar.
	* `DSK_ACT_KEY`: A key has been pressed without modifying any control:
		* `msg[4]` = key ASCII value

Putting this all together, an example of a skeleton main loop is below. This opens our previously-defined window `form1` and loops, waiting for an event. In this example, the only event implemented is the most important one: exiting the application when the user tries to close the window!

```c
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
```

That's it, our first windowed application! (The complete code for this demo is available as `windemo.c` in the `samples` folder.) Everything else we might want to do---more controls, more windows, more interactivity---can be accomplished by adding more data records and event-handling code to this skeleton.

A reference guide to the available controls and events is below, as well as information on how to accomplish some more complicated effects. See also the [System Call Reference](syscalls.md#window-management) for additional commands that can be sent to the desktop manager.

## Control reference

### C_AREA

Displays a rectangular area filled with the specified color.

*Parameter*: Color, e.g., `COLOR_ORANGE`. The parameter may optionally be OR'd with `AREA_16COLOR` (to enable 16-color mode) and `AREA_XOR` (to enable XOR mode, to be used only with 16-color mode): e.g., `COLOR_WHITE | AREA_16COLOR`.

```c
// example
_transfer Ctrl c_area1 = {1, C_AREA, -1, COLOR_ORANGE, 0, 0, 100, 80};
```

### C_TEXT

Displays a line of text in the default system font.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;           // address of text
    unsigned char color;  // 4-color mode:  (foreground << 2) | background
	                      // 16-color mode: (foreground << 4) | background
    unsigned char flags;  // one of: ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT
	                      //   OR with TEXT_16COLOR for 16-color mode
} Ctrl_Text;
```

In "fill" mode, the background of the control is first filled in with the specified background color and the text is clipped to the size of the control rather than being allowed to overflow. To activate this mode in 4-color mode, OR `.color` with `TEXT_FILL`: e.g., `(COLOR_BLACK << 2) | COLOR_ORANGE | TEXT_FILL`. To activate this mode in 16-color mode, OR `.flags` with `TEXT_FILL16`: e.g., `ALIGN_LEFT | TEXT_16COLOR | TEXT_FILL16`.

```c
// example
_transfer Ctrl_Text cd_text1 = {"Text", (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl c_text1 = {1, C_TEXT, -1, (unsigned short)&cd_text1, 10, 10, 80, 8};
```

### C_TEXT_FONT

Displays a line of text in an alternative font.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;           // address of text
    unsigned char color;  // 4-color mode:  (foreground << 2) | background
	                      // 16-color mode: (foreground << 4) | background
    unsigned char flags;  // one of: ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT
	                      //   OR with TEXT_16COLOR for 16-color mode
    char* font;           // address of font
} Ctrl_Text_Font;
```

The control height should be equal to the height of the font, and the font data must be stored in the same 16KB segment as the text (usually the **data** segment). A description of the font format can be found in the SymbOS Developer Documentation; fonts can be created using the SymbOS Font Editor application.

```c
// example
_data char fontbuf[1538]; // a font would be loaded into this buffer
_transfer Ctrl_Text_Font cd_text_font1 = {"Text", (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT, fontbuf};
_transfer Ctrl c_text_font1 = {1, C_TEXT_FONT, -1, (unsigned short)&cd_text_font1, 10, 10, 80, 8};
```

### C_TEXT_CTRL

Displays a line of "rich" text with optional control codes that can change the appearance of the text mid-line.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;              // address of text
    unsigned short maxlen;   // maximum length (in bytes) of the text
    char* font;              // address of starting font (-1 for default)
    unsigned char color;     // (foreground << 4) | background
    unsigned char underline; // 1 = start with underline on
} Ctrl_Text_Ctrl;
```

The following control bytes can be included in the text string:

* 0x00 = end of string
* 0x01 0xNN = change text color, where NN is (foreground << 4) | background
* 0x02 0xNNNN = change font, where NNNN is the address of the font, or -1 for the default font. The font must be in the same 16KB segment as the text (usually the **data** segment).
* 0x03 = switch underlining on
* 0x04 = switch underlining off
* 0x05 0xNN = insert NN pixels of extra space before the next character
* 0x08 to 0x0B = skip next (code - 8) * 2 + 1 bytes
* 0x0C to 0x1F = insert (code - 8) pixels of extra space before the next character

```c
// example
_transfer Ctrl_Text_Ctrl cd_text_ctrl1 = {"Text", 100, -1, (COLOR_BLACK << 2) | COLOR_ORANGE, 0};
_transfer Ctrl c_text_ctrl1 = {1, C_TEXT_CTRL, -1, (unsigned short)&cd_text_ctrl1, 10, 10, 80, 8};
```

### C_FRAME

Displays a rectangular frame.

*Parameter*: Color and flags:

* 4-color mode: `(area_color << 4) | (lower_right_color << 2) | upper_left_color`; OR with `AREA_FILL` to fill interior.
* 16-color mode: `(lower_right_color << 12) | (upper_right_color << 8) | area_color | AREA_16COLOR`; OR with `AREA_FILL` to fill interior.

An optional XOR mode inverts the colors underneath the control, like a rubber-band selection. In XOR mode, the parameter is `FRAME_XOR | AREA_16COLOR`.

```c
// example
_transfer Ctrl c_frame1 = {1, C_FRAME, -1, (COLOR_ORANGE << 4) | (COLOR_RED << 2) | COLOR_BLACK | AREA_FILL, 10, 10, 64, 64};
```

### C_TFRAME

Displays a rectangular frame with a line of text at the top.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;           // address of text
    unsigned char color;  // 4-color mode:  (foreground << 2) | background
	                      // 16-color mode: line color
						  //   OR with AREA_16COLOR for 16-color mode
    unsigned char color2; // 16-color mode only: (foreground << 2) | background
} Ctrl_TFrame;
```

```c
// example
_transfer Ctrl_TFrame cd_tframe1 = {"Title", COLOR_BLACK | AREA_16COLOR, (COLOR_BLACK << 2) | COLOR_LBLUE};
_transfer Ctrl c_tframe1 = {1, C_TFRAME, -1, (unsigned short)&cd_tframe1, 10, 10, 64, 64};
```

### C_PROGRESS

Displays a progress bar.

*Parameter*: Color and progress: `(progress << 8) | (empty_color << 6) | (filled_color << 4) | (lower_right_color << 2) | upper_left_color`. Progress is measured from 0 (empty) to 255 (full).

```c
// example
_transfer Ctrl c_progress1 = {1, C_PROGRESS, -1, (119 << 8) | (COLOR_ORANGE << 6) | (COLOR_RED << 4) | (COLOR_BLACK << 2) | COLOR_BLACK, 10, 10, 64, 8};
```

### C_IMAGE

Displays a standard image (4-color SGX format only).

*Control type*: `C_IMAGE`.

*Parameter*: Address of the image data. 

```c
// example
char imgbuf[198];
_transfer Ctrl c_image1 = {1, C_IMAGE, -1, (unsigned short)imgbuf, 10, 10, 24, 24};
```

### C_IMAGE_EXT

Displays an image with an extended graphics header.

*Parameter*: Address of the extended graphics header. Extended graphics are complicated, but allow plotting 16-color images and breaking up an image that is larger than 256 pixels wide/tall into multiple blocks that can be displayed side by side. The details of the graphics format are described in the SymbOS Developer Documentation; SCC provides a struct type, `Img_Header`, to implement the header itself:

```
typedef struct {
    unsigned char bytew;  // width of the complete graphic in bytes (must be even)
    unsigned char w;      // width of this block in pixels
    unsigned char h;      // height of this block in pixels
    char* addrData;       // address of graphic data + offset
    char* addrEncoding;   // address of encoding byte just before the start of the whole graphic
    unsigned short len;   // size of the complete graphic in bytes
} Img_Header;
```

```c
// example
Img_Header imghead; // fill with the appropriate data
_transfer Ctrl c_image_ext1 = {1, C_IMAGE_EXT, -1, (unsigned short)&imghead, 10, 10, 24, 24};
```

### C_IMAGE_TRANS

Same as `C_IMAGE_EXT`, except that color 0 will be transparent.

*Parameter*: Address of the extended graphics header, as above.

### C_ICON

Displays a 24x24 icon with up to two lines of text below it.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* icon;             // address of standard graphic or extended graphic header
    char* line1;            // address of first line of text
    char* line2;            // address of second line of text
    unsigned char flags;    // 4-color mode:  (foreground_color << 2) | background_color
    unsigned char color16;  // 16-color mode: (foreground_color << 4) | background_color
    unsigned char extflags; // extended mode flags
} Ctrl_Icon;
```

The following flags can be OR'd with `.flags`:

* `ICON_STD`: `.icon`. points to standard graphics data (4-color SGX)
* `ICON_EXT`: `.icon` points to extended graphics header (see `C_IMAGE_EXT`)
* `ICON_4COLOR`: icon is 4-color
* `ICON_16COLOR`: icon is 16-color (requires `ICON_EXT`)
* `ICON_MOVEABLE`: icon can be moved by the user
* `ICON_EXTOPTS`: icon has extended options

When `.flags` includes `ICON_EXTOPTS`, the following flags can be OR'd with `.extflags`:

* `ICON_MARKABLE`: icon can be marked (selected) by user
* `ICON_MARKED`: icon is currently marked (selected) by user

The width of the control must be 48 and the height 40.

```c
// example
_data char imgdata[198]; // store an icon image here
_transfer Ctrl_Icon cd_icon1 = {imgdata, "Line 1", "Line 2", (COLOR_BLACK << 2) | COLOR_YELLOW | ICON_STD | ICON_4COLOR};
_transfer Ctrl c_icon1 = {1, C_ICON, -1, (unsigned short)&cd_icon1, 10, 10, 48, 40};
```

### C_BUTTON

Displays a button. Control height must always be 12.

*Parameter*: Address of the button text.

```c
// example
_transfer Ctrl c_button1 = {1, C_BUTTON, -1, (unsigned short)"Text", 10, 10, 48, 12};
```

### C_CHECK

Displays a checkbox.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* status;         // address of status byte
    char* text;           // address of text
    unsigned char color;  // (foreground << 2) | background
} Ctrl_Check;
```

The value of the checkbox (0 = unchecked, 1 = checked) will be stored in the byte pointed to by `status`. The control height should always be 8.

```c
// example
char check1 = 0;
_transfer Ctrl_Check cd_check1 = {&check1, "Label text", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl c_check1 = {1, C_CHECK, -1, (unsigned short)&cd_check1, 10, 10, 32, 8};
```

### C_RADIO

Displays a radio button (circular checkbox). Selecting one radio button in a group will unselect all the others, allowing the user to select just one option.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* status;         // address of status byte
    char* text;           // address of text
    unsigned char color;  // (foreground << 2) | background
    unsigned char value;  // value to load into status when selected
    char* buffer;         // address of 4-byte coordinate buffer (see below)
} Ctrl_Radio;
```

When a radio button is selected, its `value` property will be loaded into the byte pointed to by `status`. Usually this is an ID of some kind (1, 2, 3...), but it can technically be anything. To determine which radio button is selected, read the value of the byte pointed to by `status` and match it to the known `value` properties of the possible radio buttons.

`buffer` points to a 4-byte static buffer used internally by the desktop manager. It should initially contain the values -1, -1, -1, -1.

All radio buttons in a group should use the same `status` and `buffer`; to create multiple groups that do not interact, give them different status bytes and coordinate buffers.

The control height should always be 8.

```c
// example
char radio = 0;
_transfer char radiocoord[4] = {-1, -1, -1, -1};

_transfer Ctrl_Radio cd_radio1 = {&radio, "First button",  (COLOR_BLACK << 2) | COLOR_ORANGE, 1, &radiocoord};
_transfer Ctrl_Radio cd_radio2 = {&radio, "Second button", (COLOR_BLACK << 2) | COLOR_ORANGE, 2, &radiocoord};
_transfer Ctrl_Radio cd_radio3 = {&radio, "Third button",  (COLOR_BLACK << 2) | COLOR_ORANGE, 3, &radiocoord};

_transfer Ctrl c_radio1 = {1, C_RADIO, -1, (unsigned short)&cd_radio1, 10, 10, 32, 8};
_transfer Ctrl c_radio2 = {2, C_RADIO, -1, (unsigned short)&cd_radio2, 10, 20, 32, 8};
_transfer Ctrl c_radio3 = {3, C_RADIO, -1, (unsigned short)&cd_radio3, 10, 30, 32, 8};
```

### C_HIDDEN

Nothing will be displayed, but any clicks to the area of the control will be sent as events for this control ID.

```c
// example
_transfer Ctrl c_hidden1 = {1, C_HIDDEN, -1, 0, 10, 30, 32, 8};
```

### C_TABS

Displays a row of tabs.

*Parameter*: Address of extended data record:

```c
typedef struct {
    unsigned char tabs;      // number of tabs
    unsigned char color;     // (lower_right_color << 6) | (upper_left_color << 4) | (foreground << 2) | background
    unsigned char selected;  // number of selected tab
} Ctrl_Tabs;
```

This data structure should be immediately followed by the number of `Ctrl_Tab` structs indicated by the `tabs` property:

```c
typedef struct {
    char* text;              // address of text
    signed char width;       // width in pixels, or -1 to autocalculate
} Ctrl_Tab;
```

The control height must be 11. SymbOS will keep track of the selected tab for us, but it is our responsibility to decide what a click event on a tab actually does (for example, hiding one page of controls and revealing another). A good way to switch between several pages of controls is to create each page as a separate `Ctrl_Group` and have a `C_COLLECTION` control pointing to the `Ctrl_Group` of the visible page. When the tab bar is clicked, change which `Ctrl_Group` the collection's `controls` property points to and redraw it.

```c
// example
_transfer Ctrl_Tabs cd_tabs = {2, (COLOR_BLACK << 6) | (COLOR_RED << 4) | (COLOR_BLACK) | COLOR_ORANGE, 1};
_transfer Ctrl_Tab cd_tab1 = {"Tab 1", -1};
_transfer Ctrl_Tab cd_tab2 = {"Tab 2", -1};

_transfer Ctrl c_tabs = {1, C_TABS, -1, (unsigned short)&cd_tabs, 10, 10, 64, 11};
```

### C_SLIDER

Displays a slider (scrollbar or value selector).

*Parameter*: Address of extended data record:

```c
typedef struct {
    unsigned char type;      // type (see below)
    unsigned char unused;
    unsigned short value;    // current value/position
    unsigned short maxvalue; // maximum value/position (minimum is 0)
    unsigned char increment; // value to increase by when clicking button
    unsigned char decrement; // value to decrease by when clicking button
} Ctrl_Slider;
```

Type is an OR'd bitmask consisting of one or more of the following flags:

* `SLIDER_H`: horizontal slider
* `SLIDER_V`: vertical slider
* `SLIDER_VALUE`: value slider
* `SLIDER_SCROLL`: scrollbar slider

For a horizontal slider, the control height must be 8 and the control width must be at least 24. For a vertical slider, the control width must be 8 and the control height must be at least 24. We are responsible for reading the slider's value and deciding what to do with it.

```c
// example
_transfer Ctrl_Slider cd_slider1 = {SLIDER_H | SLIDER_SCROLL, 0, 15, 30, 1, 1};
_transfer Ctrl c_tabs = {1, C_SLIDER, -1, (unsigned short)&cd_slider1, 10, 10, 100, 8};
```

### C_COLLECTION

Contains a collection of subcontrols. A collection behaves similarly to the main window content: controls will be clipped to the bounds of the collection, and if the full size of the content is larger than the size of the collection, the user will be able to scroll around the collection's content with scrollbars.

*Parameter*: Address of extended data record:

```c
typedef struct {
    void* controls;          // address of Ctrl_Group defining the controls in the collection
    unsigned short wfull;    // full width of content, in pixels
    unsigned short hfull;    // full height of content, in pixels
    unsigned short xscroll;  // horizontal scroll position, in pixels
    unsigned short yscroll;  // vertical scroll position, in pixels
    unsigned char flags;     // flags (see below)
} Ctrl_Collection;
```

If scrollbars are enabled, the control size must be greater than 32x32. `flags` is one of the following:

* `CSCROLL_NONE`: display no scrollbars
* `CSCROLL_H`: display horizontal scrollbar
* `CSCROLL_V`: display vertical scrollbar
* `CSCROLL_BOTH`: display both scrollbars

```c
// example
_transfer Ctrl cc_area = {1, C_AREA, -1, COLOR_ORANGE, 0, 0, 100, 100};
_transfer Ctrl_Group cg_collect = {1, 0, &cc_area};
_transfer Ctrl_Collection cd_collect = {cg_collect, 200, 100, 0, 0, CSCROLL_H};
_transfer Ctrl c_collect = {1, C_COLLECTION, -1, (unsigned short)&cd_collect, 10, 10, 100, 100};

int main(int argc, char* argv[]) {
	cg_collect.pid = _sympid; // ensure collection's Ctrl_Group has the correct process ID for sending back events
	
	/* ... */
}
```

### C_INPUT

Displays a single-line text input field.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;               // address of text buffer (in data segment)
    unsigned short scroll;    // scroll position, in bytes
    unsigned short cursor;    // cursor position, in bytes
    signed short selection;   // number of selected characters relative to cursor
    unsigned short len;       // current text length, in bytes
    unsigned short maxlen;    // maximum text length, in bytes (not including 0-terminator)
    unsigned char flags;      // flags (see below)
    unsigned char textcolor;  // (foreground << 4) | background
    unsigned char linecolor;  // (lower_right_color << 4) | upper_left_color
} Ctrl_Input;
```

`flags` is an OR'd bitmask which may contain one or more the following:

* `INPUT_PASSWORD`: show all chars as `*`
* `INPUT_READONLY`: input is read-only
* `INPUT_ALTCOLS`: use alternate colors (if this flag is not specified, the `textcolor` and `linecolor` options will be ignored)
* `INPUT_MODIFIED`: represents a bit set when the text is modified

`selection` = 0 when no characters are selected, greater than 0 when the cursor marks the start of the selection, and less than 0 when the cursor marks the end of the selection.

The control height should always be 12. Note that, if we wish the input box to be prefilled, we must be sure to set the properties (`cursor`, `len`, etc.) correctly. (For input by the user, SymbOS will update these properties automatically.)

```c
// example
_data char cd_input_buf[25];
_transfer Ctrl_Input cd_input1 = {cd_input1_buf, 0, 0, 0, 0, 24, INPUT_ALTCOLS, (COLOR_RED << 4) | COLOR_YELLOW, (COLOR_RED << 4) | COLOR_BLACK};
_transfer Ctrl c_input1 = {1, C_INPUT, -1, (unsigned short)&cd_input1, 10, 10, 100, 12};
```

### C_TEXTBOX

Displays a multi-line text input box (text editor).

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;               // address of text buffer (in data segment)
    unsigned short unused1;
    unsigned short cursor;    // cursor position, in bytes
    signed short selection;   // number of selected characters relative to cursor
    unsigned short len;       // current text length, in bytes
    unsigned short maxlen;    // maximum text length, in bytes (not including 0-terminator)
    unsigned char flags;      // flags (see below)
    unsigned char textcolor;  // (foreground << 4) | background, when using INPUT_ALTCOLS flag
    unsigned char unused2;
    char* font;               // font address, when using INPUT_ALTFONT flag (in data segment)
    unsigned char unused3;
    unsigned short lines;     // number of lines of text
    signed short wrapwidth;   // wrapping width, in pixels (-1 for no wrapping)
    unsigned short maxlines;  // maximum number of lines
    signed short xvisible;    // (used internally, set to -8 initially to force reformatting)
    signed short yvisible;    // (used internally, set to 0)
    void* self;               // address of this data record
    unsigned short xtotal;    // (used internally, set to 0)
    unsigned short ytotal;    // (used internally, set to 0)
    unsigned short xoffset;   // (used internally, set to 0)
    unsigned short yoffset;   // (used internally, set to 0)
    unsigned char wrapmode;   // WRAP_WINDOW or WRAP_WIDTH
    unsigned char tabwidth;   // tab stop width (1-255, or 0 for no tab stop)
	int column;               // TextBox_Pos() returns values here
    int line;                 // TextBox_Pos() returns values here
    char reserved[4];         // (used internally, set to 0)
} Ctrl_TextBox;
```

The data record must be immediately followed by a buffer consisting of as many 16-bit words (e.g., `unsigned short`) as there are the maximum number of lines in the text data (`maxlines`). This buffer will hold the length (in bytes) of each line in the textbox, with the high bit set if this length includes a Windows-style `\r\n` end-of-line marker at the end of the line. Due to a quirk in SCC's linker, which currently treats initialized and uninitialized arrays differently, this buffer must be given an initial value (such as `{0}`) to ensure that it is placed directly after the `Ctrl_TextBox` data structure in the **transfer** segment. (This may be improved in future releases.) For example:

```c
_transfer unsigned short lines[1000] = {0};
```

`flags` is an OR'd bitmask which may contain one or more the following:

* `INPUT_READONLY`: input is read-only
* `INPUT_ALTCOLS`: use alternate colors (if this flag is not specified, the `textcolor` option will be ignored)
* `INPUT_ALTFONT`: use alternate font (if this flag is not specified, the `font` option will be ignored)
* `INPUT_MODIFIED`: represents a bit set when the text is modified

`wrapmode` must be either `WRAP_WINDOW` (wrap at window border) or `WRAP_WIDTH` (wrap at the width specified in `wrapwidth`). To use no wrapping, set `wrapmode` = `WRAP_WIDTH` and `wrapwidth` = -1.

`self` is a bit tricky: this must be the address of the data record itself, but this cannot be set in a static initializer because, at the time the compiler is parsing the initializer, the data record's symbol has not yet been fully defined! Instead, we must set this at runtime with a statement like:

```c
cd_textbox1.self = &cd_textbox1;
```

`selection` = 0 when no characters are selected, greater than 0 when the cursor marks the start of the selection, and less than 0 when the cursor marks the end of the selection.

To update the contents, cursor, or selection of the textbox, it is recommended to use the special system calls [`TextBox_Redraw()`](syscall1.md#textbox-redraw) and [`TextBox_Select()`](syscall1.md#textbox-select) rather than trying to update all the relevant properties manually.

Note that, because the buffer must be stored in a continuous 16KB segment (usually the **data** segment), this control is effectively limited to no more than 16KB of text. Note also that, if we wish the textbox to be prefilled, we must be sure to set the properties (`cursor`, `len`, etc.) correctly. (For input by the user, SymbOS will update these properties automatically.)

```c
// example
_data char textbuf[4096];
_transfer Ctrl_TextBox cd_textbox1 = {
    textbuf,         // text address
	0, 0, 0, 0,      // unused1, cursor, selection, len
	4095, 0, 0, 0,   // maxlen, flags, textcolor, unused2
	0, 0, 0, -1,     // font, unused3, lines, wrapwidth
	1000, 0, 0,      // maxlines, xvisible, yvisible
	0,               // self
	-8, 0, 0, 0,     // xtotal, ytotal, xoffset, yoffset
	WRAP_WIDTH, 50}; // wrapmode, tabwidth
_transfer unsigned short textbox1_lines[1000] = {0}; // line-length buffer
_transfer Ctrl c_textbox1 = {1, C_TEXTBOX, -1, (unsigned short)&cd_textbox1, 0, 0, 200, 100};

int main(int argc, char* argc[]) {
	cd_textbox1.self = &cd_textbox1; // fill "self" property at runtime
	/* ... */
}
```

### C_LISTBOX

Displays a list box, which may have one or more columns. The structure of lists is somewhat complicated, and is described below.

*Parameter*: Address of a `List` data record:

```c
typedef struct {
    unsigned short lines;    // number of list rows
    unsigned short scroll;   // index of first shown row
    void* rowdata;           // address of the row data (see below)
    unsigned short unused1;
    unsigned char columns;   // number of columns (from 1 to 64)
    unsigned char sorting;   // sorting flags (see below)
    void* coldata;           // address of the column data (see below)
    unsigned short clicked;  // index of last clicked row
    unsigned char flags;     // flags (see below)
    unsigned char unused2;
} List;
```

`flags` for `List` is an OR'd bitmask which may contain one or more of the following:

* `LIST_SCROLL`: show scrollbar
* `LIST_MULTI`: allow multiple selections

`sorting` is the index of the column to sort by (from 1 to 64), optionally OR'd with `SORT_AUTO` to automatically sort on first display and/or `SORT_REVERSE` to sort in descending order (rather than the default, ascending order).

`rowdata` and `coldata` must point to additional data structures in a specific format and sequence. Column definitions consist of a series of `List_Column` structs directly after one another in the **transfer** segment, as many structs are there are columns:

```c
typedef struct {
    unsigned char flags;     // flags (see below)
    unsigned char unused1;
    unsigned short width;    // width in pixels
    char* text;              // address of title text
    unsigned short unused2;
} List_Column;
```

`flags` for `List_Column` consists of `ALIGN_LEFT`, `ALIGN_RIGHT`, or `ALIGN_CENTER`; OR'd with `LTYPE_TEXT` (for text data), `LTYPE_IMAGE` (for image data), `LTYPE_16` (for a 16-bit number), or `LTYPE_32` (for a 32-bit number). This controls how the column is aligned and sorted, as well as what is displayed on each row.

The format of row definitions is a bit trickier. Internally, each row consists of a 16-bit flags word, followed by as many 16-bit value words as there are columns; these store either the value of the row in that column (for `LTYPE_16`) or the address of the data shown in that column. For a 1-column list, we can represent this structure with a series of `List_Row` structs, directly after one another:

```c
typedef struct {
    unsigned short flags;    // flags (see below)
    char* value;             // address of row content (or value for LTYPE_16 - cast to char*)
} List_Row;
```

`flags` for `List_Row` (and its variants) is a 14-bit numeric value associated with the row. If the row is selected, the high bit of `flags` will be set; this can be tested with `row.flags & ROW_MARKED`.

Additional struct types for multi-column rows (up to 4 columns) are defined in `symbos.h`; for example:

```c
typedef struct {
    unsigned short flags;
    char* value1;
    char* value2;
} List_Row2Col;
```

The control width must be at least 11, and the control height must be at least 16. Note that column names will not be displayed in the base `C_LISTBOX` control type; to shwo them, use `C_LISTFULL` instead.

```c
// example
_transfer List_Row2Col cd_list1_r1 = {1, "Row 1", "Value 1"};
_transfer List_Row2Col cd_list1_r2 = {2, "Row 2", "Value 2"};
_transfer List_Row2Col cd_list1_r3 = {3, "Row 3", "Value 3"};

_transfer List_Column cd_list1_col1 = {ALIGN_LEFT | LTYPE_TEXT, 0, 80, "Column 1"};
_transfer List_Column cd_list1_col2 = {ALIGN_LEFT | LTYPE_TEXT, 0, 80, "Column 2"};

_transfer List cd_list1 = {3, 0, &cd_list1_r1, 0, 2, 1 | SORT_AUTO, &cd_list1_col1, 0, LIST_MULTI};

_transfer Ctrl c_list1 = {1, C_LISTBOX, -1, (unsigned short)&cd_list1, 0, 0, 160, 100};
```

### C_LISTFULL

Equivalent to `C_LISTBOX`, but also displays column titles. Clicking a column title will sort by that column. Control width must be at least 11, control height must be at least 26.

*Parameter*: Same as for `C_LISTBOX`.

### C_LISTTITLE

Displays the title of a listbox, in isolation. Control height must always be 10.

*Parameter*: Same as for `C_LISTBOX`.

### C_DROPDOWN

Equivalent to `C_LISTBOX`, but displays a dropdown list selector instead of a full list box. The control width must be at least 10, and the control height must be 11. The `LIST_SCROLL` flag should be used whenever the list is longer than 10 entries, and the `LIST_MULTI` flag should never be used.

Note that, even though dropdown lists generally only have one column and do not display a column title, we must still define a valid column struct as described under `C_LISTBOX`.

*Parameter*: Same as for `C_LISTBOX`.

## Event reference

### MSR_DSK_WCLICK

The primary event sent for most interactions with a window's controls.

* `msg[0]`: `MSR_DSK_WCLICK`
* `msg[1]`: Window ID
* `msg[2]`: Action type, one of:
	* `DSK_ACT_CLOSE`: Close button has been clicked, or the user has typed Alt+F4.
	* `DSK_ACT_MENU`: A menu option has been clicked, with:
		* `msg[8]` = Value of the clicked menu entry
	* `DSK_ACT_CONTENT` = A control has been clicked or modified, with:
		* `msg[3]`: Sub-action, one of:
			* `DSK_SUB_MLCLICK`: Left mouse button clicked
			* `DSK_SUB_MRCLICK`: Right mouse button clicked
			* `DSK_SUB_MDCLICK`: Left mouse button double clicked
			* `DSK_SUB_MMCLICK`: Middle mouse button clicked
			* `DSK_SUB_KEY`: Key pressed, with key ASCII value in `msg[4]`
		* `*(int*)&msg[4]` = Mouse X position relative to window content
		* `*(int*)&msg[6]` = Mouse y position relative to window content
		* `msg[8]` = control ID
	* `DSK_ACT_TOOLBAR`: Equivalent to `DSK_ACT_CONTENT`, but for controls in the toolbar.
	* `DSK_ACT_KEY`: A key has been pressed without modifying any control:
		* `msg[4]` = key ASCII value

### MSR_DSK_WFOCUS

The focus status of a window has changed.

* `msg[0]`: `MSR_DSK_WFOCUS`
* `msg[1]`: Window ID
* `msg[2]`: Event type, one of:
	* 0 = window lost focus
	* 1 = window received focus

### MSR_DSK_CFOCUS

Sent when the focus status of a control has changed.

* `msg[0]`: `MSR_DSK_CFOCUS`
* `msg[1]`: Window ID
* `msg[2]`: Number of newly focused control (not the control ID/value, but an index starting from 1)
* `msg[3]`: Event type, one of:
	* 0 = User navigated with mouse
	* 1 = User navigated with Tab key
	
### MSR_DSK_WRESIZ

A window has been resized by the user (including maximizing or restoring a maximized or minimized window). The new window size can be read from the window's `w` and `h` properties, although we must also check whether the window's `state` property is `WIN_MAXIMIZED`; in this case the `w` and `h` properties will reflect the restored size of the window, not its maximized size.

* `msg[0]`: `MSR_DSK_WRESIZ`
* `msg[1]`: Window ID

The maximized size of a window's main content area will depend on the window layout (i.e., toolbar height and whether it has a menu or statusbar), but can be calculated from the screen size (`Screen_Width()` and `Screen_Height()`).

### MSR_DSK_WSCROLL

The user has scrolled the main window content of a window. The new scroll position can be read from the window's `xscroll` and `yscroll` properties.

* `msg[0]`: `MSR_DSK_WSCROLL`
* `msg[1]`: Window ID

### MSR_DSK_MENCTX

The user has clicked or cancelled an open context menu.

* `msg[0]`: `MSR_DSK_MENCTX`
* `msg[1]`: Event type, one of:
	* 0 = menu cancelled
	* 1 = entry clicked
* `*(int*)&msg[2]`: Value associated with the clicked entry
* `msg[4]`: Menu entry type, one of:
	* 0 = normal entry
	* 1 = checked entry
	
### MSR_DSK_EVTCLK

The user has clicked a system tray icon associated with this application.

* `msg[0]`: `MSR_DSK_EVTCLK`
* `msg[1]`: Value associated with the system tray icon
* `msg[2]`: Mouse button pressed, one of:
	* `SYSTRAY_LEFT` = left click
	* `SYSTRAY_RIGHT` = right click
	* `SYSTRAY_DOUBLE` = double left click
	
### MSR_DSK_WMODAL

The user has clicked a window that is modal and cannot be focused. (This is useful for creating windows that disappear if the user clicks the main window.)

* `msg[0]`: `MSR_DSK_WMODAL`
* `msg[1]`: Modal window ID

## Advanced topics

### Menus

Compared to lists (see `C_LISTBOX`), menus are comparatively simple. We define a menu using a `Menu` struct directly followed by a series of `Menu_Entry` structs:

```c
typedef struct {
    unsigned short entries;  // number of Menu_Entry structs to follow
} Menu;

typedef struct {
    unsigned short flags;    // flags (see below)
    char* text;              // address of entry text
    unsigned short value;    // value to return when clicked, or address of submenu
    unsigned short unused;
} Menu_Entry;
```

`flags` is an OR'd bitmask that may consist of one or more of the following flags:

* `MENU_ACTIVE`: entry is active and can be clicked (we usually want this)
* `MENU_CHECKED`: entry has a checkmark
* `MENU_SUBMENU`: entry opens a submenu
* `MENU_SEPARATOR`: entry is a separator line

If `MENU_SUBMENU` is set, `value` points to the `Menu` struct defining the submenu to open. Note that `MENU_CHECKED` will not be updated automatically---it is our responsibility to receive menu events and take the necessary action, such as toggling the `MENU_CHECKED` flag for a given entry.

Windows can have main menus if their `WIN_MENU` flag is set and their `menu` property points to a `Menu` struct similar to the above. Menus can also be opened independently (see the [Menu_Context()](syscall1.md#menu_context) system call).

```c
// example
_transfer Menu submenu = {2};
_transfer Menu_Entry submenu_row1 = {MENU_ACTIVE, "Entry 1", 1};
_transfer Menu_Entry submenu_row2 = {MENU_ACTIVE, "Entry 2", 2};

_transfer Menu mainmenu = {3};
_transfer Menu_Entry mainmenu_row1 = {MENU_ACTIVE | MENU_SUBMENU, "Submenu", (unsigned short)&submenu};
_transfer Menu_Entry mainmenu_row2 = {MENU_ACTIVE | MENU_CHECKED, "Option", 2};
_transfer Menu_Entry mainmenu_row3 = {0, "Inactive", 3};
```

### Toolbars

Windows can have a separate "toolbar" region if their property `.toolheight` is greater than 0. This specifies the vertical height (in pixels) of the toolbar region, which will be plotted just below the titlebar or menubar (if present). This region is distinct from the window content in two ways: it does not scroll with the window content in a resizable window, and it has a separate control group data record.

The control group data record for a toolbar is structurally identical to the main control group data record for a window (including the need for a `C_AREA` control to fill in the background of the toolbar), but it is registered in the window's `.toolbar` property instead of its `.controls` property. Refreshing the toolbar must be done with `Win_Redraw_Toolbar` instead of `Win_Redraw`, and user events will be passed back with the `DSK_ACT_TOOLBAR` type instead of the `DSK_ACT_CONTENT` type.

### Resizing calculations

The default behavior of resizable windows is to show only a portion of the main window content (optionally with scrollbars to allow the user to scroll to the rest of the content). However, we can also tell the desktop manager to automatically move and resize the controls on a window using "calculation rules." Calculation rules are defined by a series of `Calc_Rule` structs, one struct per control in the window's main control group, with the format:

```c
typedef struct {
    signed short xbase; // x base
    unsigned char xmult;  // x multiplier
    unsigned char xdiv;   // x divisor
    signed short ybase; // y base
    unsigned char ymult;  // y multiplier
    unsigned char ydiv;   // y divisor
    signed short wbase; // width base
    unsigned char wmult;  // width multiplier
    unsigned char wdiv;   // width divisor
    signed short hbase; // height base
    unsigned char hmult;  // height multiplier
    unsigned char hdiv;   // height divisor
} Calc_Rule;
```

The calculation rules for a control group should be placed immediately after one another in the **transfer** segment, one `Calc_Rule` per control, and the `calcrule` property of the window's main `Ctrl_Group` set to the address of the first `Calc_Rule`. When the window is resized, the `x`, `y`, `w`, and `h` properties of each `Ctrl` in the window's main `Ctrl_Group` will be automatically recalculated according to the formula:

* `x` = `xbase` + (window_width * `xmult` / `xdiv`)
* `y` = `ybase` + (window_height * `ymult` / `ydiv`)
* `w` = `wbase` + (window_width * `wmult` / `wdiv`)
* `h` = `hbase` + (window_height * `hmult` / `hdiv`)

An easy way to think about these formulas is in terms of which control margins you want to remain a constant distance from the corresponding side of the window area:

* To keep a control's left margin constant, use `xbase` = left_margin, `xmult` = 0, `xdiv` = 1.
* To keep a control's right margin constant, use `xbase` = -(control_width + right_margin), `xmult` = 1, `xdiv` = 1.
* To keep both left and right margins constant, stretching the control to fit, use `xbase` = left_margin, `xmult` = 0, `xdiv` = 1, `wbase` = -(left_margin + right_margin), `wmult` = 1, `wdiv` = 1.
* To keep a control's top margin constant, use `ybase` = top_margin, `ymult` = 0, `ydiv` = 1.
* To keep a control's bottom margin constant, use `ybase` = -(control_height + bottom_margin), `ymult` = 1, `ydiv` = 1.
* To keep both top and bottom margins constant, stretching the control to fit, use `ybase` = top_margin, `ymult` = 0, `ydiv` = 1, `hbase` = -(top_margin + bottom_margin), `hmult` = 1, `hdiv` = 1.

For example, to resize a single control to fill the entirety of the window content except for a 12-pixel area at the top of the window content (left margin = 0, right margin = 0, top margin = 12, bottom margin = 0):

```c
_transfer Calc_Rule calcrule = {0, 0, 1, 12, 0, 1, 0, 1, 1, -12, 1, 1};
```

When using calculation rules, we should be careful about setting the maximum and minimum window sizes in the `Window` record to sizes where we know the calculations will be valid. For example, if we have a control with a left margin of 25 and a right margin of 100 and we let the user resize the window to only 50 pixels wide, the calculated width of the control will be negative! We can avoid this situation by defining an appropriate minimum width, such as 150.

(An alternative approach is to recognize the "window resized" event `MSR_DSK_WRESIZ`, manually recalculate the `x`, `y`, `w`, and `h` properties of the affected controls, and redraw any controls that have moved or been resized by this calculation. However, using calculation rules will result in a smoother experience because the desktop manager can recalculate controls as the window is being resized, rather than redrawing them twice.)

### Modal windows

Windows can be "modal", that is, unable to be focused or interacted with as long as another window (here called the "topmost" window) is open. To establish a modal relationship, we need to set two values:

* Set the `WIN_MODAL` flag for the topmost window, i.e.: `form2.flags |= WIN_MODAL`.
* Set the `.modal` property of the lower window to the window ID of the topmost window, plus 1, i.e.: `form1.modal = form1id + 1`.

When the topmost window closes, unset both properties. If the user clicks the lower window while this relationship has been established, the deskop manager sends a `MSR_DSK_WMODAL` message to notify the application. (This is particularly useful to create dialogs that should be closed automatically if the user clicks the main window.)
