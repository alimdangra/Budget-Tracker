@echo off
cd /d "%~dp0"
set "GCC=C:\msys64\ucrt64\bin\gcc.exe"
if not exist "%GCC%" (
  echo MSYS2 UCRT64 GCC was not found at C:\msys64\ucrt64\bin\gcc.exe
  echo Start MSYS2 UCRT64 and use: bash BUILD_MSYS2_UCRT64.sh
  pause
  exit /b 1
)
"%GCC%" -std=c11 -O2 BudgetTracker_Win32.c -o BudgetTracker.exe -mwindows -lgdi32 -luser32 -lm
if errorlevel 1 (
  echo Build failed. See messages above.
  pause
  exit /b 1
)
start "" "%~dp0BudgetTracker.exe"
