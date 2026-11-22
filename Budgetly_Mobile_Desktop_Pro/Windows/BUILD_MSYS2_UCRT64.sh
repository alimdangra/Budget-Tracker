#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
echo "Compiling Budgetly Windows Win32 (pure C)..."
gcc -std=c11 -O2 BudgetTracker_Win32.c -o BudgetTracker.exe -mwindows -lgdi32 -luser32 -lm
echo "Build succeeded! Starting BudgetTracker.exe"
./BudgetTracker.exe
