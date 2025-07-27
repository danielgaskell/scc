# Filesystem routines

## Contents

* [File access](#file-access)
* [Directory access](#directory-access)

## File access

File functions in SymbOS work using numerical file handles. Due to system limits only 8 files can be open at one time, so it is easy to run out of file handles if we are not careful. **Be careful to close any files you open before exiting!** Files opened with `stdio.h` functions (`fopen()`, etc.) will be closed automatically on exit, but files opened with direct system calls will not.

SymbOS does not make a distinction between opening a file for reading or writing.

**All file paths given to system calls must be absolute**, e.g., `A:\DIR\FILE.TXT`, not `FILE.TXT`. For a convenient way to convert relative paths into absolute paths, see [Dir_PathAdd()](#dir_pathadd).

A quirk to know: Due to a limitation of the AMSDOS filesystem used by, e.g., CPC floppy disks, files on this filesystem only report their length to the nearest 128-byte block. The convention is to terminate the actual file with the AMSDOS EOF character (0x1A) and then pad out the file to the nearest 128-byte boundary with garbage.

As usual, the `stdio.h` implementation includes some logic to paper over this issue when using functions such as `fgets()`, but when accessing files directly with system calls we will need to keep this in mind. Files stored on FAT filesystems (i.e., most mass storage devices) do not have this limitation, although any files copied from an AMSDOS filesystem to a FAT filesystem may retain their garbage padding at the end.

Most file operations will set `_fileerr` to an error code on failure; see [error codes](s_ref.md#error-codes).

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

### File_ReadLine()

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

Reads a compressed data block from the open file handle `id` into a buffer at bank `bank`, address `addr` and decompresses it in-place using [`Bank_Decompress()`](s_core.md#bank_decompress). `len` must contain the total resulting length of the *decompressed* data, in bytes. The seek position will be moved past the end of the block, so the next call to `File_ReadComp()` will read the next part of the file.

**This function is only available in SymbOS 4.0 and higher.** For details on the structure of a compressed data block, see the documentation for [`Bank_Decompress()`](s_core.md#bank_decompress).

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

### File_ErrMsg()

```c
void File_ErrMsg(void* modalWin);
```

Displays a message box with the current error in `_fileerr`, if any. `modalWin` specifies the address of a `Window` data record that should be declared modal, if any; this window will not be able to be focused until the message box is closed. If `modalWin` = 0, no window will be declared modal.

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

Retrieves the system timestamp of the file at the absolute path stored in bank `bank`, address `path`. The option `which` can be `TIME_MODIFIED` or `TIME_CREATED`. Timestamps can be read with the utility function [`Time2Obj()`](s_device.md#time2obj).

*Return value*: On success, returns the timestamp. On failure, sets `_fileerr` and returns 0.

*SymbOS name*: `Directory_Property_Get` (`DIRPRR`).

### Dir_SetTime()

```c
unsigned char Dir_SetTime(unsigned char bank, char* path, unsigned char which,
                          unsigned long timestamp);
```

Sets the system timestamp of the file at the absolute path stored in bank `bank`, address `path`. The option `which` can be `TIME_MODIFIED` or `TIME_CREATED`. Timestamps can be created with the utility function [`Obj2Time()`](s_device.md#obj2time).

*Return value*: On success, returns 0. On failure, sets and returns `_fileerr`.

*SymbOS name*: `Directory_Property_Set` (`DIRPRS`).

### Dir_PathAdd()

```c
char* Dir_PathAdd(char* path, char* addition, char* dest);
```

An SCC convenience function that constructs an absolute file path from a base path (at address `path`) and a relative path addition (at address `addition`), storing the result in address `dest`. This is mainly used to turn relative paths into absolute paths for the file manager functions. (This function is similar to the system function [`Shell_PathAdd()`](s_shell.md#shell_pathadd), but is available even in windowed applications that do not use SymShell.)

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
