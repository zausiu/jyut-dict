@echo off
chcp 65001

set FOLDER=%1
set FOLDER=%FOLDER:"=%
set EXTENSION=.lnk
set APPLICATIONNAME=Jyut Dictionary
set UNINSTALLNAME=Uninstall %APPLICATIONNAME%

set FRENCHNAME=Dictionnaire Jyut
set FRENCHUNINSTALL=Désinstaller %FRENCHNAME%
set SIMPLIFIEDNAME=粤语字典
set SIMPLIFIEDUNINSTALL=卸载%SIMPLIFIEDNAME%
set TRADITIONALNAME=粵語字典
set TRADITIONALUNINSTALL=移除%TRADITIONALNAME%

set OLDNAME=%FOLDER%%APPLICATIONNAME%%EXTENSION%
set NEWFRENCHNAME=%FRENCHNAME%%EXTENSION%
set NEWSIMPLIFIEDNAME=%SIMPLIFIEDNAME%%EXTENSION%
set NEWTRADITIONALNAME=%TRADITIONALNAME%%EXTENSION%

set OLDUNINSTALLNAME=%FOLDER%%UNINSTALLNAME%%EXTENSION%
set FRENCHUNINSTALLNAME=%FRENCHUNINSTALL%%EXTENSION%
set SIMPLIFIEDUNINSTALLNAME=%SIMPLIFIEDUNINSTALL%%EXTENSION%
set TRADITIONALUNINSTALLNAME=%TRADITIONALUNINSTALL%%EXTENSION%

FOR /F "tokens=3" %%a IN ('reg query "HKCU\Control Panel\Desktop" /v PreferredUILanguages ^| find "PreferredUILanguages"') DO set UILanguage=%%a 
IF "%UILanguage%" == "fr-CA " (
	rename "%OLDNAME%" "%NEWFRENCHNAME%"
	rename "%OLDUNINSTALLNAME%" "%FRENCHUNINSTALLNAME%"
)
IF "%UILanguage%" == "fr-FR " (
	rename "%OLDNAME%" "%NEWFRENCHNAME%"
	rename "%OLDUNINSTALLNAME%" "%FRENCHUNINSTALLNAME%"
)
IF "%UILanguage%" == "zh-CN " (
	rename "%OLDNAME%" "%NEWSIMPLIFIEDNAME%"
	rename "%OLDUNINSTALLNAME%" "%SIMPLIFIEDUNINSTALLNAME%"
)
IF "%UILanguage%" == "zh-HK " (
	rename "%OLDNAME%" "%NEWTRADITIONALNAME%"
	rename "%OLDUNINSTALLNAME%" "%TRADITIONALUNINSTALLNAME%"
)
IF "%UILanguage%" == "zh-MO " (
	rename "%OLDNAME%" "%NEWTRADITIONALNAME%"
	rename "%OLDUNINSTALLNAME%" "%TRADITIONALUNINSTALLNAME%"
)
IF "%UILanguage%" == "zh-TW " (
	rename "%OLDNAME%" "%NEWTRADITIONALNAME%"
	rename "%OLDUNINSTALLNAME%" "%TRADITIONALUNINSTALLNAME%"
)