# System call reference

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

## Messaging

### Msg_Send()
```c
unsigned char Msg_Send(char rec_pid, char send_pid, char* msg);
```

Sends the message in `*msg` to process ID `send_pid`. `rec_pid` is the process ID that should receive the response, if any; usually this should be our own process ID (`_sympid`). `*msg` must be in the **transfer** segment.

*Return value*: 0 = message queue is full; 1 = message sent successfully; 2 = receiver process does not exist.

*SymbOS name*: `Message_Send` (`MSGSND`).

### Msg_Receive()

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

### Msg_Sleep()

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

### Idle()

```c
void Idle(void);
```
Return CPU time to SymbOS and idle until something wakes it up---for example, an incoming message.

*SymbOS name*: `Multitasking_SoftInterrupt` (`RST #30`).

## Memory management

Applications are able to address more that 64KB of memory by reserving additional blocks of banked memory. These blocks cannot be addressed directly using C pointers and variables, but we can read/write/copy data to them using system functions.

### Mem_Reserve()

```c
unsigned char Mem_Reserve(unsigned char bank, unsigned char type, unsigned short len, unsigned char* bankVar, char** addrVar);
```

Reserve a block of banked memory in bank `bank` of length `len`, in bytes. `bank` may be from 0 to 15; 0 means "any bank can be used." `type` may be one of: 0 = located anywhere; 1 = reserve within a 16KB address block (like the **data** segment); 2 = reserve within the last 16KB address block (like the **transfer** segment).

Two variables must be passed by reference to store the address of the resulting block of banked memory: `bankVar` (type `unsigned char`), which stores the bank, and `addrVar` (type `char*`), which stores the address.

**Note that, to avoid memory leaks, memory reserved with `Mem_Reserve()` ***must*** be manually released with `Mem_Release()` before program exit!** SymbOS does not have the resources to track this automatically; it is up to us.

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Memory_Get` (`MEMGET`).

### Mem_Release()

```c
void Mem_Release(unsigned char bank, char* addr, unsigned short len);
```

Releases a block of banked memory previously reserved with `Mem_Reserve()`. `bank` is the bank of the reserved memory, which must be from 1 to 15; `addr` is the address; and `len` is the length of the reserved block, in bytes.

**Be careful to ensure that `bank`, `addr`, and `len` exactly match a contiguous block of memory that was previously reserved with `Mem_Reserve()`!** SymbOS does not keep track of this independently, so we can corrupt memory if we pass invalid information.

*SymbOS name*: `Memory_Free` (`MEMFRE`).

### Mem_Resize()

```c
unsigned char Mem_Resize(unsigned char bank, char* addr, unsigned short oldlen, unsigned short newlen);
```

Attempts to resize a block of banked memory previously reserved with `Mem_Reserve()`. `bank` is the bank of the reserved memory, which must be from 1 to 15; `addr` is the address; `oldlen` is the previous length of the reserved block, in bytes; and `newlen` is the requested new length, in bytes.

Shortening a block will always work. Lengthening a block will only work if the required addresses (immediately after the end of the old block) are available, which is unlikely if the user has started any new applications since the block was reserved. A more robust alternative is the SCC-specific helper function `Mem_ResizeX()` (see below).

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Memory_Resize` (`MEMSIZ`).

### Mem_ResizeX()

```c
unsigned char Mem_ResizeX(unsigned char bank, unsigned char type, char* addr,
                          unsigned short oldlen, unsigned short newlen,
                          unsigned char* bankVar, char** addrVar);
```

A more robust SCC extension to `Mem_Resize()`, above. Attempts to resize a block of banked memory previously reserved with `Mem_Reserve()` by first trying calling `Mem_Resize()`; if this does not succeed, it tries again by reserving a new block of the desired size, copying the old block to the new block, and releasing the old block. `bank` is the bank of the reserved memory, which must be from 1 to 15. `type` may be one of: 0 = located anywhere; 1 = only move within a 16KB address block (like the **data** segment); 2 = only move within the last 16KB address block (like the **transfer** segment). `addr` is the previous address; `oldlen` is the previous length of the reserved block, in bytes; and `newlen` is the requested new length, in bytes.

Two variables must be passed by reference to store the address of the resulting block: `bankVar` (type `unsigned char`), which stores the bank, and `addrVar` (type `char*`), which stores the address. Note that a moved block may be in any bank, not just the same bank as the previous block.

*Return value*: 0 = success, 1 = out of memory.

### Mem_Longest()

```c
unsigned short Mem_Longest(unsigned char bank, unsigned char type);
```

Returns (in bytes) the longest area of contiguous memory within bank `bank` that could be reserved with `Mem_Reserve()`. `bank` may be from 0 to 15; 0 means "any bank can be used." `type` may be one of: 0 = located anywhere; 1 = reserve within a 16KB address block (like the **data** segment); 2 = reserve within the last 16KB address block (like the **transfer** segment).

*SymbOS name*: `Memory_Information` (`MEMINF`).

### Mem_Free()

```c
unsigned long Mem_Free(void);
```

Returns the total amount of free memory, in bytes.

*SymbOS name*: `Memory_Summary` (`MEMSUM`).

### Mem_Banks()

```c
unsigned char Mem_Banks(void);
```

Returns the total number of existing 64KB extended RAM banks.

*SymbOS name*: `Memory_Summary` (`MEMSUM`).

## Memory read/write

### Bank_ReadWord()

```c
unsigned short Bank_ReadWord(unsigned char bank, char* addr);
```

Returns the two-byte word at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_ReadWord` (`BNKRWD`).

### Bank_WriteWord()

```c
void Bank_WriteWord(unsigned char bank, char* addr, unsigned short val);
```

Writes the two-byte word `val` to memory at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_WriteWord` (`BNKWWD`).

### Bank_ReadByte()

```c
unsigned char Bank_ReadByte(unsigned char bank, char* addr);
```

Returns the byte at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_ReadByte` (`BNKRBT`).

### Bank_WriteByte()

```c
void Bank_WriteByte(unsigned char bank, char* addr, unsigned char val);
```

Writes the byte `val` to memory at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_WriteByte` (`BNKWBT`).

### Bank_Copy()

```c
void Bank_Copy(unsigned char bankDst, char* addrDst, unsigned char bankSrc, char* addrSrc, unsigned short len);
```

Copies `len` bytes of memory from bank `bankSrc`, address `addrSrc` to bank `bankDst`, address `addrDst`.

*SymbOS name*: `Banking_Copy` (`BNKCOP`).

### Bank_Get()

```c
unsigned char Bank_Get(void);
```

Returns the bank number in which the app's main process is running. (Normally it is easier to use the `_symbank` global for this purpose.)

*SymbOS name*: `Banking_GetBank` (`BNKGET`).

## System status

### Sys_Counter()

```c
unsigned long Sys_Counter(void);
```

Returns the system counter, which increments 50 times per second. This can be used to calculate time elapsed for regulating framerates in games, etc.

*SymbOS name*: `Multitasking_GetCounter` (`MTGCNT`).

### Sys_IdleCount()

```c
unsigned short Sys_IdleCount(void);
```

Returns the idle process counter, which increments every 64 microseconds. This can be used to calculate CPU usage.

*SymbOS name*: `Multitasking_GetCounter` (`MTGCNT`).

## Screen status

### Screen_Mode()

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

### Screen_Colors()

```c
unsigned char Screen_Colors(void);
```

Returns the number of displayed colors in the current screen mode (2, 4, or 16).

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

### Screen_Width()

```c
unsigned short Screen_Width(void);
```

Returns the horizontal width of the screen, in pixels.

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

### Screen_Height()

```c
unsigned short Screen_Height(void);
```

Returns the vertical height of the screen, in pixels.

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

## Mouse status

### Mouse_X()

```c
unsigned short Mouse_X(void);
```

Returns the horizontal position of the mouse pointer, in pixels.

*SymbOS name*: `Device_MousePosition` (`MOSGET`).

### Mouse_Y()

```c
unsigned short Mouse_Y(void);
```

Returns the vertical position of the mouse pointer, in pixels.

*SymbOS name*: `Device_MousePosition` (`MOSGET`).

### Mouse_Buttons()

```c
unsigned char Mouse_Buttons(void);
```

Returns the current status of the mouse buttons as a bitmask. We can perform a binary AND of the return value with `BUTTON_LEFT`, `BUTTON_RIGHT`, and `BUTTON_MIDDLE` to determine whether the respective button is currently pressed:

```c
lbut = Mouse_Buttons() & BUTTON_LEFT;
```

*SymbOS name*: `Device_MouseKeyStatus` (`MOSKEY`).

## Keyboard status

### Key_Down()

```c
unsigned char Key_Down(unsigned char scancode);
```

Returns 1 if the key specified by `scancode` is currently down, otherwise 0. **Note that keys are tested by *scancode*, not by their ASCII value!** A set of [scancode constants](#keyboard-scancodes) are provided for convenience.

*SymbOS name*: `Device_KeyTest` (`KEYTST`).

### Key_Status()

```c
unsigned short Key_Status(void);
```

Returns the status of the modifier keys as a bitmask. We can perform a binary AND of the return value with `SHIFT_DOWN`, `CTRL_DOWN`, `ALT_DOWN`, and `CAPSLOCK_DOWN` to determine whether the respective modifier key is currently applied:

```c
caps = Key_Status() & CAPSLOCK_DOWN;
```

*SymbOS name*: `Device_KeyStatus` (`KEYSTA`).

### Key_Put()

```c
void Key_Put(unsigned char keychar);
```

Pushes the ASCII code `keychar` into the keyboard buffer as if it had been pressed on the keyboard.

*SymbOS name*: `Device_KeyPut` (`KEYPUT`).

### Key_Multi()

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

## Window management

### Win_Open()

```c
signed char Win_Open(unsigned char bank, void* addr);
```

Opens the window whose data record (a struct of type `Window`) is at bank `bank`, address `addr`. This must be in the **transfer** segment.

*Return value*: On success, returns the window ID. On failure, returns -1.

*SymbOS name*: `Window_Open_Command` (`MSC_DSK_WINOPN`).

### Win_Close()

```c
void Win_Close(unsigned char winID);
```

Closes the window with the ID `winID`.

*SymbOS name*: `Window_Close_Command` (`MSC_DSK_WINCLS`).

### Win_Redraw()

```c
void Win_Redraw(unsigned char winID, signed char what, unsigned char first);
```

Redraws one or more controls in the main window content of window `winID`. `what` can be either (1) -1, meaning "redraw entire window content"; (2) the control ID of a single control to redraw; or (3) a negative number (from -2 to -16) indicating how many controls should be redrawn (from 2 to 16), in which case `first` indicates the control ID of the first control to redraw.

For performance reasons the window will only actually be redrawn if it has focus. To force redraw of the window even when it does not have focus, see `Win_Redraw_Ext()`.

*SymbOS name*: `Window_Redraw_Content_Command` (`MSC_DSK_WININH`).

### Win_RedrawExt()

```c
void Win_RedrawExt(unsigned char winID, signed char what, unsigned char first);
```

Equivalent to `Win_Redraw()`, but redraws controls whether or not the window has focus. (This is slightly slower than `Win_Redraw()` because SymbOS must check for window overlap.)

*SymbOS name*: `Window_Redraw_ContentExtended_Command` (`MSC_DSK_WINDIN`).

### Win_Redraw_Area()

```c
void Win_Redraw_Area(unsigned char winID, unsigned char what, unsigned char first,
                     unsigned short x, unsigned short y, unsigned short w, unsigned short h);
```

Equivalent to `Win_Redraw()`, but only redraws controls within the box specified by the upper left coordinates `x` and `y`, width `w`, and height `h` (in pixels). Note that these coordinates are relative to the window content, including any scroll. This command is particularly useful for redrawing only part of a large graphic area (such as a game playfield), since it is much faster than redrawing the entire area.

*SymbOS name*: `Window_Redraw_ContentArea_Command` (`MSC_DSK_WINPIN`).

### Win_Redraw_Toolbar()

```c
void Win_Redraw_Toolbar(unsigned char winID, signed char what, unsigned char first);
```

Equivalent to `Win_Redraw()`, but redraws controls in the window's toolbar control group instead of main window content.

*SymbOS name*: `Window_Redraw_Toolbar_Command` (`MSC_DSK_WINTOL`).

### Win_Redraw_Menu()

```c
void Win_Redraw_Menu(unsigned char winID);
```

If the window has focus, redraws the menu of window `WinID`.

*SymbOS name*: `Window_Redraw_Menu_Command` (`MSC_DSK_WINMEN`).

### Win_Redraw_Title()

```c
void Win_Redraw_Title(unsigned char winID);
```

If the window has focus, redraws the titlebar of window `WinID`.

*SymbOS name*: `Window_Redraw_Title_Command` (`MSC_DSK_WINTIT`).

### Win_Redraw_Status()

```c
void Win_Redraw_Status(unsigned char winID);
```

If the window has focus, redraws the statusbar of window `WinID`.

*SymbOS name*: `Window_Redraw_Status_Command` (`MSC_DSK_WINSTA`).

### Win_Redraw_Slider()

```c
void Win_Redraw_Slider(unsigned char winID);
```

If the window has focus and is resizable, redraws the main content area's scrollbars on window `WinID`.

*SymbOS name*: `Window_Redraw_Status_Command` (`MSC_DSK_WINSTA`).

### Win_Redraw_Sub()

```c
void Win_Redraw_Sub(unsigned char winID, unsigned char collection, unsigned char control);
```

Redraws the single control with the ID `control` inside the control collection with the ID `collection` on window `winID`.

*SymbOS name*: `Window_Redraw_SubControl_Command` (`MSC_DSK_WINSIN`).

### Win_ContentX()

```c
void Win_ContentX(unsigned char winID, unsigned short newX);
```

Changes the horizontal scroll position (`.xscroll`) of window `winID` to `newX` pixels. If the window has focus, the relevant portions of the content will be redraw.

This command is faster than updating the `.xscroll` record of the window and redrawing manually because only the newly visible portion of the window will be redrawn from scratch. The scroll position can be changed even if the window is not resizable by the user.

*SymbOS name*: `Window_Set_ContentX_Command` (`MSC_DSK_WINMVX`).

### Win_ContentY()

```c
void Win_ContentY(unsigned char winID, unsigned short newX);
```

Changes the vertical scroll position (`.yscroll`) of window `winID` to `newY` pixels. If the window has focus, the relevant portions of the content will be redraw.

This command is faster than updating the `.yscroll` record of the window and redrawing manually because only the newly visible portion of the window will be redrawn from scratch. The scroll position can be changed even if the window is not resizable by the user.

*SymbOS name*: `Window_Set_ContentY_Command` (`MSC_DSK_WINMVY`).

### Win_Focus()

```c
void Win_Focus(unsigned char winID);
```

Gives the window `winID` focus, bringing it to the front of the desktop.

*SymbOS name*: `Window_Focus_Command` (`MSC_DSK_WINTOP`).

### Win_Maximize()

```c
void Win_Maximize(unsigned char winID);
```

Maximizes the window `winID`.

*SymbOS name*: `Window_Size_Maximize_Command` (`MSC_DSK_WINMAX`).

### Win_Minimize()

```c
void Win_Minimize(unsigned char winID);
```

Minimizes the window `winID`.

*SymbOS name*: `Window_Size_Minimize_Command` (`MSC_DSK_WINMIN`).

### Win_Restore()

```c
void Win_Restore(unsigned char winID);
```

Restores the window `winID` to normal size, if maximized.

*SymbOS name*: `Window_Size_Restore_Command` (`MSC_DSK_WINMID`).

### Win_Move()

```c
void Win_Move(unsigned char winID, unsigned short newX, unsigned short newY);
```

If the window is not maximized, moves the window `winID` to horizontal position `newX`, vertical position `newY` on the screen (in pixels).

*SymbOS name*: `Window_Set_Position_Command` (`MSC_DSK_WINMOV`).

### Win_Resize()

```c
void Win_Resize(unsigned char winID, unsigned short newW, unsigned short newH);
```

Resizes the window `winID` so that the main content of the window has the width `newW` and the heigth `newH` (in pixels). Note that this is the size of the main *content*, not the size of the entire window; the titlebar, menubar, toolbar, statusbar, and scrollbars (if any) may add additional size.

*SymbOS name*: `Window_Set_Size_Command` (`MSC_DSK_WINSIZ`).

### Menu_Context()

```c
int Menu_Context(unsigned char bank, char* addr, int x, int y);
```

Opens a context menu at the specified `x` and `y` coordinates on the screen (in pixels). The menu data is a `Menu` struct (and associated `Menu_Entry` structs) located at bank `bank`, address `addr` in the **transfer** segment. If `x` = -1, the context menu will be opened at the current mouse position.

*SymbOS name*: `Menu_Context_Command` (`MSC_DSK_MENCTX`).

### Select_Pos()

```c
char Select_Pos(unsigned short* x, unsigned short* y, unsigned short w, unsigned short h);
```

Opens a "rubber band" selector (dotted rectangle) at the specified `*x` and `*y` position on the screen (in pixels), with width `w` and height `h` (in pixels). The user will be able to change the position (but not the size) of this selector by moving the mouse, until they either confirm their selection by releasing the left mouse button or cancel it by pressing ESC. (This is usually used for drag-and-drop operations triggered by first pressing the left mouse button.) The final position of the selector will be written back to the variables passed by reference in `x` and `y`.

*Return value*: On successful completion, returns 1. If the user cancelled the operation by pressing ESC, returns 0.

*SymbOS name*: `VirtualControl_Position_Command` (`MSC_DSK_CONPOS`).

### Select_Size()

```c
char Select_Size(unsigned short x, unsigned short y, unsigned short* w, unsigned short* h);
```

Opens a "rubber band" selector (dotted rectangle) at the specified `x` and `y` position on the screen (in pixels), with width `*w` and height `*h` (in pixels). The user will be able to change the size (but not the position) of this selector by moving the mouse, until they either confirm their selection by releasing the left mouse button or cancel it by pressing ESC. (This is usually used for drag-and-drop operations triggered by first pressing the left mouse button.) The final size of the selector will be written back to the variables passed by reference in `w` and `h`.

*Return value*: On successful completion, returns 1. If the user cancelled the operation by pressing ESC, returns 0.

*SymbOS name*: `VirtualControl_Size_Command` (`MSC_DSK_CONSIZ`).

## File access

File functions in SymbOS work using numerical file handles. Due to system limits only 8 files can be open at one time, so it is easy to run out of file handles if we are not careful. **Be careful to close any files you open before exiting!** Files opened with `stdio.h` functions (`fopen()`, etc.) will be closed automatically on exit, but files opened with direct system calls will not.

SymbOS does not make a distinction between opening a file for reading or writing.

**All file paths given to system calls must be absolute**, e.g., `A:\DIR\FILE.TXT`, not `FILE.TXT`. For a convenient way to convert relative paths into absolute paths, see [Dir_PathAdd()](#dir-pathadd).

A quirk to know: Due to a limitation of the AMSDOS filesystem used by, e.g., CPC floppy disks, files on this filesystem only report their length to the nearest 128-byte block. The convention is to terminate the actual file with the AMSDOS EOF character (0x1A) and then pad out the file to the nearest 128-byte boundary with garbage.

As usual, the `stdio.h` implementation includes some logic to paper over this issue when using functions such as `fgets()`, but when accessing files directly with system calls we will need to keep this in mind. Files stored on FAT filesystems (i.e., most mass storage devices) do not have this limitation, although any files copied from an AMSDOS filesystem to a FAT filesystem may retain their garbage padding at the end.

### File_Open()

```c
unsigned char File_Open(unsigned char bank, char* path);
```

Opens the file at the absolute path stored in bank `bank`, address `path` and returns the system file handle. The seek position will be set to the start of the file.

*Return value*: On success, returns the file handle (0 to 7). On failure, sets and returns `_fileerr` (which will always be greater than 7).

*SymbOS name*: `File_Open` (`FILOPN`).

### File_New()

```c
unsigned char File_New(unsigned char bank, char* path);
```

Creates a new file at the absolute path stored in bank `bank`, address `path` and returns the system file handle. If the file already existed, it will be emptied. The seek position will be set to the start of the file.

*Return value*: On success, returns the file handle (0 to 7). On failure, sets and returns `_fileerr` (which will always be greater than 7).

*SymbOS name*: `File_New` (`FILNEW`).

### File_Close()

```c
unsigned char File_Close(unsigned char id);
```

Closes the file handle `id`.

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `File_Close` (`FILCLO`).

### File_Read()

```c
unsigned short File_Read(unsigned char id, unsigned char bank, 
                         char* addr, unsigned short len);
```

Reads `len` bytes from the open file handle `id` into a buffer at bank `bank`, address `addr`. The seek position will be moved forward by `len` bytes, so the next call to `File_Read()` will read the next part of the file.

*Return value*: On success, returns the number of bytes read. On failure, sets `_fileerr` and returns 0.

*SymbOS name*: `File_Input` (`FILINP`).

### File_ReadLine

```c
unsigned char File_ReadLine(unsigned char id, unsigned char bank, char* addr);
```

Reads a line of text (up to 254 bytes) from the open file handle `id` into a buffer at bank `bank`, address `addr`. Text lines can be terminated by character 13 (`\r`), character 10 (`\n`), a Windows-style combination of both (`\r\n`), or the AMSDOS EOF character 0x1A. To avoid overflow, the buffer should be at least 255 bytes long. The seek position will be moved forward to the start of the next line.

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `File_LineInput` (`FILLIN`).

### File_Write()

```c
unsigned short File_Write(unsigned char id, unsigned char bank,
                          char* addr, unsigned short len)
```

Writes `len` bytes from the buffer at bank `bank`, address `addr` to open file handle `id`. The seek position will be moved forward by `len` bytes, so the next call to `File_Write()` will write to the next part of the file.

*Return value*: On success, returns the number of bytes written. On failure, sets `_fileerr` and returns 0.

*SymbOS name*: `File_Output` (`FILOUT`).

### File_Seek()

```c
long File_Seek(unsigned char id, long offset, unsigned char ref);
```

Sets the seek position of the open file handle `id` to `offset` bytes relative to the reference point `ref`, which may be one of:

* `SEEK_SET`: relative to the start of the file
* `SEEK_CUR`: relative to the current seek position
* `SEEK_END`: relative to the end of the file

For `SEEK_CUR` and `SEEK_END`, `offset` can be positive or negative. Some examples:

```c
File_Seek(f, 546, SEEK_SET);     // set seek position to byte 546
File_Seek(f, -16, SEEK_CUR);     // move seek position backwards 16 bytes
len = File_Seek(f, 0, SEEK_END); // get length by seeking to file end
```

(When using `File_Seek()` to determine the length of a file, as in the last example, note that the result may slightly overestimate the actual file length on AMSDOS filesystems---see the caveats at [the start of this chapter](#file-access).)

*Return value*: On success, returns the new seek position. On failure, sets `_fileerr` and returns -1.

*SymbOS name*: `File_Pointer` (`FILPOI`).

## Directory access

### Dir_Read()

```c
int Dir_Read(char* path, unsigned char attrib, void* buf,
             unsigned short len, unsigned short skip);
```

Reads the contents of the directory indicated by the absolute path stored at address `path`. This path may contain Windows-style wildcards (`*` and `?`, e.g., `*.txt`), in which case only matching filenames will be returned. `attrib` is a bitmask consisting of an OR'd list of attribute types that should *not* be included in the results:

* `ATTRIB_READONLY`: do not include read-only files
* `ATTRIB_HIDDEN`: do not include hidden files
* `ATTRIB_SYSTEM`: do not include system files
* `ATTRIB_VOLUME`: do not include volume information files (recommended)
* `ATTRIB_DIR`: do not include directories
* `ATTRIB_ARCHIVE`: do not include archived files

The first `skip` entries will be skipped---this is useful for reading a large number of filenames by performing multiple calls to `Dir_Read()`. Filenames will not be sorted.

The resulting directory information will be written to the buffer at address `buf`, with a maximum length of `len` bytes. This buffer is structured as an array of `DirEntry` structs:

```c
typedef struct {
    long len;             // file length in bytes
    unsigned short date;  // system datestamp
    unsigned short time;  // system timestamp
    unsigned char attrib; // attribute bitmask
    char name[13];        // filename (zero-terminated)
} DirEntry;
```

An example of a complete call to `Dir_Read()`:

```c
DirEntry dirbuf[64];
Dir_Read("A:\\FILES\\*.TXT", ATTRIB_VOLUME | ATTRIB_DIR,
         dirbuf, sizeof(dirbuf), 0);
printf("%s\n", dirbuf[0].name);
```

*Return value*: On success, returns the number of files found (which may be zero, if no matching files were found). On failure, sets `_fileerr` and returns -1.

*SymbOS name*: Internally, calls `Directory_Input` (`DIRINP`), but does some extra formatting work on the output. For the raw version of this call, see `Dir_ReadRaw()`.

### Dir_ReadRaw()

```c
int Dir_ReadRaw(unsigned char bank, char* path, unsigned char attrib,
                unsigned char bufbank, void* addr, unsigned short len,
                unsigned short skip);
```

Equivalent to `Dir_Read()`, but executes the system call directly without converting the output into an array of `DirEntry` structs. For details on the raw output format, see the [SymbOS developer documentation](https://symbos.org/download.htm).

*Return value*: On success, returns the number of files found (which may be zero, if no matching files were found). On failure, sets `_fileerr` and returns -1.

*SymbOS name*: `Directory_Input` (`DIRINP`).

### Dir_ReadExt()

```c
int Dir_ReadExt(unsigned char bank, char* path, unsigned char attrib,
                unsigned char bufbank, void* addr, unsigned short len,
                unsigned short skip, unsigned char cols);
```

An extended version of `Dir_Read()` that automatically formats directory information into a set of list controls for the use of windowed file-manager-type applications. The behavior of this function is fairly complicated (and its uses are fairly esoteric), so see the [SymbOS developer documentation](https://symbos.org/download.htm) for further details.

*SymbOS name*: `Directory_Input_Extended` (`DEVDIR`).

### Dir_New()

```c
unsigned char Dir_New(unsigned char bank, char* path);
```

Creates a new directory from the absolute path stored in bank `bank`, address `path`.

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `Directory_New` (`DIRNEW`).

### Dir_Rename()

```c
unsigned char Dir_Rename(unsigned char bank, char* path, char* newname);
```

Renames the file or directory indicated by the absolute path stored in bank `bank`, address `path` to the new name stored in bank `bank`, address `newname`. (The new name is just a name, e.g., `FILE.TXT`, and does not contain the full path.)

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `Directory_Rename` (`DIRREN`).

### Dir_Move()

```c
unsigned char Dir_Move(unsigned char bank, char* pathSrc, char* pathDst);
```

Moves the file indicated by the absolute path stored in bank `bank`, address `pathSrc` to the new absolute path stored in bank `bank`, address `pathDst`.

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `Directory_Move` (`DIRMOV`).

### Dir_Delete()

```c
unsigned char Dir_Delete(unsigned char bank, char* path);
```

Deletes the file indicated by the absolute path stored in bank `bank`, address `path`.

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `Directory_DeleteFile` (`DIRDEL`).

### Dir_DeleteDir()

```c
unsigned char Dir_DeleteDir(unsigned char bank, char* path);
```

Deletes the directory indicated by the absolute path stored in bank `bank`, address `path`.

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `Directory_DeleteDirectory` (`DIRRMD`).

### Dir_GetAttrib()

```c
signed char Dir_GetAttrib(unsigned char bank, char* path);
```

Retrieves the file attributes of the file at the absolute path stored in bank `bank`, address `path`. The attributes are an OR'd bitmask consisting of one or more of`ATTRIB_READONLY`, `ATTRIB_HIDDEN`, `ATTRIB_SYSTEM`, `ATTRIB_VOLUME`, `ATTRIB_DIR`and `ATTRIB_ARCHIVE`.

*Return value*: On success, returns a bitmask of the file's attributes. On failure, sets `_fileerr` and returns -1.

*SymbOS name*: `Directory_Property_Get` (`DIRPRR`).

### Dir_SetAttrib()

```c
unsigned char Dir_SetAttrib(unsigned char bank, char* path, unsigned char attrib);
```

Sets the file attributes of the file at the absolute path stored in bank `bank`, address `path` to the bitmask `attrib`, which is an OR'd bitmask consisting of one or more of`ATTRIB_READONLY`, `ATTRIB_HIDDEN`, `ATTRIB_SYSTEM`, `ATTRIB_VOLUME`, `ATTRIB_DIR`and `ATTRIB_ARCHIVE`.

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `Directory_Property_Set` (`DIRPRS`).

### Dir_PathAdd()

```c
char* Dir_PathAdd(char* path, char* addition, char* dest);
```

An SCC convenience function that constructs an absolute file path from a base path (at address `path`) and a relative path addition (at address `addition`), storing the result in address `dest`. This is mainly used to turn relative paths into absolute paths for the file manager functions. (This function is similar to the system function [`Shell_PathAdd()`](#shell-pathadd), but is available even in windowed applications that do not use SymShell.)

Any relative path elements in the addition (`..\`, etc.) will be resolved. If `path` = 0, the absolute path will be relative to the absolute path in which the current application's `.exe` or `.com` file is located; this is useful for loading accompanying data files that should always be in the same directory as the `.exe`.

Examples:
```c
char abspath[256];

Dir_PathAdd("C:\SYMBOS\APPS", "..\MUSIC\MP3\LALALA.MP3", abspath);
// yields: C:\SYMBOS\MUSIC\MP3\LALALA.MP3

Dir_PathAdd("A:\GRAPHICS\NATURE", "\SYMBOS", abspath);
// yields: A:\SYMBOS

Dir_PathAdd("C:\ARCHIVE", "*.ZIP", abspath);
// yields: C:\ARCHIVE\*.ZIP

Dir_PathAdd("A:\ARCHIVE", "C:\SYMBOS", abspath);
// yields: C:\SYMBOS

Dir_PathAdd(0, "FILE.DAT", abspath);
// if the app is located at C:\APPS\DEMO.EXE, yields: C:\APPS\FILE.DAT
```

*Return value*: Returns `dest`.

## Shell functions

SymShell functions will only be available if the application is associated with a running instance of SymShell. To ensure that an application is started in SymShell, make sure that it has the file extension `.com` instead of `.exe`.

Several globals provide useful information about the SymShell instance:

```c
unsigned char _shellpid;    // SymShell process ID
unsigned char _shellwidth;  // console width, in characters
unsigned char _shellheight; // console height, in characters
unsigned char _shellver;    // SymShell version
unsigned char _shellerr;    // error code of last shell command
```

If `_shellpid` = 0, there is no SymShell instance. `_shellver` is a two-digit number where the tens digit is the major version and the ones digit is the minor version, e.g., 21 = 2.1.

Most shell functions allow specifying a *channel*. In general, channel 0 is the standard input/output, which is usually the keyboard (in) and text window (out) but may also be a file or stream if some type of redirection is active. This is similar to the behavior of stdin/stdout in standard C (although note that there is no direct equivalent to stderr). Channel 1 is always the physical keyboard (in) or text window (out), even if redirection is active on channel 0. Usually we want channel 0.

Note that SymShell returns the Windows-style ASCII character 13 (`\r`) for the "Enter" key, *not* the Unix-style ASCII character 10 (`\n`), as is more common in C. Likewise, when sending text to the console, note that SymShell expects the Windows-style line terminator `\r\n` rather than the Unix-style `\n` that is more common in C. If we only send `\n`, SymShell will take this literally, only performing a line feed (`\n`, going down a line) but not a carriage return (`\r`, going back to the start of the next line)! The stdio implementation (`printf()`, etc.) includes some logic to paper over these differences and understand the Unix-style convention, but when working with SymShell functions directly, we will need to be more careful.

### Shell_CharIn()

```c
int Shell_CharIn(unsigned char channel);
```

Requests an input character from the specified  `channel`. If this is the console keyboard and there is no character waiting in the keybuffer, SymShell will pause until the user presses a key.

*Return value*: On success, returns the ASCII value of the character (including [extended ASCII codes](#extended-ascii-codes) for special keys). If we have hit EOF on an input stream, returns -1. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharInput_Command` (`MSC_SHL_CHRINP`).

### Shell_CharOut()

```c
signed char Shell_CharOut(unsigned char channel, unsigned char val);
```

Sends ASCII character `val` to the specified  `channel`.

While this is the standard way to output a single character to the console, note that outputting long strings by repeatedly calling `Shell_CharOut()` will be very slow, because for every character sent, SymShell must (1) receive the message, (2) redraw the screen, and (3) send a response message. Sending a single longer string with `Shell_StringOut()` only requires one set of messages and one redraw and is therefore  much more efficient.

*Return value*: On success, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharOutput_Command` (`MSC_SHL_CHROUT`).

### Shell_CharTest()

```c
int Shell_CharTest(unsigned char channel, unsigned char lookahead);
```

Behaves like `Shell_CharIn()`, except that if there is no character waiting in the keybuffer, it will return 0 immediately without waiting for input. If `lookahead` = 0, any character found will be returned but left in the keybuffer; if `lookahead` = 1, the character will be returned and removed from the keybuffer.

This function requires SymShell 2.3 or greater and will always return 0 on earlier versions. This function currently only works for physical keyboard input, not redirected streams.

*Return value*: If a key is waiting, returns the ASCII value of the character (including [extended ASCII codes](#extended-ascii-codes) for special keys). If no key is waiting, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharTest_Command` (`MSC_SHL_CHRTST`).

### Shell_StringIn()

```c
signed char Shell_StringIn(unsigned char channel, unsigned char bank, char* addr);
```

Requests a line of input from the specified  `channel`, terminated by the Enter key. If this is the console keyboard, SymShell will pause and accept input until the user presses Enter. The input will be written to memory (zero-terminated) at bank `bank`, address `addr`.

Input may be up to 255 characters in length, plus a zero-terminator, so the write buffer should always be at least 256 bytes long.

*Return value*: On success, returns 0. If we have hit EOF on an input stream, returns -1. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_StringInput_Command` (`MSC_SHL_STRINP`).

### Shell_StringOut()

```c
signed char Shell_StringOut(unsigned char channel, unsigned char bank,
                            char* addr, unsigned char len);
```

Sends the string at bank `bank`, address `addr` to the specified  `channel`. The string can be up to 255 bytes long and must be zero-terminated. `len` must contain the length of the string (without the zero-terminator); the `string.h` function `strlen()` is a good way to determine this.

*Return value*: On success, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_StringOutput_Command` (`MSC_SHL_STROUT`).

### Shell_Exit()

```c
void Shell_Exit(unsigned char type);
```

Informs SymShell that the app is closing down, so it can stop waiting for input/output messages from the app. (Normally this is handled automatically by `exit()`, but we can also do it manually.) If `type` = 0, the app is exiting normally and should be unregistered with SymShell. If `type` = 1, the app is going into "blur" mode: it is still running in the background, but no longer plans to output anything to the shell.

*SymbOS name*: `SymShell_Exit_Command` (`MSC_SHL_EXIT`).

### Shell_PathAdd()

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

## Clipboard functions

### Clip_Put()

```c
unsigned char Clip_Put(unsigned char bank, char* addr, unsigned short len, unsigned char type);
```

Stores `len` bytes of data from bank `bank`, address `addr` into the system clipboard. `type` may be one of: 1 = text, 2 = extended graphic, 3 = item list, 4 = desktop icon shortcut.

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Clipboard_Put` (`BUFPUT`).

### Clip_Get()

```c
unsigned short Clip_Get(unsigned char bank, char* addr, unsigned short len, unsigned char type);
```

Retrieves up to `len` bytes of data from the system clipboard and stores it in bank `bank`, address `addr`. `type` may be one of: 1 = text, 2 = extended graphic, 3 = item list, 4 = desktop icon shortcut. Data will only be retrieved if (1) the type of the data in the clipboard matches the requested type, and (2) the data length is not greater than `len`.

*Return value*: length of received data, in bytes.

*SymbOS name*: `Clipboard_Get` (`BUFGET`).

### Clip_Type()

```c
unsigned char Clip_Type(void);
```

Returns the type of data in the clipboard, if any (0 = empty, 1 = text, 2 = extended graphic, 3 = item list, 4 = desktop icon shortcut).

*SymbOS name*: `Clipboard_Status` (`BUFSTA`).

### Clip_Len()

```c
unsigned short Clip_Len(void);
```

Returns the length of data in the clipboard, in bytes.

*SymbOS name*: `Clipboard_Status` (`BUFSTA`).

## System tray

### Systray_Add()

```c
signed char Systray_Add(unsigned char bank, char* addr, unsigned char code);
```

Add the icon whose graphic is at bank `bank`, address `addr` to the system tray on the taskbar. These can be clicked by the user, which will generate an event (`MSR_DSK_EVTCLK`) with the reference value `code`. The icon must be an 8x8 4-color SGX graphics object.

*Return value*: On success, returns an icon ID, which can be used to later remove the icon with `Systray_Remove()`. If there are no more icon slots available, returns -1.

*SymbOS name*: `SystrayIcon_Add_Command` (`MSC_DSK_STTADD`).

### Systray_Remove()

```c
void Systray_Remove(unsigned char id);
```

Remove the icon with the ID `id` from the system tray.

*SymbOS name*: `SystrayIcon_Remove_Command` (`MSC_DSK_STTREM`).

## Reference

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
