# Graphics Library

While SymbOS provides features (primarily the `C_IMAGE` and `C_IMAGE_EXT` controls) for directly plotting images, the graphics format is internally complicated and depends on the exact platform and screen mode in which the application is running. The SCC graphics library simplifies this process by implementing a set of standard functions for loading and plotting images and sprites, as well as directly manipulating pixels on a canvas.

**Note:** All graphics functions are thread-safe, except that only one canvas may be [active](#initializing-canvases) at a time. However, multiple threads can draw to the same canvas at the same time.

## Contents

* [Using the library](#using-the-library)
* [Using canvases](#using-canvases)
  * [Creating canvases](#creating-canvases)
  * [Initializing canvases](#initializing-canvases)
  * [Refreshing the display](#refreshing-the-display)
* [Drawing functions](#drawing-functions)
  * [Function reference](#drawing-functions)
* [Sprite functions](#sprite-functions)
  * [Converting images](#converting-images)
  * [Image sets and masks](#image-sets-and-masks)
  * [Function reference](#gfx_load)
* [Raw images](#raw-images)
  * [Raw image controls](#raw-image-controls)
  * [Function reference](#gfx_load_raw)
* [Advanced topics](#advanced-topics)
  * [Moving sprites](#moving-sprites)
  * [Memory problems](#memory-problems)
* [Reference](#reference)
  * [Color palette](#color-palette)

## Using the library

To use the library, include the `graphics.h` header:

```c
#include <graphics.h>
```

Additionally, use the `-lgfx` option when compiling to specify that the executable should be linked with the graphics library `libgfx.a`:

```bash
cc source.c -lgfx
```

## Using canvases

The SCC graphics library provides a set of functions for working with "canvases", bitmapped image buffers that can be displayed in SymbOS windows using the `C_IMAGE_EXT` control. We can plot sprites, draw lines, and directly edit the displayed image in other ways.

### Creating canvases

To create a canvas, first define a `char` buffer with the size `(width * height / 2) + 24`:

```c
_data char canvas[(128*64/2) + 24];
```

Canvas size is limited to 504x255 pixels, and **the width of a canvas in pixels must be a multiple of 8**. To work correctly on all platforms, a single canvas must also stay within a single 16KB memory segment; the easiest way to guarantee this is to place it in either the **data** or **transfer** segment. (This 16KB limit unfortunately means that the maximum *practical* size of a single canvas is much smaller than 504x255: e.g., 180x180, 240x136, 504x64, etc.)

To show this canvas in a window, we can use the `C_IMAGE_EXT` control. Set the `param` argument to point to the address of the canvas, and the `w` and `h` arguments to match the desired pixel width and height of the canvas:

```c
_transfer Ctrl c_canvas =
    {1,                      // control ID
	 C_IMAGE_EXT,            // control type
	 -1,                     // canvas bank (-1 = same as this record)
	 (unsigned short)canvas, // address of canvas
	 10, 10,                 // x, y
	 128, 64};               // w, h
```

If a canvas is greater than 252 pixels in width, we must instead create two `C_IMAGE_EXT` controls to display two halves (left and right) of the same canvas. These controls should be defined as follows:

* The width of the left control should be 252 pixels.
* The width of the right control should be (total width - 252) pixels.
* The `x` position of the right control should be 252 pixels greater than the `x` position of the left control.
* The `param` value of the right control should point to (canvas + 10) rather than (canvas).

For example, for a canvas 304x16 pixels in size:

```c
_transfer Ctrl c_canvas1 = {1, C_IMAGE_EXT, -1, (unsigned short)canvas,      10, 10, 252, 16};
_transfer Ctrl c_canvas2 = {2, C_IMAGE_EXT, -1, (unsigned short)canvas + 10, 10, 10,  52, 16};
```

### Initializing canvases

**Before opening the window in which the canvas is displayed**, we must initialize it with the `Gfx_Init()` function:

```c
void Gfx_Init(char* canvas, unsigned short w, unsigned char h);
```

For example, to initialize the 128x64 pixel canvas defined at the start of this section:

```c
Gfx_Init(canvas, 128, 64);
```

**Before drawing to a canvas**, we must select it as the active canvas using `Gfx_Select()`. If desired, we can maintain multiple canvases and switch which is being drawn to using this function.

```c
void Gfx_Select(char* canvas);
```

For example, to select the canvas defined at the start of this section:

```c
Gfx_Select(canvas);
```

### Refreshing the display

Updating the canvas with a graphics function does not automatically update the screen to display the changes. This means that graphics are automatically double-buffered, reducing flicker, but also that we must manually tell SymbOS to redraw the canvas (which is often the slowest part of the whole process). The simplest method is to redraw the entire canvas by using `Win_Redraw()` system call to redraw its control:

```c
Win_Redraw(winID, 1, 0); // redraw control ID 1 (the canvas's C_IMAGE_EXT control above)
```

However, redrawing a full canvas can visibly take a fraction of a second on a 4 MHz processor, which is nonideal for games with fluid animation. An alternative is to use the `Win_Redraw_Area()` system call to redraw only the part of the canvas that has changed. This is much faster, but requires us to keep track of the coordinates that have changed and issue the correct call:

```c
Win_Redraw_Area(winID, 1, 0, 10, 10, 16, 16); // redraw only the 16x16 pixel region
                                              // starting at x=10, y=10 in control ID 1
```

**Important**: The coordinates for `Win_Redraw_Area` are relative to the *window content*, not the canvas! This means that, if the canvas is not at position `x` = 0, `y` = 0 in the window, the coordinates passed to `Win_Redraw_Area` must have the canvas position added in.

It is important to understand that refreshes occur asynchronously. We are telling the desktop manager to refresh the control at its earliest convenience, but our code will keep running in the meantime, and there is no way to determine when the refresh has actually occurred (except that, once the screen is refreshed, there will be a delay of at least 20 milliseconds before it can be refreshed again). This is helpful for realtime games, where waiting for a refresh would slow everything down, but can also create subtle traps. In particular, if we are continuously updating the canvas, what is actually shown on the screen will not be the contents of the canvas *at the time the refresh was requested*, but the contents of the canvas some fraction of a second later *when the refresh is actually performed.* Care should therefore be taken not to "get ahead of" the screen refresh by, e.g., plotting a sprite, requesting a refresh, and then immediately moving it somewhere else.

## Drawing functions

### Gfx_Pixel()

```c
void Gfx_Pixel(unsigned short x, unsigned char y, unsigned char color);
```

Plots a single pixel with the color `color` to the pixel coordinates `x`, `y` on the currently active canvas. No bounds-checking is performed, so be sure that the coordinates are actually valid. In 4-color modes, higher colors will be automatically rendered down to 4 colors.

### Gfx_Safe_Pixel()

```c
void Gfx_Safe_Pixel(unsigned short x, unsigned char y, unsigned char color);
```

Equivalent to `Gfx_Pixel()`, but performs a simple bounds check and skips plotting the pixel if the coordinates are invalid for the current canvas. This function should be used instead of `Gfx_Pixel()` in cases where it is not known ahead of time that the pixel coordinates are valid.

### Gfx_Value()

```
unsigned char Gfx_Value(unsigned short x, unsigned char y);
```

Returns the color of the pixel on the active canvas at pixel coordinates `x`, `y`.

### Gfx_Line()

```c
void Gfx_Line(unsigned short x0, unsigned char y0,
              unsigned short x1, unsigned char y1,
			  unsigned char color);
```

Draws a 1-pixel-thick line of color `color` from point `x0`, `y0` to point `x1`, `y1` on the current canvas. No bounds-checking is performed, so be sure that the coordinates are actually valid. In 4-color modes, higher colors will be automatically rendered down to 4 colors.

### Gfx_LineB()

```c
void Gfx_LineB(unsigned short x0, unsigned char y0,
               unsigned short x1, unsigned char y1,
			   unsigned char color, unsigned char bits);
```

Equivalent to `Gfx_Line()`, but only pixels corresponding to bits set in the bitmask `bits` will be drawn (starting from the most significant bit and repeating after 8 bits/pixels). For example, to draw a dashed line (two pixels on + two pixels off, repeating), we could use the bitmask `0xCC`.

### Gfx_HLine()

```c
void Gfx_HLine(unsigned short x, unsigned char y, unsigned short w, unsigned char color);
```

Draws a 1-pixel-thick line horizontal line of color `color` from point `x`, `y` to the point `w` pixels to the right. This is faster than using `Gfx_Line()` for cases where the line is known to be horizontal. No bounds-checking is performed, so be sure that the coordinates are actually valid. In 4-color modes, higher colors will be automatically rendered down to 4 colors.

### Gfx_VLine()

```c
void Gfx_VLine(unsigned short x, unsigned char y, unsigned short h, unsigned char color);
```

Draws a 1-pixel-thick line vertical line of color `color` from point `x`, `y` to the point `h` pixels below. This is faster than using `Gfx_Line()` for cases where the line is known to be vertical. No bounds-checking is performed, so be sure that the coordinates are actually valid. In 4-color modes, higher colors will be automatically rendered down to 4 colors.

### Gfx_Box()

```c
void Gfx_Box(unsigned short x0, unsigned char y0,
             unsigned short x1, unsigned short y1,
			 unsigned char color);
```

Draws an unfilled rectangle of color `color` whose upper left corner is at the pixel coordinates `x0`, `y0` and whose lower right corner is at the pixel coordinates `x1`, `y1`. No bounds-checking is performed, so be sure that the coordinates are actually valid. In 4-color modes, higher colors will be automatically rendered down to 4 colors.

### Gfx_BoxF()

```c
void Gfx_BoxF(unsigned short x0, unsigned char y0,
              unsigned short x1, unsigned short y1,
			  unsigned char color);
```

Draws a filled rectangle of color `color` whose upper left corner is at the pixel coordinates `x0`, `y0` and whose lower right corner is at the pixel coordinates `x1`, `y1`. No bounds-checking is performed, so be sure that the coordinates are actually valid. In 4-color modes, higher colors will be automatically rendered down to 4 colors.

### Gfx_Circle()

```c
void Gfx_Circle(signed short x0, signed short y0, signed short radius, unsigned char color);
```

Draws an unfilled circle with the color `color`, a radius of `radius` pixels, and its centerpoint at `x0`, `y0`. The circle may overlap the edges of the canvas. In 4-color modes, higher colors will be automatically rendered down to 4 colors.

### Gfx_Text()

```c
void Gfx_Text(unsigned short x, unsigned char y, char* text, unsigned char color, char* font);
```

Draws the text string `text` at the pixel coordinates `x`, `y` on the currently active canvas, using color `color` and the font at the address `font`. If `font` = 0, the system font will be used. No clipping is performed, so be sure that the coordinates are actually valid (including that the text will not overflow the right margin). In 4-color modes, higher colors will be automatically rendered down to 4 colors. 

Drawing text in 16-color mode is currently slower than in 4-color mode. Drawing text using the system font is also a bit slower than using a custom font because of the need for banked memory access. (This is the opposite of how system text controls behave, where using the system font is faster.) Unlike system controls, it does not matter which segment custom font data is stored in.

A description of the font format can be found in the SymbOS Developer Documentation; fonts can be created using the SymbOS Font Editor application.

### Gfx_ScrollX()

*Currently only available in development builds of SCC.*

```c
void Gfx_ScrollX(int pixels);
```

Scrolls the entirety of the currently active canvas `pixels` pixels right (when `pixels` > 0) or left (when `pixels` < 0). `pixels` will be rounded down to the nearest 4 pixels in 4-color mode or to the nearest 2 pixels in 16-color mode; this limitation enables scrolling to be a very fast byte-copy operation (even faster than a window refresh).

The margin of the canvas that was just "scrolled onscreen" (e.g., the rightmost 4 columns of pixels when scrolling by -4) will be initially filled with garbage; we are responsible for overdrawing it with any content that should become visible as a result of the scroll.

### Gfx_ScrollY()

*Currently only available in development builds of SCC.*

```c
void Gfx_ScrollY(int pixels);
```

Scrolls the entirety of the currently active canvas `pixels` pixels down (when `pixels` > 0) or up (when `pixels` < 0). Unlike `Gfx_ScrollX()`, scrolling will be pixel-perfect, without rounding down. This is a very fast byte-copy operation (even faster than a window refresh).

The margin of the canvas that was just "scrolled onscreen" (e.g., the bottom 4 rows of pixels when scrolling by -4) will be initially filled with garbage; we are responsible for overdrawing it with any content that should become visible as a result of the scroll.

### Gfx_Clear()

```c
void Gfx_Clear(char* canvas, unsigned char color);
```

Clears the entirety of canvas `canvas` to color `color`. In 4-color modes, higher colors will be automatically rendered down to 4 colors. (This is faster than filling the entire canvas using `Gfx_BoxF()`.)

## Sprite functions

We can load images in SGX format from disk and plot them directly on the canvas, or copy portions of the canvas as images for saving to disk or subsequent plotting. Images handled in this way **must have a width that is a multiple of 4 pixels** and cannot exceed 252x255 pixels in width/height. (This means that we cannot load, e.g., complete desktop backgrounds as sprites.)

### Converting images

Images can be converted to SGX format with software such as [MSX Viewer 5](https://marmsx.msxall.com/msxvw/msxvw5/index_en.php) (classic version). The graphics library is able to correctly handle the erroneous header information MSX Viewer 5 creates for smaller 16-color images, so most images created by MSX Viewer 5 should be usable without modification.

Alternatively, the command-line tool `gfx2sgx` provided with the desktop version of SCC can be used to convert graphics in a variety of formats (`.bmp`, `.jpg`, `.png`, `.gif`, `.tga`, and more) to SGX format, e.g.:

```bash
gfx2sgx image.png
```

This will convert `image.png` into a 16-color SGX image in the same folder (`image.sgx`). Images are converted pixel-perfect by Euclidean similarity to the SymbOS palette colors, so any necessary scaling or recoloring must be done using other graphics software (such as GIMP or Photoshop) prior to conversion. A typical workflow might be to create a custom color palette matching the [default SymbOS palette](#color-palette), create pixel art using this palette, export it an RGB `.png` file, and run it through `gfx2sgx`. For more complicated conversions (e.g., converting a true-color JPEG photo to a 16-color SGX image), automatic palette-conversion tools like GIMP's indexed color quantization may be useful (Image > Mode > Indexed, then convert back to RGB with Image > Mode > RGB before exporting to `.png`).

### Image sets and masks

`gfx2sgx` allows the creation of multi-image sets ("tilesets" or "sprite sheets") from a single image. To create an image set, specify the width and height of each tile on the command-line. For example, to split the image `tiles.png` evenly into 24x16 pixel tiles:

```bash
gfx2sgx tiles.png 24 16
```

The resulting `.gfx` file can be loaded using `Gfx_Load_Set()` and individual tiles plotted using `Gfx_Put_Set()`, described below. Creating tilesets in this way prevents us from having to load numerous small files from disk. Tiles are extracted from the original image left-to-right, top-to-bottom, with the first tile (from the upper left of the original image) having the index 0.

A standard technique for creating transparent sprites is to use bitwise plotting modes (see `Gfx_Put()`, below). First, we plot a "mask" image with `PUT_AND` (bitwise AND); this image should consist of color 0 for all pixels that are to be overwritten, and color 15 for all pixels that are to be transparent. Then, we plot a "foreground" image with `PUT_OR` (bitwise OR); this image should consist of color 0 for all pixels that are to be transparent, and normal colors for all pixels that are to be overwritten. `gfx2sgx` can automatically split a transparent `.png` image into the necessary mask and foreground layers using the `-m` option. For example, to split the transparent 48x32-pixel image `tiles.png` evenly into six 16x16-pixel tiles, each with a mask layer:

```bash
gfx2sgx tiles.png -m 16 16
```

The mask for each tile will be placed immediately before it, i.e., the image:

![Tile image](tileset.png)

...becomes:

![Extracted tiles with mask](tileset_mask.png)

The resulting tiles can be drawn transparently by first plotting the mask tile with mode `PUT_AND` and then plotting the foreground tile with `PUT_OR`:

![Technique demonstration](tileset_and.png)

### Gfx_Load()

```c
unsigned char Gfx_Load(char* filename, char* buffer);
```

Loads a graphics asset file (in 4-color or 16-color SGX format) from the path `filename` into the buffer `buffer`. If the path `filename` is relative rather than absolute, it will be treated as relative to the current executable's path. If the current screen mode does not match the format of the file, it will be converted automatically to match using `Gfx_Prep()` (listed below).

`buffer` should be a `char` array the size of the file, in bytes (rounding up to the nearest 128 bytes), or else `(width * height / 2) + 4` bytes long, whichever is greater.

*Return value*: On success, returns 0. On failure, sets `_fileerr` and returns 1.

### Gfx_Load_Set()

```c
unsigned char Gfx_Load_Set(char* filename, char* buffer);
```

Loads an image set file created with `gfx2sgx` (see above) from the path `filename` into the buffer `buffer`. If the path `filename` is relative rather than absolute, it will be treated as relative to the current executable's path. If the current screen mode does not match the format of the tiles, they will be converted automatically to match using `Gfx_Prep()` (listed below).

`buffer` should be a `char` array the size of the file, in bytes (rounding up to the nearest 128 bytes).

*Return value*: On success, returns 0. On failure, sets `_fileerr` and returns 1.

### Gfx_Put()

```c
void Gfx_Put(char* image, unsigned short x, unsigned char y, unsigned char mode);
```

Plots the image located in the buffer `image` to the pixel coordinates `x`, `y`. The plotting mode `mode` is one of:

* `PUT_SET` - overwrites the pixels on the canvas with the pixels in the image.
* `PUT_AND` - performs a bitwise AND between the pixels on the canvas and the pixels in the image.
* `PUT_OR` - performs a bitwise OR between the pixels on the canvas and the pixels in the image.
* `PUT_XOR` - performs a bitwise XOR between the pixels on the canvas and the pixels in the image.

No bounds-checking or clipping is performed, so be sure that the coordinates are actually valid.

Plotting is fast when the image can be copied directly without bit-shifting or modification (i.e., when `mode` is `PUT_SET` and `x` is a multiple of 4, or, for 16-color displays, a multiple of 2), but somewhat slower in all other cases. For performance-critical applications, a common trick to speed up sprite-plotting is to maintain four different versions of a sprite, each shifted horizontally by 1 pixel, so that sprites can always be plotted at `x` coordinates that are multiples of 4. However, the performance gains of this method when using a `mode` other than `PUT_SET` may be marginal.

### Gfx_Put_Set()

```c
void Gfx_Put_Set(char* image, unsigned short x, unsigned char y,
                 unsigned char mode, unsigned char tile);
```

Equivalent to `Gfx_Put()`, above, but plots tile number `tile` (where the first tile is `tile` = 0) from an image set loaded with `Gfx_Load_Set()`.

### Gfx_Get()

```c
void Gfx_Get(char* image, unsigned short x, unsigned char y, unsigned char w, unsigned char h);
```

Creates an image from the portion of the active canvas starting at pixels coordinates `x`, `y`, with width `w` pixels and height `h` pixels, storing the resulting image in the `char` buffer `image`. `w` must be a multiple of 8, and this buffer should be at least `(w * h) / 2 + 4` bytes long. The resulting image will be in the same format expected by `Gfx_Put()`. No bounds-checking is performed, so be sure that the coordinates are actually valid.

### Gfx_Save()

```c
unsigned char Gfx_Save(char* filename, char* buffer);
```

Saves the image from the buffer `buffer` to the path `filename`. If the path `filename` is relative rather than absolute, it will be treated as relative to the current executable's path. Images will be saved in 4-color or 16-color SGX format, depending on the current screen mode.

Note that `buffer` is a `char` buffer into which an image has been loaded using `Gfx_Load()` or `Gfx_Get()`, *not* a complete canvas.

*Return value*: On success, returns 0. On failure, sets `_fileerr` and returns 1.

### Gfx_Prep()

```c
void Gfx_Prep(char* buffer);
```

If the bit depth of the image located in the buffer `buffer` does not match the current screen mode (i.e., 4-color vs. 16-color), or the image has the wrong type of header, converts it to the correct format. Since 4-color images require more space when converted to 16-color, `buffer` should be at least `(width * height / 2) + 4` bytes long regardless of the original size of the image data.

This function is normally called automatically by `Gfx_Load()`, but we can also invoke it manually to convert image data embedded in the source as a character array into the correct format. E.g.:

```c
_transfer char icon[35] = {0x02, 0x08, 0x08, 0xFF, 0xFF, 0xF8, 0xF1, 0xF8, 0xF1,
                           0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0x8F, 0x1F, 0xFF, 0xFF};
						   
int main(int argc, char* argv[]) {
	/* ... other graphics setup here ... */
	Gfx_Prep(icon);
	Gfx_Put(icon, 0, 0);
	/* ... other commands here ... */
}
```

Character arrays like this can be created by running [`gfx2sgx`](#converting-images) with the `-c` option, which will output the image data as a character array in a `.c` file instead of as binary data in a `.sgx` file:

```bash
gfx2sgx icon.png -c
```

### Gfx_Prep_Set()

```c
void Gfx_Prep_Set(char* buffer);
```

Equivalent to `Gfx_Prep()`, but preps image sets instead of single images.

### Gfx_TileAddr()

*Currently only available in development builds of SCC.*

```c
char* Gfx_TileAddr(char* image, unsigned char tile);
```

Returns the address of tile number `tile` within the image set at the address `image`.

## Raw images

*Currently only available in development builds of SCC.*

### Raw image controls

Most of the graphics library deals with canvases, which allow graphics to be edited and redraw on the fly. However, if we just want to display static images as part of a form, we can often achieve this more efficiently by simply creating a [`C_IMAGE`](s_ctrls.md#c_image) or [`C_IMAGE_EXT`](s_ctrls.md#c_image_ext) control and setting the parameter to the address of the static image data.

**Step 1: Generate a compatible image file**

For 16-color images, use [`gfx2sgx`](#converting-images) to convert the desired image into an .SGX file. For 4-color images, use [MSX Viewer 5](https://marmsx.msxall.com/msxvw/msxvw5/index_en.php) (classic version) or use `gfx2sgx` with the `-4` command-line option to force 4-color mode:

```bash
gfx2sgx image.png -4
```

**Step 2: Load the image into a buffer**

Load the image data into a suitably sized buffer that does not cross a 16KB boundary, e.g., in the **data** or **transfer** segments. The utility function [`Gfx_Load_Raw()`](#gfx_load_raw) is the recommended way to do this, since this will perform the necessary updates to the the [extended graphics header](s_ctrls.md#c_image_ext) automatically.

(Note that, since we will not be manipulating this image after it is loaded, we can technically store it in [banked memory](s_core.md#memory-management) if needed. For this reason, all raw image functions allow specifying the memory bank. If banked memory is used, remember to set the `.bank` property of the image control correctly and free the reserved memory with [`Mem_Release()`](s_core.md#mem_release) before exit.)

**Step 3: Create an image control**

For a 4-color image, use a [`C_IMAGE`](s_ctrls.md#c_image) control and set the parameter to the address of the image buffer:

```
_data char imgbuf[256]; // load the image data into this buffer
_transfer Ctrl c_image1 = {1, C_IMAGE, -1, (unsigned short)imgbuf, 10, 10, 24, 24};
```

For a 16-color iamge, use a [`C_IMAGE_EXT`](s_ctrls.md#c_image_ext) control and set the parameter to the address of the image buffer:

```
_data char imgbuf[384]; // load the image data into this buffer
_transfer Ctrl c_image1 = {1, C_IMAGE_EXT, -1, (unsigned short)imgbuf, 10, 10, 24, 24};
```

### Gfx_Load_Raw()

```c
unsigned char Gfx_Load_Raw(char* filename, unsigned char bank, char* buffer);
```

Loads a raw image file (in 4-color or 16-color SGX format) from the path `filename` into memory bank `bank`, address `buffer`. If the path `filename` is relative rather than absolute, it will be treated as relative to the current executable's path. `buffer` should be a `char` array the size of the file, in bytes (rounding up to the nearest 128 bytes), which does not cross a 16KB segment boundary

The difference between this function and the regular [`Gfx_Load()`](#gfx_load) is that this function prepares the image file for direct use by a [control](#raw-image-controls), not as a graphics asset that can be plotted on a [canvas](#using-canvases) using `Gfx_Put()`. If the image is meant to be plotted on a canvas, use `Gfx_Load()` instead.

*Return value*: On success, returns 0. On failure, sets `_fileerr` and returns 1.

### Gfx_Load_Set_Raw()

```c
unsigned char Gfx_Load_Set_Raw(char* filename, unsigned char bank, char* buffer);
```

Loads an [image set file](#image-sets-and-masks) created with `gfx2sgx` from the path `filename` into memory bank `bank`, address `buffer`. If the path `filename` is relative rather than absolute, it will be treated as relative to the current executable's path. `buffer` should be a `char` array the size of the file, in bytes (rounding up to the nearest 128 bytes).

The difference between this function and the regular [`Gfx_Load_Set()`](#gfx_load_set) is that this function prepares the image tiles for direct use by [controls](#raw-image-controls), not as graphics assets that can be plotted on a [canvas](#using-canvases) using `Gfx_Put_Set()`. If the images are meant to be plotted on a canvas, use `Gfx_Load_Set()` instead.

This function is a convenient way to load a large number of raw images at once. For the purpose of setting image control parameters, the address of each tile in the tileset can be determined using [`Gfx_TileAddr_Raw()`](#gfx_tileaddr_raw).

*Return value*: On success, returns 0. On failure, sets `_fileerr` and returns 1.

### Gfx_Prep_Raw()

```c
void Gfx_Prep_Raw(unsigned char bank, char* buffer);
```

If the memory at bank `bank`, address `buffer` contains a raw 16-color image, prepares the image's extended header so that it can be drawn correctly by a `C_IMAGE_EXT` control. (This function is normally called automatically by `Gfx_Load_Raw()`, but we can also invoke it manually to convert image data [embedded in the source as a character array](#gfx_prep).)

### Gfx_Prep_Set_Raw()

```c
void Gfx_Prep_Set_Raw(unsigned char bank, char* buffer);
```

Equivalent to `Gfx_Prep_Raw()`, but preps image sets instead of single images.

### Gfx_TileAddr_Raw()

```c
char* Gfx_TileAddr_Raw(unsigned char bank, char* image, unsigned char tile);
```

Returns the address of tile number `tile` within the image set located in memory bank `bank`, address `image`. (The only difference between this and `Gfx_TileAddr()` is that the bank may be specified.)

## Advanced topics

### Moving sprites

Because `graphics.h` implement raw canvases and images, it does not have any built-in features for redrawing the background behind a sprite after it is moved or deleted. Anything drawn to a canvas will simply stay there until overdrawn by something else. However, several simple techniques can be employed to create moveable sprites, depending on the specific needs of the application:

* Before plotting a sprite, use `Gfx_Get()` to copy the background behind it into a temporary buffer. Then, when the sprite needs to be moved, erase it by plotting the old background on top of it with `Gfx_Put()`.
* For graphics based on multiple layers of regularly-spaced tiles (like an RPG), simply replot any affected tiles from the bottom up with `Gfx_Put()`, redrawing the background over the sprite.
* A flexible (but memory-hungry) solution is to maintain two canvases: a "background" canvas containing the background, and a "visible" canvas actually shown in the window. To move a sprite, use `Gfx_Select()`, `Gfx_Get()`, and `Gfx_Put()` to copy the relevant parts of the "background" canvas over the sprite's location on the "visible" canvas.
* If only a few sprites are needed, consider drawing them using separate [`C_IMAGE_TRANS`](s_ctrls.md#c_image_trans) controls pointing to [raw images](#raw-images).

### Memory problems

In practice, the biggest challenge when developing graphics-heavy programs (like games) is running out of memory. For speed, the graphics library requires that all canvases and graphics assets be located in the application's main 64KB address space (the **code**, **data**, and **transfer** segments). This is fine if we are only using the graphics library for small accents like icons or toolbars, but imagine an RPG that wants to keep a 230x140 canvas, a set of 100 16x16 tiles, and a set of 32 16x16 masked sprites in memory at the same time. That leaves only 19KB for the game code itself, assuming a byte-perfect memory map---not much to work with!

Further problems arise from the need to keep canvases within a single 16KB memory segment. (This is necessary to prevent graphics corruption on some platforms, like the MSX.) A common (if confusing) way to get into trouble is to have a large **code** segment, a large canvas in the **data** segment, and a comparatively empty **transfer** segment. SymbOS may then have trouble arranging the segments in a legal order even if the total file size is <64KB.

To understand what is going on, imagine a game with a 40KB **code** segment, a 16KB **data** segment, and a 3KB **transfer** segment. The **code** segment is loaded first, while the **transfer** segment must be in the uppermost 16KB of memory:

| `0x0000` | `0x4000` | `0x8000` | `0xC000` |
| -------- | -------- | -------- | -------- |
| `CCCCCC` | `CCCCCC` | `CCC`    | `TT`     |

Where does the **data** segment (`DDDDDD`) go? In theory it could go between the **code** and **transfer** segments, but this would result in it crossing a 16KB segment boundary, which is not allowed:

| `0x0000` | `0x4000` | `0x8000` | `0xC000` |
| -------- | -------- | -------- | -------- |
| `CCCCCC` | `CCCCCC` | `CCCDDD` | `DDDTT`  |

So, SymbOS throws an "out of memory" error even though the total file size is <64KB. The easiest solution to this problem is usually to move some data from the **code** segment to the **transfer** segment:

| `0x0000` | `0x4000` | `0x8000` | `0xC000` |
| -------- | -------- | -------- | -------- |
| `CCCCCC` | `CCCCCC` | `DDDDDD` | `TTTTT`  |

But what if we're just hitting the 64KB limit? Some suggestions, from least to most complicated:

* Eliminate all unnecessary canvas and buffer space. For example, in games, it is often unnecessary to actually draw the entire play field using a single large canvas. If there is a significant amount of whitespace, consider breaking the canvas up into several smaller canvases and drawing the whitespace using a `C_AREA` control instead (which takes almost no memory).
* Store graphics assets in [banked memory](s_core.md#memory-management) and only load them into the application's main 64KB address space when needed. (This works best when draw-time is not critical, or when we only need a portion of all available graphics assets at one time.)
* For repeating elements, instead of using one large `C_IMAGE_EXT` control pointing to one large canvas, use several small `C_IMAGE_EXT` controls pointing to the same small canvas. Overlapping [`C_IMAGE_TRANS`](s_ctrl.md#c_image_trans) controls could also be used to create transparent sprites without requiring a separate mask image.
* Most complicated, but theoretically useful for game engines: perform all graphics rendering in a dedicated [process](s_task.md#processes) with its own 64KB address space, telling the rendering process what to do using [inter-process messaging](s_core.md#messaging) or a shared memory space in banked memory.

## Reference

### Color palette

The default SymbOS color palette is shown below, along with approximate HTML hex codes and equivalent C constants (from `symbos/windows.h`). In 4-color modes, colors greater than 3 will be automatically rendered down to colors 0-3 following the pattern shown on the right side of the color preview (i.e., `color & 3`). Constants in International English spellings (e.g., `COLOUR_WHITE`) are also available.

| Color | ID | Keyword | Hex |
| ----- | -- | ------- | --- |
| ![#f7f790](https://placehold.co/15x15/f7f790/f7f790.png) > ![#f7f790](https://placehold.co/15x15/f7f790/f7f790.png)| 0 | `COLOR_YELLOW` | `#F7F790` |
| ![#060606](https://placehold.co/15x15/060606/060606.png) > ![#060606](https://placehold.co/15x15/060606/060606.png) | 1 | `COLOR_BLACK` | `#060606` |
| ![#f79006](https://placehold.co/15x15/f79006/f79006.png) > ![#f79006](https://placehold.co/15x15/f79006/f79006.png) | 2 | `COLOR_ORANGE` | `#F79006` |
| ![#900606](https://placehold.co/15x15/900606/900606.png) >  ![#900606](https://placehold.co/15x15/900606/900606.png) | 3 | `COLOR_RED` | `#F79006` |
| ![#06f7f7](https://placehold.co/15x15/06f7f7/06f7f7.png) > ![#f7f790](https://placehold.co/15x15/f7f790/f7f790.png) | 4 | `COLOR_CYAN` | `#06F7F7` |
| ![#060690](https://placehold.co/15x15/060690/060690.png) > ![#060606](https://placehold.co/15x15/060606/060606.png) | 5 | `COLOR_DBLUE` | `#060690` |
| ![#9090f7](https://placehold.co/15x15/9090f7/9090f7.png) > ![#f79006](https://placehold.co/15x15/f79006/f79006.png) | 6 | `COLOR_LBLUE` | `#9090F7` |
| ![#0606f7](https://placehold.co/15x15/0606f7/0606f7.png) > ![#900606](https://placehold.co/15x15/900606/900606.png) | 7 | `COLOR_BLUE` | `#0606F7` |
| ![#f7f7f7](https://placehold.co/15x15/f7f7f7/f7f7f7.png) > ![#f7f790](https://placehold.co/15x15/f7f790/f7f790.png) | 8 | `COLOR_WHITE` | `#F7F7F7` |
| ![#069006](https://placehold.co/15x15/069006/069006.png) > ![#060606](https://placehold.co/15x15/060606/060606.png) | 9 | `COLOR_GREEN` | `#069006` |
| ![#06f706](https://placehold.co/15x15/06f706/06f706.png) > ![#f79006](https://placehold.co/15x15/f79006/f79006.png) | 10 | `COLOR_LGREEN` | `#06F706` |
| ![#f706f7](https://placehold.co/15x15/f706f7/f706f7.png) > ![#900606](https://placehold.co/15x15/900606/900606.png) | 11 | `COLOR_MAGENTA` | `#F706F7` |
| ![#f7f706](https://placehold.co/15x15/f7f706/f7f706.png) > ![#f7f790](https://placehold.co/15x15/f7f790/f7f790.png) | 12 | `COLOR_LYELLOW` | `#F7F706` |
| ![#909090](https://placehold.co/15x15/909090/909090.png) > ![#060606](https://placehold.co/15x15/060606/060606.png) | 13 | `COLOR_GRAY` | `#909090` |
| ![#f79090](https://placehold.co/15x15/f79090/f79090.png) > ![#f79006](https://placehold.co/15x15/f79006/f79006.png) | 14 | `COLOR_PINK` | `#F79090` |
| ![#f70606](https://placehold.co/15x15/f70606/f70606.png) > ![#900606](https://placehold.co/15x15/900606/900606.png) | 15 | `COLOR_LRED` | `#F70606` |
