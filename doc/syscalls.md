# System call reference

The system calls in this reference are all available after including `symbos.h`:

```c
#include <symbos.h>
```

When cross-compiling on a modern computer, it is usually easiest to just include the entirety of `symbos.h`. However, this is a large file, so when compiling on SymbOS, we can reduce compilation time and memory usage by only including the sub-headers that are actually needed (e.g., `symbos/shell.h` or `symbos/windows.h`). See the subsection introductions for what functions are in which sub-header.

These headers are not 100% comprehensive; SymbOS provides some additional system calls not implemented in `symbos.h`, mainly low-level calls for dealing with storage devices, system configuration, and complicated applications that alter system functionality or execute code in multiple banks (SCC is not well-suited for this). These calls are discussed in the [SymbOS developer documentation](https://symbos.org/download.htm). It is assumed that, if you need these calls, you are probably already doing something complicated enough that a few extra wrapper functions won't be useful.

## Subsections

* [System variables](#system-variables)
* [Kernel routines](s_core.md)
	* [Messaging](s_core.md#messaging)
	* [Memory management](s_core.md#memory-management)
	* [Memory read/write](s_core.md#memory-readwrite)
	* [System status](s_core.md#system-status)
* [Shell routines](s_shell.md)
	* [Interacting with SymShell](s_shell.md#interacting-with-symshell)
	* [Shell functions](s_shell.md#shell-functions)
	* [Shell control codes](s_shell.md#shell-control-codes)
* [Window routines](s_window.md)
	* [Window management](s_window.md#window-management)
	* [Window status](s_window.md#window-status)
	* [Control reference](s_ctrl.md)
	* [Event reference](s_event.md)
* [Desktop features](s_desk.md)
	* [Popup dialogs](s_desk.md#popup-dialogs)
	* [Context menus](s_desk.md#context-menus)
	* [Rubber band select](s_desk.md#rubber-band-select)
	* [System tray](s_desk.md#system-tray)
	* [Clipboard](s_desk.md#clipboard)
* [Filesystem routines](s_file.md)
	* [File access](s_file.md#file-access)
	* [Directory access](s_file.md#directory-access)
* [Multitasking routines](s_task.md)
	* [Processes](s_task.md#processes)
	* [Timers](s_task.md#timers)
	* [Multithreading](s_task.md#multithreading)
* [Device routines](s_device.md)
	* [Screen status](s_device.md#screen-status)
	* [Mouse status](s_device.md#mouse-status)
	* [Keyboard status](s_device.md#keyboard-status)
	* [Time functions](s_device.md#time-functions)
	* [System configuration](s_device.md#system-configuration)
* [Sound routines](s_sound.md)
	* [Creating/getting sounds](s_sound.md#creatinggetting-sounds)
	* [Sound functions](s_sound.md#sound-functions)
* [Printer routines](s_print.md)
	* [Printing via the daemon](s_print.md#printing-via-the-daemon)
	* [Printing via PrintIt](s_print.md#printing-via-printit)
	* [Direct printer functions](s_print.md#direct-printer-functions)
* [Reference tables](s_ref.md)
	* [Keyboard scancodes](#keyboard-scancodes)
	* [Keyboard ASCII codes](#keyboard-ascii-codes)
	* [Colors](#colors)
	* [Error codes](#error-codes)

## Related libraries

* [Graphics library](graphics.md)
* [Network library](network.md)

## System variables

```c
char* _symmsg;
```

A 14-byte buffer for sending messages. This is used internally by most system calls, but can be used for our own purposes when manually sending messages with `Msg_Send()` and similar functions.

Several other global variables and constants contain useful information about the system:

```c
unsigned char _sympid;	     // process ID of the current app
unsigned char _symappid;     // application ID of the current app
unsigned char _symbank;      // main bank number of the current app
unsigned short _symversion;  // SymbOS version, as major/minor decimal (i.e., SymbOS 3.1 = 31)
char* _segcode;              // start address of the code segment + 0x100
char* _segdata;              // start address of the data segment
char* _segtrans;             // start address of the transfer segment
unsigned short _segcodelen;  // length of the code segment
unsigned short _segdatalen;  // length of the data segment
unsigned short _segtranslen; // length of the transfer segment
```

(The other contents of the application header can be accessed directly with the struct `_symheader`, not documented here; see definition in `symbos/header.h`.)
