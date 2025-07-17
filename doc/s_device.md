# Device routines

In addition to `symbos.h`, these functions can be found in `symbos/device.h`.

## Contents

* [Screen status](#screen-status)
* [Mouse status](#mouse-status)
* [Keyboard status](#keyboard-status)
* [Time functions](#time-functions)
* [System configuration](#system-configuration)

Other device-related subsections:

* [Sound routines](s_sound.md)
* [Printer routines](s_print.md)

## Screen status

### Screen_Mode()

```c
unsigned char Screen_Mode(void);
```

Returns the current screen mode, which depends on the current platform:

| Mode  | Platform  | Resolution | Colors  |
| ------| --------- | ---------- | ------- |
| 0     | PCW       | 720x255    | 2       |
| 1     | CPC/EP    | 320x200    | 4       |
| 2     | CPC/EP    | 640x200    | 2       |
| 5     | MSX       | 256x212    | 16      |
| 6     | MSX       | 512x212    | 4       |
| 7     | MSX       | 512x212    | 16      |
| 8     | G9K       | 384x240    | 16      |
| 9     | G9K       | 512x212    | 16      |
| 10    | G9K       | 768x240    | 16      |
| 11    | G9K       | 1024x212   | 16      |

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

### Screen_Mode_Set()

```c
void Screen_Mode_Set(char mode, char force, char vwidth);
```

Sets the current screen mode. `mode` is the screen mode (see `Screen_Mode()`). If `force` = 1, the mode will be forced to change. For G9K modes only, `vwidth` specifies the virtual desktop width, one of:

* 0 = no virtual desktop
* 1 = 512 pixels wide
* 2 = 1000 pixels wide

### Screen_Colors()

```c
unsigned char Screen_Colors(void);
```

Returns the number of displayed colors in the current screen mode (2, 4, or 16).

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

### Screen_Width()

```c
unsigned short Screen_Width(void);
```

Returns the horizontal width of the screen, in pixels.

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

### Screen_Height()

```c
unsigned short Screen_Height(void);
```

Returns the vertical height of the screen, in pixels.

*SymbOS name*: `Device_ScreenMode` (`SCRGET`).

### Screen_Redraw()

```c
void Screen_Redraw(void);
```

Reinitializes the desktop background and redraws the entire screen.

*SymbOS name*: `DesktopService_RedrawComplete` (`DSK_SRV_DSKPLT`).

### Color_Get()

```c
unsigned short Color_Get(char color);
```

Retrieves the true palette color of the indexed color `color`.

*Return value*: A 12-bit color, with the components:

* `(value >> 8)` = red component (0 to 15)
* `(value >> 4) & 0x0F` = green component (0 to 15)
* `value & 0x0F` = blue component (0 to 15)

*SymbOS name*: `DesktopService_ColourGet` (`DSK_SRV_COLGET`).

### Color_Set()

```c
void Color_Set(char color, unsigned short value);
```

Sets the true palette color of the indexed color `color` to the 12-bit color `value`, which has the format:

* `(value >> 8)` = red component (0 to 15)
* `(value >> 4) & 0x0F` = green component (0 to 15)
* `value & 0x0F` = blue component (0 to 15)

*SymbOS name*: `DesktopService_ColourSet` (`DSK_SRV_COLSET`).

### Text_Width()

```c
unsigned short Text_Width(unsigned char bank, char* addr, int maxlen);
```

Returns the width (in pixels) of the text string at bank `bank`, address `address` if it were plotted with the current system font. `maxlen` contains the maximum number of characters to measure; if the text is terminated by character 0 or 13 (`\r`), use -1.

*SymbOS name*: `Screen_TextLength` (`TXTLEN`).

### Text_Height()

```c
unsigned short Text_Height(unsigned char bank, char* addr, int maxlen);
```

Returns the height (in pixels) of the text string at bank `bank`, address `address` if it were plotted with the current system font. `maxlen` contains the maximum number of characters to measure; if the text is terminated by character 0 or 13 (`\r`), use -1. (This function effectively just returns the pixel height of the system font, so we can just run it once with a dummy string like "A" and cache the result.)

*SymbOS name*: `Screen_TextLength` (`TXTLEN`).

## Mouse status

### Mouse_X()

```c
unsigned short Mouse_X(void);
```

Returns the horizontal position of the mouse pointer, in pixels.

*SymbOS name*: `Device_MousePosition` (`MOSGET`).

### Mouse_Y()

```c
unsigned short Mouse_Y(void);
```

Returns the vertical position of the mouse pointer, in pixels.

*SymbOS name*: `Device_MousePosition` (`MOSGET`).

### Mouse_Buttons()

```c
unsigned char Mouse_Buttons(void);
```

Returns the current status of the mouse buttons as a bitmask. We can perform a binary AND of the return value with `BUTTON_LEFT`, `BUTTON_RIGHT`, and `BUTTON_MIDDLE` to determine whether the respective button is currently pressed:

```c
lbut = Mouse_Buttons() & BUTTON_LEFT;
```

*SymbOS name*: `Device_MouseKeyStatus` (`MOSKEY`).

### Mouse_Dragging()

*Currently only available in development builds of SCC.*

```c
unsigned char Mouse_Dragging(unsigned char delay);
```

A utility function for determining whether a mouse click is the start of a drag-and-drop operation (or just a normal click). After detecting a click, call `Mouse_Dragging()` with the `delay` parameter indicating a short length of time to wait (in 100ths of a second, e.g., 50 = half a second). If the user releases the mouse within this time limit, this function immediately returns 0, indicating a single click. If time expires with the button still down, or the user moves the mouse without releasing the button, it immediately returns a nonzero bitmask (equivalent to [`Mouse_Buttons()`](#mouse_buttons)) indicating which buttons are being held down for a drag-and-drop operation.

*Return value*: 0 for a click, nonzero bitmask for a drag.

## Keyboard status

### Key_Down()

```c
unsigned char Key_Down(unsigned char scancode);
```

Returns 1 if the key specified by `scancode` is currently down, otherwise 0. **Note that keys are tested by *scancode*, not by their ASCII value!** A set of [scancode constants](s_ref.md#keyboard-scancodes) are provided for convenience.

*SymbOS name*: `Device_KeyTest` (`KEYTST`).

### Key_Status()

```c
unsigned short Key_Status(void);
```

Returns the status of the modifier keys as a bitmask. We can perform a binary AND of the return value with `SHIFT_DOWN`, `CTRL_DOWN`, `ALT_DOWN`, and `CAPSLOCK_DOWN` to determine whether the respective modifier key is currently applied:

```c
caps = Key_Status() & CAPSLOCK_DOWN;
```

*SymbOS name*: `Device_KeyStatus` (`KEYSTA`).

### Key_Put()

```c
void Key_Put(unsigned char keychar);
```

Pushes the ASCII code `keychar` into the keyboard buffer as if it had been pressed on the keyboard.

*SymbOS name*: `Device_KeyPut` (`KEYPUT`).

### Key_Multi()

```c
unsigned char Key_Multi(unsigned char scancode1, unsigned char scancode2,
                        unsigned char scancode3, unsigned char scancode4,
                        unsigned char scancode5, unsigned char scancode6);
```


Like `Key_Test()`, but tests up to six keys simultaneously. This may save time when testing large numbers of keys for (e.g.) a game. The return value is a bitmask:

* Bit 0: set if key `scancode1` is pressed
* Bit 1: set if key `scancode2` is pressed
* Bit 2: set if key `scancode3` is pressed
* Bit 3: set if key `scancode4` is pressed
* Bit 4: set if key `scancode5` is pressed
* Bit 5: set if key `scancode6` is pressed

**Note that keys are tested by *scancode*, not by their ASCII value!** A set of [scancode constants](s_ref.md#keyboard-scancodes) are provided for convenience.

*SymbOS name*: `Device_KeyMulti` (`KEYMUL`).

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

## System configuration

### Sys_Path()

*Currently only available in development builds of SCC.*

```c
char* Sys_Path(void);
```

Returns the system path (e.g., `C:\SYMBOS\`) as a string, which will always end in a backslash (`\`).

*SymbOS name*: `System_Information` (`SYSINF`).

### Sys_Type()

*Currently only available in development builds of SCC.*

```c
unsigned short Sys_Type(void);
```

Returns the type of machine on which SymbOS is running, one of: `TYPE_CPC`, `TYPE_ENTERPRISE`, `TYPE_MSX`, `TYPE_PCW`, `TYPE_NC`, `TYPE_SVM`, `TYPE_CPC464`, `TYPE_CPC664`, `TYPE_CPC6128`, `TYPE_CPC464PLUS`, `TYPE_CPC6128PLUS`, `TYPE_MSX1`, `TYPE_MSX2`, `TYPE_MSX2PLUS`, `TYPE_MSXTURBOR`, `TYPE_PCW8`, `TYPE_PCW9`, `TYPE_PCW16`, `TYPE_NC100`, `TYPE_NC150`, `TYPE_NC200`, `TYPE_SVM` (SymbOSVM), or `TYPE_OTHER` (anything undefined, included for forward-compatibility with potential future ports).

The general class of machine can be obtained by AND'ing this value with one of `TYPE_CPC`, `TYPE_MSX`, `TYPE_ENTERPRISE`, `TYPE_PCW`, `TYPE_NC`, or `TYPE_SVM`.

*SymbOS name*: `System_Information` (`SYSINF`).

### Sys_GetDrives()

*Currently only available in development builds of SCC.*

```c
void Sys_GetDrives(void* dest);
```

Loads available drives into `dest`, which must be an 8-member array of type `Device_Info` located in the **transfer** segment. `Device_Info` has the format:

```c
typedef struct {
    unsigned char letter;  // ASCII drive letter or 0 = undefined
    unsigned short config; // drive setup (see below)
    unsigned char unused;
    char name[12];         // device name, zero-terminated
} Device_Info;
```

The member `config` is a bitmask defining the system drive setup (partition number, etc.), which depends on the drive type; most of this is not relevant to user applications, so see the [SymbOS developer documentation](https://symbos.org/download.htm) for details. Example:

```c
_transfer Device_Info drives[8];
Sys_GetDrives(&drives);
```

*SymbOS name*: `System_Information` (`SYSINF`).

### Sys_DriveInfo()

*Currently only available in development builds of SCC.*

```c
void Sys_DriveInfo(char letter, Drive_Info* obj);
```

Loads information about the drive with ASCII drive letter `letter` into `obj`, a struct of type `Drive_Info` passed by reference:

```c
typedef struct {
    unsigned char status;      // status (see below)
    unsigned char type;        // medium type (see below)
    unsigned char removeable;  // 0 or 1
    unsigned char fs;          // filesystem (see below)
    unsigned char sectors;     // sectors per cluster
    unsigned long clusters;    // total clusters
} Drive_Info;
```

`status` may be one of `DRIVE_NONE`, `DRIVE_READY`, `DRIVE_NOTREADY`, or `DRIVE_CORRUPT`. `fs` may be one of `FS_AMSDOS_DATA`, `FS_AMSDOS_SYS`, `FS_PCW_180K`, `FS_FAT12`, `FS_FAT16`, or `FS_FAT32`. `type` may be one of:

* `TYPE_FLOPPY_SS` - single-sided floppy (Amstrad, PCW)
* `TYPE_FLOPPY_DS` - double-sided floppy (FAT12)
* `TYPE_RAMDRIVE` - RAM drive
* `TYPE_HARDDRIVE` - IDE/SCSI/SD/MMC mass storage drive

*SymbOS name*: `Directory_DriveInformation` (`DIRINF`).

### Sys_DriveFree()

*Currently only available in development builds of SCC.*

```c
unsigned long Sys_DriveFree(char letter);
```

Returns the total number of free 512-byte sectors on the drive with the ASCII drive letter `letter`. (Note that this calculation may take some time on a large FAT16 device.)

*SymbOS name*: `Directory_DriveInformation` (`DIRINF`).

### Sys_GetConfig()

*Currently only available in development builds of SCC.*

```c
void Sys_GetConfig(char* dest, unsigned short offset, unsigned char len);
```

A low-level function that copies `len` bytes starting at `offset` within the SymbOS configuration information into the buffer at address `dest`, which must be in the **transfer** segment. `offset` = 0 is byte 163 in `SYMBOS.INI` (i.e., the system path). For information on offsets and what information can be retrieved this way, see the SymbOS Developer Documentation.

*SymbOS name*: `System_Information` (`SYSINF`).
