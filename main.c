/*
 * BuildNow3DS - Build Practice Trainer
 * Platform : Nintendo 3DS (Old/New) - ARM11 userland homebrew
 * Runtime  : libctru + citro2d
 * Build    : devkitARM + libctru + citro2d (devkitPro)
 *
 * Features:
 *   - Build timer with best-time tracking
 *   - Edit drill mode (90-turn, cone, L-shape)
 *   - Target placement tracker
 *   - Score/stats overlay on bottom screen
 */

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "ui.h"
#include "timer.h"
#include "drills.h"

#define SCREEN_TOP_WIDTH  400
#define SCREEN_TOP_HEIGHT 240
#define SCREEN_BOT_WIDTH  320
#define SCREEN_BOT_HEIGHT 240

// Clearcolor RGBA8
#define CLR_BG    C2D_Color32(0x1a, 0x1a, 0x2e, 0xFF)
#define CLR_PANEL C2D_Color32(0x16, 0x21, 0x3e, 0xFF)
#define CLR_HUD   C2D_Color32(0x0f, 0x3d, 0x57, 0xFF)
#define CLR_WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define CLR_GREEN C2D_Color32(0x00, 0xE6, 0x76, 0xFF)
#define CLR_RED   C2D_Color32(0xFF, 0x45, 0x45, 0xFF)
#define CLR_GOLD  C2D_Color32(0xFF, 0xD7, 0x00, 0xFF)

// ---- globals ----------------------------------------------------------------
static C3D_RenderTarget *s_topLeft  = NULL;
static C3D_RenderTarget *s_bottom   = NULL;
static C2D_TextBuf       s_textbuf;
static AppState          s_state;

// ---- prototypes -------------------------------------------------------------
static void scene_init(void);
static void scene_exit(void);
static void scene_render(void);
static void handle_input(void);

// ---- entry ------------------------------------------------------------------
int main(int argc, char **argv) {
    (void)argc; (void)argv;

    // hardware init
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    s_topLeft = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    s_bottom  = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    s_textbuf = C2D_TextBufNew(4096);

    scene_init();

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;   // START = exit (kitchen law: no soft-refuse, hard exit only)

        handle_input();
        timer_tick(&s_state.drill_timer);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_TargetClear(s_topLeft, CLR_BG);
            C2D_TargetClear(s_bottom,  CLR_PANEL);
            C2D_SceneBegin(s_topLeft);
                render_top_screen(&s_state, s_textbuf);
            C2D_SceneBegin(s_bottom);
                render_bottom_screen(&s_state, s_textbuf);
        C3D_FrameEnd(0);

        C2D_TextBufClear(s_textbuf);
    }

    scene_exit();
    C2D_TextBufDelete(s_textbuf);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}

// ---- scene ------------------------------------------------------------------
static void scene_init(void) {
    memset(&s_state, 0, sizeof(AppState));
    s_state.mode       = MODE_MENU;
    s_state.best_time  = 0xFFFFFFFF;   // sentinel = no record yet
    s_state.score      = 0;
    drill_init(&s_state);
}

static void scene_exit(void) {
    // nothing to free beyond textbuf (handled in main)
}

static void handle_input(void) {
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    switch (s_state.mode) {
        case MODE_MENU:
            if (kDown & KEY_A)     { s_state.mode = MODE_DRILL; drill_start(&s_state); }
            if (kDown & KEY_B)     { s_state.mode = MODE_STATS; }
            if (kDown & KEY_X)     { s_state.mode = MODE_SETTINGS; }
            if (kDown & KEY_DRIGHT){ s_state.selected_drill = (s_state.selected_drill + 1) % DRILL_COUNT; }
            if (kDown & KEY_DLEFT) { s_state.selected_drill = (s_state.selected_drill + DRILL_COUNT - 1) % DRILL_COUNT; }
            break;

        case MODE_DRILL:
            // A = place / confirm edit
            if (kDown & KEY_A)     drill_input(&s_state, INPUT_CONFIRM);
            // B = rotate piece
            if (kDown & KEY_B)     drill_input(&s_state, INPUT_ROTATE);
            // X = reset drill
            if (kDown & KEY_X)     { drill_start(&s_state); }
            // Y = toggle edit mode
            if (kDown & KEY_Y)     drill_input(&s_state, INPUT_TOGGLE_EDIT);
            // Circle pad → cursor movement (8 dirs)
            circlePosition cpad;
            hidCircleRead(&cpad);
            if (cpad.dx >  60)     drill_input(&s_state, INPUT_RIGHT);
            if (cpad.dx < -60)     drill_input(&s_state, INPUT_LEFT);
            if (cpad.dy >  60)     drill_input(&s_state, INPUT_UP);
            if (cpad.dy < -60)     drill_input(&s_state, INPUT_DOWN);
            // SELECT = back to menu
            if (kDown & KEY_SELECT){ s_state.mode = MODE_MENU; timer_stop(&s_state.drill_timer); }
            (void)kHeld;
            break;

        case MODE_STATS:
        case MODE_SETTINGS:
            if (kDown & KEY_B)     { s_state.mode = MODE_MENU; }
            break;

        default: break;
    }
}
