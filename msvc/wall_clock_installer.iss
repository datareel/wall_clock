; Demonstrates copying 3 files and creating an icon.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=NHC OPS Clock
AppVersion=2026.201
WizardStyle=modern dynamic
DefaultDirName={autopf}\NHC OPS Clock
DefaultGroupName=NHC OPS Clock
UninstallDisplayIcon={app}nhc_ops_clock.exe
Compression=lzma2
SolidCompression=yes
OutputDir=E:\dgaer_dev\cpp\wxwin.nws\nhc_ops_clock_windows\msvc

[Files]
Source: "E:\dgaer_dev\cpp\wxwin.nws\nhc_ops_clock_windows\msvc\nhc_ops_clock.exe"; DestDir: "{app}"
Source: "E:\dgaer_dev\cpp\wxwin.nws\nhc_ops_clock_windows\msvc\nhc_ops_clock.exe.manifest"; DestDir: "{app}"
Source: "E:\dgaer_dev\cpp\wxwin.nws\nhc_ops_clock_windows\html\*"; DestDir: "C:\www\html\clock"; Flags: recursesubdirs

[Icons]
Name: "{group}\NHC OPS Clock"; Filename: "{app}\nhc_ops_clock.exe"
