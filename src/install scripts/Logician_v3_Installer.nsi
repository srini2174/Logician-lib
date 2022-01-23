; DecisionLogic_Installer.nsi
;
; 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install hte Logician Suite into a directory that the user selects,

;--------------------------------
!include x64.nsh

; The name of the installer
Name "Logician Suite 3.0.0"

; The file to write
OutFile "Logician_v300_Setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Logician

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Logician" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------


; The stuff to install
Section "Visual C++ 2013 Runtime"
	; VS2013 Redist
	SetOutPath $TEMP
	File "vcredist_x86.exe"
	File "vcredist_x64.exe"
	
${If} ${RunningX64}
		ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\12.0\VC\Runtimes\x64" "Installed"
		StrCmp $1 1 installed64VCRuntime
        ExecWait '"vcredist_x64.exe" /install /passive /norestart'
		
		installed64VCRuntime:
		
		ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\12.0\VC\Runtimes\x86" "Installed"
		StrCmp $1 1 installedVCRuntime
		ExecWait '"vcredist_x86.exe" /install /passive /norestart'
${Else}
		ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\12.0\VC\Runtimes\x86" "Installed"
		StrCmp $1 1 installedVCRuntime
        ExecWait '"vcredist_x86.exe" /install /passive /norestart'
${EndIf}

	installedVCRuntime:
	
	Delete /REBOOTOK "vcredist_x86.exe"
	Delete /REBOOTOK "vcredist_x64.exe"	
SectionEnd

Section "Logician Suite"
	; Set output path to the installation directory.
		
	SetOutPath $INSTDIR\VC12
	File "..\tags\v3.0.0\VC12\EDSEngine.lib"
	File "..\tags\v3.0.0\VC12\EDSEngineNET.dll"
	File "..\tags\v3.0.0\VC12\RelationalObjectModel.lib"
	File "..\tags\v3.0.0\VC12\ROMNET.dll"
	File "..\tags\v3.0.0\VC12\LogicianDebuggerWPF.dll"
	File "..\tags\v3.0.0\VC12\WPFToolkit.dll"
	SetOutPath $INSTDIR\VC12\x64
	File "..\tags\v3.0.0\VC12\x64\EDSEngine.lib"
	File "..\tags\v3.0.0\VC12\x64\EDSEngineNET.dll"
	File "..\tags\v3.0.0\VC12\x64\RelationalObjectModel.lib"
	File "..\tags\v3.0.0\VC12\x64\ROMNET.dll"
	File "..\tags\v3.0.0\VC12\x64\LogicianDebuggerWPF.dll"
	File "..\tags\v3.0.0\VC12\x64\WPFToolkit.dll"

	; Set output path to the installation directory.
	SetOutPath $INSTDIR\DecisionLogic
	; Put file there
	File "..\tags\v3.0.0\VC12\DecisionLogic.exe"
	File "..\tags\v3.0.0\DecisionLogic\vc_mswu\DecisionLogicHelp.htm"
	File "..\tags\v3.0.0\DecisionLogic\vc_mswu\Figure1.png"
	File "..\tags\v3.0.0\DecisionLogic\vc_mswu\Figure2.png"
	File "..\tags\v3.0.0\DecisionLogic\vc_mswu\Figure3.png"
	File "..\tags\v3.0.0\DecisionLogic\vc_mswu\Figure4.png"
	File "..\tags\v3.0.0\DecisionLogic\vc_mswu\Figure5.png"
	File "..\tags\v3.0.0\DecisionLogic\vc_mswu\Figure6.png"
	
	SetOutPath $INSTDIR
	File "..\tags\v3.0.0\LogicianJS\KnowledgeBase.js"
	File "..\tags\v3.0.0\LogicianJS\ROMNode.js"
	
	SetOutPath $INSTDIR\ajaxslt
	File "..\tags\v3.0.0\LogicianJS\ajaxslt\*.*"	
	
	SetOutPath $INSTDIR
	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\DecisionLogic "Install_Dir" "$INSTDIR"
	
	; Write the file association for DecisionLogic to registry
	WriteRegStr HKCR .dlp "" "DecisionLogicProject"
	WriteRegStr HKCR DecisionLogicProject\shell\open\command "" '$INSTDIR\DecisionLogic\DecisionLogic.exe "%1"'
	WriteRegStr HKCR DecisionLogicProject\DefaultIcon "" '$INSTDIR\DecisionLogic\DecisionLogic.exe'
	
	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Logician" "DisplayName" "Logician Libraries"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Logician" "UninstallString" '"$INSTDIR\uninstall_logician_libraries.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Logician" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Logician" "NoRepair" 1
	WriteUninstaller "uninstall_logician_libraries.exe"

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
	CreateDirectory "$SMPROGRAMS\DecisionLogic"
	CreateShortCut "$SMPROGRAMS\DecisionLogic\Uninstall.lnk" "$INSTDIR\uninstall_logician_libraries.exe" "" "$INSTDIR\DecisionLogic\uninstall_logician_libraries.exe" 0
	SetOutPath $INSTDIR\DecisionLogic
	CreateShortCut "$SMPROGRAMS\DecisionLogic\DecisionLogic.lnk" "$INSTDIR\DecisionLogic\DecisionLogic.exe" "" "$INSTDIR\DecisionLogic\DecisionLogic.exe" 0
SectionEnd

;--------------------------------

; Uninstaller

Section "un.LogicianSuite"

	; Remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Logician"
	DeleteRegKey HKLM SOFTWARE\Logician
	DeleteRegKey HKCU SOFTWARE\Logician
	DeleteRegKey HKCR .dlp
	DeleteRegKey HKCR DecisionLogicProject

	; Remove files and uninstaller
	Delete $INSTDIR\DecisionLogic\*.*	
	Delete $INSTDIR\VC12\x64\*.*
	Delete $INSTDIR\VC12\*.*
	Delete $INSTDIR\ajaxslt\*.*
	Delete $INSTDIR\*.*
	Delete $INSTDIR\uninstall_logician_libraries.exe
	
	; Remove shortcuts, if any
	Delete "$SMPROGRAMS\DecisionLogic\*.*"

	; Remove directories used
	RMDir "$SMPROGRAMS\DecisionLogic"
	RMDir "$INSTDIR\DecisionLogic"
	RMDir "$INSTDIR\VC12\x64"
	RMDir "$INSTDIR\VC12"
	RMDir "$INSTDIR\ajaxslt"
	RMDir "$INSTDIR"

SectionEnd

Function .onInit
 
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\Logician" \
  "UninstallString"
  StrCmp $R0 "" done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "Logician is already installed. $\n$\nClick `OK` to remove the \
  previous version or `Cancel` to cancel this upgrade." \
  IDOK uninst
  Abort
 
;Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:
 
done:
 
FunctionEnd