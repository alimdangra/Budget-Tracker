BUDGETLY | FINANCE UI (PURE C) | NOVEMBER 2026
=====================================================

A) WINDOWS: This is the easiest, immediate upgrade to your current project.
   Folder: Windows/
   Double-click RUN_BUDGETLY_WINDOWS.bat, or open MSYS2 UCRT64 in Windows/
   and run: bash BUILD_MSYS2_UCRT64.sh
   Replaces plain system confirmation boxes with styled GDI confirmation cards.
   Existing data formats and original comments are preserved in Core.inc.

B) DESKTOP CROSS-PLATFORM UI / MOBILE SOURCE:
   Folder: Portable/
   This is a separate SDL2 + SDL2_ttf C frontend sharing the original C logic.
   It has touch-aware screens and custom dialogs for income/expense/account delete,
   warning, forecasts and inline form errors.
   See Portable/SETUP.md. Requires SDL2 library on Windows.

C) ANDROID: Source/integration guidance at Portable/android/BUILD_ANDROID.md.
   An Android APK is NOT INCLUDED. Build with Android Studio SDK/NDK.

D) iPHONE: Source/integration guidance at Portable/ios/BUILD_XCODE.md.
   An iPhone .ipa is NOT INCLUDED. Build on macOS + Xcode + signing.

For original Windows account files, keep all USERNAME_*.txt and accounts.txt
in the same Windows/ folder as BudgetTracker.exe (or copy them there after
backing them up). Portable mobile app stores separate files in its own sandbox.

SECURITY: Legacy school project account storage saves passwords as plain text.
Do not use real banking credentials; production deployment requires proper
password hashing, data encryption and device sync/security work.
