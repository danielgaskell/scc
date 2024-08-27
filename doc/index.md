

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
In the C world this type of modular build is usually done with a Makefile. SCC does not currently have its own `make` utility, but we can use the one from MinGW (not documented here). In practice SymbOS projects are usually small enough that we can just maintain a single main source file (potentially with `#include` directives to merge in subsidiary files) and compile it directly with `cc`.

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
While SCC's `stdio` functions are meant to work relatively seamlessly in SymShell, they are somewhat slow and bulky and are not entirely optimized for SymShell's display model. When writing code from scratch for SymbOS, consider designing it around the [system shell functions](#system-call-reference) (`Shell_StringOut()`, `Shell_CharIn()`, etc.) rather than the standard `stdio` functions.

## Windowed applications

Writing windowed applications in SCC is not inherently difficult, but requires a thorough knowledge of SymbOS's desktop model. SCC provides the necessary headers and data types to interact with the SymbOS desktop manager, but there are no shortcuts here; writing a windowed application requires careful coding, testing, and regularly cross-referencing the documentation to ensure that every byte is where it needs to be. The most important data structures and functions are summarized here, but you may also find it useful to consult the [SymbOS developer documentation](https://symbos.org/download.htm).

(For a more plug-and-play experience, try the useful---if quirky---[Quigs](https://symbos.org/quigs.htm) language.)

We first need to include the header file `symbos.h`, which defines many of the features referenced in the following sections:

```c
#include <symbos.h>
```

### Memory segments

A SymbOS executable is divided into three memory segments. For console apps written with normal libc functionality, it is not generally necessary to think about this, but many SymbOS system calls make important distinctions between memory segments:

1. **Code**: Contains code and globals (unless manually placed into a different segment - see below). Can contain up to 64KB of code and data.
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

To display a window, we need to tell the desktop manager to open it. `symbos.h` provides a number of helper functions for making system calls (see the [reference list of all functions](#system-call-reference)); the first one we need is `Win_Open()`:

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

### Menus

### Lists

### Resizing calculations

### Modal windows

## System call reference

The following calls are all available after including `symbos.h`:

```c
#include <symbos.h>
```

These headers are not 100% comprehensive; SymbOS provides some additional system calls not implemented in `symbos.h`, mainly low-level calls for dealing with storage devices, system configuration, and complicated applications that alter system functionality or execute code in multiple banks (SCC is not well-suited for this). These calls are discussed in the [SymbOS developer documentation](https://symbos.org/download.htm). It is assumed that, if you need these calls, you are probably already doing something complicated enough that a few extra wrapper functions won't be useful.

## System variables

```c
char* _symmsg;
```
A 14-byte buffer for sending messages. This is used internally by most system calls, but can be used for our own purposes when manually sending messages with `Msg_Send()` and similar functions.

```c
unsigned char _sympid;	     // process ID of the current app
unsigned char _symappid;     // application ID of the current app
unsigned char _symbank;      // main bank number of the current app
char* _segcode;              // start address of the code segment + 0x100
char* _segdata;              // start address of the data segment
char* _segtrans;             // start address of the transfer segment
unsigned short _segcodelen;  // length of the code segment
unsigned short _segdatalen;  // length of the data segment
unsigned short _segtranslen; // length of the transfer segment
```

The other contents of the application header can be accessed directly with the struct `_symheader`, not documented here (see definition in `symbos.h`).

### Messaging

#### Msg_Send()
```c
unsigned char Msg_Send(char rec_pid, char send_pid, char* msg);
```

Sends the message in `*msg` to process ID `send_pid`. `rec_pid` is the process ID that should receive the response, if any; usually this should be our own process ID (`_sympid`). `*msg` must be in the **transfer** segment.

*Return value*: 0 = message queue is full; 1 = message sent successfully; 2 = receiver process does not exist.

*SymbOS name*: `Message_Send` (`MSGSND`).

#### Msg_Receive()

```c
unsigned short Msg_Receive(char rec_pid, char send_pid, char* msg);
```
Checks for a message sent from process ID `send_pid` to process ID `rec_pid` and, if one is waiting, stores it in `*msg`. Usually `rec_pid` should be our own process ID (`_sympid`). If `send_pid` is -1, checks for messages from any process. `*msg` must be in the **transfer** segment.

*Return value*: Low byte: 0 = no message available, 1 = message received. High byte: sender process ID. Extract with, e.g.,

```c
rec = result & 0xFF;
pid = result >> 8;
```

*SymbOS name*: `Message_Receive` (`MSGGET`).

#### Msg_Sleep()

```c
unsigned short Msg_Sleep(char rec_pid, char send_pid, char* msg);
```
Checks for a message sent from process ID `send_pid` to process ID `rec_pid`. If one is waiting, stores it in `*msg`. If there is no message, returns CPU time to SymbOS and waits until a message is available or the process is woken up for another reason. Usually `rec_pid` should be our own process ID (`_sympid`). If `send_pid` is -1, checks for messages from any process. `*msg` must be in the **transfer** segment.


*Return value*: Low byte: 0 = no message available, 1 = message received. High byte: sender process ID. Extract with, e.g.,

```c
rec = result & 0xFF;
pid = result >> 8;
```

Note that processes can be "woken up" for multiple reasons, so returning from `Msg_Sleep()` does not necessarily mean that the desired message has been received. We must check the return value or the contents of `*msg` to be sure. For example, to loop until a message is actually received:

```c
while (!(Msg_Sleep(_sympid, -1, _symmsg) & 0x01));
```

*SymbOS name*: `Message_Sleep_And_Receive` (`MSGSLP`).

#### Idle()

```c
void Idle(void);
```
Return CPU time to SymbOS and idle until something wakes it up---for example, an incoming message.

*SymbOS name*: `Multitasking_SoftInterrupt` (`RST #30`).

### Memory management

Applications are able to address more that 64KB of memory by reserving additional blocks of banked memory. These blocks cannot be addressed directly using C pointers and variables, but we can read/write/copy data to them using system functions.

#### Mem_Reserve()

```c
unsigned char Mem_Reserve(unsigned char bank, unsigned char type, unsigned short len, unsigned char* bankVar, char** addrVar);
```

Reserve a block of banked memory in bank `bank` of length `len`, in bytes. `bank` may be from 0 to 15; 0 means "any bank can be used." `type` may be one of: 0 = located anywhere; 1 = reserve within a 16KB address block (like the **data** segment); 2 = reserve within the last 16KB address block (like the **transfer** segment).

Two variables must be passed by reference to store the address of the resulting block of banked memory: `bankVar` (type `unsigned char`), which stores the bank, and `addrVar` (type `char*`), which stores the address.

**Note that, to avoid memory leaks, memory reserved with `Mem_Reserve()` ***must*** be manually released with `Mem_Release()` before program exit!** SymbOS does not have the resources to track this automatically; it is up to us.

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Memory_Get` (`MEMGET`).

#### Mem_Release()

```c
void Mem_Release(unsigned char bank, char* addr, unsigned short len);
```

Releases a block of banked memory previously reserved with `Mem_Reserve()`. `bank` is the bank of the reserved memory, which must be from 1 to 15; `addr` is the address; and `len` is the length of the reserved block, in bytes.

**Be careful to ensure that `bank`, `addr`, and `len` exactly match a contiguous block of memory that was previously reserved with `Mem_Reserve()`!** SymbOS does not keep track of this independently, so we can corrupt memory if we pass invalid information.

*SymbOS name*: `Memory_Free` (`MEMFRE`).

#### Mem_Resize()

```c
unsigned char Mem_Resize(unsigned char bank, char* addr, unsigned short oldlen, unsigned short newlen);
```

Attempts to resize a block of banked memory previously reserved with `Mem_Reserve()`. `bank` is the bank of the reserved memory, which must be from 1 to 15; `addr` is the address; `oldlen` is the previous length of the reserved block, in bytes; and `newlen` is the requested new length, in bytes.

Shortening a block will always work. Lengthening a block will only work if the required addresses (immediately after the end of the old block) are available, which is unlikely if the user has started any new applications since the block was reserved. A more robust alternative is the SCC-specific helper function `Mem_ResizeX()` (see below).

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Memory_Resize` (`MEMSIZ`).

#### Mem_ResizeX()

```c
unsigned char Mem_ResizeX(unsigned char bank, unsigned char type, char* addr,
                          unsigned short oldlen, unsigned short newlen,
                          unsigned char* bankVar, char** addrVar);
```

A more robust SCC extension to `Mem_Resize()`, above. Attempts to resize a block of banked memory previously reserved with `Mem_Reserve()` by first trying calling `Mem_Resize()`; if this does not succeed, it tries again by reserving a new block of the desired size, copying the old block to the new block, and releasing the old block.  `bank` is the bank of the reserved memory, which must be from 1 to 15. `type` may be one of: 0 = located anywhere; 1 = only move within a 16KB address block (like the **data** segment); 2 = only move within the last 16KB address block (like the **transfer** segment). `addr` is the previous address; `oldlen` is the previous length of the reserved block, in bytes; and `newlen` is the requested new length, in bytes.

Two variables must be passed by reference to store the address of the resulting block: `bankVar` (type `unsigned char`), which stores the bank, and `addrVar` (type `char*`), which stores the address. Note that a moved block may be in any bank, not just the same bank as the previous block.

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: N/A

#### Mem_Longest()

```c
unsigned short Mem_Longest(unsigned char bank, unsigned char type);
```

Returns (in bytes) the longest area of contiguous memory within bank `bank` that could be reserved with `Mem_Reserve()`. `bank` may be from 0 to 15; 0 means "any bank can be used." `type` may be one of: 0 = located anywhere; 1 = reserve within a 16KB address block (like the **data** segment); 2 = reserve within the last 16KB address block (like the **transfer** segment).

*SymbOS name*: `Memory_Information` (`MEMINF`).

#### Mem_Free()

```c
unsigned long Mem_Free(void);
```

Returns the total amount of free memory, in bytes.

*SymbOS name*: `Memory_Summary` (`MEMSUM`).

#### Mem_Banks()

```c
unsigned char Mem_Banks(void);
```

Returns the total number of existing 64KB extended RAM banks.

*SymbOS name*: `Memory_Summary` (`MEMSUM`).

### Memory read/write

#### Bank_ReadWord()

```c
unsigned short Bank_ReadWord(unsigned char bank, char* addr);
```

Returns the two-byte word at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_ReadWord` (`BNKRWD`).

#### Bank_WriteWord()

```c
void Bank_WriteWord(unsigned char bank, char* addr, unsigned short val);
```

Writes the two-byte word `val` to memory at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_WriteWord` (`BNKWWD`).


#### Bank_ReadByte()

```c
unsigned char Bank_ReadByte(unsigned char bank, char* addr);
```

Returns the byte at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_ReadByte` (`BNKRBT`).

#### Bank_WriteByte()

```c
void Bank_WriteByte(unsigned char bank, char* addr, unsigned char val);
```

Writes the byte `val` to memory at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_WriteByte` (`BNKWBT`).

#### Bank_Copy()

```c
void Bank_Copy(unsigned char bankDst, char* addrDst, unsigned char bankSrc, char* addrSrc, unsigned short len);
```

Copies `len` bytes of memory from bank `bankSrc`, address `addrSrc` to bank `bankDst`, address `addrDst`.

*SymbOS name*: `Banking_Copy` (`BNKCOP`).

#### Bank_Get()

```c
unsigned char Bank_Get(void);
```

Returns the bank number in which the app's main process is running. (Normally it is easier to use the `_symbank` global for this purpose.)

*SymbOS name*: `Banking_GetBank` (`BNKGET`).

### Clipboard functions

#### Clip_Put()

```c
unsigned char Clip_Put(unsigned char bank, char* addr, unsigned short len, unsigned char type);
```

Stores `len` bytes of data from bank `bank`, address `addr` into the system clipboard. `type` may be one of: 1 = text, 2 = extended graphic, 3 = item list, 4 = desktop icon shortcut.

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Clipboard_Put` (`BUFPUT`).

#### Clip_Get()

```c
unsigned short Clip_Get(unsigned char bank, char* addr, unsigned short len, unsigned char type);
```

Retrieves up to `len` bytes of data from the system clipboard and stores it in bank `bank`, address `addr`. `type` may be one of: 1 = text, 2 = extended graphic, 3 = item list, 4 = desktop icon shortcut. Data will only be retrieved if (1) the type of the data in the clipboard matches the requested type, and (2) the data length is not greater than `len`.

*Return value*: length of received data, in bytes.

*SymbOS name*: `Clipboard_Get` (`BUFGET`).

#### Clip_Type()

```c
unsigned char Clip_Type(void);
```

Returns the type of data in the clipboard, if any (0 = empty, 1 = text, 2 = extended graphic, 3 = item list, 4 = desktop icon shortcut).

*SymbOS name*: `Clipboard_Status` (`BUFSTA`).

#### Clip_Len()

```c
unsigned short Clip_Len(void);
```

Returns the length of data in the clipboard, in bytes.

*SymbOS name*: `Clipboard_Status` (`BUFSTA`).

### System status

#### Sys_Counter()

```c
unsigned long Sys_Counter(void);
```

Returns the system counter, which increments 50 times per second. This can be used to calculate time elapsed for regulating framerates in games, etc.

*SymbOS name*: `Multitasking_GetCounter` (`MTGCNT`).

#### Sys_IdleCount()

```c
unsigned short Sys_IdleCount(void);
```

Returns the idle process counter, which increments every 64 microseconds. This can be used to calculate CPU usage.

*SymbOS name*: `Multitasking_GetCounter` (`MTGCNT`).

### Screen status

#### Screen_Mode()

```c
unsigned char Screen_Mode(void);
```

Returns the current screen mode, which depends on the current platform:

| Mode  | Platform  | Resolution | Colors  |
| ------| --------- | ---------- | ------- |
| 0     | PCW       | 720x255    | 2       |
| 1     | CPC/EP    | 320x200    | 4       |
| 2     | CPC/EP    | 640x200    | 2       |
| 5     | MSX       | 256x212    | 16      |
| 6     | MSX       | 512x212    | 4       |
| 7     | MSX       | 512x212    | 16      |
| 8     | G9K       | 384x240    | 16      |
| 9     | G9K       | 512x212    | 16      |
| 10    | G9K       | 768x240    | 16      |
| 11    | G9K       | 1024x212   | 16      |

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

#### Screen_Colors()

```c
unsigned char Screen_Colors(void);
```

Returns the number of displayed colors in the current screen mode (2, 4, or 16).

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

#### Screen_Width()

```c
unsigned short Screen_Width(void);
```

Returns the horizontal width of the screen, in pixels.

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

#### Screen_Height()

```c
unsigned short Screen_Height(void);
```

Returns the vertical height of the screen, in pixels.

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

### Mouse status

#### Mouse_X()

```c
unsigned short Mouse_X(void);
```

Returns the horizontal position of the mouse pointer, in pixels.

*SymbOS name*: `Device_MousePosition` (`MOSGET`).

#### Mouse_Y()

```c
unsigned short Mouse_Y(void);
```

Returns the vertical position of the mouse pointer, in pixels.

*SymbOS name*: `Device_MousePosition` (`MOSGET`).

#### Mouse_Buttons()

```c
unsigned char Mouse_Buttons(void);
```

Returns the current status of the mouse buttons as a bitmask. We can perform a binary AND of the return value with `BUTTON_LEFT`, `BUTTON_RIGHT`, and `BUTTON_MIDDLE` to determine whether the respective button is currently pressed:

```c
lbut = Mouse_Buttons() & BUTTON_LEFT;
```

*SymbOS name*: `Device_MouseKeyStatus` (`MOSKEY`).

### Keyboard status

#### Key_Down()

```c
unsigned char Key_Down(unsigned char scancode);
```

Returns 1 if the key specified by `scancode` is currently down, otherwise 0. **Note that keys are tested by *scancode*, not by their ASCII value!** A set of [scancode constants](#keyboard-scancodes) are provided for convenience.

*SymbOS name*: `Device_KeyTest` (`KEYTST`).

#### Key_Status()

```c
unsigned short Key_Status(void);
```

Returns the status of the modifier keys as a bitmask. We can perform a binary AND of the return value with `SHIFT_DOWN`, `CTRL_DOWN`, `ALT_DOWN`, and `CAPSLOCK_DOWN` to determine whether the respective modifier key is currently applied:

```c
caps = Key_Status() & CAPSLOCK_DOWN;
```

*SymbOS name*: `Device_KeyStatus` (`KEYSTA`).

#### Key_Put()

```c
void Key_Put(unsigned char keychar);
```

Pushes the ASCII code `keychar` into the keyboard buffer as if it had been pressed on the keyboard.

*SymbOS name*: `Device_KeyPut` (`KEYPUT`).

#### Key_Multi()

```c
unsigned char Key_Multi(unsigned char scancode1, unsigned char scancode2,
                        unsigned char scancode3, unsigned char scancode4,
                        unsigned char scancode5, unsigned char scancode6);
```


Like `Key_Test()`, but tests up to six keys simultaneously. This may save time when testing large numbers of keys for (e.g.) a game. The return value is a bitmask:

* Bit 0: set if key `scancode1` is pressed
* Bit 1: set if key `scancode2` is pressed
* Bit 2: set if key `scancode3` is pressed
* Bit 3: set if key `scancode4` is pressed
* Bit 4: set if key `scancode5` is pressed
* Bit 5: set if key `scancode6` is pressed

**Note that keys are tested by *scancode*, not by their ASCII value!** A set of [scancode constants](#keyboard-scancodes) are provided for convenience.

*SymbOS name*: `Device_KeyMulti` (`KEYMUL`).

### Shell functions

SymShell functions will only be available if the application is associated with a running instance of SymShell. To ensure that an application is started in SymShell, make sure that it has the file extension `.com` instead of `.exe`.

Several globals provide useful information about the SymShell instance:

```c
extern unsigned char _shellpid;    // SymShell process ID
extern unsigned char _shellwidth;  // console width, in characters
extern unsigned char _shellheight; // console height, in characters
extern unsigned char _shellver;    // SymShell version
extern unsigned char _shellerr;    // error code of last shell command
```

If `_shellpid` = 0, there is no SymShell instance. `_shellver` is a two-digit number where the tens digit is the major version and the ones digit is the minor version, e.g., 21 = 2.1.

Most shell functions allow specifying a *channel*. In general, channel 0 is the standard input/output, which is usually the keyboard (in) and text window (out) but may also be a file or stream if some type of redirection is active. This is similar to the behavior of stdin/stdout in standard C (there is no direct equivalent to stderr). Channel 1 is always the physical keyboard (in) or text window (out), even if redirection is active on channel 0. Usually we want channel 0.

Note that SymShell returns the Windows-style ASCII character 13 (`\r`) for the "Enter" key, *not* the Unix-style ASCII character 10 (`\n`), as is more common in C. Likewise, when sending text to the console, note that SymShell expects the Windows-style line terminator `\r\n` rather than the Unix-style `\n` that is more common in C. If we only send `\n`, SymShell will take this literally, only performing a line feed (`\n`, going down a line) but not a carriage return (`\r`, going back to the start of the next line)! The stdio implementation (`printf()`, etc.) includes some logic to paper over these differences and understand the Unix-style convention, but when working with SymShell functions directly, we will need to be more careful.

#### Shell_CharIn()

```c
int Shell_CharIn(unsigned char channel);
```

Requests an input character from the specified  `channel`. If this is the console keyboard and there is no character waiting in the keybuffer, SymShell will pause until the user presses a key.

*Return value*: On success, returns the ASCII value of the character (including [extended ASCII codes](#extended-ascii-codes) for special keys). If we have hit EOF on an input stream, returns -1. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharInput_Command` (`MSC_SHL_CHRINP`).

#### Shell_CharOut()

```c
signed char Shell_CharOut(unsigned char channel, unsigned char val);
```

Sends ASCII character `val` to the specified  `channel`.

While this is the standard way to output a single character to the console, note that outputting long strings by repeatedly calling `Shell_CharOut()` will be very slow, because for every character sent, SymShell must (1) receive the message, (2) redraw the screen, and (3) send a response message. Sending a single longer string with `Shell_StringOut()` only requires one set of messages and one redraw and is therefore  much more efficient.

*Return value*: On success, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharOutput_Command` (`MSC_SHL_CHROUT`).

#### Shell_CharTest()

```c
int Shell_CharTest(unsigned char channel, unsigned char lookahead);
```

Behaves like `Shell_CharIn()`, except that if there is no character waiting in the keybuffer, it will return 0 immediately without waiting for input. If `lookahead` = 0, any character found will be returned but left in the keybuffer; if `lookahead` = 1, the character will be removed from the keybuffer.

This function requires SymShell 2.3 or greater and will always return -1 (EOF) on earlier versions. This function currently only works for physical keyboard input, not redirected streams.

*Return value*: On success, returns the ASCII value of the character (including [extended ASCII codes](#extended-ascii-codes) for special keys). If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharTest_Command` (`MSC_SHL_CHRTST`).

#### Shell_StringIn()

```c
signed char Shell_StringIn(unsigned char channel, unsigned char bank, char* addr);
```

Requests a line of input from the specified  `channel`, terminated by the Enter key. If this is the console keyboard, SymShell will pause and accept input until the user presses Enter. The input will be written to memory (zero-terminated) at bank `bank`, address `addr`.

Input may be up to 255 characters in length, plus a zero-terminator, so the write buffer should always be at least 256 bytes long.

*Return value*: On success, returns 0. If we have hit EOF on an input stream, returns -1. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_StringInput_Command` (`MSC_SHL_STRINP`).

#### Shell_StringOut()

```c
signed char Shell_StringOut(unsigned char channel, unsigned char bank,
                            char* addr, unsigned char len);
```

Sends the string at bank `bank`, address `addr` to the specified  `channel`. The string can be up to 255 bytes long and must be zero-terminated. `len` must contain the length of the string (without the zero-terminator); the `string.h` function `strlen()` is a good way to determine this.

*Return value*: On success, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_StringOutput_Command` (`MSC_SHL_STROUT`).

#### Shell_Exit()

```c
void Shell_Exit(unsigned char type);
```

Informs SymShell that the app is closing down, so it can stop waiting for input/output messages from the app. (Normally this is handled automatically by `exit()`, but we can also do it manually.) If `type` = 0, the app is exiting normally and should be unregistered with SymShell. If `type` = 1, the app is going into "blur" mode: it is still running in the background, but no longer plans to output anything to the shell.

*SymbOS name*: `SymShell_Exit_Command` (`MSC_SHL_EXIT`).

#### Shell_PathAdd()

```c
void Shell_PathAdd(unsigned char bank, char* path, char* addition, char* dest);
```

A utility function that constructs an absolute file path from a base path (at bank `bank`, address `path`) and a relative path addition (at bank `bank`, address `addition`), storing the result in bank `bank`, address `dest`. This is mainly used to turn relative paths into absolute paths for the file manager functions.

Any relative path elements in the addition (`..\`, etc.) will be resolved. If `path` = 0, the absolute path will be relative to the current shell path (i.e., the path set by the CD command). The base path should not end with a slash or backslash.

Examples:
```c
char abspath[256];

Shell_PathAdd(_symbank, "C:\SYMBOS\APPS", "..\MUSIC\MP3\LALALA.MP3", abspath);
// yields: C:\SYMBOS\MUSIC\MP3\LALALA.MP3

Shell_PathAdd(_symbank, "A:\GRAPHICS\NATURE", "\SYMBOS", abspath);
// yields: A:\SYMBOS

Shell_PathAdd(_symbank, "C:\ARCHIVE", "*.ZIP", abspath);
// yields: C:\ARCHIVE\*.ZIP

Shell_PathAdd(_symbank, "A:\ARCHIVE", "C:\SYMBOS", abspath);
// yields: C:\SYMBOS
```

*SymbOS name*: `SymShell_PathAdd_Command` (`MSC_SHL_PTHADD`).

## Other considerations

### The `malloc()` heap

### Keyboard scancodes

| Code | Code | Code | Code | Code | 
| ---- | ---- | ---- | ---- | ---- |
| `SCAN_0` | `SCAN_G` | `SCAN_W` | `SCAN_UP` | `SCAN_FIRE_1` |
| `SCAN_1` | `SCAN_H` | `SCAN_X` | `SCAN_DOWN` | `SCAN_FIRE_2` |
| `SCAN_2` | `SCAN_I` | `SCAN_Y` | `SCAN_LEFT` | `SCAN_JOY_DOWN` |
| `SCAN_3` | `SCAN_J` | `SCAN_Z` | `SCAN_RIGHT` | `SCAN_JOY_LEFT` |
| `SCAN_4` | `SCAN_K` | `SCAN_F0` | `SCAN_ALT` | `SCAN_JOY_RIGHT` |
| `SCAN_5` | `SCAN_L` | `SCAN_F1` | `SCAN_AT` | `SCAN_JOY_UP` |
| `SCAN_6` | `SCAN_M` | `SCAN_F2` | `SCAN_BSLASH` | `SCAN_LBRACKET` |
| `SCAN_7` | `SCAN_N` | `SCAN_F3` | `SCAN_CAPSLOCK` | `SCAN_MINUS` |
| `SCAN_8` | `SCAN_O` | `SCAN_F4` | `SCAN_CARET` | `SCAN_PERIOD` |
| `SCAN_9` | `SCAN_P` | `SCAN_F5` | `SCAN_CLR` | `SCAN_RBRACKET` |
| `SCAN_A` | `SCAN_Q` | `SCAN_F6` | `SCAN_COLON` | `SCAN_RETURN` |
| `SCAN_B` | `SCAN_R` | `SCAN_F7` | `SCAN_COMMA` | `SCAN_SEMICOLON` |
| `SCAN_C` | `SCAN_S` | `SCAN_F8` | `SCAN_CTRL` | `SCAN_SHIFT` |
| `SCAN_D` | `SCAN_T` | `SCAN_F9` | `SCAN_DEL` | `SCAN_SLASH` |
| `SCAN_E` | `SCAN_U` | `SCAN_FDOT` | `SCAN_ENTER` | `SCAN_SPACE` |
| `SCAN_F` | `SCAN_V` | `SCAN_ESC` | `SCAN_TAB` | |

### Extended ASCII codes

| Code | Code | Code | Code |
| ---- | ---- | ---- | ---- |
| `KEY_UP` | `KEY_FDOT` | `KEY_ALT_M` | `KEY_ALT_0` |
| `KEY_DOWN` | `KEY_ALT_AT` | `KEY_ALT_N` | `KEY_ALT_1` |
| `KEY_LEFT` | `KEY_ALT_A` | `KEY_ALT_O` | `KEY_ALT_2` |
| `KEY_RIGHT` | `KEY_ALT_B` | `KEY_ALT_P` | `KEY_ALT_3` |
| `KEY_F0` | `KEY_ALT_C` | `KEY_ALT_Q` | `KEY_ALT_4` |
| `KEY_F1` | `KEY_ALT_D` | `KEY_ALT_R` | `KEY_ALT_5` |
| `KEY_F2` | `KEY_ALT_E` | `KEY_ALT_S` | `KEY_ALT_6` |
| `KEY_F3` | `KEY_ALT_F` | `KEY_ALT_T` | `KEY_ALT_7` |
| `KEY_F4` | `KEY_ALT_G` | `KEY_ALT_U` | `KEY_ALT_8` |
| `KEY_F5` | `KEY_ALT_H` | `KEY_ALT_V` | `KEY_ALT_9` |
| `KEY_F6` | `KEY_ALT_I` | `KEY_ALT_W` |  |
| `KEY_F7` | `KEY_ALT_J` | `KEY_ALT_X` |  |
| `KEY_F8` | `KEY_ALT_K` | `KEY_ALT_Y` |  |
| `KEY_F9` | `KEY_ALT_L` | `KEY_ALT_Z` |  |


### Building SCC

The current primary build target for SCC is Windows. Install MinGW, ensure that its `bin` folder is in the system path, and then run the `make.bat` batch files found throughout the SCC source tree to compile the relevant parts of the codebase. (This really ought to transition to proper Makefiles, but whatever.)
