# Reference tables

## Contents

* [Keyboard scancodes](#keyboard-scancodes)
* [Keyboard ASCII codes](#keyboard-ascii-codes)
* [Colors](#colors)
* [Error codes](#error-codes)

## Keyboard scancodes

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

## Keyboard ASCII codes

In addition to `symbos.h`, these definitions can be found in `symbos/keys.h`.

| Code         | Code        | Code         | Code         |
| ------------ | ----------- | ------------ | ------------ |
| `KEY_CTRL_A` | `KEY_ALT_A` | `KEY_ALT_0`  | `KEY_TAB`    |
| `KEY_CTRL_B` | `KEY_ALT_B` | `KEY_ALT_1`  | `KEY_BACK`   |
| `KEY_CTRL_C` | `KEY_ALT_C` | `KEY_ALT_2`  | `KEY_DEL`    |
| `KEY_CTRL_D` | `KEY_ALT_D` | `KEY_ALT_3`  | `KEY_ENTER`  |
| `KEY_CTRL_E` | `KEY_ALT_E` | `KEY_ALT_4`  | `KEY_ESC`    |
| `KEY_CTRL_F` | `KEY_ALT_F` | `KEY_ALT_5`  | `KEY_INS`    |
| `KEY_CTRL_G` | `KEY_ALT_G` | `KEY_ALT_6`  | `KEY_PRINT`  |
| `KEY_CTRL_H` | `KEY_ALT_H` | `KEY_ALT_7`  | `KEY_HOME`   |
| `KEY_CTRL_I` | `KEY_ALT_I` | `KEY_ALT_8`  | `KEY_END`    |
| `KEY_CTRL_J` | `KEY_ALT_J` | `KEY_ALT_9`  | `KEY_CTRL_1` |
| `KEY_CTRL_K` | `KEY_ALT_K` | `KEY_ALT_AT` | `KEY_CTRL_2` |
| `KEY_CTRL_L` | `KEY_ALT_L` | `KEY_F0`     | `KEY_CTRL_3` |
| `KEY_CTRL_M` | `KEY_ALT_M` | `KEY_F1`     | `KEY_CTRL_4` |
| `KEY_CTRL_N` | `KEY_ALT_N` | `KEY_F2`     | `KEY_CTRL_5` |
| `KEY_CTRL_O` | `KEY_ALT_O` | `KEY_F3`     | `KEY_CTRL_6` |
| `KEY_CTRL_P` | `KEY_ALT_P` | `KEY_F4`     | `KEY_CTRL_7` |
| `KEY_CTRL_Q` | `KEY_ALT_Q` | `KEY_F5`     | `KEY_CTRL_8` |
| `KEY_CTRL_R` | `KEY_ALT_R` | `KEY_F6`     |              |
| `KEY_CTRL_S` | `KEY_ALT_S` | `KEY_F7`     |              |
| `KEY_CTRL_T` | `KEY_ALT_T` | `KEY_F8`     |              |
| `KEY_CTRL_U` | `KEY_ALT_U` | `KEY_F9`     |              |
| `KEY_CTRL_V` | `KEY_ALT_V` | `KEY_FDOT`   |              |
| `KEY_CTRL_W` | `KEY_ALT_W` | `KEY_UP`     |              |
| `KEY_CTRL_X` | `KEY_ALT_X` | `KEY_DOWN`   |              |
| `KEY_CTRL_Y` | `KEY_ALT_Y` | `KEY_LEFT`   |              |
| `KEY_CTRL_Z` | `KEY_ALT_Z` | `KEY_RIGHT`  |              |

## Colors

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

## Error codes

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

The following errors are primarily issued by the sound interface (stored in `_sounderr`):

* `ERR_NOSOUND`: No sound daemon
* `ERR_TOOMANY`: Too many effects collections already loaded

For network error codes, see [Network Library](network.md).
