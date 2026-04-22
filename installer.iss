[Setup]
AppName=Lua Patcher
AppVersion=1.3.6
AppPublisher=leVi Studios
AppPublisherURL=https://github.com/sayedalimollah2602-prog
DefaultDirName={autopf}\Lua Patcher
DefaultGroupName=Lua Patcher
UninstallDisplayIcon={app}\LuaPatcher.exe
Compression=zip
SolidCompression=no
OutputDir=dist_setup
OutputBaseFilename=LuaPatcher_Setup

[Files]
Source: "build\LuaPatcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "logo.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Lua Patcher"; Filename: "{app}\LuaPatcher.exe"
Name: "{autodesktop}\Lua Patcher"; Filename: "{app}\LuaPatcher.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Run]
Filename: "{app}\LuaPatcher.exe"; Description: "Launch Lua Patcher"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Delete the settings when uninstalled (Wipes login session)
Type: filesandordirs; Name: "{userappdata}\leVi Studios\LuaPatcher"
