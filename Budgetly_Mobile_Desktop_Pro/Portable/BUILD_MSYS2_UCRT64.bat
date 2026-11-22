@echo off
cd /d "%~dp0"
set "SHELL=C:\msys64\msys2_shell.cmd"
if exist "%SHELL%" (
  "%SHELL%" -ucrt64 -here -no-start -c "bash BUILD_MSYS2_UCRT64.sh"
) else (
  echo Could not find C:\msys64\msys2_shell.cmd
  echo Open MSYS2 UCRT64 and run: bash BUILD_MSYS2_UCRT64.sh
  pause
)
