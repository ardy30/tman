# Frequently Asked Qestions #

---


**How can I set custom colors for the background and foreground of the task list?**

Create the following keys in the registry (`HKCU\Software\DaProfik\tMan`):
  * `ClrMenuBg [DWORD]`: color of the task list background.
  * `ClrMenuFg [DWORD]`: color of the task list text.
  * `ClrMenuHiBg [DWORD]`: color of the hilight bar background of the task list.
  * `ClrMenuHiFg [DWORD]`: color of the hilight bar text of the task list.

Color format is the following: 0xBBGGRR (hex). Thus, if you want a blue text, insert 0xFF0000 (hex) for `ClrMenuFg`.

**How do I disable the soft-reset confirmation dialog?**

Create a registry key `ConfirmSoftReset [DWORD]` in `HKCU\Software\DaProfik\tMan` and set its value to `0` (zero).

**I do not want WM5 styled buttons on my WM5 device. How can I switch to old styled buttons?**

Create a registry key `OldStyleButton [DWORD]` in `HKCU\Software\DaProfik\tMan` and set its value to `1` (one).

**How can I disable the tMan's dot in top bar?**

Create a registry key `ShowDot [DWORD]` in `HKCU\Software\DaProfik\tMan` and set its value to `0` (zero).

**How to make WiFi On/Off work?**

You need WiFi device name. Look for it in `HKLM\Comm` (mine is for example `TIACXWLN1`, but YMMV).

Create a registry key named `WiFiDevice [STRING]` in `HKCU\Software\DaProfik\tMan` and set its value to the name you have found in the previous step.

**Alt+F4 is not working for me.**

This feature had pretty nasty side-effects, so it was removed...

**WiFi On/Off is working for me, but after soft-reset, it does not work any more, unless I use the built-in plugin for WiFi from my OEM.**

Yes. This is known behavior. I tried to fix this problem, but with no success. I do not know the source of this problem. If you do, let me know.