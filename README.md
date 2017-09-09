# Font-Scaling Control

This repository contains an ATL-based class, `CFontScalingControl`, that responds to `WM_SETFONT`
messages and automatically scales the window's children up or down to maintain their sizes and
positions relative to the metrics of the old font.

To use this class in your code:

1. Add the [`include`](./include) directory to your header-search paths, and include `CFontScalingDialog.h` in your code.
1. Make your `CWindow`/`CDialog` subclass inherit from `CFontScalingControl<T>` in addition to its other superclasses. The type parameter needs to be the name of the class you are including this code in (so the class can get to its `HWND`).
1. Add `CHAIN_MSG_MAP(CFontScalingControl<T>)` to the _top_ of your message map (where `T` is the name of the class you are including this code in).

Once a `WM_SETFONT` message is sent to the window (from any source), the font-scaling code will trigger.
A utility function that gets an `HFONT` from the system message-box font is provided for your convenience,
as that is the font recommended by the Windows user-interface guidelines.
