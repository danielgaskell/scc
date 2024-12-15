# System call reference (con't)

## Contents

* [System variables](syscall1.md#system-variables)
* [Messaging](syscall1.md#messaging)
* [Memory management](syscall1.md#memory-management)
* [Memory read/write](syscall1.md#memory-read-write)
* [System status](syscall1.md#system-status)
* [Screen status](syscall1.md#screen-status)
* [Mouse status](syscall1.md#mouse-status)
* [Keyboard status](syscall1.md#keyboard-status)
* [Window management](syscall1.md#window-management)
* [Onscreen popups](syscall1.md#onscreen-popups)
* [File access](#file-access)
* [Directory access](#directory-access)
* [Shell functions](#shell-functions)
* [Process management](#process-management)
* [Timer management](#timer-management)
* [Clipboard functions](#clipboard-functions)
* [Time functions](#time-functions)
* [System tray](#system-tray)
* [Network interface](#network-interface)
* [Multithreading](#multithreading)
* [Reference tables](#reference-tables)

## File access

File functions in SymbOS work using numerical file handles. Due to system limits only 8 files can be open at one time, so it is easy to run out of file handles if we are not careful. **Be careful to close any files you open before exiting!** Files opened with `stdio.h` functions (`fopen()`, etc.) will be closed automatically on exit, but files opened with direct system calls will not.

SymbOS does not make a distinction between opening a file for reading or writing.

**All file paths given to system calls must be absolute**, e.g., `A:\DIR\FILE.TXT`, not `FILE.TXT`. For a convenient way to convert relative paths into absolute paths, see [Dir_PathAdd()](#dir-pathadd).

A quirk to know: Due to a limitation of the AMSDOS filesystem used by, e.g., CPC floppy disks, files on this filesystem only report their length to the nearest 128-byte block. The convention is to terminate the actual file with the AMSDOS EOF character (0x1A) and then pad out the file to the nearest 128-byte boundary with garbage.

As usual, the `stdio.h` implementation includes some logic to paper over this issue when using functions such as `fgets()`, but when accessing files directly with system calls we will need to keep this in mind. Files stored on FAT filesystems (i.e., most mass storage devices) do not have this limitation, although any files copied from an AMSDOS filesystem to a FAT filesystem may retain their garbage padding at the end.

Most file operations will set `_fileerr` to an error code on failure; see [error codes](#error-codes).

In addition to `symbos.h`, these functions can be found in `symbos/file.h`.

### File_Open()

```c
unsigned char File_Open(unsigned char bank, char* path);
```

Opens the file at the absolute path stored in bank `bank`, address `path` and returns the system file handle. The seek position will be set to the start of the file.

*Return value*: On success, returns the file handle (0 to 7). On failure, sets and returns `_fileerr` (which will always be greater than 7).

*SymbOS name*: `File_Open` (`FILOPN`).

### File_New()

```c
unsigned char File_New(unsigned char bank, char* path, unsigned char attrib);
```

Creates a new file at the absolute path stored in bank `bank`, address `path` and returns the system file handle. If the file already existed, it will be emptied. The seek position will be set to the start of the file.

`attrib` is an OR'd list of any or none of the following: `ATTRIB_READONLY`, `ATTRIB_HIDDEN`, `ATTRIB_SYSTEM`, `ATTRIB_ARCHIVE`.

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

### File_ReadComp()

```c
unsigned short File_ReadComp(unsigned char id, unsigned char bank, 
                             char* addr, unsigned short len);
```

Reads a compressed data block from the open file handle `id` into a buffer at bank `bank`, address `addr` and decompresses it in-place using [`Bank_Decompress()`](syscall1.md#bank_decompress). `len` must contain the total resulting length of the *decompressed* data, in bytes. The seek position will be moved past the end of the block, so the next call to `File_ReadComp()` will read the next part of the file.

**This function is only available in SymbOS 4.0 and higher.** For details on the structure of a compressed data block, see the documentation for [`Bank_Decompress()`](syscall1.md#bank_decompress).

*Return value*: On success, returns the number of bytes read. On failure, sets `_fileerr` and returns 0.

*SymbOS name*: `File_Compressed` (`FILCPR`).

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

In addition to `symbos.h`, these functions can be found in `symbos/file.h`.

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
    unsigned long time;   // system timestamp - decode with Time2Obj()
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

### Dir_GetTime()

```c
unsigned long Dir_GetTime(unsigned char bank, char* path, unsigned char which);
```

Retrieves the system timestamp of the file at the absolute path stored in bank `bank`, address `path`. The option `which` can be `TIME_MODIFIED` or `TIME_CREATED`. Timestamps can be read with the utility function [`Time2Obj()`](#time2obj).

*Return value*: On success, returns the timestamp. On failure, sets `_fileerr` and returns 0.

*SymbOS name*: `Directory_Property_Get` (`DIRPRR`).

### Dir_SetTime()

```c
unsigned char Dir_SetTime(unsigned char bank, char* path, unsigned char which,
                          unsigned long timestamp);
```

Sets the system timestamp of the file at the absolute path stored in bank `bank`, address `path`. The option `which` can be `TIME_MODIFIED` or `TIME_CREATED`. Timestamps can be created with the utility function [`Obj2Time()`](#obj2time).

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

Most shell functions will set `_shellerr` to an error code on failure; see [error codes](#error-codes).

In addition to `symbos.h`, these functions can be found in `symbos/shell.h`.

### A note on exiting

When an app's host SymShell instance is forcibly closed by clicking the "X" button, SymShell will send message 0 to the app (i.e., `msg[0]` = 0) to tell it to exit. It is important that the app obey this message and exit, or else it will end up "orphaned", running in the background and forever waiting for responses from a SymShell window that no longer exists.

The default behavior of the shell routines below is to watch for message 0 and call `exit(0)` to quit immediately if detected. This is usually what we want and does not require any special consideration of message 0 on our part. However, if the app needs to shut down more gracefully (e.g., by calling [`Mem_Release()`](syscall1.md#mem_release) to free allocated memory), we can override this behavior by setting the global variable `_shellexit` = 1 at the beginning of `main()`. When `_shellexit` is nonzero, shell functions will instead respond to message 0 by setting `_shellexit` = 2 and behaving as if there is no available shell (i.e., `_shellerr` = `ERR_NOSHELL`). We can then watch for `_shellexit` = 2 and perform any necessary shutdown manually.

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

### Shell_Print()

```c
signed char Shell_Print(char* addr);
```

An SCC convenience function; calls `Shell_StringOut(0, _symbank, addr, strlen(addr))`. This saves some bytes and hassle from the most common use-case for `Shell_StringOut()`.

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

## Process management

SymbOS makes a distinction between an *application ID*, which is associated with a single application, and a *process ID*, which is associated with a single process. An application may potentially open multiple processes, although most do not. *Pay close attention to which is required by a given function!*

In addition to `symbos.h`, these functions can be found in `symbos/proc.h`.

### App_Run()

```c
unsigned short App_Run(char bank, char* path, char suppress);
```

Runs the app whose absolute path is at bank `bank`, address `path`. If `suppress` = 1, any errors that occur during loading will be supressed; otherwise, they will appear in a message box. If the path points to a non-executable file of a known type (such as `.txt`), it will be opened with its associated application, exactly like double-clicking it in SymCommander.

*Return value*: On success, returns the application ID as the low byte and the process ID of the application's main process as the high byte. These can be extracted with:

```c
appid = result & 0xFF;
procid = result >> 8;
```

The process ID will always be greater than 3, so if the return value is greater than 3, the app was opened successfully.

On failure, returns an error code:

* `APPERR_NOFILE`: the file does not exist.
* `APPERR_UNKNOWN`: the file is not executable or of a known type.
* `APPERR_LOAD`: file system error (sets `_fileerr` with error code).
* `APPERR_NOMEM`: out of memory.

*SymbOS name*: `Program_Run_Command` (`MSC_SYS_PRGRUN`).

### App_End()

```c
void App_End(char appID);
```

Forcibly closes the app with the application ID `appID`. SymbOS will close any processes, windows, and other system resources registered to the app, but since it is possible for apps to reserve some resources (particularly file handles and banked memory) without explicitly registering them to the app, memory leaks may occur. (Treat this command like force-killing an application in the Task Manager: safe if we control the app and know exactly what it is doing, but a last-resort otherwise.)

*SymbOS name*: `Program_End_Command` (`MSC_SYS_PRGEND`).

### App_Search()

```c
unsigned short App_Search(char bank, char* idstring);
```

Searches for the running application with the first 12 bytes of the internal application name matching the string at bank `bank`, address `idstring`. (This is the application name listed in the Task Manager and which can be set with the `-N` command-line option to `cc`.) This function is mainly used for determining the process IDs of shared services such as network drivers so we can send messages to them.

*Return value*: If a matching app has been found, returns the application ID as the low byte and the process ID of the application's main process as the high byte. These can be extracted with:

```c
appid = result & 0xFF;
procid = result >> 8;
```

If no matching app can be found, returns 0.

*SymbOS name*: `Program_SharedService_Command` (`MSC_SYS_PRGSRV`).

### App_Service()

```c
unsigned short App_Service(char bank, char* idstring);
```

Similar to `App_Search()`, but used to connect with system shared services. The behavior is identical to `App_Search`, with two differences:

1) If the specified app is not currently running, SymbOS will attempt to start it from the system path. The `idstring` must have the exact format `%NNNNNNNN.EE` (`%`, eight ASCII characters for the filename, `.`, and two ASCII characters for the first part of the extension). The last character will be filled in automatically based on the current system type:

* `C` for Amstrad CPC
* `P` for Amstrad PCW
* `M` for MSX

For example, if the system path were `C:\SYMBOS` and the system were an Amstrad CPC, `App_Service(_symbank, "%GAMEDRVR.EX")` would try to load `C:\SYMBOS\GAMEDRVR.EXC`.

2) After the app is successfully located or started, an internal counter will be incremented that indicates how many apps are using the service. This allows SymbOS to automatically decide whether the service is still being used and should remain open. (We can unregister with the service with `App_Release()`.)

*Return value*: If the application could be found or was successfully loaded, returns the application ID as the low byte and the process ID of the application's main process as the high byte. These can be extracted with:

```c
appid = result & 0xFF;
procid = result >> 8;
```

The process ID will always be greater than 3, so if the return value is greater than 3, the app was found or opened successfully.

If the application could not be found or was not successfully loaded, returns an error code:

* `APPERR_NOFILE`: the file does not exist.
* `APPERR_UNKNOWN`: the file is not executable or of a known type.
* `APPERR_LOAD`: file system error (sets `_fileerr` with error code).
* `APPERR_NOMEM`: out of memory.

*SymbOS name*: `Program_SharedService_Command` (`MSC_SYS_PRGSRV`).

### App_Release()

```c
void App_Release(char appID);
```

Decrements the service counter incremented by `App_Service()`, so SymbOS knows when the shared service is no longer being used and can be closed. The parameter `appID` specifies the application ID of the service app to release.

*SymbOS name*: `Program_SharedService_Command` (`MSC_SYS_PRGSRV`).

### Proc_Add()

```c
signed char Proc_Add(unsigned char bank, void* header, unsigned char priority);
```

Launches a new process based on the information given in bank `bank`, address `header`. The process will be started with the priority `priority`, from 1 (highest) to 7 (lowest). The standard priority for application is 4.

Usually if we just want to run an executable file from disk, we should use [`App_Run()`](#app-run), not `Proc_Add()`. This function is for a lower-level operation, starting a new process running code we have already defined in memory. This may be code we have loaded from a file, or even part of our application's own main code; see [`thread_start()`](#multithreading) for a wrapper function that uses this to implement multithreading.

`header` must point to a data structure in the **transfer** segment with the struct type `ProcHeader`:

```c
typedef struct {
	unsigned short ix;  // initial value of IX register pair
	unsigned short iy;  // initial value of IY register pair
	unsigned short hl;  // initial value of HL register pair
	unsigned short de;  // initial value of DE register pair
	unsigned short bc;  // initial value of BC register pair
	unsigned short af;  // initial value of AF register pair
	void* startAddr;    // address of routine to run
	unsigned char pid;  // process ID, set by kernel
} ProcHeader;
```

In addition, when the process is launched, its internal stack will begin at the address immediately before this header and grow downwards. So, we must define space for the stack immediately before the header in the **transfer** segment. `startAddr` can be an absolute address, or (as in the example below) the address of a `void` function in our own main code to run as a separate thread. Due to a quirk in SCC's linker, which currently treats initialized and uninitialized arrays differently, this buffer must be given an initial value (such as `{0}`) to ensure that it is placed directly before the `ProcHeader` data structure in the **transfer** segment. (This may be improved in future releases.) For example:

```c
char subprocID;

void proccode(void) {
	/* ... process code, do something here ... */
	Proc_Delete(subprocID); // end the subprocess (rather than returning)
}

_transfer char procstack[256] = {0};
_transfer ProcHeader prochead = {0, 0, 0, 0, 0, 0, // initial register values
                                 proccode};        // address of routine to run
								 
int main(int argc, char* argv[]) {
	subprocID = Proc_Add(_symbank, &prochead, 4);
	/* ... */
}
```

(See [`Multithreading`](#multithreading) for a simpler way to implement threads within our own code, as well as some important discussion about behaviors to avoid when doing this.)

*Return value*: On success, returns the process ID of the newly launched process. On failure, returns -1.

*SymbOS name*: `Multitasking_Add_Process_Command` (`MSC_KRL_MTADDP`).

### Proc_Delete()

```c
void Proc_Delete(unsigned char pid);
```

Forcibly stops execution of the process with the process ID `pid`. (This is most useful for processes we have launched ourselves with `Proc_Add()`; for stopping an entire application and freeing its resources, see [`App_End()`](#app-end).)

*SymbOS name*: `Multitasking_Delete_Process_Command` (`MSC_KRL_MTDELP`).

### Proc_Sleep()

```c
void Proc_Sleep(unsigned char pid);
```

Forcibly puts the process with the process ID `pid` into "sleep" mode. It will not continue execution until it receives a message or is woken up by another system function (such as `Proc_Wake()`).

*SymbOS name*: `Multitasking_Sleep_Process_Command` (`MSC_KRL_MTSLPP`).

### Proc_Wake()

```c
void Proc_Wake(unsigned char pid);
```

Wakes up the process with the process ID `pid` from "sleep" mode.

*SymbOS name*: `Multitasking_Sleep_Process_Command` (`MSC_KRL_MTSLPP`).

### Proc_Priority()

```c
void Proc_Priority(unsigned char pid, unsigned char priority);
```

Sets the scheduler priority of the process with the process ID `pid` to `priority`, which may be from 1 (highest) to 7 (lowest). The default priority is usually 4. A process is allowed to change its own priority.

*SymbOS name*: `Multitasking_Sleep_Process_Command` (`MSC_KRL_MTSLPP`).

## Timer management

SymbOS implements two types of clock events that fire at regular intervals: **counters** and **timers**. These can be used in cases where we need something to occur at regular intervals of real time (such as movement in a game), rather than at regular intervals of CPU time.

A **counter** is the simplest method. A byte in memory can be registered as a counter, after which it will be incremented automatically from 1 to 50 times per second. We can then check the value of this byte regularly, and if it has changed, perform whatever action needs to be done.

A **timer** is more complicated. It is essentially a new process that is only given CPU time for brief, regular intervals every 1/50th or 1/60th of a second (depending on the screen vsync frequency of the platform). See `Timer_Add()` and `Proc_Add()` for details on how to define this process.

In addition to `symbos.h`, these functions can be found in `symbos/proc.h`.

### Counter_Add()

```c
unsigned char Counter_Add(unsigned char bank, char* addr, unsigned char pid, unsigned char speed);
```

Registers a new **counter** byte at bank `bank`, address `addr`, incrementing every `speed`/50 seconds. (For example, to increment the counter twice per second, set `speed` = 25, because 25/50 = 0.5 seconds) The process ID `pid` is the process to register this counter with (usually our own, `_sympid`).

*Return value*: On success, returns 0. On failure, returns 1.

*SymbOS name*: `Timer_Add_Counter_Command` (`MSC_KRL_TMADDT`).

### Counter_Delete()

```c
void Counter_Delete(unsigned char bank, char* addr);
```

Unregisters the **counter** byte at bank `bank`, address `addr` so it no longer increments automatically.

*SymbOS name*: `Timer_Delete_Counter_Command` (`MSC_KRL_TMDELT`).

### Counter_Clear()

```c
void Counter_Clear(unsigned char pid);
```

Unregisters all **counter** bytes associated with the process ID *pid*.

*SymbOS name*: `Timer_Delete_AllProcessCounters_Command` (`MSC_KRL_TMDELP`).

### Timer_Add()

```c
signed char Timer_Add(unsigned char bank, void* header);
```

Behaves identically to `Proc_Add()`, but launches the new process as a **timer**. (See [`Proc_Add()`](#proc-add) for details on how a new process is implemented.) The timer code should ideally be short to ensure that it can fully complete in the allotted time, even under higher CPU load. Typically, we want to implement the timer code as a short loop that ends by calling `Idle()`; the timer process will then execute the loop contents, finish, go to sleep, and be woken up 1/50th of a second later for another pass through the loop:

```c
void timer_loop(void) {
	while (1) {
		/* ... do something here ... */
		Idle();
	}
}
```

*Return value*: On success, returns the timer ID needed to stop the timer later with `Timer_Delete()`. On failure, returns -1.

*SymbOS name*: `Multitasking_Add_Timer_Command` (`MSC_KRL_MTADDT`).

### Timer_Delete()

```c
void Timer_Delete(unsigned char id);
```

Stops execution of a timer previously launched by `Timer_Add()`. The parameter `id` is the timer ID returned by `Timer_Add()`.

*SymbOS name*: `Multitasking_Delete_Timer_Command` (`MSC_KRL_MTDELT`).

## Clipboard functions

In addition to `symbos.h`, these functions can be found in `symbos/clip.h`.

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

## Time functions

These functions may only behave as expected on systems that have a realtime clock (RTC).

In addition to `symbos.h`, these functions can be found in `symbos/time.h`.

### Time_Get()

```c
void Time_Get(SymTime* addr);
```

Loads the current time into the `SymTime` struct at the address `addr`, which has the format:

```c
typedef struct {
    unsigned char second; // 0 to 59
    unsigned char minute; // 0 to 59
    unsigned char hour;   // 0 to 23
    unsigned char day;    // 1 to 31
    unsigned char month;  // 1 to 12
    unsigned short year;  // 1900 to 2100
    signed char timezone; // -12 to +13 (UTC)
} SymTime;
```

*SymbOS name*: `Device_TimeGet` (`TIMGET`).

### Time_Set()

```c
void Time_Set(SymTime* addr);
```

Sets the current time to the values in the `SymTime` struct at the address `addr`. (See `Time_Get()` for the format.)

*SymbOS name*: `Device_TimeSet` (`TIMSET`).

### Time2Obj()

```c
void Time2Obj(unsigned long timestamp, SymTime* obj);
```

An SCC utility function that decodes the system timestamp `timestamp` (obtained from, e.g., `Dir_GetTime()`) into the `SymTime` struct at the address `addr`. (See `Time_Get()` for the format.)

### Obj2Time()

```c
unsigned long Obj2Time(SymTime* obj);
```

An SCC utility function that converts the `SymTime` struct at the address `addr` into a system timestamp. (See `Time_Get()` for the format.)

*Return value*: A 32-bit system timestamp.

## System tray

In addition to `symbos.h`, these functions can be found in `symbos/systray.h`.

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

## Network interface

Network capabilities are only available if an appropriate network daemon is running. Use `Net_Init()` to initialize the network and connect to the daemon.

Network errors are recorded in the global variable `_neterr`, documented [below](#error-codes).

In addition to `symbos.h`, these functions can be found in `symbos/network.h`.

### Net_Init()

```c
unsigned char Net_Init(void);
```

Initializes the network interface, if present. This should be called before using any other network functions.

*Return value*: On success, sets `_netpid` to the process ID of the network daemon and returns 0. On failure, returns `ERR_OFFLINE`.

### TCP_OpenClient()

```c
signed char TCP_OpenClient(unsigned long ip, signed short lport, unsigned short rport);
```

Opens a client TCP connection to the IPv4 address `ip` (formatted as a 32-bit number) on local port `lport`, connecting to remote port `rport`. For client connections, `lport` should usually be set to -1 to obtain a dynamic port number.

*Return value*: On success, returns a socket handle to the new connection. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Open` (`TCPOPN`).

### TCP_OpenServer()

```c
signed char TCP_OpenServer(unsigned short lport);
```

Opens a server TCP connection on local port `lport`.

*Return value*: On success, returns a socket handle to the new connection. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Open` (`TCPOPN`).

### TCP_Close()

```c
signed char TCP_Close(unsigned char handle);
```

Closes and releases the TCP connection associated with the socket `handle`, without first sending a disconnect signal. (This is intended for when the remote host has already closed the connection with us; see also `TCP_Disconnect()`.)

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Close` (`TCPCLO`).

### TCP_Status()

```c
signed char TCP_Status(unsigned char handle, NetStat* obj);
```

Returns the status of the TCP connection associated with the socket `handle` and stores the results in the `NetStat` struct `obj`, which has the format:

```c
typedef struct {
    unsigned char status;    // status (see below)
    unsigned long ip;        // remote IP address
    unsigned short rport;    // remote port
	unsigned char datarec;   // 1 = data received, 0 = none
    unsigned short bytesrec; // received bytes waiting in buffer
} NetStat;
```

`status` may be one of `TCP_OPENING`, `TCP_OPENED`, `TCP_CLOSING`, or `TCP_CLOSED`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Status` (`TCPSTA`).

### TCP_Receive()

```c
signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj);
```

Moves data which has been received from the remote host associated with socket `handle` to the memory at bank `bank`, address `addr`. Up to `len` bytes will be moved (or the actual amount in the buffer, whichever is less).

`obj` is an optional pointer to a `TCP_Trans` struct, into which additional information about the transfer will be loaded. This parameter may be set to NULL to omit this information. The structure of the struct is:

```c
typedef struct {
    unsigned short transferred;  // bytes transferred to destination
    unsigned short remaining;    // bytes remaining in the buffer
} TCP_Trans;
```

A subtlety: Note that setting `len` to the total number of available bytes and calling `TCP_Receive()` is not guaranteed to leave the buffer empty, because the network daemon can receive additional bytes at any time. The main situation where this matters is when the app wants to empty the buffer and wait for a message from the network daemon alerting it when new data arrives. However, the network daemon will only send such a message when adding data *to an empty buffer*, so if the initial `TCP_Receive()` call does not actually empty the buffer completely, no message will arrive. One way to avoid this is to check the returned `TCP_Trans.remaining` value and keep calling `TCP_Receive()` until this value is actually zero; alternatively, the app can not rely on receiving a message.

*Return value*: On success, returns 0 and loads information into `obj`, if specified. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Receive` (`TCPRCV`).

### TCP_Send()

```c
signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj);
```

Sends data from memory bank `bank`, address `addr`, to the host associated with the socket `handle`. Up to `len` bytes will be sent, although note that not all the requested data may be sent at once; if some remains (check `obj.remaining`), the application should idle briefly and then try to send the remainder.

`obj` is an optional pointer to a `TCP_Trans` struct, into which additional information about the transfer will be loaded. This parameter may be set to NULL to omit this information. The structure of the struct is:

```c
typedef struct {
    unsigned short transferred;  // bytes transferred to destination
    unsigned short remaining;    // bytes remaining in the buffer
} TCP_Trans;
```

*Return value*: On success, returns 0 and loads information into `obj`, if specified. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Send` (`TCPSND`).

### TCP_Skip()

```c
signed char TCP_Skip(unsigned char handle, unsigned short len);
```

Skips and throws away `len` bytes of data which have already been received from the host associated with the socket `handle`. `len` must be equal to or smaller than the total number of bytes in the buffer (see `TCP_Status()`).

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Skip` (`TCPSKP`).

### TCP_Flush()

```c
signed char TCP_Flush(unsigned char handle);
```

Flushes the send buffer immediately. Some network hardware or software may hold data in the send buffer for a brief period of time before sending; this command requests to send it immediately.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Flush` (`TCPFLS`).

### TCP_Disconnect()

```c
signed char TCP_Disconnect(unsigned char handle);
```

Sends a disconnect signal to the remote host associated with the socket `handle`, closes the TCP connection, and releases the socket. (This is intended for we want to initiate the disconnection with the remote host; see also `TCP_Close()`.)

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Disconnect` (`TCPDIS`).

### UDP_Open()

```c
signed char UDP_Open(unsigned char type, unsigned short lport, unsigned char bank);
```

Opens a UDP session on local port `lport`. Data for this session will be stored in RAM bank `bank`

*Return value*: On success, returns a socket handle to the new session. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Open` (`UDPOPN`).

### UDP_Close()

```c
signed char UDP_Close(unsigned char handle);
```

Closes and releases the UDP session associated with the socket `handle`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Close` (`UDPCLO`).

### UDP_Status()

```c
signed char UDP_Status(unsigned char handle, NetStat* obj);
```

Returns the status of the UDP session associated with the socket `handle` and stores the results in the `NetStat` struct `obj`, which has the same format as for `TCP_Status()`:

```c
typedef struct {
    unsigned char status;    // status
    unsigned long ip;        // remote IP address
    unsigned short rport;    // remote port
	unsigned char datarec;   // n/a
    unsigned short bytesrec; // received bytes waiting in buffer
} NetStat;
```

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Status` (`UDPSTA`).

### UDP_Receive()

```c
signed char UDP_Receive(unsigned char handle, char* addr);
```

Moves data which has been received from the remote host associated with socket `handle` to the memory at address `addr` (in the bank specified to `UDP_Open()`). The entire packet will be transferred at once, so be sure that there is enough space for an entire packet at the destination address. UDP packets have a theoretical limit of 65507 bytes, but we can also check how much data is waiting with `UDP_Status()`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Receive` (`UDPRCV`).

### UDP_Send()

```c
signed char UDP_Send(unsigned char handle, char* addr, unsigned short len, unsigned long ip, unsigned short rport)
```

Sends a data packet from the memory address `addr` (in the bank specified to `UDP_Open()`) to the host associated with the socket `handle`. If sending fails becaus the buffer is full, the application should idle briefly and try again.

`obj` is an optional pointer to a `TCP_Trans` struct, into which additional information about the transfer will be loaded. This parameter may be set to NULL to omit this information. The structure of the struct is:

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Send` (`UDPSND`).

### UDP_Skip()

```c
signed char UDP_Skip(unsigned char handle);
```

Skips and throws away a complete packet which has already been received from the host associated with the socket `handle`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Skip` (`UDPSKP`).

### DNS_Resolve()

```c
unsigned long DNS_Resolve(unsigned char bank, char* addr);
```

Performs a DNS lookup and attempts to resolve the host IP/URL stored in the string at bank `bank`, address `addr` into an IPv4 address.

*Return value*: On success, returns the IPv4 address (as a 32-bit integer). On failure, sets `_neterr` and returns 0.

*SymbOS name*: `DNS_Resolve` (`DNSRSV`).

### DNS_Verify()

```c
unsigned char DNS_Verify(unsigned char bank, char* addr);
```

Verifies whether the IP/URL stored in the string at bank `bank`, address `addr` is a valid IP or domain address. This function does not interact with the network hardware, so can be used to quickly determine whether an address is valid before initiating a full network request.

*Return value*: On success, returns `DNS_IP` for a valid IP address, `DNS_DOMAIN` for a valid domain address, or `DNS_INVALID` for an invalid address. On failure, sets `_neterr` and returns `DNS_INVALID`.

*SymbOS name*: `DNS_Verify` (`DNSVFY`).

## Multithreading

Yes, **SCC supports multithreading** (!), thanks to SymbOS's elegant system for spawning subprocesses. Internally, threads are implemented as subprocesses of the main application (see [`Proc_Add()`](#proc_add) and [`Proc_Delete()`](#proc_delete), but the convenience functions `thread_start()` and `thread_quit()` are provided to reduce the amount of setup required.

**Warning**: When multithreading, we have to worry about all the problems that come with multithreading---race conditions, deadlocks, concurrent access, reentrancy, etc. The current implementation of libc is also not generally designed with thread-safety in mind, so while most small utility functions (`memcpy()`, `strcat()`, etc.) are thread-safe, others are not---in particular, much of `stdio.h`. Any function that relies on temporarily storing data in a static buffer (rather than local variables) is not thread-safe and may misbehave if two threads call it at the same time. When in doubt, check the library source code to verify that a function does not rely on any static/global variables, or write your own reentrant substitute (e.g., using only local variables, or with a semaphore system that sets a global variable when the shared resource is being used and, if it is already set, loops until it is unset by whatever other thread is using the resource). Any use of 32-bit data types (**long**, **float**, **double**) is also currently not thread-safe, as these internally rely on static "extended" registers, meaning that only one thread at a time can safely use 32-bit data types.

Standard SymbOS system calls that do not use 32-bit data types (`File_Open()`, etc.) should all be thread-safe, as these use a semaphore system to ensure that only one message is passed in `_symmsg` at the same time.

In addition to `symbos.h`, these functions can be found in `symbos/threads.h`.

### thread_start()

```c
signed char thread_start(void* routine, char* env, unsigned short envlen);
```

Spawns a new thread by launching `routine`, which should be a `void` function in our code that takes no parameters. Stack and other information for the new thread will be stored in the environment buffer `env`, which must be in the **transfer** segment. The length of this buffer is passed as `envlen` and should generally be at least 256 bytes (more if the thread will make heavy use of the stack, e.g., with deep recursion or large local buffers).

Example:

```c
_transfer char env1[256];

void threadmain(void) {
	/* ...thread code goes here, which may call other functions... */
	/* ... */
	thread_quit(env1); // quit thread (see below)
}

int main(int argc, char* argv[]) {
	thread_start(threadmain, env1, sizeof(env1));
	/* ...main thread continues here... */
}
```

*Return value*: On success, returns the process ID of the new thread, which will also be stored as the last byte in `env`. On failure, returns -1.

### thread_quit()

```c
void thread_quit(char* env);
```

Quits the running thread associated with the environment buffer `env`. (This function will not return, so it should only be used inside the thread in question, to quit itself. To forcibly end a running thread from inside a different thread, use [`Proc_Delete()`](#proc_delete).)

## Reference tables

### Keyboard scancodes

In addition to `symbos.h`, these definitions can be found in `symbos/keys.h`.

| Code     | Code     | Code     | Code      | Code          | 
| -------- | -------- | -------- | --------- | ------------- |
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

In addition to `symbos.h`, these definitions can be found in `symbos/keys.h`.

| Code        | Code         | Code        | Code        |
| ----------- | ------------ | ----------- | ----------- |
| `KEY_UP`    | `KEY_TAB`    | `KEY_ALT_M` | `KEY_ALT_0` |
| `KEY_DOWN`  | `KEY_ALT_AT` | `KEY_ALT_N` | `KEY_ALT_1` |
| `KEY_LEFT`  | `KEY_ALT_A`  | `KEY_ALT_O` | `KEY_ALT_2` |
| `KEY_RIGHT` | `KEY_ALT_B`  | `KEY_ALT_P` | `KEY_ALT_3` |
| `KEY_F0`    | `KEY_ALT_C`  | `KEY_ALT_Q` | `KEY_ALT_4` |
| `KEY_F1`    | `KEY_ALT_D`  | `KEY_ALT_R` | `KEY_ALT_5` |
| `KEY_F2`    | `KEY_ALT_E`  | `KEY_ALT_S` | `KEY_ALT_6` |
| `KEY_F3`    | `KEY_ALT_F`  | `KEY_ALT_T` | `KEY_ALT_7` |
| `KEY_F4`    | `KEY_ALT_G`  | `KEY_ALT_U` | `KEY_ALT_8` |
| `KEY_F5`    | `KEY_ALT_H`  | `KEY_ALT_V` | `KEY_ALT_9` |
| `KEY_F6`    | `KEY_ALT_I`  | `KEY_ALT_W` | `KEY_BACK`  |
| `KEY_F7`    | `KEY_ALT_J`  | `KEY_ALT_X` | `KEY_DEL`   |
| `KEY_F8`    | `KEY_ALT_K`  | `KEY_ALT_Y` | `KEY_ENTER` |
| `KEY_F9`    | `KEY_ALT_L`  | `KEY_ALT_Z` | `KEY_ESC`   |
| `KEY_FDOT`  |              |             |             |

### Colors

In addition to `symbos.h`, these definitions can be found in `symbos/windows.h`. International English synonyms (`COLOUR`, `GREY`) are also available.

| Value | 4-color         | 16-color      |
| ----- | --------------- | ------------- |
|  0 | COLOR_YELLOW | COLOR_YELLOW  |
|  1 | COLOR_BLACK  | COLOR_BLACK   |
|  2 | COLOR_ORANGE | COLOR_ORANGE  |
|  3 | COLOR_RED    | COLOR_RED     |
|  4 |              | COLOR_CYAN    |
|  5 |              | COLOR_DBLUE   |
|  6 |              | COLOR_LBLUE   |
|  7 |              | COLOR_BLUE    |
|  8 |              | COLOR_WHITE   |
|  9 |              | COLOR_GREEN   |
| 10 |              | COLOR_LGREEN  |
| 11 |              | COLOR_MAGENTA |
| 12 |              | COLOR_LYELLOW |
| 13 |              | COLOR_GRAY    |
| 14 |              | COLOR_PINK    |
| 15 |              | COLOR_LRED    |

### Error codes

The following errors are primarily issued by file commands (stored in `_fileerr`):

* `ERR_DEVINIT`: Device not initialized
* `ERR_DAMAGED`: Media is damaged
* `ERR_NOPART`: Partition does not exist
* `ERR_UNPART`: Unsupported media or partition
* `ERR_READ`: Error during sector read/write
* `ERR_SEEK`: Error during seek
* `ERR_ABORT`: Abort during volume access
* `ERR_NOVOL`: Unknown volume
* `ERR_TOOMANY`: No free filehandle
* `ERR_NODEV`: Device does not exist
* `ERR_NOPATH`: Path does not exist
* `ERR_NOFILE`: File does not exist
* `ERR_FORBIDDEN`: Access is forbidden
* `ERR_BADNAME`: Invalid path or filename
* `ERR_NOHANDLE`: Filehandle does not exist
* `ERR_DEVSLOT`: Device slot already occupied
* `ERR_FILEORG`: Error in file organization
* `ERR_BADDEST`: Invalid destination name
* `ERR_EXISTS`: File/path already exists
* `ERR_BADCODE`: Invalid subcommand code
* `ERR_BADATTRIB`: Invalid attribute
* `ERR_DIRFULL`: Directory is full
* `ERR_DISKFULL`: Media is full
* `ERR_PROTECT`: Media is write-protected
* `ERR_NOTREADY`: Device not ready
* `ERR_NOTEMPTY`: Directory is not empty
* `ERR_BADDEV`: Invalid destination device
* `ERR_FILESYS`: Not supported by file system
* `ERR_UNDEV`: Unsupported device
* `ERR_RONLY`: File is read-only
* `ERR_NOCHANNEL`: Device channel unavailable
* `ERR_NOTDIR`: Destination is not a directory
* `ERR_NOTFILE`: Destination is not a file
* `ERR_UNDEFINED`: Undefined error

The following errors are primarily issued by SymShell commands (stored in `_shellerr`):

* `ERR_NOPROC`: Process ID is not registered with SymShell
* `ERR_DEVFULL`: Destination device is full
* `ERR_RINGFULL`: Internal ring buffer is full
* `ERR_MOREPROC`: Too many processes registered with SymShell
* `ERR_NOSHELL`: No SymShell session available (`_shellpid` = 0)

The following errors are primarily issued by the network interface (stored in `_neterr`):

* `ERR_OFFLINE`: Offline/not connected/no network daemon
* `ERR_NOHW`: No hardware setup
* `ERR_NOIP`: No IP configuration
* `ERR_HARDWARE`: Unknown hardware error
* `ERR_WIFI`: WiFi error (SymbiFace 3)
* `ERR_NOSOCKET`: No more free sockets
* `ERR_BADSOCKET`: Socket does not exist
* `ERR_SOCKETTYPE`: Wrong socket type
* `ERR_SOCKETUSED`: Socket is already in use by another process
* `ERR_BADDOMAIN`: Invalid domain string
* `ERR_TIMEOUT`: Connection timeout
* `ERR_RECURSION`: Recursion not supported
* `ERR_TRUNCATED`: Truncated response
* `ERR_TOOLARGE`: Packet too large
* `ERR_CONNECT`: TCP connection not yet established
