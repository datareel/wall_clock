; Wall Clock installer (Inno Setup 6)
;
; Build the program first (nmake), then:
;   "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" wall_clock_installer.iss
; or: nmake installer

#define AppVersion "2.0"

[Setup]
AppId={{3B6C1F0E-8D52-4C2A-9E1B-6F4A7D20C915}
AppName=Wall Clock
AppVersion={#AppVersion}
AppVerName=Wall Clock {#AppVersion}
AppPublisher=DataReel Software Development
AppPublisherURL=https://datareel.com
DefaultDirName={autopf}\Wall Clock
DefaultGroupName=Wall Clock
DisableProgramGroupPage=yes
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputBaseFilename=wall_clock_setup_{#AppVersion}
OutputDir=output
LicenseFile=LICENSE
SetupIconFile=icons\wall_clock.ico
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
UninstallDisplayName=Wall Clock
UninstallDisplayIcon={app}\wall_clock.exe

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "wall_clock.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "html\*"; DestDir: "{app}\html"; Flags: ignoreversion recursesubdirs
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Wall Clock"; Filename: "{app}\wall_clock.exe"
Name: "{group}\Wall Clock (full screen)"; Filename: "{app}\wall_clock.exe"; Parameters: "--fullscreen"
Name: "{autodesktop}\Wall Clock"; Filename: "{app}\wall_clock.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\wall_clock.exe"; Description: "{cm:LaunchProgram,Wall Clock}"; Flags: nowait postinstall skipifsilent
