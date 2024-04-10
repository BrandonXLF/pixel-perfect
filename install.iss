[Setup]
AppName = Pixel Perfect
AppVersion = 1.0.0
DefaultDirName = {autopf}\PixelPerfect
OutputBaseFilename = Pixel Perfect Installer
PrivilegesRequired = lowest
ArchitecturesInstallIn64BitMode = x64
DefaultGroupName = PixelPerfect
SetupIconFile = media\icon.ico
UninstallDisplayName = Pixel Perfect
UninstallDisplayIcon = {app}\pixel-perfect.exe,0
OutputDir = x64
AppPublisher = Brandon Fowler
WizardImageStretch = no

[Types]
Name: "standard"; Description: "Standard installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: desktop; Description: Create a desktop icon; Types: standard
Name: start; Description: Add to start menu; Types: standard

[Files]
Source: "x64\Release\pixel-perfect.exe"; DestDir: "{app}"; DestName: pixel-perfect.exe

[Icons]
Name: "{commondesktop}\Pixel Perfect"; Filename: "{app}\pixel-perfect.exe"; Components: desktop; Check: IsAdminLoggedOn
Name: "{userdesktop}\Pixel Perfect"; Filename: "{app}\pixel-perfect.exe"; Components: desktop; Check: not IsAdminLoggedOn
Name: "{group}\Pixel Perfect"; Filename: "{app}\pixel-perfect.exe"; Components: start;

[Run]
Filename: "{app}\pixel-perfect.exe"; Description: "Launch Pixel Perfect"; Flags: postinstall

