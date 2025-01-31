# System call reference

The following calls are all available after including `symbos.h`:

```c
#include <symbos.h>
```

When cross-compiling on a modern computer, it is usually easiest to just include the entirety of `symbos.h`. However, this is a large file, so when compiling on SymbOS, we can reduce compilation time and memory usage by only including the sub-headers that are actually needed (e.g., `symbos/shell.h` or `symbos/windows.h`). See the section introductions for what functions are in which sub-header.

These headers are not 100% comprehensive; SymbOS provides some additional system calls not implemented in `symbos.h`, mainly low-level calls for dealing with storage devices, system configuration, and complicated applications that alter system functionality or execute code in multiple banks (SCC is not well-suited for this). These calls are discussed in the [SymbOS developer documentation](https://symbos.org/download.htm). It is assumed that, if you need these calls, you are probably already doing something complicated enough that a few extra wrapper functions won't be useful.

## Contents

* [System variables](#system-variables)
* [Messaging](#messaging)
* [Memory management](#memory-management)
* [Memory read/write](#memory-read-write)
* [System status](#system-status)
* [Screen status](#screen-status)
* [Mouse status](#mouse-status)
* [Keyboard status](#keyboard-status)
* [Window management](#window-management)
* [Onscreen popups](#onscreen-popups)
* [File access](syscall2.md#file-access)
* [Directory access](syscall2.md#directory-access)
* [Shell functions](syscall2.md#shell-functions)
* [Process management](syscall2.md#process-management)
* [Timer management](syscall2.md#timer-management)
* [Clipboard functions](syscall2.md#clipboard-functions)
* [Time functions](syscall2.md#time-functions)
* [System tray](syscall2.md#system-tray)
* [Sound](syscall2.md#sound)
* [Network interface](syscall2.md#network-interface)
* [Multithreading](syscall2.md#multithreading)
* [Reference tables](syscall2.md#reference-tables)

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

The other contents of the application header can be accessed directly with the struct `_symheader`, not documented here (see definition in `symbos/header.h`).

## Messaging

In addition to `symbos.h`, these functions can be found in `symbos/core.h`.

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

In addition to `symbos.h`, these functions can be found in `symbos/memory.h`.

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
unsigned char Mem_ResizeX(unsigned char bank, unsigned char type, char* addr,
                          unsigned short oldlen, unsigned short newlen,
                          unsigned char* bankVar, char** addrVar);
```

Attempts to resize a block of banked memory previously reserved with `Mem_Reserve()`. (This is accomplished manually by reserving a new block of the desired size, copying the old block to the new block, and releasing the old block.) `bank` is the bank of the reserved memory, which must be from 1 to 15; `addr` is the address; `oldlen` is the previous length of the reserved block, in bytes; and `newlen` is the requested new length, in bytes.

Two variables must be passed by reference to store the address of the resulting block: `bankVar` (type `unsigned char`), which stores the bank, and `addrVar` (type `char*`), which stores the address. Note that the new location of the block may be in any bank, not just the same bank as the previous block.

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

In addition to `symbos.h`, these functions can be found in `symbos/memory.h`.

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

### Bank_Decompress()

```c
void Bank_Decompress(unsigned char bank, char* addrDst, char* addrSrc);
```

Decompresses the compressed data block located at bank `bank`, address `addrSrc` into memory at bank `bank`, address `addrDst`. The addresses must be arranged such that the last address of the decompressed data will be the same as the last address of the original compressed data. That is, we need to know the length of the uncompressed data ahead of time, and load the compressed data into the end of this buffer, with `addrSrc` = `addrDest` + *(uncompressed length)* - *(compressed length)*. The data will then be decompressed "in place" to fill the buffer completely from start to finish.

The structure of a compressed data block is as follows:

* 2 bytes (`unsigned short`): length of the block, minus these two bytes
* 4 bytes: the last four bytes of the data (uncompressed)
* 2 bytes (`unsigned short`): the number of uncompressed bytes before the compressed data begins (e.g., for metadata; usually 0)
* (some amount of uncompressed data, or nothing)
* (some amount of data compressed using the [ZX0 algorithm](https://github.com/einar-saukas/ZX0), minus the last four bytes given above)

**This function is only available in SymbOS 4.0 and higher.**

*SymbOS name*: `Banking_Decompress` (`BNKCPR`).

## System status

In addition to `symbos.h`, these functions can be found in `symbos/core.h`.

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

In addition to `symbos.h`, these functions can be found in `symbos/device.h`.

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

### Screen_Mode_Set()

```c
void Screen_Mode_Set(char mode, char force, char vwidth);
```

Sets the current screen mode. `mode` is the screen mode (see `Screen_Mode()`). If `force` = 1, the mode will be forced to change. For G9K modes only, `vwidth` specifies the virtual desktop width, one of:

* 0 = no virtual desktop
* 1 = 512 pixels wide
* 2 = 1000 pixels wide

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

### Screen_Redraw()

```c
void Screen_Redraw(void);
```

Reinitializes the desktop background and redraws the entire screen.

*SymbOS name*: `DesktopService_RedrawComplete` (`DSK_SRV_DSKPLT`).

### Color_Get()

```c
unsigned short Color_Get(char color);
```

Retrieves the true palette color of the indexed color `color`.

*Return value*: A 12-bit color, with the components:

* `(value >> 8)` = red component (0 to 15)
* `(value >> 4) & 0x0F` = green component (0 to 15)
* `value & 0x0F` = blue component (0 to 15)

*SymbOS name*: `DesktopService_ColourGet` (`DSK_SRV_COLGET`).

### Color_Set()

```c
void Color_Set(char color, unsigned short value);
```

Sets the true palette color of the indexed color `color` to the 12-bit color `value`, which has the format:

* `(value >> 8)` = red component (0 to 15)
* `(value >> 4) & 0x0F` = green component (0 to 15)
* `value & 0x0F` = blue component (0 to 15)

*SymbOS name*: `DesktopService_ColourSet` (`DSK_SRV_COLSET`).

### Text_Width()

```c
unsigned short Text_Width(unsigned char bank, char* addr, int maxlen);
```

Returns the width (in pixels) of the text string at bank `bank`, address `address` if it were plotted with the current system font. `maxlen` contains the maximum number of characters to measure; if the text is terminated by character 0 or 13 (`\r`), use -1.

*SymbOS name*: `Screen_TextLength` (`TXTLEN`).

### Text_Height()

```c
unsigned short Text_Height(unsigned char bank, char* addr, int maxlen);
```

Returns the height (in pixels) of the text string at bank `bank`, address `address` if it were plotted with the current system font. `maxlen` contains the maximum number of characters to measure; if the text is terminated by character 0 or 13 (`\r`), use -1. (This function effectively just returns the pixel height of the system font, so we can just run it once with a dummy string like "A" and cache the result.)

*SymbOS name*: `Screen_TextLength` (`TXTLEN`).

### Sys_Version()

```c
unsigned short Sys_Version(void);
```

Returns the current SymbOS version, with the major version number in the tens digit and the minor version number in the ones digit (i.e., SymbOS 3.1 = `31`).

*SymbOS name*: `System_Information` (`SYSINF`).

### Sys_Path()

*Currently only available in development builds of SCC.*

```c
char* Sys_Path(void);
```

Returns the system path (e.g., `C:\SYMBOS\`) as a string, which will always end in a backslash (`\`).

*SymbOS name*: `System_Information` (`SYSINF`).

### Sys_GetConfig()

*Currently only available in development builds of SCC.*

```c
void Sys_GetConfig(char* dest, unsigned short offset, unsigned char len);
```

A low-level function that copies `len` bytes starting at `offset` within the SymbOS configuration information into the buffer at address `dest`, which must be in the **transfer** segment. `offset` = 0 is byte 163 in `SYMBOS.INI` (i.e., the system path). For information on offsets and what information can be retrieved this way, see the SymbOS Developer Documentation.

*SymbOS name*: `System_Information` (`SYSINF`).

## Mouse status

In addition to `symbos.h`, these functions can be found in `symbos/device.h`.

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

### Mouse_Dragging()

```c
unsigned char Mouse_Dragging(unsigned char delay);
```

A utility function for determining whether a mouse click is the start of a drag-and-drop operation (or just a normal click). After detecting a click, call `Mouse_Dragging()` with the `delay` parameter indicating a short length of time to wait (in 1/50ths of a second, e.g., 25 = half a second). If the user releases the mouse within this time limit, this function immediately returns 0, indicating a single click. If time expires with the button still down, or the user moves the mouse without releasing the button, it immediately returns a nonzero bitmask (equivalent to [`Mouse_Buttons()`](#mouse_buttons)) indicating which buttons are being held down for a drag-and-drop operation.

*Return value*: 0 for a click, nonzero bitmask for a drag.

## Keyboard status

In addition to `symbos.h`, these functions can be found in `symbos/device.h`.

### Key_Down()

```c
unsigned char Key_Down(unsigned char scancode);
```

Returns 1 if the key specified by `scancode` is currently down, otherwise 0. **Note that keys are tested by *scancode*, not by their ASCII value!** A set of [scancode constants](syscall2.md#keyboard-scancodes) are provided for convenience.

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

**Note that keys are tested by *scancode*, not by their ASCII value!** A set of [scancode constants](syscall2.md#keyboard-scancodes) are provided for convenience.

*SymbOS name*: `Device_KeyMulti` (`KEYMUL`).

## Window management

In addition to `symbos.h`, these functions can be found in `symbos/windows.h`.

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

### Win_Redraw_Ext()

```c
void Win_Redraw_Ext(unsigned char winID, signed char what, unsigned char first);
```

Equivalent to `Win_Redraw()`, but redraws controls whether or not the window has focus. (This is slightly slower than `Win_Redraw()` because SymbOS must check for window overlap.)

*SymbOS name*: `Window_Redraw_ContentExtended_Command` (`MSC_DSK_WINDIN`).

### Win_Redraw_Area()

```c
void Win_Redraw_Area(unsigned char winID, unsigned char what, unsigned char first,
                     unsigned short x, unsigned short y, unsigned short w, unsigned short h);
```

Equivalent to `Win_Redraw()`, but only redraws controls within the box specified by the upper left coordinates `x` and `y`, width `w`, and height `h` (in pixels). This command is particularly useful for redrawing only part of a large graphic area (such as a game playfield), since it is much faster than redrawing the entire area.

Note that these coordinates are relative to the window content, including any scroll. Note that the behavior for resizable windows changed in SymbOS 4.0 to be more consistent with non-resizable windows; in prior versions, the calculation was `x = xpos - form1.xscroll + 8` (i.e., offset by 8 and not including scroll).

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

### Win_Width()

```c
unsigned short Win_Width(Window* win);
```

A utility function that returns the width of the visible content area of the window `win`, in pixels. (Note that the window is passed as the address of the relevant `Window` record, *not* as the window ID!)

When determining the visible size of a resizable window, this function should be used instead of directly reading the `Window.w` record. This is because the `Window.w` record contains the width the window *wants* to be, not necessarily its true current size; for example, when a window is maximized, `Window.w` will be the original "restored" width rather than the true "maximized" width. This function handles all the necessary calculations for determining the true width automatically.

### Win_Height()

```c
unsigned short Win_Height(Window* win);
```

A utility function that returns the height of the visible content area of the window `win`, in pixels. (Note that the window is passed as the address of the relevant `Window` record, *not* as the window ID!)

When determining the visible size of a resizable window, this function should be used instead of directly reading the `Window.h` record. This is because the `Window.h` record contains the height the window *wants* to be, not necessarily its true current size; for example, when a window is maximized, `Window.h` will contain the original "restored" height rather than the true "maximized" height. This function handles all the necessary calculations for determining the true height automatically.

### Win_X()

*Currently only available in development builds of SCC.*

```c
int Win_X(Window* win);
```

A utility function that returns the absolute screen X position of the visible content area of the window `win`, in pixels. (Note that the window is passed as the address of the relevant `Window` record, *not* as the window ID!)

This is mainly useful for translating between absolute and relative screen position when using functions such as [`Select_Pos()`](#select_pos). This function is more reliable than simply reading the `Window.x` record because it accounts for how SymbOS handles maximized windows.

### Win_Y()

*Currently only available in development builds of SCC.*

```c
int Win_Y(Window* win);
```

A utility function that returns the absolute screen Y position of the visible content area of the window `win`, in pixels. (Note that the window is passed as the address of the relevant `Window` record, *not* as the window ID!)

This is mainly useful for translating between absolute and relative screen position when using functions such as [`Select_Pos()`](#select_pos). This function is more reliable than simply reading the `Window.y` record because it accounts for how SymbOS lays out the titlebar, menubar, and toolbar, as well as maximized windows.

### TextBox_Pos()

```c
unsigned char TextBox_Pos(Window* win, Ctrl* textbox);
```

Requests the current cursor position of a multiline textbox control as a column and row (passed as a pointer to its `Ctrl` struct with the parameter `textbox`). For internal reasons, this command will only work if the textbox currently has keyboard focus, and we must also pass a pointer to its host window's `Window` struct as the parameter `win`.

*Return value*: On success, returns 1, and the cursor column and line (starting at 0) will be loaded into the `column` and `line` properties of the textbox's `Ctrl_TextBox` struct. On failure, returns 0. (Yes, this is convoluted. If we just need the cursor position in bytes from the start of the file, we can examine the `cursor` position of the textbox's `Ctrl_TextBox` struct directly.)

*SymbOS name*: `KEYPUT 29`.

### TextBox_Redraw()

```c
void TextBox_Redraw(Window* win, Ctrl* textbox);
```

Tells the specified multi-line textbox (passed as a pointer to its `Ctrl` struct with the parameter `textbox`) that its content has been modified and it should reformat and redraw its text. For internal reasons, this command will only work if the textbox currently has keyboard focus, and we must also pass a pointer to its host window's `Window` struct as the parameter `win`.

*SymbOS name*: `KEYPUT 30`.

### TextBox_Select()

```c
void TextBox_Select(Window* win, Ctrl* textbox, int cursor, int selection);
```

Updates the cursor position and selection of the specified multi-line textbox (passed as a pointer to its `Ctrl` struct with the parameter `textbox`), scrolling and redrawing the textbox as necessary. `cursor` is the new cursor position, in bytes from the start of the complete text; `selection` is the number of characters to select (0 for none, greater than 0 for the cursor to mark the start of the selection, and less than 0 for the cursor to mark the end of the selection). For internal reasons, this command will only work if the textbox currently has keyboard focus, and we must also pass a pointer to its host window's `Window` struct with the parameter `win`.

*SymbOS name*: `KEYPUT 31`.

## Onscreen popups

In addition to `symbos.h`, these functions can be found in `symbos/popups.h`.

### MsgBox()

```c
unsigned char MsgBox(char* line1, char* line2, char* line3, unsigned int pen,
                     unsigned char type, char* icon, void* modalWin);
```

Opens a message box onscreen. `line1`, `line2`, and `line3` are three text strings that will be displayed. `pen` is the text color, one of `COLOR_BLACK`, `COLOR_RED`, `COLOR_ORANGE`, or `COLOR_YELLOW` (usually we want `COLOR_BLACK`). `type` is a OR'd bitmask of one or more of the following options:

* `BUTTON_OK`: display an "OK" button
* `BUTTON_YN`: display "Yes" and "No" buttons
* `BUTTON_YNC`: display "Yes", "No", and "Cancel" buttons
* `TITLE_DEFAULT`: title the box "Error!" if there is no custom icon, otherwise "Info"
* `TITLE_INFO`: title the box "Info"
* `TITLE_WARNING`: title the box "Warning"
* `TITLE_CONFIRM`: title the box "Confirmation"
* `MSGBOX_ICON`: use a custom icon

`icon` is the address of a 24x24 custom icon image, in 4-color SGX format. If `icon` = 0, a default icon will be used.

`modalWin` specifies the address of a `Window` data record that should be declared modal, if any; this window will not be able to be focused until the message box is closed. If `modalWin` = 0, no window will be declared modal.

Note that only pure info messages (BUTTON_OK, not modal) can have multiple instances open on the screen at the same time. SymbOS implements more complex message boxes as a single window shared by all processes; if the message box cannot be opened because it is already in use by another process, this function will return `MSGBOX_FAILED` (0). This function is also NOT thread-safe.

*Return value*: One of:

* `MSGBOX_FAILED` (0): another process is already using the complex message box.
* `MSGBOX_OK`: the user clicked the "OK" button.
* `MSGBOX_YES`: the user cliked the "Yes" button.
* `MSGBOX_NO`: the user clicked the "No" button.
* `MSGBOX_CANCEL`: the user clicked the "Cancel" button or the window Close button.

*SymbOS name*: `Dialogue_Infobox_Command` (`MSC_SYS_SYSWRN`)

### FileBox()

```c
unsigned char FileBox(char* path, char* filter, unsigned char flags, unsigned char attribs,
                      unsigned short entries, unsigned short bufsize, void* modalWin);
```

Opens a file selector box onscreen. `path` can be the absolute path of the directory to start in, the absolute path of a file to preselect, or 0 to start in the default location. If `path` is a directory, it should end with a backslash (`\`). `filter` is a three-byte string containing a file extension filter, which can contain Window-style wildcards (`*` or `?`); for example, `*  ` to show all files or `TXT` to show only `.txt` files. (If shorter than 3 bytes, this string should be padded to 3 bytes with spaces.) 'flags' is an OR'd bitmask consisting of one or more of the following options:

* `FILEBOX_OPEN`: label box "open"
* `FILEBOX_SAVE`: label box "save"
* `FILEBOX_FILE`: select an individual file
* `FILEBOX_DIR`: select a directory

`attribs` is an OR'd bitmask consisting of one or more of the following options:

* `ATTRIB_READONLY`: do not include read-only files
* `ATTRIB_HIDDEN`: do not include hidden files
* `ATTRIB_SYSTEM`: do not include system files
* `ATTRIB_VOLUME`: do not include volume information files (recommended)
* `ATTRIB_DIR`: do not include directories
* `ATTRIB_ARCHIVE`: do not include archived files

`entries` specifies the maximum number of entries (recommended 128) and `buflen` specifies the length of the directory buffer to create (recommended 8000). `modalWin` specifies the address of a `Window` data record that should be declared modal, if any; this window will not be able to be focused until the file selector is closed. If `modalWin` = 0, no window will be declared modal.

Note that SymbOS implements the file selector as a single window shared by all processes; if the file selector cannot be opened because it is already in use by another process, this function will return `FILEBOX_FAILED`.

*Return value*: One of:

* `FILEBOX_OK` (0): the user has selected a file and clicked "OK". The absolute path of the selected file or directory will be stored in the global string `FileBoxPath`.
* `FILEBOX_CANCEL`: the user canceled file selection.
* `FILEBOX_FAILED`: another process is already using the file selector.
* `FILEBOX_NOMEM`: not enough memory available to open the file selector.
* `FILEBOX_NOWIN`: no window ID is available to open the file selector.

*SymbOS name*: `Dialogue_FileSelector_Command` (`MSC_SYS_SELOPN`)

### Menu_Context()

```c
void Menu_Context(unsigned char bank, void* addr, int x, int y);
```

Opens a context menu at the specified `x` and `y` coordinates on the screen (in pixels). The menu data is a `Menu` struct (and associated `Menu_Entry` structs) located at bank `bank`, address `addr` in the **transfer** segment. If `x` = -1, the context menu will be opened at the current mouse position.

This function just opens the menu. Any user interactions with the menu will be passed back as a `MSR_DSK_MENCTX` message, which we should handle in our app's main event loop:

* `msg[1]` = 1 if entry clicked, 0 if menu cancelled
* `*(int*)&msg[2]` = value of clicked entry
* `msg[4]` = 1 if entry had a checkmark, 0 otherwise

*SymbOS name*: `Menu_Context_Command` (`MSC_DSK_MENCTX`).

### Select_Pos()

```c
char Select_Pos(unsigned short* x, unsigned short* y, unsigned short w, unsigned short h);
```

Opens a "rubber band" selector (dotted rectangle) at the specified absolute `*x` and `*y` position on the screen (in pixels), with width `w` and height `h` (in pixels). The user will be able to change the position (but not the size) of this selector by moving the mouse, until they either confirm their selection by releasing the left mouse button or cancel it by pressing ESC. (This is usually used for drag-and-drop operations triggered by first pressing the left mouse button.) The final position of the selector will be written back to the variables passed by reference in `x` and `y`.

Note that absolute screen coordinates are used, not coordinates relative to window content. To translate between the two, see [`Win_X()`](#win_x) and [`Win_Y()`](#win_y).

*Return value*: On successful completion, returns 1. If the user cancelled the operation by pressing ESC, returns 0.

*SymbOS name*: `VirtualControl_Position_Command` (`MSC_DSK_CONPOS`).

### Select_Size()

```c
char Select_Size(unsigned short x, unsigned short y, unsigned short* w, unsigned short* h);
```

Opens a "rubber band" selector (dotted rectangle) at the specified absolute `x` and `y` position on the screen (in pixels), with width `*w` and height `*h` (in pixels). The user will be able to change the size (but not the position) of this selector by moving the mouse, until they either confirm their selection by releasing the left mouse button or cancel it by pressing ESC. (This is usually used for drag-and-drop operations triggered by first pressing the left mouse button.) The final size of the selector will be written back to the variables passed by reference in `w` and `h`.

Note that absolute screen coordinates are used, not coordinates relative to window content. To translate between the two, see [`Win_X()`](#win_x) and [`Win_Y()`](#win_y).

*Return value*: On successful completion, returns 1. If the user cancelled the operation by pressing ESC, returns 0.

*SymbOS name*: `VirtualControl_Size_Command` (`MSC_DSK_CONSIZ`).
