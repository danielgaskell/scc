# Control reference

This section contains a reference to all available window controls (buttons, text, etc.). For details on how to use these controls in windows, see [SymbOS Programming](symbos.md).

### C_AREA

Displays a rectangular area filled with the specified color.

*Parameter*: Color, e.g., `COLOR_ORANGE`. The parameter may optionally be OR'd with `AREA_16COLOR` (to enable 16-color mode) and `AREA_XOR` (to enable XOR mode, to be used only with 16-color mode): e.g., `COLOR_WHITE | AREA_16COLOR`.

```c
// example
_transfer Ctrl c_area1 = {1, C_AREA, -1, COLOR_ORANGE, 0, 0, 100, 80};
```

### C_TEXT

Displays a line of text in the default system font.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;           // address of text
    unsigned char color;  // 4-color mode:  (foreground << 2) | background
                          // 16-color mode: (foreground << 4) | background
    unsigned char flags;  // one of: ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT
                          //   OR with TEXT_16COLOR for 16-color mode
} Ctrl_Text;
```

In "fill" mode, the background of the control is first filled in with the specified background color and the text is clipped to the size of the control rather than being allowed to overflow. To activate this mode in 4-color mode, OR `.color` with `TEXT_FILL`: e.g., `(COLOR_BLACK << 2) | COLOR_ORANGE | TEXT_FILL`. To activate this mode in 16-color mode, OR `.flags` with `TEXT_FILL16`: e.g., `ALIGN_LEFT | TEXT_16COLOR | TEXT_FILL16`.

```c
// example
_transfer Ctrl_Text cd_text1 = {"Text", (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT};
_transfer Ctrl c_text1 = {1, C_TEXT, -1, (unsigned short)&cd_text1, 10, 10, 80, 8};
```

### C_TEXT_FONT

Displays a line of text in an alternative font.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;           // address of text
    unsigned char color;  // 4-color mode:  (foreground << 2) | background
                          // 16-color mode: (foreground << 4) | background
    unsigned char flags;  // one of: ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT
                          //   OR with TEXT_16COLOR for 16-color mode
    char* font;           // address of font
} Ctrl_Text_Font;
```

The control height should be equal to the height of the font, and the font data must be stored in the same 16KB segment as the text (usually the **data** segment). A description of the font format can be found in the SymbOS Developer Documentation; fonts can be created using the SymbOS Font Editor application.

```c
// example
_data char fontbuf[1538]; // a font would be loaded into this buffer
_transfer Ctrl_Text_Font cd_text_font1 =
    {"Text", (COLOR_BLACK << 2) | COLOR_ORANGE, ALIGN_LEFT, fontbuf};
_transfer Ctrl c_text_font1 =
    {1, C_TEXT_FONT, -1, (unsigned short)&cd_text_font1, 10, 10, 80, 8};
```

### C_TEXT_CTRL

Displays a line of "rich" text with optional control codes that can change the appearance of the text mid-line. This renders more slowly than `C_TEXT`, but allows for very complex effects.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;              // address of text
    unsigned short maxlen;   // maximum length (in bytes) of the text
    char* font;              // address of starting font (-1 for default)
    unsigned char color;     // (foreground << 4) | background
    unsigned char underline; // 1 = start with underline on
} Ctrl_Text_Ctrl;
```

The following control bytes can be included in the text string:

* 0x00 = end of string
* 0x01 0xNN = change text color, where NN is (foreground << 4) | background
* 0x02 0xAAAA = change font, where AAAA is the address of the font, or -1 for the default font. The font must be in the same 16KB segment as the text (usually the **data** segment).
* 0x03 = switch underlining on
* 0x04 = switch underlining off
* 0x05 0xNN = insert NN pixels of extra space before the next character
* 0x06 0xNN 0xBB 0xAAAA = plot a [graphics canvas](graphics.md) as an inline image, where NN is the Y position relative to the line (0 = 128 pixels up, 128 = in line with text, 255 = 128 pixels down); BB is the bank number of the canvas, or -1 for "same bank as text"; and AAAA is the address of the canvas + 1. (This feature is only available in SymbOS 4.0 and up.)
* 0x08 to 0x0B = skip next (code - 8) * 2 + 1 bytes
* 0x0C to 0x1F = insert (code - 8) pixels of extra space before the next character

```c
// example
_transfer Ctrl_Text_Ctrl cd_text_ctrl1 =
    {"Text \x03underlined\x04 and not", 100, -1, (COLOR_BLACK << 2) | COLOR_ORANGE, 0};
_transfer Ctrl c_text_ctrl1 =
    {1, C_TEXT_CTRL, -1, (unsigned short)&cd_text_ctrl1, 10, 10, 80, 8};
```

### C_FRAME

Displays a rectangular frame.

*Parameter*: Color and flags:

* 4-color mode: `(area_color << 4) | (lower_right_color << 2) | upper_left_color`; OR with `AREA_FILL` to fill interior.
* 16-color mode: `(lower_right_color << 12) | (upper_right_color << 8) | area_color | AREA_16COLOR`; OR with `AREA_FILL` to fill interior.

An optional XOR mode inverts the colors underneath the control, like a rubber-band selection. In XOR mode, the parameter is just `FRAME_XOR`.

```c
// example
_transfer Ctrl c_frame1 =
    {1, C_FRAME, -1, (COLOR_ORANGE << 4) | (COLOR_RED << 2) | COLOR_BLACK | AREA_FILL,
	 10, 10, 64, 64};
```

### C_TFRAME

Displays a rectangular frame with a line of text at the top.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;           // address of text
    unsigned char color;  // 4-color mode:  (foreground << 2) | background
                          // 16-color mode: line color
                          //   OR with AREA_16COLOR for 16-color mode
    unsigned char color2; // 16-color mode only: (foreground << 4) | background
} Ctrl_TFrame;
```

```c
// example
_transfer Ctrl_TFrame cd_tframe1 =
    {"Title", COLOR_BLACK | AREA_16COLOR, (COLOR_BLACK << 2) | COLOR_LBLUE};
_transfer Ctrl c_tframe1 =
    {1, C_TFRAME, -1, (unsigned short)&cd_tframe1, 10, 10, 64, 64};
```

### C_GRID

Displays repeated horizontal or vertical lines. By overlapping two `C_GRID` controls, one horizontal and one vertical, it is also possible to draw a grid of boxes. (This control is only available in SymbOS 4.0 and up.)

*Parameter*: Address of extended data record:

```c
typedef struct {
    unsigned char color;   // line color, OR'd with either GRID_HORIZONTAL or GRID_VERTICAL
    unsigned char lines;   // number of lines to draw
    unsigned char spacing; // (spacing << 1) | GRID_FIXED
} Ctrl_Grid;
```

Grid lines will start being drawn at the control's location `x`, `y` and continue down (for horizontal lines) or right (for vertical lines) for the specified number of lines. The `spacing` parameter requires some explanation. If this value is OR'd with `GRID_FIXED` (e.g., `(8 << 1) | GRID_FIXED`), all the lines will be drawn with the specified spacing (8 pixels in the preceding example). If `GRID_FIXED` is omitted, however, each line will have its own spacing. In this case, the `spacing` parameter indicates the spacing for the first line, and the `spacing` parameter for subsequent lines will be drawn from a `char` array that must immediately follow the `Ctrl_Grid` record (one `char` per remaining line). (The values in this array are treated exactly like the original `spacing` parameter, so once an array element with `GRID_FIXED` is encountered, the desktop manager will stop reading the array and draw all subsequent lines with the same spacing.)

```c
// example
_transfer Ctrl_Grid cd_grid1 = {COLOR_BLACK | GRID_HORIZONTAL, 10, (8 << 1)};
_transfer char grid1_sp[9] = {(8 << 1), (10 << 1), (12 << 1),
                              (8 << 1), (10 << 1), (12 << 1),
                              (8 << 1), (10 << 1), (12 << 1)};
                               
_transfer Ctrl c_grid1 = {1, C_GRID, -1, (unsigned short)&cd_grid1, 10, 10, 1000, 1000};
```

### C_PROGRESS

Displays a progress bar.

*Parameter*: Color and progress: `(progress << 8) | (empty_color << 6) | (filled_color << 4) | (lower_right_color << 2) | upper_left_color`. Progress is measured from 0 (empty) to 255 (full).

```c
// example
_transfer Ctrl c_progress1 =
    {1, C_PROGRESS, -1,
	 (119 << 8) | (COLOR_ORANGE << 6) | (COLOR_RED << 4) | (COLOR_BLACK << 2) | COLOR_BLACK,
	 10, 10, 64, 8};
```

### C_IMAGE

Displays a standard image (4-color SGX format only). (See the [graphics library](graphics.md#raw-images) for how to use this in practice.)

*Control type*: `C_IMAGE`.

*Parameter*: Address of the image data. 

```c
// example
_data char imgbuf[198];
_transfer Ctrl c_image1 = {1, C_IMAGE, -1, (unsigned short)imgbuf, 10, 10, 24, 24};
```

### C_IMAGE_EXT

Displays an image with an extended graphics header. (See the [graphics library](graphics.md) for how to use this in practice.)

*Parameter*: Address of the extended graphics header. Extended graphics are complicated, but allow plotting 16-color images and breaking up an image that is larger than 256 pixels wide/tall into multiple blocks that can be displayed side by side. The details of the graphics format are described in the SymbOS Developer Documentation. In practice, it is usually easier to use the [graphics library](graphics.md) rather than trying to deal with extended graphics headers directly. However, if needed, SCC provides a struct type (`Img_Header`) to implement the header itself:

```
typedef struct {
    unsigned char bytew;  // width of the complete graphic in bytes (must be even)
    unsigned char w;      // width of this block in pixels
    unsigned char h;      // height of this block in pixels
    char* addrData;       // address of graphic data + offset
    char* addrEncoding;   // address of encoding byte just before the start of the whole graphic
    unsigned short len;   // size of the complete graphic in bytes
} Img_Header;
```

```c
// example
_data Img_Header imghead; // fill with the appropriate data
_transfer Ctrl c_image_ext1 = {1, C_IMAGE_EXT, -1, (unsigned short)&imghead, 10, 10, 24, 24};
```

### C_IMAGE_TRANS

Same as `C_IMAGE_EXT`, except that color 0 will be transparent.

*Parameter*: Address of the extended graphics header, as above.

### C_ICON

Displays a 24x24 icon with up to two lines of text below it.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* icon;             // address of graphic or graphic header (e.g., a canvas)
    char* line1;            // address of first line of text
    char* line2;            // address of second line of text
    unsigned char flags;    // 4-color mode:  (foreground_color << 2) | background_color
    unsigned char color16;  // 16-color mode: (foreground_color << 4) | background_color
    unsigned char extflags; // extended mode flags
} Ctrl_Icon;
```

The following flags can be OR'd with `.flags`:

* `ICON_STD`: `.icon`. points to standard graphics data (4-color SGX)
* `ICON_EXT`: `.icon` points to extended graphics header, e.g., a [canvas](graphics.md#using-canvases) (see `C_IMAGE_EXT`)
* `ICON_16COLOR`: use 16-color mode for text colors (this does **not** mean the icon itself is 16-color, which is determined by the image header).
* `ICON_MOVEABLE`: icon can be moved by the user
* `ICON_EXTOPTS`: icon has extended options

When `.flags` includes `ICON_EXTOPTS`, the following flags can be OR'd with `.extflags`:

* `ICON_MARKABLE`: icon can be marked (selected) by user
* `ICON_MARKED`: icon is currently marked (selected) by user

The width of the control must be 48 and the height 40.

```c
// example
_data char imgdata[198]; // store an icon image here
_transfer Ctrl_Icon cd_icon1 =
    {imgdata, "Line 1", "Line 2", (COLOR_BLACK << 2) | COLOR_YELLOW | ICON_STD | ICON_4COLOR};
_transfer Ctrl c_icon1 = {1, C_ICON, -1, (unsigned short)&cd_icon1, 10, 10, 48, 40};
```

### C_BUTTON

Displays a button. Control height must always be 12.

*Parameter*: Address of the button text.

```c
// example
_transfer Ctrl c_button1 = {1, C_BUTTON, -1, (unsigned short)"Text", 10, 10, 48, 12};
```

### C_CHECK

Displays a checkbox.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* status;         // address of status byte
    char* text;           // address of text
    unsigned char color;  // (foreground << 2) | background
} Ctrl_Check;
```

The value of the checkbox (0 = unchecked, 1 = checked) will be stored in the byte pointed to by `status`, which should be in the **transfer** segment. The control height should always be 8.

```c
// example
_transfer char check1 = 0;
_transfer Ctrl_Check cd_check1 = {&check1, "Label text", (COLOR_BLACK << 2) | COLOR_ORANGE};
_transfer Ctrl c_check1 = {1, C_CHECK, -1, (unsigned short)&cd_check1, 10, 10, 32, 8};
```

### C_RADIO

Displays a radio button (circular checkbox). Selecting one radio button in a group will unselect all the others, allowing the user to select just one option.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* status;         // address of status byte
    char* text;           // address of text
    unsigned char color;  // (foreground << 2) | background
    unsigned char value;  // value to load into status when selected
    char* buffer;         // address of 4-byte coordinate buffer (see below)
} Ctrl_Radio;
```

When a radio button is selected, its `value` property will be loaded into the byte pointed to by `status`, which should be in the **transfer** segment. Usually this is an ID of some kind (1, 2, 3...), but it can technically be anything. To determine which radio button is selected, read the value of the byte pointed to by `status` and match it to the known `value` properties of the possible radio buttons.

`buffer` points to a 4-byte static buffer used internally by the desktop manager. It should initially contain the values -1, -1, -1, -1 and be in the **transfer** segment.

All radio buttons in a group should use the same `status` and `buffer`; to create multiple groups that do not interact, give them different status bytes and coordinate buffers.

The control height should always be 8.

```c
// example
_transfer char radio = 0;
_transfer char radiocoord[4] = {-1, -1, -1, -1};

_transfer Ctrl_Radio cd_radio1 =
    {&radio, "First button",  (COLOR_BLACK << 2) | COLOR_ORANGE, 1, radiocoord};
_transfer Ctrl_Radio cd_radio2 =
    {&radio, "Second button", (COLOR_BLACK << 2) | COLOR_ORANGE, 2, radiocoord};
_transfer Ctrl_Radio cd_radio3 =
    {&radio, "Third button",  (COLOR_BLACK << 2) | COLOR_ORANGE, 3, radiocoord};

_transfer Ctrl c_radio1 = {1, C_RADIO, -1, (unsigned short)&cd_radio1, 10, 10, 32, 8};
_transfer Ctrl c_radio2 = {2, C_RADIO, -1, (unsigned short)&cd_radio2, 10, 20, 32, 8};
_transfer Ctrl c_radio3 = {3, C_RADIO, -1, (unsigned short)&cd_radio3, 10, 30, 32, 8};
```

### C_HIDDEN

Nothing will be displayed, but any clicks to the area of the control will be sent as events for this control ID.

```c
// example
_transfer Ctrl c_hidden1 = {1, C_HIDDEN, -1, 0, 10, 30, 32, 8};
```

### C_TABS

Displays a row of tabs.

*Parameter*: Address of extended data record:

```c
typedef struct {
    unsigned char tabs;      // number of tabs
    unsigned char color;     // (lower_right << 6) | (upper_left << 4) | (fore << 2) | back
    unsigned char selected;  // number of selected tab
} Ctrl_Tabs;
```

This data structure should be immediately followed by the number of `Ctrl_Tab` structs indicated by the `tabs` property:

```c
typedef struct {
    char* text;              // address of text
    signed char width;       // width in pixels, or -1 to autocalculate
} Ctrl_Tab;
```

The control height must be 11. SymbOS will keep track of the selected tab for us, but it is our responsibility to decide what a click event on a tab actually does (for example, hiding one page of controls and revealing another). The event-handling code is also responsible for refreshing the tab control so the updated selection is actually displayed. A good way to switch between several pages of controls is to create each page as a separate `Ctrl_Group` and have a `C_COLLECTION` control pointing to the `Ctrl_Group` of the visible page. When the tab bar is clicked, change which `Ctrl_Group` the collection's `controls` property points to and redraw it.

```c
// example
_transfer Ctrl_Tabs cd_tabs =
    {2, (COLOR_BLACK << 6) | (COLOR_RED << 4) | (COLOR_BLACK << 2) | COLOR_ORANGE, 1};
_transfer Ctrl_Tab cd_tab1 = {"Tab 1", -1};
_transfer Ctrl_Tab cd_tab2 = {"Tab 2", -1};

_transfer Ctrl c_tabs = {1, C_TABS, -1, (unsigned short)&cd_tabs, 10, 10, 64, 11};
```

### C_SLIDER

Displays a slider (scrollbar or value selector).

*Parameter*: Address of extended data record:

```c
typedef struct {
    unsigned char type;      // type (see below)
    unsigned char unused;
    unsigned short value;    // current value/position
    unsigned short maxvalue; // maximum value/position (minimum is 0)
    signed char increment;   // value change when clicking up/right button
    signed char decrement;   // value change when clicking down/left button
} Ctrl_Slider;
```

Type is an OR'd bitmask consisting of one or more of the following flags:

* `SLIDER_H`: horizontal slider
* `SLIDER_V`: vertical slider
* `SLIDER_VALUE`: value slider
* `SLIDER_SCROLL`: scrollbar slider

For a horizontal slider, the control height must be 8 and the control width must be at least 24. For a vertical slider, the control width must be 8 and the control height must be at least 24. We are responsible for reading the slider's value and deciding what to do with it.

```c
// example
_transfer Ctrl_Slider cd_slider1 = {SLIDER_H | SLIDER_SCROLL, 0, 15, 30, 1, -1};
_transfer Ctrl c_tabs = {1, C_SLIDER, -1, (unsigned short)&cd_slider1, 10, 10, 100, 8};
```

### C_COLLECTION

Contains a collection of subcontrols. A collection behaves similarly to the main window content: controls will be clipped to the bounds of the collection, and if the full size of the content is larger than the size of the collection, the user will be able to scroll around the collection's content with scrollbars.

*Parameter*: Address of extended data record:

```c
typedef struct {
    void* controls;          // address of Ctrl_Group defining the controls in the collection
    unsigned short wfull;    // full width of content, in pixels
    unsigned short hfull;    // full height of content, in pixels
    unsigned short xscroll;  // horizontal scroll position, in pixels
    unsigned short yscroll;  // vertical scroll position, in pixels
    unsigned char flags;     // flags (see below)
} Ctrl_Collection;
```

If scrollbars are enabled, the control size must be greater than 32x32. `flags` is one of the following:

* `CSCROLL_NONE`: display no scrollbars
* `CSCROLL_H`: display horizontal scrollbar
* `CSCROLL_V`: display vertical scrollbar
* `CSCROLL_BOTH`: display both scrollbars

```c
// example
_transfer Ctrl cc_area = {1, C_AREA, -1, COLOR_ORANGE, 0, 0, 100, 100};
_transfer Ctrl_Group cg_collect = {1, 0, &cc_area};
_transfer Ctrl_Collection cd_collect = {&cg_collect, 200, 100, 0, 0, CSCROLL_H};
_transfer Ctrl c_collect = {1, C_COLLECTION, -1, (unsigned short)&cd_collect, 10, 10, 100, 100};

int main(int argc, char* argv[]) {
    cg_collect.pid = _sympid; // ensure collection's Ctrl_Group has the correct
	                          // process ID for sending back events
    /* ... */
}
```

### C_INPUT

Displays a single-line text input field.

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;               // address of text buffer (in data segment)
    unsigned short scroll;    // scroll position, in bytes
    unsigned short cursor;    // cursor position, in bytes
    signed short selection;   // number of selected characters relative to cursor
    unsigned short len;       // current text length, in bytes
    unsigned short maxlen;    // maximum text length, in bytes (not including 0-terminator)
    unsigned char flags;      // flags (see below)
    unsigned char textcolor;  // (foreground << 4) | background
    unsigned char linecolor;  // (lower_right_color << 4) | upper_left_color
} Ctrl_Input;
```

`flags` is an OR'd bitmask which may contain one or more the following:

* `INPUT_PASSWORD`: show all chars as `*`
* `INPUT_READONLY`: input is read-only
* `INPUT_ALTCOLS`: use alternate colors (if this flag is not specified, the `textcolor` and `linecolor` options will be ignored)
* `INPUT_MODIFIED`: represents a bit set when the text is modified

`selection` = 0 when no characters are selected, greater than 0 when the cursor marks the start of the selection, and less than 0 when the cursor marks the end of the selection.

The control height should always be 12. Note that, if we wish the input box to be prefilled, we must be sure to set the properties (`cursor`, `len`, etc.) correctly. (For input by the user, SymbOS will update these properties automatically.)

```c
// example
_data char cd_input_buf[25];
_transfer Ctrl_Input cd_input1 =
    {cd_input1_buf, 0, 0, 0, 0, 24, INPUT_ALTCOLS,
     (COLOR_RED << 4) | COLOR_YELLOW, (COLOR_RED << 4) | COLOR_BLACK};
_transfer Ctrl c_input1 = {1, C_INPUT, -1, (unsigned short)&cd_input1, 10, 10, 100, 12};
```

### C_TEXTBOX

Displays a multi-line text input box (text editor).

*Parameter*: Address of extended data record:

```c
typedef struct {
    char* text;               // address of text buffer (in data segment)
    unsigned short unused1;
    unsigned short cursor;    // cursor position, in bytes
    signed short selection;   // number of selected characters relative to cursor
    unsigned short len;       // current text length, in bytes
    unsigned short maxlen;    // maximum text length, in bytes (not including 0-terminator)
    unsigned char flags;      // flags (see below)
    unsigned char textcolor;  // (foreground << 4) | background, when using INPUT_ALTCOLS flag
    unsigned char unused2;
    char* font;               // font address, when using INPUT_ALTFONT flag (in data segment)
    unsigned char unused3;
    unsigned short lines;     // number of lines of text
    signed short wrapwidth;   // wrapping width, in pixels (-1 for no wrapping)
    unsigned short maxlines;  // maximum number of lines
    signed short xvisible;    // (used internally, set to -8 initially to force reformatting)
    signed short yvisible;    // (used internally, set to 0)
    void* self;               // address of this data record
    unsigned short xtotal;    // (used internally, set to 0)
    unsigned short ytotal;    // (used internally, set to 0)
    unsigned short xoffset;   // (used internally, set to 0)
    unsigned short yoffset;   // (used internally, set to 0)
    unsigned char wrapping;   // WRAP_WINDOW or WRAP_WIDTH
    unsigned char tabwidth;   // tab stop width (1-255, or 0 for no tab stop)
    int column;               // TextBox_Pos() returns values here
    int line;                 // TextBox_Pos() returns values here
    char reserved[4];         // (used internally, set to 0)
} Ctrl_TextBox;
```

The data record must be immediately followed by a buffer consisting of as many 16-bit words (e.g., `unsigned short`) as there are the maximum number of lines in the text data (`maxlines`). This buffer will hold the length (in bytes) of each line in the textbox, with the high bit set if this length includes a Windows-style `\r\n` end-of-line marker at the end of the line. Due to a quirk in SCC's linker, which currently treats initialized and uninitialized arrays differently, this buffer must be given an initial value (such as `{0}`) to ensure that it is placed directly after the `Ctrl_TextBox` data structure in the **transfer** segment. (This may be improved in future releases.) For example:

```c
_transfer unsigned short lines[1000] = {0};
```

`flags` is an OR'd bitmask which may contain one or more the following:

* `INPUT_READONLY`: input is read-only
* `INPUT_ALTCOLS`: use alternate colors (if this flag is not specified, the `textcolor` option will be ignored)
* `INPUT_ALTFONT`: use alternate font (if this flag is not specified, the `font` option will be ignored)
* `INPUT_MODIFIED`: represents a bit set when the text is modified

`wrapmode` must be either `WRAP_WINDOW` (wrap at window border) or `WRAP_WIDTH` (wrap at the width specified in `wrapwidth`). To use no wrapping, set `wrapmode` = `WRAP_WIDTH` and `wrapwidth` = -1.

`self` is a bit tricky: this must be the address of the data record itself, but this cannot be set in a static initializer because, at the time the compiler is parsing the initializer, the data record's symbol has not yet been fully defined! Instead, we must set this at runtime with a statement like:

```c
cd_textbox1.self = &cd_textbox1;
```

`selection` = 0 when no characters are selected, greater than 0 when the cursor marks the start of the selection, and less than 0 when the cursor marks the end of the selection.

To update the contents, cursor, or selection of the textbox, it is recommended to use the special system calls [`TextBox_Redraw()`](c_window.md#textbox_redraw) and [`TextBox_Select()`](c_window.md#textbox_select) rather than trying to update all the relevant properties manually.

Note that, because the buffer must be stored in a continuous 16KB segment (usually the **data** segment), this control is effectively limited to no more than 16KB of text. Note also that, if we wish the textbox to be prefilled, we must be sure to set the properties (`cursor`, `len`, etc.) correctly. (For input by the user, SymbOS will update these properties automatically.)

```c
// example
_data char textbuf[4096];
_transfer Ctrl_TextBox cd_textbox1 = {
    textbuf,         // text address
    0, 0, 0, 0,      // unused1, cursor, selection, len
    4095, 0, 0, 0,   // maxlen, flags, textcolor, unused2
    0, 0, 0, -1,     // font, unused3, lines, wrapwidth
    1000, -8, 0,     // maxlines, xvisible, yvisible
    0,               // self
    200, 100, 0, 0,  // xtotal, ytotal, xoffset, yoffset
    WRAP_WIDTH, 20}; // wrapping, tabwidth
_transfer unsigned short textbox1_lines[1000] = {0}; // line-length buffer
_transfer Ctrl c_textbox1 = {1, C_TEXTBOX, -1, (unsigned short)&cd_textbox1, 0, 0, 200, 100};

int main(int argc, char* argc[]) {
    cd_textbox1.self = &cd_textbox1; // fill "self" property at runtime
    /* ... */
}
```

### C_LISTBOX

Displays a list box, which may have one or more columns. The structure of lists is somewhat complicated, and is described below.

*Parameter*: Address of a `List` data record:

```c
typedef struct {
    unsigned short lines;     // number of list rows
    unsigned short scroll;    // index of first shown row
    void* rowdata;            // address of the row data (see below)
    unsigned short status;    // (only used by tree views, see C_TREE)
    unsigned char columns;    // number of columns (from 1 to 64)
    unsigned char sorting;    // sorting flags (see below)
    void* coldata;            // address of the column data (see below)
    unsigned short clicked;   // index of last clicked row
    unsigned char flags;      // flags (see below)
    unsigned char resorted;   // (SymbOS 4.0+ sets this to 1 when the user re-sorts the list)
    unsigned short treelines; // (only used by tree views, see C_TREE)
    unsigned short treefirst; // (only used by tree views, see C_TREE)
} List;
```

`flags` for `List` is an OR'd bitmask which may contain one or more of the following:

* `LIST_SCROLL`: show scrollbar
* `LIST_MULTI`: allow multiple selections

`sorting` is the index of the column to sort by (from 1 to 64), optionally OR'd with `SORT_AUTO` to automatically sort on first display and/or `SORT_REVERSE` to sort in descending order (rather than the default, ascending order).

`rowdata` and `coldata` must point to additional data structures in a specific format and sequence. Column definitions consist of a series of `List_Column` structs directly after one another in the **transfer** segment, as many structs are there are columns:

```c
typedef struct {
    unsigned char flags;     // flags (see below)
    unsigned char sortskip;  // number of characters to skip when sorting (normally 0)
    unsigned short width;    // width in pixels
    char* text;              // address of title text
    unsigned short unused2;
} List_Column;
```

`flags` for `List_Column` controls how the column is aligned and sorted, as well as what is displayed on each row. It consists of one of `ALIGN_LEFT`, `ALIGN_RIGHT`, or `ALIGN_CENTER`; OR'd with one of `LTYPE_TEXT` (for text data), `LTYPE_IMAGE` (for image data), `LTYPE_16` (for a 16-bit number), or `LTYPE_32` (for a 32-bit number). Alternatively, on SymbOS 4.0 and up, `flags` may just be set to `LTYPE_CTRL`, with no other flags (indicating text with control codes---see [`C_TEXT_CTRL`](#c_text_ctrl) for what codes are available).

`sortskip` (only available in SymbOS 4.0 and up) indicates the index of the first character to compare when sorting text rows. Normally this should be 0 (i.e., "sort from the first character"), but we can set it >0 to skip leading characters (like control characters).

The format of row definitions is a bit trickier. Internally, each row consists of a 16-bit flags word, followed by as many 16-bit value words as there are columns; these store either the value of the row in that column (for `LTYPE_16`) or the address of the data shown in that column. For a 1-column list, we can represent this structure with a series of `List_Row` structs, directly after one another:

```c
typedef struct {
    unsigned short flags; // flags (see below)
    char* value;          // address of row content (or value for LTYPE_16 - cast to char*)
} List_Row;
```

`flags` for `List_Row` (and its variants) is a 14-bit numeric value associated with the row. If the row is selected, the high bit of `flags` will be set; this can be tested with `row.flags & ROW_MARKED`.

Additional struct types for multi-column rows (up to 4 columns) are defined in `symbos/windows.h`; for example:

```c
typedef struct {
    unsigned short flags;
    char* value1;
    char* value2;
} List_Row2Col;
```

The control width must be at least 11, and the control height must be at least 16. Note that column names will not be displayed in the base `C_LISTBOX` control type; to show them, use `C_LISTFULL` instead.

```c
// example
_transfer List_Column cd_list1_col1 = {ALIGN_LEFT | LTYPE_TEXT, 0, 80, "Column 1"};
_transfer List_Column cd_list1_col2 = {ALIGN_LEFT | LTYPE_TEXT, 0, 80, "Column 2"};

_transfer List_Row2Col cd_list1_r1 = {1, "Row 1", "Value 1"};
_transfer List_Row2Col cd_list1_r2 = {2, "Row 2", "Value 2"};
_transfer List_Row2Col cd_list1_r3 = {3, "Row 3", "Value 3"};

_transfer List cd_list1 =
    {3, 0, &cd_list1_r1, 0, 2, 1 | SORT_AUTO, &cd_list1_col1, 0, LIST_MULTI};

_transfer Ctrl c_list1 = {1, C_LISTBOX, -1, (unsigned short)&cd_list1, 0, 0, 160, 100};
```

### C_LISTFULL

Equivalent to `C_LISTBOX`, but also displays column titles. Clicking a column title will sort by that column. Control width must be at least 11, control height must be at least 26.

*Parameter*: Same as for `C_LISTBOX`.

### C_LISTTITLE

Displays the column titles of a listbox or tree view, in isolation. Control height must always be 10.

*Parameter*: Same as for `C_LISTBOX`.

### C_DROPDOWN

Equivalent to `C_LISTBOX`, but displays a dropdown list selector instead of a full list box. The control width must be at least 11, and the control height must be 10. The `LIST_SCROLL` flag should be used whenever the list is longer than 10 entries, and the `LIST_MULTI` flag should never be used. The `.clicked` property indicates the currently selected row.

Note that, even though dropdown lists generally only have one column and do not display a column title, we must still define a valid column struct as described under `C_LISTBOX`.

*Parameter*: Same as for `C_LISTBOX`.

### C_TREE

Equivalent to `C_LISTBOX`, but displays a tree view listbox where the user can fold or expand rows as groups of nested nodes. (This control is only available in SymbOS 4.0 and up.)

*Parameter*: Same as for `C_LISTBOX`, with the following differences:

* `.sorting` should be set to `SORT_TREE`.
* `.flags` should never have `LIST_MULTI` set.

The format for columns and rows is also slightly different. Columns are defined in the usual way, except that `flags` for the first `List_Column` record should either be `LTYPE_TREE` (for normal text) or `LTYPE_TREE | LTYPE_CTRL` (for [text with control codes](#c_text_ctrl)). Rows are defined slightly differently, using `Tree_Row` structs instead of `List_Row` structs:

```c
typedef struct {
    unsigned char indent; // indentation level (see below)
    unsigned char flags;  // flags (see below)
    char* value;          // address of row content (or value for LTYPE_16 - cast to char*)
    unsigned short id;    // numerical ID of row
} Tree_Row;
```

`flags` for `Tree_Row` (and its variants) is an OR'd bitmask of one or more of the following:

* `TREE_NODE` - indicates that this row is a node (and starts a group of collapsible child rows).
* `TREE_EXPANDED` - indicates that subsequent child rows are currently expanded and visible.
* `TREE_HIDDEN` - indicates that this row is currently hidden.
* `TREE_MARKED` - indicates that this row is currently marked/selected.

Conceptually, a tree consists of a series of rows, each of which can be either a node (which will be followed by more child rows) or a leaf (which has no children). The first node has `indent` = 0, and any children of a node should have `indent` set to 1 greater than the `indent` of the parent. When the user collapses a node, all of its children (i.e., all contiguous rows following it with a greater `indent` than the node that was clicked) will be hidden.

Additional struct types for multi-column rows (up to 4 columns) are defined in `symbos/windows.h`; for example:

```c
typedef struct {
    unsigned char indent;
    unsigned char flags;
    char* value1;
    char* value2;
    unsigned short id;
} Tree_Row2Col;
```

**Note that all visibility flags, etc. must be initialized correctly** for how we want the tree to initially look. (Any subsequent modifications by the user will be tracked automatically.) Indentation levels must be correct, and all collapsed lines must have `TREE_HIDDEN` set. In addition, the text strings for the first column must begin with a special character followed by a space, which will be used to display the node symbols:

* `\x7F ` - for a leaf with no children.
* `\x81 ` - for a collapsed node.
* `\x82 ` - for an expanded node.

In addition to being dispatched as a normal `DSK_ACT_CONTENT` event, any user events that collapse or expand a node will be recorded in the `status` property of the tree's `List` struct. If `status & 0x8000` is nonzero, a node event has taken place, and the relevant information can be extracted as:

* `status & 0x0FFF` - the numerical ID of the node row
* `status & 0x4000` - nonzero if expanded, otherwise collapsed

Control width must be at least 11, and control height must be at least 16. Note that column titles will not be displayed automatically; to show them, use a separate `C_LISTTITLE` control, as in the example below.

Tip: By using the `LTYPE_TREE | LTYPE_CTRL` setting and using [control codes](#c_text_ctrl) to draw inline images, it is possible to include icons in the tree view.

```c
// example
_transfer List_Column cd_tree1_col1 = {LTYPE_TREE, 0, 80, "Column 1"};
_transfer List_Column cd_tree1_col2 = {ALIGN_LEFT | LTYPE_TEXT, 0, 80, "Column 2"};

_transfer Tree_Row2Col cd_tree1_r1 = {0, TREE_NODE | TREE_EXPANDED, "\x82 Node 1", "Value 1", 1};
_transfer Tree_Row2Col cd_tree1_r2 = {1, 0,                         "\x7F Leaf 1", "Value 2", 2};
_transfer Tree_Row2Col cd_tree1_r3 = {1, TREE_NODE | TREE_EXPANDED, "\x82 Node 2", "Value 3", 3};
_transfer Tree_Row2Col cd_tree1_r4 = {2, 0,                         "\x7F Leaf 2", "Value 4", 4};
_transfer Tree_Row2Col cd_tree1_r5 = {0, TREE_NODE,                 "\x81 Node 3", "Value 5", 5};
_transfer Tree_Row2Col cd_tree1_r6 = {1, TREE_HIDDEN,               "\x7F Leaf 3", "Value 6", 6};

_transfer List cd_tree1 = {6, 0, &cd_tree1_r1, 0, 2, SORT_TREE, &cd_tree1_col1, 0, LIST_SCROLL};

_transfer Ctrl c_treetitle1   = {1, C_LISTTITLE, -1, (unsigned short)&cd_tree1, 0, 0,  160, 10};
_transfer Ctrl c_treecontent1 = {2, C_TREE,      -1, (unsigned short)&cd_tree1, 0, 10, 160, 100};
```

