
# Introduction

Writing C will rot your brain. Only a true C programmer would look at the following code fragment and think it was an "elegant" solution to anything:

```c
addr = (((_MemStruct)membank).ptr->addr & (ver > 4 ? 7 : attr));
*((char*)(++addr)) += 'A';
```

C is a terrible, very bad, no-good language that provides you with an army of ways to shoot yourself in the foot. It is also obviously the best language for many kinds of systems programming, where we want to express ideas in a relatively high-level way while retaining the knowledge of where every byte is going and why. This---in addition to the vast amount of open-source code already written in C---makes it useful for programming 8-bit systems.

SCC is an ANSI C compiler that produces executables for [SymbOS](https://symbos.org). If you want a nice Visual Basic-style experience with a GUI form editor and event-driven programming, you should probably check out the [Quigs IDE](https://symbos.org/quigs.htm) instead. But if you already know C and want to write SymbOS software, or you want to port existing C code to SymbOS, or you just want to do something more low-level and complicated than is possible in Quigs, read on.

(This documentation assumes you are familiar with standard C syntax, particularly structs, pointers, and typecasting, as well as the various weird ways you can combine them.)

# Using SCC

## Compilation

To compile a single source file into a single SymbOS executable, just run it through `cc` on the command line:

```bash
cc file.c
```

Under the hood, SCC consists of a multi-stage toolchain typical for C compilers: a preprocessor (`cpp`), multiple compiler stages (`cc0`, `cc1`, and `cc2`), an assembler (`as`), linker (`ld`), optimizer (`copt`), and relocation tool (`reloc`), which performs the final stage of building a SymbOS executable by patching in the relocation table produced by the linker. For complex projects, these can be run independently to built multiple files and objects into a single executable. For example, to compile two source files (`file1.c` and `file2.c`) into object files (`file1.o`and `file2.o`) and link them with the system libraries into a single executable:

```bash
cc -c file1.c
cc -c file2.c
ld -o file.exe -R file.rel ..\lib\crt0.o file1.o file2.o ..\lib\libc.a ..\lib\libsym.a ..\lib\libz80.a
reloc file.exe file.rel
```
In the C world this type of modular build is usually done with a Makefile. SCC does not currently have its own`make` utility, but we can use the one from MinGW (not documented here). In practice SymbOS projects are usually small enough that we can just maintain a single main source file (potentially with `#include` directives to merge in subsidiary files) and compile it directly with `cc`.

A good way to determine what `cc` is doing under the hood (particularly for linking) is to run it with the `-V` option, which outputs each command as it is run.

## Features

* Full build chain with preprocessor, object files, linker, etc.
* Standard ANSI C syntax, with good support for most typical usage.
* Standard data types and structures: `char`, `short`, `int`, `long`, `float`, `double`, `signed`, `unsigned`, `struct`, `union`, `enum`, `auto`, `static`, `register`, `extern`, `typedef`.
* A proper libc port [work in progress] so existing code can be compiled unmodified.
* Headers, typedefs, and support functions for most SymbOS system calls (`symbos.h`).
* Clean handling of SymbOS segments, with data, buffers, and literals located correctly in the executable without duplication (other compilers struggle with this). Keywords `_data` and `_transfer` allow specifying the segment of globals.
* The underlying code is 8-bit-friendly, so it should (eventually!) be possible to build SCC to run natively on SymbOS.

## Limitations

* **Not all libc functions are available, well-tested, and/or correctly implemented yet**. Platform-independent ones should all work, but any code requiring I/O or calls to the operating system should be tested carefully.
* No high-level optimizations like subexpression merging. (Just write efficiently.)
* The libc implementation is not very fast and favors portability over speed.
* For the usual Z80 reasons, 8-bit arithmetic (`char`) will always be much faster than 16-bit (`int`) and particularly floating-point (`float`, `double`) arithmetic. Declaring variables `unsigned` may also improve efficiency where applicable.
* For speed, functions from `math.h` are mostly `float` by default rather than `double`.
* The usual SymbOS limits apply, such as 64KB and 16KB limits on particular segments (see below).
* No named static struct initializers like `{.x = 1, .y = 2}`---use `{1, 2}` instead and just be careful about member order.
* No K&R-style function declarations.
* Not all C99 and C11 additions are supported.
* `const` and `volatile` are accepted but don't actually do anything.
* Local variables are technically scoped to their function, not their block.
* If you run into type-mismatch problems with unusual pointer data types (like struct members that are typed as pointers to another typedef'd struct---this can happen when working with complex data structures for windows), try `void*` instead.

# SymbOS programming

## Console applications

For the most part, console applications meant to run in SymShell can be written in normal C style using the functions in `stdio.h` (`printf()`, `fgets()`, etc.). No additional headers are necessary. The only subtlety is that SymbOS expects console applications to have the file extension `.com` rather than `.exe`. We can rename the executable file after compilation, or tell `cc` to use a custom output name:

```bash
cc -o file.com file.c
```
While SCC's `stdio` functions are meant to work relatively seamlessly in SymShell, they are not entirely optimized for SymShell's display model. In particular, SymShell is much faster at displaying long strings in one go than a series of individual characters. When writing code from scratch for SymbOS, consider designing it around the system shell functions (`Shell_StringOut()`, `Shell_CharIn()`, etc.) rather than the standard `stdio` functions.

## Windowed applications

Writing windowed applications in SCC is not inherently difficult, but requires a thorough knowledge of SymbOS's desktop model. SCC provides the necessary headers and data types to interact with the SymbOS desktop manager, but there are no shortcuts here; writing a windowed application requires careful coding, testing, and regularly cross-referencing the documentation to ensure that every byte is where it needs to be. The most important data structures and functions are summarized here, but you may also find it useful to consult the [SymbOS developer documentation](https://symbos.org/download.htm).

(For a more plug-and-play experience, try the useful---if quirky---[Quigs](https://symbos.org/quigs.htm) language.)

We first need to include the header file `symbos.h`, which defines many of the features referenced in the following sections:

```c
#include <symbos.h>
```

### Memory segments

A SymbOS executable is divided into three memory segments. For console apps written with normal libc functionality, it is not generally necessary to think about this, but many SymbOS system functions make important distinctions between memory segments:

1. **Code**: Contains code and globals (unless manually placed into a different segment - see below). Can contain up to 64KB of code and data.
2. **Data**: Stores data that must not cross a 16KB page boundary. This is required by many system functions, particularly those dealing with text or image data, so SCC places string literals in this segment by default. Can contain up to 16KB of data.
3. **Transfer**: Stores data that must be relocated to the upper 16KB of address space so it can be accessed by (mainly) the desktop manager. Most data structures relating to desktop windows and controls should be placed here. Can contain up to 16KB of data.

Globals are placed in the **code** segment by default, but we can manually assign a global to the **data** or **transfer** segments by adding `_data` or `_transfer` to its declaration:

```c
_transfer char imgdata[256];
```

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
    void* menu;             // address of the menu struct
    void* controls;         // address of the control group struct
    void* toolbar;          // address of the control group struct for the toolbar
    unsigned short toolheight; // toolbar height, in pixels
    char reserved1[6];
    unsigned char modal;    // modal window ID + 1
    char reserved2[140];
} Window;
```

Some commentary on these elements is useful.

`.state` is one of the following: `WIN_CLOSED`, `WIN_NORMAL`, `WIN_MAXIMIZED`, or `WIN_MINIMIZED`.

`.flags` is an OR'd list of one or more of the following flags:
	* `WIN_ICON` = show window icon
	* `WIN_RESIZABLE` = window can be resized
	* `WIN_CLOSE` = show close button
	* `WIN_TOOLBAR` = show toolbar
	* `WIN_TITLE` = show titlebar
	* `WIN_MENU` = show menubar
	* `WIN_STATUS` = show statusbar
	* `WIN_ADJUSTX` = automatically adjust the content width to the window width
	* `WIN_ADJUSTY` = automatically adjust the content height to the window height
	* `WIN_NOTTASKBAR` = do not display in taskbar
	* `WIN_NOTMOVEABLE` = window cannot be moved
	* `WIN_MODAL` = modal window ([see below](#modal-windows))

For windows that have the `WIN_RESIZABLE` flag, there is an important distinction between the size of the *window* (given by `.w` and `.h`) and the size of its *content* (given by `.wfull` and `.hfull`). The window will have scroll bars at the edges, and if the size of the window is smaller than the size of the content, the user will be able to use these scroll bars to move around and view the whole content of the window. This can be used to accomplish a variety of interesting techniques by adjusting the content of the window in response to the user scrolling or resizing the window; see [resizing calculations](#resizing-calculations) for some discussion of this.

The window icon is an 8x8 4-color SGX image. The `.icon` member can be set to 0 if there is no icon, but this will also prevent the window from being displayed correctly in the taskbar. Images can be converted to SGX format using software such as [MSX Viewer 5](https://marmsx.msxall.com/msxvw/msxvw5/index_en.php); a simple default icon is:

```c
_transfer char icon[19] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};
```

The content of the window data structure can be defined using a static initializer. Note that SCC does not currently support named initializers such as `.x = 10`, so we will instead need to list all necessary elements in order, being careful not to omit any. Here's an example window definition:

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
    void* calcrule;           // address of resizing calculation (see below)
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

For more complicated control types, `.param` holds the address of the control's extended data record. (Note that we will need to cast this address to `(unsigned short)` or a compatible type for SCC to accept it, since default C syntax does not allow assigning a pointer directly to a scalar variable.) Each control type has a different extended data record format. For example, to declare a text label, we would use `Ctrl_Text`:

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

To display a window, we need to tell the desktop manager to open it. `symbos.h` provides a number of helper functions for making system calls; the first one we need is `Win_Open()`:

```c
extern char Win_Open(char bank, void* addr);
```

This opens the window defined at address `addr` in memory bank `bank`, and returns a window ID that will be needed to identify the window in subsequent commands. Usually, this is the main bank the application is running in, which can be found in the global variable `_symbank`. So, to open the window we defined earlier:

```c
char winID;
winID = Win_Open(_symbank, &form1);
```

Another important call is `Win_Close()`, which closes an open window:

```c
extern void Win_Close(char winID);
```

Windows will also be closed automatically on app exit.

(Additional calls are [documented below](#command-reference).)

### Handling events

SymbOS apps communicate with the desktop manager via interprocess messages. `symbos.h` provides helper functions that handle most of the necessary messages automatically (as with `Win_Open()` above). However, we still need to understand how messaging works internally in order to set up the main loop that will receive event notifications from the desktop manager.

Messages are passed via a small buffer which must be placed in the **transfer** segment. The standard message buffer in SCC is `_symmsg`, which is suitable for most tasks:

```c
extern char _symmsg[14];
```

The core functions for sending and receiving messages are:

```c
extern short Msg_Send(char rec_pid, char send_pid, char* msg);
extern short Msg_Receive(char rec_pid, char send_pid, char* msg);
extern short Msg_Sleep(char rec_pid, char send_pid, char* msg);
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
		* `(int)&msg[4]` = Mouse X position relative to window content
		* `(int)&msg[6]` = Mouse y position relative to window content
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

### Control reference

### Event reference

### Command reference

### Menus

### Lists

### Resizing calculations

### Modal windows

## Other considerations

### The `malloc()` heap

### Building SCC

The current primary build target for SCC is Windows. Install MinGW, ensure that its `bin` folder is in the system path, and then run the `make.bat` batch files found throughout the SCC source tree to compile the relevant parts of the codebase. (This really ought to transition to proper Makefiles, but whatever.)
