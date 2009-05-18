; installer.nsi
;
; NSIS installer script to make Daimonin Client windoze installer.
; By Alex Tokar (http://www.atokar.net/), April 13 2009
; To make the installer, place this script in your client
; directory and run makensis client_installer.nsi

;--------------------------------

; The name of the installer
Name "Daimonin B5 Client"

; The file to write
OutFile "Daimonin-B5.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Daimonin\client-0.10.0

; License file
LicenseData License

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Daimonin-B5" "Install_Dir"

; Branding text
BrandingText "Created by ATokar.net Dev Team"

;--------------------------------

; Pages
Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Client (required)"
  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Files
  File "daimonin_start.exe"
  File "daimonin.exe"
  File "*.dll"
  File "License"
  File "*.dat"
  File "README.txt"
  File "daimonin.p0"
  ; cache directory
  CreateDirectory $INSTDIR\cache
  SetOutPath $INSTDIR\cache
  File "cache\*.*"
  ; gfx_user directory
  CreateDirectory $INSTDIR\gfx_user
  SetOutPath $INSTDIR\gfx_user
  File "gfx_user\*.*"
  ; logs directory
  CreateDirectory $INSTDIR\logs
  SetOutPath $INSTDIR\logs
  File "logs\*.*"
  ; media directory
  CreateDirectory $INSTDIR\media
  SetOutPath $INSTDIR\media
  File "media\*.*"
  ; settings directory
  CreateDirectory $INSTDIR\settings
  SetOutPath $INSTDIR\settings
  File "settings\*.*"
  ; skins directory
  CreateDirectory $INSTDIR\skins
  SetOutPath $INSTDIR\skins
  File "skins\*.*"
  ; skins/subred directory
  CreateDirectory $INSTDIR\skins\subred
  SetOutPath $INSTDIR\skins\subred
  File "skins\subred\*.*"
  ; skins/subred/bitmaps directory
  CreateDirectory $INSTDIR\skins\subred\bitmaps
  SetOutPath $INSTDIR\skins\subred\bitmaps
  File "skins\subred\bitmaps\*.*"
  ; skins/subred/icons directory
  CreateDirectory $INSTDIR\skins\subred\icons
  SetOutPath $INSTDIR\skins\subred\icons
  File "skins\subred\icons\*.*"
  ; srv_files directory
  CreateDirectory $INSTDIR\srv_files
  SetOutPath $INSTDIR\srv_files
  File "srv_files\*.*"
  ; update directory
  CreateDirectory $INSTDIR\update
  SetOutPath $INSTDIR\update
  File "update\*.*"
  ; sfx directory
  CreateDirectory $INSTDIR\sfx
  SetOutPath $INSTDIR\sfx
  File "sfx\*.*"
  ; Revert back to installation directory, otherwise we will get shortcuts linking to the update directory.
  SetOutPath $INSTDIR
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Daimonin-B5 "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for windoze
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-B5" "DisplayName" "Daimonin B5 Client"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-B5" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-B5" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-B5" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Daimonin-B5"
  CreateShortCut "$SMPROGRAMS\Daimonin-B5\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Daimonin-B5\Daimonin B5 Client.lnk" "$INSTDIR\daimonin_start.exe" "" "$INSTDIR\daimonin_start.exe" 0
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-B5"
  DeleteRegKey HKLM SOFTWARE\Daimonin-B5

  ; Remove shortcuts, if any
  RMDir /r /REBOOTOK "$SMPROGRAMS\Daimonin-B5"

  RMDir /r /REBOOTOK "$INSTDIR"
SectionEnd
; EOF

