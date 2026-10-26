# Wall Clock

Wall Clock is a full screen clock display for Windows and Linux. It shows UTC time
on the top half of the screen and local time on the bottom half, each with
the day of the week and the date, in large red seven-segment digits.

While the clock window is open the program keeps the display on, so the
screen saver and the idle screen lock do not start. That makes it suitable
for a wall display or an operations room monitor that nobody touches.
Closing the program returns the computer to its normal power settings.

![Wall Clock icon](icons/wall_clock.png)

## Using it

    wall_clock.exe [--fullscreen] [URL]      (Windows)
    wall_clock [--fullscreen] [URL]          (Linux)

| Key | Action |
| --- | --- |
| F11 or double click | Toggle full screen |
| Esc | Leave full screen |
| F5 | Reload the page |

The installer adds a "Wall Clock (full screen)" Start menu shortcut that
starts with `--fullscreen`. Put it in the Startup folder to start the
clock when a display computer logs in.

The clock face is the web page `html\index.html` in the program folder.

## Showing another page

To show a different page, set `url=` in `wall_clock.ini` in the program
folder (for example `C:\Program Files\Wall Clock\wall_clock.ini`) and
restart Wall Clock:

    [clock]
    url=https://example.com/clock/
    fullscreen=1

`url` can be a web address, a file path, or a path relative to the
program folder. Leave it empty for the built-in clock. `fullscreen=1`
starts in full screen mode. The installer lets ordinary users edit this
file and keeps it on upgrades.

A URL or `--fullscreen` on the command line overrides the INI file, so a
shortcut can still open a different page.

A group policy that forces the screen to lock after a set time
(`InactivityTimeoutSecs`) can still lock the session; the program only
stops the idle timers that it is allowed to.

On Linux the program asks the desktop not to blank or lock the screen
through D-Bus: the GNOME session manager (RHEL 9 and 10, X11 or Wayland)
or, on other desktops, the freedesktop screen saver interface (KDE, Xfce
and others). The request ends when the program closes. A locked-down
lock policy can still lock the screen.

## Requirements

* Windows 10 or 11, 64-bit.
* The Microsoft Edge WebView2 Runtime, which is part of Windows 11 and
  current Windows 10. Without it the program falls back to the older
  Internet Explorer web view.

Linux:

* RHEL 9 or 10 (or Rocky/Alma), with the EPEL repository for wxWidgets.
* A graphical desktop. GNOME, the RHEL default, is tested.

## Installing on Linux

RPMs for RHEL 9 and 10 are on the download page. They need EPEL for
wxWidgets:

    sudo dnf install epel-release
    sudo dnf install ./wall_clock-2.2-1.el9.x86_64.rpm     (or .el10)

## Building on Linux

    sudo dnf install epel-release
    sudo dnf install gcc-c++ make wxGTK-devel
    make -f makefile.linux
    sudo make -f makefile.linux install

This installs the program, the page and `wall_clock.ini` in
`/opt/wall_clock`, a `wall_clock` link in `/usr/local/bin`, and a
"Wall Clock" desktop menu entry that starts in full screen mode.
`PREFIX=` changes the program folder. Reinstalling keeps an edited
`wall_clock.ini`.

To build the RPM instead: `sudo dnf install rpm-build`, then
`rpmbuild -ta wall_clock-2.2.tar.gz`.

On Linux each user can also set the page in
`~/.config/wall_clock/wall_clock.ini`, which wins over the one in
`/opt/wall_clock`. To start the clock at login, copy
`/usr/share/applications/wall_clock.desktop` to `~/.config/autostart/`.

## Building on Windows

Wall Clock is built with Visual Studio 2022 or 2026 (MSVC) and a static
release build of wxWidgets 3.2 with the Edge web view turned on.

1. Unpack wxWidgets 3.2 (for example to `E:\3plibs\wxWidgets64-3.2.4-edge`).
2. Download the `Microsoft.Web.WebView2` NuGet package
   (https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2), which is
   a zip file, and unpack it to `3rdparty\webview2` in the wxWidgets tree.
3. In `include\wx\msw\setup.h` set `wxUSE_WEBVIEW_EDGE` and
   `wxUSE_WEBVIEW_EDGE_STATIC` to 1.
4. From an x64 developer command prompt:

        cd build\msw
        nmake -f makefile.vc TARGET_CPU=X64 BUILD=release RUNTIME_LIBS=static

5. Build Wall Clock from this folder:

        nmake WXDIR=E:\3plibs\wxWidgets64-3.2.4-edge

`wall_clock.exe` has no DLL dependencies besides Windows itself. To make
`output\wall_clock_setup_2.2.exe`, install Inno Setup 6 and run
`nmake installer`.

## Icon

`icons\wall_clock.ico` is drawn by `tools/make_icon.py` from simple shapes,
so it is original artwork. To redraw it and `icons/wall_clock.png` (needs Python 3 and Pillow):

    python3 tools/make_icon.py

## License

GNU General Public License version 3, see `LICENSE`.
