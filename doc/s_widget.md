# Desktop widgets

**The features described below are currently only available in development builds of SCC.**

When "SymbOS extensions" are active, SymbOS allows the user to add "widgets" to their desktop (right-click > New > Widget). These are essentially normal SymbOS applications, but with certain specific behaviors expected by SymbOS.

Source code for a minimal example widget (`widget.c`) can be found in the `sample` directory.

In addition to `symbos.h`, the functions referenced below can be found in `symbos/widget.h`.

## Contents

* [Compiling as a widget](#compiling-as-a-widget)
* [Widget windows](#widget-windows)
* [Message flow](#message-flow)

## Compiling as a widget

Widgets have the file extension `.wdg` and contain additional header information. To compile as a widget, use the `-w` command-line option in `cc`. This option takes a list of numbers separated by commas, with the format:

```bash
flags,width1,height1[,width2,height2[,width3,height3[,width4,height4]]]
```

`flags` can be either 1 (indicating that the widget has a "properties" dialog---see below) or 0 (indicating that it does not). The pairs of "width" and "height" represent the possible sizes of the widget, in pixels; the user will be able to select these sizes from a list. For example, to compile `widget.c` as a widget with a properties dialog and two possible sizes, 75x40 and 100x50:

```bash
cc widget.c -w 1,75,40,100,50
```

## Widget windows

Rather than defining their own window, widgets use a single window that is managed by the desktop. We only define the controls to display in this window, using a [control collection](s_ctrl.md#C_COLLECTION). For example, the following code (from the `widget.c` sample) defines a control collection with two controls, a black `C_AREA` control to fill in the background and a `C_TEXT` control to draw some text:

```c
// control extended data records
_transfer Ctrl_Text cd_text1 = {"Hello world", (COLOR_YELLOW << 2) | COLOR_BLACK, ALIGN_LEFT};

// control data records
_transfer Ctrl c_area = {0, C_AREA, -1, COLOR_BLACK, 0, 0, 10000, 10000};
_transfer Ctrl c_text1 = {1, C_TEXT, -1, (unsigned short)&cd_text1, 4, 4, 100, 8};

// control group and collection
_transfer Ctrl_Group ctrls = {2, 0, &c_area};
_transfer Ctrl_Collection ctrls_col = {&ctrls, 10000, 10000, 0, 0, CSCROLL_NONE};
```

(See [SymbOS Programming](symbos.md) and the [Control Reference](s_ctrl.md) to review how control groups and control collections work.)

We tell SymbOS what control collection to use with the `Widget_Init()` function; see the next section.

## Message flow

Event messages sent to a widget are similar to (but slightly different than) those sent to a normal app. The most important message is `MSC_WDG_SIZE`, which alerts the widget that it has been opened or resized. We should handle this message using the function `Widget_Init()`, which performs the necessary setup:

```c
void Widget_Init(Ctrl_Collection* col, char* msg, unsigned char send_pid);
```

`col` is the address of the widget's main control collection (see above), `msg` is the buffer containing the `MSC_WDG_SIZE` message we just received, and `send_pid` is the process ID of that sent us the message. (We can get this from the return value of [`Msg_Sleep()`](s_core.md#msg_sleep).) 

In addition, we must handle the following messages correctly:

* Message 0 (`msg[0] == 0`) is a request for the widget to close itself.
* Most user interactions (but not all) will arrive as `MSC_WDG_CLICK` messages rather than `MSR_DSK_WCLICK` messages, so we must watch for both.
* `MSC_WDG_PROP` indicates that the user has clicked the "Properties" menu option. (This can only happen if we have compiled the widget with `flags` = 1; see above.) What we do with this information is up to us, but generally we would open some kind of settings window.

A complete event-loop skeleton for a widget is a follows (from the `widget.c` sample):

```c
int main(int argc, char *argv[]) {
    unsigned short resp;
    unsigned char sender_pid;

	// event loop
	while (1) {
		resp = Msg_Sleep(_sympid, -1, _symmsg);
		if (resp & 1) { // check if we actually received a message
            sender_pid = resp >> 8; // get sender PID from high byte of resp

            if (_symmsg[0] == 0) {
                // message 0 = request to close
                exit(0);

            } else if (_symmsg[0] == MSR_DSK_WCLICK || _symmsg[0] == MSC_WDG_CLICK) {
                // normal window interaction
                switch (_symmsg[2]) {
                    case DSK_ACT_CLOSE:
                        // request to close
                        exit(0);

                    case DSK_ACT_CONTENT:
                        // user interacted with widget controls
                        // ...
                        break;
                }

            } else if (_symmsg[0] == MSC_WDG_SIZE) {
                // widget was loaded or resized
                Widget_Init(&ctrls_col, _symmsg, sender_pid);

            } else if (_symmsg[0] == MSC_WDG_PROP) {
                // user asked to open properties window
                // ...
            }
		}
	}
}
```

## Managing the window

In addition to setting up the window, `Widget_Init()` sets the following global variables:

* `_widget_win` - the window ID in which the widget's control collection is contained.
* `_widget_col` - the control ID of the collection within this window.
* `_widget_w` - the current widget width, in pixels.
* `_widget_h` - the current widget height, in pixels.

We need `_widget_win` and `_widget_col` to redraw controls with [`Win_Redraw_Sub()`](s_window.md#win_redraw_sub) when they change:

```c
Win_Redraw_Sub(_widget_win, _widget_col, 3);
```

