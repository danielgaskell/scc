# Event reference

This section contains a reference to all event messages sent by the desktop service (button clicks, etc.). For details on how to trap and respond to these events, see [SymbOS Programming](symbos.md).

### MSR_DSK_WCLICK

The primary event sent for most interactions with a window's controls.

* `msg[0]`: `MSR_DSK_WCLICK`
* `msg[1]`: Window ID
* `msg[2]`: Action type, one of:
    * `DSK_ACT_CLOSE`: Close button has been clicked, or the user has typed Alt+F4.
    * `DSK_ACT_MENU`: A menu option has been clicked, with:
        * `msg[8]` = Value of the clicked menu entry
    * `DSK_ACT_CONTENT` = A control has been clicked or modified, with:
        * `msg[3]`: Sub-action, one of:
            * `DSK_SUB_MLCLICK`: Left mouse button clicked
            * `DSK_SUB_MRCLICK`: Right mouse button clicked
            * `DSK_SUB_MDCLICK`: Left mouse button double clicked
            * `DSK_SUB_MMCLICK`: Middle mouse button clicked
            * `DSK_SUB_KEY`: Key pressed, with key ASCII value in `msg[4]`
        * `*(int*)&msg[4]` = Mouse X position relative to window content
        * `*(int*)&msg[6]` = Mouse y position relative to window content
        * `msg[8]` = control ID
    * `DSK_ACT_TOOLBAR`: Equivalent to `DSK_ACT_CONTENT`, but for controls in the toolbar.
    * `DSK_ACT_KEY`: A key has been pressed without modifying any control:
        * `msg[4]` = key ASCII value

### MSR_DSK_WFOCUS

The focus status of a window has changed.

* `msg[0]`: `MSR_DSK_WFOCUS`
* `msg[1]`: Window ID
* `msg[2]`: Event type, one of:
    * 0 = window lost focus
    * 1 = window received focus

### MSR_DSK_CFOCUS

Sent when the focus status of a control has changed.

* `msg[0]`: `MSR_DSK_CFOCUS`
* `msg[1]`: Window ID
* `msg[2]`: Number of newly focused control (not the control ID/value, but an index starting from 1)
* `msg[3]`: Event type, one of:
    * 0 = User navigated with mouse
    * 1 = User navigated with Tab key
    
### MSR_DSK_WRESIZ

A window has been resized by the user (including maximizing or restoring a maximized or minimized window). The new window size can be determined using [`Win_Width()`](s_window.md#win_width) and [`Win_Height()`](s_window.md#win_height). (We can also read the `h` and `w` properties of the window directly, but these may not match the actual window size in all cases, e.g., when the window is maximized.)

* `msg[0]`: `MSR_DSK_WRESIZ`
* `msg[1]`: Window ID

### MSR_DSK_WSCROLL

The user has scrolled the main window content of a window. The new scroll position can be read from the window's `xscroll` and `yscroll` properties.

* `msg[0]`: `MSR_DSK_WSCROLL`
* `msg[1]`: Window ID

### MSR_DSK_MENCTX

The user has clicked or cancelled an open context menu.

* `msg[0]`: `MSR_DSK_MENCTX`
* `msg[1]`: Event type, one of:
    * 0 = menu cancelled
    * 1 = entry clicked
* `*(int*)&msg[2]`: Value associated with the clicked entry
* `msg[4]`: Menu entry type, one of:
    * 0 = normal entry
    * 1 = checked entry
    
### MSR_DSK_EVTCLK

The user has clicked a system tray icon associated with this application.

* `msg[0]`: `MSR_DSK_EVTCLK`
* `msg[1]`: Value associated with the system tray icon
* `msg[2]`: Mouse button pressed, one of:
    * `SYSTRAY_LEFT` = left click
    * `SYSTRAY_RIGHT` = right click
    * `SYSTRAY_DOUBLE` = double left click
    
### MSR_DSK_WMODAL

The user has clicked a window that is modal and cannot be focused. (This is useful for creating windows that disappear if the user clicks the main window.)

* `msg[0]`: `MSR_DSK_WMODAL`
* `msg[1]`: Modal window ID

