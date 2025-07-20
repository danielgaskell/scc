## Desktop features

## Contents

* [Popup dialogs](#popup-dialogs)
* [Context menus](#context-menus)
* [Rubber band select](#rubber-band-select)
* [Clipboard](#clipboard)
* [System tray](#system-tray)

## Popup dialogs

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

`icon` is the address of a 24x24 custom icon image, in 4-color SGX format. If `icon` = 0, a default icon will be used. The variable `_symicon` stores the address of the application's main 4-color icon.

`modalWin` specifies the address of a `Window` data record that should be declared modal, if any; this window will not be able to be focused until the message box is closed. If `modalWin` = 0, no window will be declared modal.

Note that only pure info messages (BUTTON_OK, not modal) can have multiple instances open on the screen at the same time. SymbOS implements more complex message boxes as a single window shared by all processes; if the message box cannot be opened because it is already in use by another process, this function will return `MSGBOX_FAILED` (0). This function is also NOT thread-safe.

*Return value*: One of:

* `MSGBOX_FAILED` (0): another process is already using the complex message box.
* `MSGBOX_OK`: the user clicked the "OK" button.
* `MSGBOX_YES`: the user cliked the "Yes" button.
* `MSGBOX_NO`: the user clicked the "No" button.
* `MSGBOX_CANCEL`: the user clicked the "Cancel" button or the window Close button.

*SymbOS name*: `Dialogue_Infobox_Command` (`MSC_SYS_SYSWRN`)

### InputBox()

*Currently only available in development builds of SCC.*

```c
signed char InputBox(char* title, char* line1, char* line2, char* buffer,
                     unsigned short buflen, void* modalWin);
```

Opens a text input box onscreen. `title` is the window title to display, `line1` and `line2` are two lines of text to display as the prompt, `buffer` is the buffer to store the inputted text, and `buflen` is the length of the buffer (including zero terminator, so the maximum length of the input string is `buflen` - 1). If `buffer` already contains text, it will be used as the default value of the text field.

`modalWin` specifies the address of a `Window` data record that should be declared modal, if any; this window will not be able to be focused until the message box is closed. If `modalWin` = 0, no window will be declared modal.

This function is thread-safe, but only one `InputBox` will be visible at a time (a semaphore is used to ensure other threads wait their turn).

*Return value*: 0 if the user clicked "OK" or -1 if the user clicked "Cancel". In either case, the user's text input will be stored in `buffer`.

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

## Context menus

In addition to `symbos.h`, these functions can be found in `symbos/popups.h`.

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

## Rubber band select

In addition to `symbos.h`, these functions can be found in `symbos/popups.h`.

### Select_Pos()

```c
char Select_Pos(unsigned short* x, unsigned short* y, unsigned short w, unsigned short h);
```

Opens a "rubber band" selector (dotted rectangle) at the specified absolute `*x` and `*y` position on the screen (in pixels), with width `w` and height `h` (in pixels). The user will be able to change the position (but not the size) of this selector by moving the mouse, until they either confirm their selection by releasing the left mouse button or cancel it by pressing ESC. (This is usually used for drag-and-drop operations triggered by first pressing the left mouse button.) The final position of the selector will be written back to the variables passed by reference in `x` and `y`.

Note that absolute screen coordinates are used, not coordinates relative to window content. To translate between the two, see [`Win_X()`](s_window.md#win_x) and [`Win_Y()`](s_window.md#win_y). Note also that the popup will not appear if the initial coordinates are offscreen (this includes `x` = 0, `y` = 0 because the box is drawn *around* the specified coordinates, so this would place the selector at -1, -1).

*Return value*: On successful completion, returns 1. If the user cancelled the operation by pressing ESC, returns 0.

*SymbOS name*: `VirtualControl_Position_Command` (`MSC_DSK_CONPOS`).

### Select_Size()

```c
char Select_Size(unsigned short x, unsigned short y, unsigned short* w, unsigned short* h);
```

Opens a "rubber band" selector (dotted rectangle) at the specified absolute `x` and `y` position on the screen (in pixels), with width `*w` and height `*h` (in pixels). The user will be able to change the size (but not the position) of this selector by moving the mouse, until they either confirm their selection by releasing the left mouse button or cancel it by pressing ESC. (This is usually used for drag-and-drop operations triggered by first pressing the left mouse button.) The final size of the selector will be written back to the variables passed by reference in `w` and `h`.

Note that absolute screen coordinates are used, not coordinates relative to window content. To translate between the two, see [`Win_X()`](s_window.md#win_x) and [`Win_Y()`](s_window.md#win_y). Note also that the popup will not appear if the initial coordinates are offscreen (this includes `x` = 0, `y` = 0 because the box is drawn *around* the specified coordinates, so this would place the selector at -1, -1).

*Return value*: On successful completion, returns 1. If the user cancelled the operation by pressing ESC, returns 0.

*SymbOS name*: `VirtualControl_Size_Command` (`MSC_DSK_CONSIZ`).

## Clipboard

In addition to `symbos.h`, these functions can be found in `symbos/clip.h`.

### Clip_Put()

```c
unsigned char Clip_Put(unsigned char bank, char* addr, unsigned short len,
                       unsigned char type);
```

Stores `len` bytes of data from bank `bank`, address `addr` into the system clipboard. `type` may be one of: 1 = text, 2 = extended graphic, 3 = item list, 4 = desktop icon shortcut.

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Clipboard_Put` (`BUFPUT`).

### Clip_Get()

```c
unsigned short Clip_Get(unsigned char bank, char* addr, unsigned short len,
                        unsigned char type);
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
