# Budgetly Pure C — shared cross-platform frontend

The project has one shared C application frontend (`src/Budgetly_Portable.c`),
which includes the team's original commented C logic from
`src/BudgetTracker_Core.inc`. All confirmations (delete income, expense,
account) are **custom finance-themed modal cards**, not plain system dialogs.
There are also red inline form errors, a Back button, Enter-to-submit, a
spending meter, 80/85/90% visual warnings, custom savings limit, and a what-if
planner including vehicle/transport costs.

## Windows MSYS2 UCRT64 (portable app)

Open **MSYS2 UCRT64**. First install packages:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf
```

Use File Explorer to open the `Portable` folder and launch a UCRT64 shell in
that directory, or `cd` to the actual folder path. Then run:

```bash
bash BUILD_MSYS2_UCRT64.sh
```

Or build manually:

```bash
gcc -std=c11 -O2 src/Budgetly_Portable.c -o Budgetly_Portable.exe $(pkg-config --cflags --libs sdl2 SDL2_ttf) -lm
./Budgetly_Portable.exe
```

The native `Windows/` version needs **no SDL2 dependencies** and uses the
`Windows/BUILD_MSYS2_UCRT64.sh` build instead.

## Android and iPhone

- Android build preparation: `android/BUILD_ANDROID.md` (Android Studio/SDK/NDK)
- iPhone build preparation: `ios/BUILD_XCODE.md` (Mac/Xcode/signing)

**Limitations:** Android APK and iOS IPA binaries are not generated here. Both
platforms need their official build toolchains and SDL2/SDL2_ttf libraries.
The source has been syntax-checked in C, but full device UI testing is still
required. Device fonts are loaded from system font paths; a licensed bundled
font may be required for some iPhones.

## Data and privacy

- Windows Win32 app uses its executable folder and current text-file formats.
- Portable SDL app uses `SDL_GetPrefPath("Budgetly", "BudgetTracker")` on each
  platform. Sign-up data and entries are **per device**; no cloud sync is
  implemented. Importing data requires a device-appropriate file transfer.
- Legacy text files store unencrypted passwords. This is a first-semester
  project, **not** a production banking application.
- Existing arrays support 100 incomes and 100 expenses per user.
