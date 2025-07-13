# Shell functions

For simple shell I/O, we can just use standard C stdio functions (`printf()`, etc.). However, for more direct control (or to avoid the overhead of importing stdio), we can interact directly with SymShell using the functions described below.

SymShell functions will only be available if the application is associated with a running instance of SymShell. To ensure that an application is started in SymShell, make sure that it has the file extension `.com` instead of `.exe`.

In addition to `symbos.h`, these functions can be found in `symbos/shell.h`.

## Contents

* [Interacting with SymShell](#interacting-with-symshell)
* [Shell functions](#shell-functions)
* [Shell control codes](#shell-control-codes)

## Interacting with SymShell

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

Most shell functions will set `_shellerr` to an error code on failure; see [error codes](s_ref.md#error-codes).

### A note on exiting

When an app's host SymShell instance is forcibly closed by clicking the "X" button, SymShell will send message 0 to the app (i.e., `msg[0]` = 0) to tell it to exit. It is important that the app obey this message and exit, or else it will end up "orphaned", running in the background and forever waiting for responses from a SymShell window that no longer exists.

The default behavior of the shell routines below is to watch for message 0 and call `exit(0)` to quit immediately if detected. This is usually what we want and does not require any special consideration of message 0 on our part. However, if the app needs to shut down more gracefully (e.g., by calling [`Mem_Release()`](s_core.md#mem_release) to free allocated memory), we can override this behavior by setting the global variable `_shellexit` = 1 at the beginning of `main()`. When `_shellexit` is nonzero, shell functions will instead respond to message 0 by setting `_shellexit` = 2 and behaving as if there is no available shell (i.e., `_shellerr` = `ERR_NOSHELL`). We can then watch for `_shellexit` = 2 and perform any necessary shutdown manually.

## Shell functions

### Shell_CharIn()

```c
int Shell_CharIn(unsigned char channel);
```

Requests an input character from the specified  `channel`. If this is the console keyboard and there is no character waiting in the keybuffer, SymShell will pause until the user presses a key.

*Return value*: On success, returns the ASCII value of the character (including [extended ASCII codes](s_ref.md#keyboard-ascii-codes) for special keys). If we have hit EOF on an input stream, returns -1. If another error has occurred, returns -2 and sets `_shellerr`.

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

*Return value*: If a key is waiting, returns the ASCII value of the character (including [extended ASCII codes](s_ref.md#keyboard-ascii-codes) for special keys). If no key is waiting, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

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

### Shell_Print()

```c
signed char Shell_Print(char* addr);
```

A convenience function that calls `Shell_StringOut(0, _symbank, addr, strlen(addr))`. This saves some bytes and hassle from the most common use-case for `Shell_StringOut()`.

*Return value*: On success, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

### Shell_Locate()

*Currently only available in development builds of SCC.*

```c
signed char Shell_Locate(unsigned char col, unsigned char row);
```

Set the cursor to column `col`, row `row`. (Internally, this just calls `Shell_Print()` with the appropriate [control codes](#shell-control-codes).) Row and column numbers start at 1.

*Return value*: On success, returns 0. If another error has occurred, returns -2 and sets `_shellerr`.

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

### Shell_CharWatch()

```c
signed char Shell_CharWatch(unsigned char bank, char* addr);
```

Creates a "character watch byte" that we can read directly to determine if there is keyboard input pending in SymShell, without the overhead of repeatedly calling `Shell_CharTest()`. The watch byte will be established at bank `bank`, address `addr`. As soon as a key is pressed, its value will be written to the byte; if the keyboard buffer is empty, 0 will be written. We can then call `Shell_CharIn()` to read the key from the buffer.

This function requires SymShell 2.3 or greater and will fail with `_shellerr` = `ERR_RINGFULL` on earlier versions.

*Return value*: On success, returns 0. On failure, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharWatch_Command` (`MSC_SHL_CHRWTC`).

### Shell_StopWatch()

```c
signed char Shell_StopWatch(unsigned char bank, char* addr);
```

Deactivates the "character watch byte" at bank `bank`, address `addr`, which was previously established using `Shell_CharWatch()`.

This function requires SymShell 2.3 or greater and will fail with `_shellerr` = `ERR_RINGFULL` on earlier versions.

*Return value*: On success, returns 0. On failure, returns -2 and sets `_shellerr`.

*SymbOS name*: `SymShell_CharWatch_Command` (`MSC_SHL_CHRWTC`).

## Shell control codes

SymShell interprets ASCII characters 1-31 as control characters; these are summarized below.

| Sequence    | Meaning                                         |
| ----------- | ----------------------------------------------- |
| `0x02`         | Switch cursor off |
| `0x03`         | Switch cursor on |
| `0x04`         | Save current cursor position |
| `0x05`         | Restore saved cursor position |
| `0x06`         | Reactivate text output (see `0x15`) |
| `0x08`         | Move cursor one character to the right |
| `0x09`         | Move cursor one character to the left |
| `0x0A`         | Move cursor one character down |
| `0x0B`         | Move cursor one character up |
| `0x0C`         | Clear screen and place cursor at 1,1 |
| `0x0D`         | Carriage return (move cursor to start of line) |
| `0x0E` `n`     | Move cursor multiple characters.(1) |
| `0x10`         | Clears the character under the cursor. |
| `0x11`         | Clear line from cursor left. |
| `0x12`         | Clear line from cursor right. |
| `0x13`         | Clear column from cursor up. |
| `0x14`         | Clear column from cursor down. |
| `0x15`         | Deactivate text output (see `0x06`) |
| `0x16`         | Set a tab at the current cursor column |
| `0x17`         | Clear a tab at the current cursor column |
| `0x18`         | Clear all tabs |
| `0x19`         | Jump to next tab |
| `0x1C` `c` `r` | Resize the terminal window to `c` columns by `r` rows.(2) |
| `0x1D` `0x01`  | Scroll window up one line, without affecting cursor |
| `0x1D` `0x02`  | Scroll window down one line, without affecting cursor |
| `0x1E`         | Reset cursor to 1,1 |
| `0x1F` `c` `r` | Move cursor to column `c`, row `r` |

(1) Characters to move is specified as follows:

* `n` = 1 to 80 = move `n` characters right
* `n` = 81 to 160 = move (`n` - 80) characters left
* `n` = 161 to 185 = ``move (`n` - 160) characters down
* `n` = 186 to 210 = move (`n` - 185) characters up.

The cursor will not cross any borders.

(2) Maximum terminal size is 80x25 (or 80x24 on MSX). Depending on platform and context, this command may not do anything.
