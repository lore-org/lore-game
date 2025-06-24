@echo off
:: BatchGotAdmin
::-------------------------------------
REM  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params = %*:"="
    echo UAC.ShellExecute "cmd.exe", "/c %~s0 %params%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
::--------------------------------------

if not defined VCPKG_ROOT (
    set VCPKG_ROOT=%USERPROFILE%/.vcpkg
    git clone https://github.com/microsoft/vcpkg "%VCPKG_ROOT%"
) else (
    set VCPKG_ROOT=%USERPROFILE%/.vcpkg
)

setx VCPKG_ROOT "%USERPROFILE%/.vcpkg"

powershell.exe -NoProfile -ExecutionPolicy Bypass "& {& '%VCPKG_ROOT%/scripts/bootstrap.ps1' -disableMetrics %*}"

echo "%SystemRoot%"
echo "%VCPKG_ROOT%/vcpkg.exe"

mklink "%SystemRoot%/vcpkg.exe" "%VCPKG_ROOT%/vcpkg.exe"