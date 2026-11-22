# iOS build notes: C application source, native iOS wrapper from SDL2

Requires **macOS, Xcode and Apple's iOS SDK**. This is a source code project,
not an .ipa and not an installable app. Apple code signing is required for
physical iPhone deployment.

1. Download SDL2 2.x source from https://github.com/libsdl-org/SDL/releases
   and SDL2_ttf 2.x from https://github.com/libsdl-org/SDL_ttf/releases.
2. Open the SDL2 Xcode project, build the `SDL2.xcframework` target for iOS.
   Build `SDL2_ttf.xcframework` from its iOS framework project / source as well,
   including its FreeType dependency.
3. In Xcode, create a new iOS application with an SDL2-compatible iOS target.
   Add `../src/Budgetly_Portable.c` and `../src/BudgetTracker_Core.inc` to the
   target. The `.inc` file is included by C source and should **not** be compiled
   as a separate translation unit.
4. Add SDL2 and SDL2_ttf frameworks to Link Binary With Libraries; add their
   header search paths. Ensure SDL2's iOS UIKit lifecycle wrapper is used and
   disable any duplicate UI template `main`/`UIApplicationMain` entrypoint.
5. Select a valid Team under Signing & Capabilities, an iPhone simulator or
   connected iPhone as the target, and build/run. Use `SDL2/README-ios` as the
   primary reference for SDK-specific steps.
6. If iOS does not expose any of the preconfigured system font paths, provide
   your *own licensed* TTF file in the app bundle as `assets/UIFont.ttf` and set
   the application's working directory for that asset (or update `openFonts`).

User data lives under SDL_GetPrefPath (iOS application sandbox), so this iOS
installation does not automatically sync with Windows or Android accounts.
