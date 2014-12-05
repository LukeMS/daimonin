; client_installer.nsi
;
; NSIS installer script to make Daimonin Client windoze installer.
; By Alex Tokar (http://www.atokar.net/), April 13 2009
; To make the installer, place this script in your client
; directory and run, eg:
;   makensis.exe /DFLAVOUR="dev" /DVERSION="0.10.1" client_installer.nsi
; (See below for the commandline defines.)

;--------------------------------

; These are failsafes and should be set on the commandline with the /D switch:
; FLAVOUR should be one of "dev" or "main".
; VERSION should be the correct "x.y.z".
!define \ifndef FLAVOUR "dummy"
!define \ifndef VERSION "?.?.?"

; The name of the installer
Name "Daimonin Client (${FLAVOUR})"

; The file to write
OutFile "Daimonin-${FLAVOUR}-${VERSION}.exe"

; The default installation directory
; We need a warning to users not to install to \Program Files\ etc as this
; will prevent the updater working in Vista or later.
InstallDir C:\Daimonin\client-${FLAVOUR}

; License file
LicenseData License

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Daimonin-${FLAVOUR}" "Install_Dir"

; Branding text
BrandingText "Created by Daimonin Dev Team"

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
;  File "daimonin_start.exe"
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
  ; man directory
  CreateDirectory $INSTDIR\man
  ;SetOutPath $INSTDIR\man
  ;File "man\*.*"
  ; man/commands directory
  CreateDirectory $INSTDIR\man\commands
  SetOutPath $INSTDIR\man\commands
  File "man\commands\*.*"
  ; media directory
  CreateDirectory $INSTDIR\media
  SetOutPath $INSTDIR\media
  File "media\*.*"
  ; settings directory
  CreateDirectory $INSTDIR\settings
  SetOutPath $INSTDIR\settings
  File "settings\*.*"
  ; bitmaps directory
  CreateDirectory $INSTDIR\bitmaps
  SetOutPath $INSTDIR\bitmaps
  File "bitmaps\*.*"
  ; icons directory
  CreateDirectory $INSTDIR\icons
  SetOutPath $INSTDIR\icons
  File "icons\*.*"
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
  WriteRegStr HKLM SOFTWARE\Daimonin-${FLAVOUR} "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for windoze
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-${FLAVOUR}" "DisplayName" "Daimonin Client (${FLAVOUR})"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-${FLAVOUR}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-${FLAVOUR}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-${FLAVOUR}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Daimonin-${FLAVOUR}"
  CreateShortCut "$SMPROGRAMS\Daimonin-${FLAVOUR}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
;  CreateShortCut "$SMPROGRAMS\Daimonin-${FLAVOUR}\Daimonin Client (${FLAVOUR}).lnk" "$INSTDIR\daimonin_start.exe" "" "$INSTDIR\daimonin_start.exe" 0
  CreateShortCut "$SMPROGRAMS\Daimonin-${FLAVOUR}\Daimonin Client (${FLAVOUR}).lnk" "$INSTDIR\daimonin.exe" "" "$INSTDIR\daimonin.exe" 0
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Daimonin-${FLAVOUR}"
  DeleteRegKey HKLM SOFTWARE\Daimonin-${FLAVOUR}

  ; Remove shortcuts, if any
  RMDir /r /REBOOTOK "$SMPROGRAMS\Daimonin-${FLAVOUR}"

  RMDir /r /REBOOTOK "$INSTDIR"
SectionEnd
; EOF

