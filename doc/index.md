
# Introduction

Writing C will rot your brain. Only a true C programmer would look at the following code fragment and think it was an "elegant" solution to anything:

```c
result = (((_MemStruct)membank).ptr->addr & (ver > 4 ? 7 : attr));
*(++result) = 'A';
```
C is a terrible, very bad, no-good language that provides you with an army of ways to shoot yourself in the foot. It is also obviously the best language for many kinds of systems programming where we want to express ideas in a relatively high-level way while retaining the knowledge of what every byte is doing in memory. This---in addition to the vast amount of open-source code already written in C---makes it useful for programming 8-bit systems.

SCC is an ANSI C compiler that produces executables for [SymbOS](https://symbos.org). If you want a nice Visual Basic-style experience with a GUI form editor and event-driven programming, you should probably check out the [Quigs IDE](https://symbos.org/quigs.htm) instead. But if you already know C and want to write SymbOS software, or you want to port existing code to SymbOS, or you just want to do something more low-level and complicated than is possible in Quigs, read on.

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

Writing windowed applications in SCC is not inherently difficult, but requires a thorough knowledge of SymbOS's desktop model. SCC provides the necessary headers and data types to interact with the SymbOS desktop manager, but there are no shortcuts here; writing a windowed application requires careful coding, testing, and regularly cross-referencing both the SCC header files and the [SymbOS developer documentation](https://symbos.org/download.htm) to make sure every byte is where it needs to be. (For a more plug-and-play experience, try the useful---if quirky---[Quigs](https://symbos.org/quigs.htm) language.)

We first need to include the header file `symbos.h`, which defines many of the functions and data structures referenced in the following sections:

```c
#include <symbos.h>
```

### Memory segments

A SymbOS executable is divided into three memory segments. For console apps written with normal libc functionality, it is not generally necessary to think about this, but many SymbOS system functions make important distinctions between memory segments;

1. **Code**: Contains code and globals (unless manually placed into a different segment - see below). Can contain up to 64KB of code and data.
2. **Data**: Stores data that must not cross a 16KB page boundary. This is required by many system functions, particularly those dealing with text or image data, so SCC places string literals in this segment by default. Can contain up to 16KB of data.
3. **Transfer**: Stores data that must be relocated to the upper 16KB of address space so it can be accessed by (mainly) the desktop manager. Most data structures relating to desktop windows and controls should be placed here. Can contain up to 16KB of data.

Globals are placed in the **code** segment by default, but we can manually assign a global to the **data** or **transfer** segments by adding `_data` or `_transfer` to its declaration:

```c
_transfer char imgdata[256];
```

### Windows and controls

### Desktop commands

`symbos.h` provides a number of helper functions for making system calls. The first one we need is `Win_Open()`:

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

(Additional calls are documented below.)

### Handling events

SymbOS apps communicate with the desktop manager via *interprocess messages*. `symbos.h` provides helper functions that handle most of the necessary messages automatically (as with `Win_Open()` above). However, we still need to understand how messaging works internally in order to set up our main loop that receives event notifications from the desktop manager.

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

In each case `rec_pid` is the "inbound" process ID that should receive the response (i.e., our own process ID, which can be found in the global variable `_sympid`). The process ID of the desktop manager is always 2, but to receive messages from any process, we can use -1.

After opening our starting window, we need to go into a loop that:

1. Calls `Msg_Sleep()` to wait for a message;
2. Checks the message type in the first byte, `msg[0]`; and
3. Takes any necessary action.

Message type codes are documented in the SymbOS developer documentation and defined in `symbos.h`. The most immediately useful message is `MSR_DSK_WCLICK`, which is sent for most interactions with a window or form. The return message then has the format:

* `msg[0]`: `MSR_DSK_WCLICK`
* `msg[1]`: Window ID
* `msg[2]`: Action type, one of:
	* `DSK_ACT_CLOSE`: The close button has been clicked, or the user has pressed Alt+F4.
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

An example of a skeleton main loop is below. This opens our previously-defined window `form1` and loops, waiting for an event. In this example, the only event implemented is the most important one: exiting the application when the user tries to close the window!

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

## Other considerations

### The `malloc()` heap

### Building SCC

The current primary build target for SCC is Windows. Install MinGW, ensure that its `bin` folder is in the system path, and then run the `make.bat` batch files found throughout the SCC source tree to compile the relevant parts of the codebase. (This really ought to transition to proper Makefiles, but whatever.)
