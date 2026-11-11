BuildNow3DS — Build Practice Trainer
=====================================
Platform : Nintendo 3DS (Old + New 3DS)
Type     : Homebrew (.3dsx)
Author   : bench/olliE

HOW TO BUILD
------------
1. Install devkitPro: https://devkitpro.org/wiki/Getting_Started
2. Install 3DS libraries:
   sudo dkp-pacman -S 3ds-dev

3. Set env:
   export DEVKITPRO=/opt/devkitpro
   export DEVKITARM=$DEVKITPRO/devkitARM
   export PATH=$DEVKITARM/bin:$PATH

4. Build:
   cd BuildNow3DS/
   make

   → BuildNow3DS.3dsx + BuildNow3DS.smdh

HOW TO RUN
----------
Option A — Luma3DS CFW (recommended):
  Copy BuildNow3DS.3dsx to /3ds/BuildNow3DS/ on SD card
  Launch via Homebrew Launcher (hold SELECT at boot with Luma)

Option B — Citra emulator (PC):
  File → Load 3DSX → select BuildNow3DS.3dsx

CONTROLS
--------
In all modes:
  [START] = exit

Menu:
  [<] [>] DPAD  = select drill
  [A]           = start selected drill
  [B]           = view stats screen

Drill:
  [Circle pad]  = move cursor on grid
  [A]           = place piece at cursor
  [B]           = cycle piece type (Floor / Wall / Ramp)
  [X]           = reset drill (keep timer running)
  [Y]           = toggle edit mode
  [SELECT]      = return to menu

FEATURES
--------
- 4 drills: 90 Turn, Cone, L-Shape, Free Mode
- Millisecond timer (mm:ss.cs format)
- Best-time tracking per session
- Score (+10 correct / -5 mistake)
- Mistake counter
- Step progress bar on top screen
- Piece type indicator on bottom screen
- Completion banner with time

STRUCTURE
---------
source/
  main.c      — init, main loop, input dispatch
  drills.c    — drill definitions + step validation
  ui.c        — citro2d rendering (top + bottom screens)
include/
  game.h      — shared AppState, enums
  drills.h    — DrillState, DrillStep
  timer.h     — DrillTimer (inline, osGetTime-based)
  ui.h        — render declarations
Makefile      — devkitARM build
romfs/        — (empty, reserved for future audio/assets)

NOTES / GOTCHAS
---------------
- osGetTime() returns milliseconds on 3DS — no manual tick conversion needed
- citro2d Z-layer is 0.5f throughout; adjust if layering issues appear on
  New 3DS at 800px wide mode (this build targets 400px top only)
- icon.png must be 48×48 RGBA PNG for smdhtool
  → included as a placeholder; replace with your art
- circle pad dead zone is ±60 raw units — tune in main.c if cursor drifts
- ROMFS is empty; add audio/sndfiles there and wire via ndsp when ready

MISSING (name it, bench builds it):
- Audio feedback (ndsp)
- Persistent best-time save (FS API)
- Touch screen input (bottom screen tap)
- New 3DS 800px wide top support
- Network leaderboard
