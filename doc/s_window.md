# Window routines

In addition to `symbos.h`, these functions can be found in `symbos/windows.h`.

## Contents

* [Window management](#window-management)
* [Window status](#window-status)
* [Clipboard](#clipboard)
* [System tray](#system-tray)

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
void Win_Redraw_Sub(unsigned char winID, unsigned char collection,
                    unsigned char what, unsigned char first);
```

Redraws one or more controls inside the control collection with the ID `collection` on window `winID`. `what` can be either (1) the control ID of a single control to redraw, or (2) a negative number (from -2 to -16) indicating how many controls should be redrawn (from 2 to 16), in which case `first` indicates the control ID of the first control to redraw.

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

When determining the visible size of a resizable window, this function should be used instead of directly reading the `Window.w` record. This is because the `Window.w` record contains the width the window "wants" to be, not necessarily its true current size; for example, when a window is maximized, `Window.w` will be the original "restored" width rather than the true "maximized" width. This function handles all the necessary calculations for determining the true width automatically.

### Win_Height()

```c
unsigned short Win_Height(Window* win);
```

A utility function that returns the height of the visible content area of the window `win`, in pixels. (Note that the window is passed as the address of the relevant `Window` record, *not* as the window ID!)

When determining the visible size of a resizable window, this function should be used instead of directly reading the `Window.h` record. This is because the `Window.h` record contains the height the window "wants" to be, not necessarily its true current size; for example, when a window is maximized, `Window.h` will contain the original "restored" height rather than the true "maximized" height. This function handles all the necessary calculations for determining the true height automatically.

## Window status

### Win_X()

*Currently only available in development builds of SCC.*

```c
int Win_X(Window* win);
```

A utility function that returns the absolute screen X position of the visible content area of the window `win`, in pixels. (Note that the window is passed as the address of the relevant `Window` record, *not* as the window ID!)

This is mainly useful for translating between absolute and relative screen position when using functions such as [`Select_Pos()`](s_desk.md#select_pos). This function is more reliable than simply reading the `Window.x` record because it accounts for how SymbOS handles maximized windows.

### Win_Y()

*Currently only available in development builds of SCC.*

```c
int Win_Y(Window* win);
```

A utility function that returns the absolute screen Y position of the visible content area of the window `win`, in pixels. (Note that the window is passed as the address of the relevant `Window` record, *not* as the window ID!)

This is mainly useful for translating between absolute and relative screen position when using functions such as [`Select_Pos()`](s_desk.md#select_pos). This function is more reliable than simply reading the `Window.y` record because it accounts for how SymbOS lays out the titlebar, menubar, and toolbar, as well as maximized windows.

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
