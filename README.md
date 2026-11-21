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

    wall_clock.exe [--fullscreen] [--timezone ZONE] [URL]      (Windows)
    wall_clock [--fullscreen] [--timezone ZONE] [URL]          (Linux)

| Key | Action |
| --- | --- |
| F11 or double click | Toggle full screen |
| Esc | Leave full screen |
| F5 | Reload the page |
| Z, or click the zone name | Pick the bottom panel's time zone |
| A, or click "Alarms" (top right) | Set alarms and reminders |

The top panel shows UTC. The bottom panel shows the computer's time zone,
or any other time zone: press Z (or click the zone name under the bottom
clock), type part of a name such as `Chicago` or `Tokyo`, and pick it.
The choice is saved as `timezone=` in `wall_clock.ini`, and
`--timezone America/Chicago` on the command line overrides it.

Alarms and reminders: press A (or click "Alarms" in the top right corner), add a time and an optional label, and
light the day buttons for the days it repeats (no day lit rings once).
Alarm times use the bottom panel's time zone. When an alarm goes off the
screen flashes, the label shows in large letters, the clock beeps and
comes to the front until you press Stop (or any key) or Snooze (S, five
minutes). The list is saved in `wall_clock.ini` under `[alarms]`.

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
through D-Bus: the GNOME session manager (RHEL 8, 9 and 10, X11 or Wayland)
or, on other desktops, the freedesktop screen saver interface (KDE, Xfce
and others). The request ends when the program closes. A locked-down
lock policy can still lock the screen.

## Requirements

* Windows 10 or 11, 64-bit.
* The Microsoft Edge WebView2 Runtime, which is part of Windows 11 and
  current Windows 10. Without it the program falls back to the older
  Internet Explorer web view.

Linux:

* RHEL 8, 9 or 10 (or Rocky/Alma). RHEL 9 and 10 need the EPEL repository
  for wxWidgets; the RHEL 8 RPM has wxWidgets built in.
* A graphical desktop. GNOME, the RHEL default, is tested.

## Installing on Linux

RPMs for RHEL 8, 9 and 10 are on the download page. The RHEL 9 and 10
RPMs need EPEL for wxWidgets:

    sudo dnf install epel-release
    sudo dnf install ./wall_clock-2.4-1.el9.x86_64.rpm     (or .el10)

The RHEL 8 RPM has wxWidgets 3.2 linked in (EPEL 8 only has 3.0, which
is too old) and needs only the RHEL repositories:

    sudo dnf install ./wall_clock-2.4-1.el8.x86_64.rpm

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
`rpmbuild -ta wall_clock-2.4.tar.gz`.

On RHEL 8, build the RPM with a private static wxWidgets 3.2.4 (no EPEL):

    sudo dnf install rpm-build gcc-c++ make gtk3-devel webkit2gtk3-devel libSM-devel
    mkdir -p ~/rpmbuild/SOURCES
    cp wall_clock-2.4.tar.gz wxWidgets-3.2.4.tar.bz2 ~/rpmbuild/SOURCES/
    tar xzf wall_clock-2.4.tar.gz --strip-components=1 wall_clock-2.4/wall_clock.spec
    rpmbuild -bb wall_clock.spec

wxWidgets-3.2.4.tar.bz2 is on the wxWidgets GitHub releases page.

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
`output\wall_clock_setup_2.4.exe`, install Inno Setup 6 and run
`nmake installer`.

## Icon

`icons\wall_clock.ico` is drawn by `tools/make_icon.py` from simple shapes,
so it is original artwork. To redraw it and `icons/wall_clock.png` (needs Python 3 and Pillow):

    python3 tools/make_icon.py

## License

GNU General Public License version 3, see `LICENSE`.
