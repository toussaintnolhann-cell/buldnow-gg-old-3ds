/*
 * ui.c - citro2d rendering for top + bottom screens
 * Lang: C99 | File: ui.c | Runtime: libctru + citro2d
 *
 * Top screen  (400×240): isometric-ish grid + cursor + drill ghost
 * Bottom screen (320×240): HUD — timer, score, stats, controls
 */
#include "ui.h"
#include "game.h"
#include "timer.h"
#include <stdio.h>
#include <string.h>

// ---- color palette ----------------------------------------------------------
#define CLR_BG        C2D_Color32(0x1a,0x1a,0x2e,0xFF)
#define CLR_PANEL     C2D_Color32(0x16,0x21,0x3e,0xFF)
#define CLR_GRID_LINE C2D_Color32(0x2a,0x2a,0x4e,0xFF)
#define CLR_CURSOR    C2D_Color32(0x00,0xE6,0x76,0xCC)
#define CLR_TARGET    C2D_Color32(0xFF,0xD7,0x00,0xAA)
#define CLR_FLOOR     C2D_Color32(0x44,0x88,0xCC,0xFF)
#define CLR_WALL      C2D_Color32(0x88,0x44,0xCC,0xFF)
#define CLR_RAMP      C2D_Color32(0xCC,0x88,0x22,0xFF)
#define CLR_WHITE     C2D_Color32(0xFF,0xFF,0xFF,0xFF)
#define CLR_GREEN     C2D_Color32(0x00,0xE6,0x76,0xFF)
#define CLR_RED       C2D_Color32(0xFF,0x45,0x45,0xFF)
#define CLR_GOLD      C2D_Color32(0xFF,0xD7,0x00,0xFF)
#define CLR_DIM       C2D_Color32(0x88,0x88,0x88,0xFF)

// ---- grid layout (top screen) -----------------------------------------------
#define CELL_SZ   24
#define GRID_OX   40     // origin x on top screen
#define GRID_OY   20     // origin y on top screen

static void draw_text(C2D_TextBuf tbuf, float x, float y,
                      float sz, uint32_t clr, const char *str) {
    C2D_Text txt;
    C2D_TextParse(&txt, tbuf, str);
    C2D_TextOptimize(&txt);
    C2D_DrawText(&txt, C2D_WithColor, x, y, 0.5f, sz, sz, clr);
}

static void draw_cell(int gx, int gy, uint32_t color) {
    float x = GRID_OX + gx * CELL_SZ;
    float y = GRID_OY + gy * CELL_SZ;
    C2D_DrawRectSolid(x + 1, y + 1, 0, CELL_SZ - 2, CELL_SZ - 2, color);
}

static void draw_cell_outline(int gx, int gy, uint32_t color) {
    float x = GRID_OX + gx * CELL_SZ;
    float y = GRID_OY + gy * CELL_SZ;
    // 4 edges
    C2D_DrawRectSolid(x,            y,            0, CELL_SZ, 2,       color);
    C2D_DrawRectSolid(x,            y+CELL_SZ-2,  0, CELL_SZ, 2,       color);
    C2D_DrawRectSolid(x,            y,            0, 2,        CELL_SZ, color);
    C2D_DrawRectSolid(x+CELL_SZ-2, y,             0, 2,        CELL_SZ, color);
}

// ---- top screen: grid + drill state -----------------------------------------
void render_top_screen(AppState *s, C2D_TextBuf tbuf) {
    // grid lines
    for (int r = 0; r <= GRID_H; r++) {
        float y = GRID_OY + r * CELL_SZ;
        C2D_DrawRectSolid(GRID_OX, y, 0, GRID_W * CELL_SZ, 1, CLR_GRID_LINE);
    }
    for (int c = 0; c <= GRID_W; c++) {
        float x = GRID_OX + c * CELL_SZ;
        C2D_DrawRectSolid(x, GRID_OY, 0, 1, GRID_H * CELL_SZ, CLR_GRID_LINE);
    }

    // placed cells
    for (int gy = 0; gy < GRID_H; gy++) {
        for (int gx = 0; gx < GRID_W; gx++) {
            CellType t = s->grid[gy][gx].type;
            if (t == CELL_EMPTY) continue;
            uint32_t clr = CLR_FLOOR;
            if (t == CELL_WALL)  clr = CLR_WALL;
            if (t == CELL_RAMP)  clr = CLR_RAMP;
            draw_cell(gx, gy, clr);
        }
    }

    // target highlight (current drill step)
    if (s->mode == MODE_DRILL &&
        s->drill.status == DSTATE_RUNNING &&
        s->drill.current_step < s->drill.total_steps) {
        const DrillStep *step = &s->drill.steps[s->drill.current_step];
        draw_cell(step->gx, step->gy, CLR_TARGET);
    }

    // cursor
    if (s->mode == MODE_DRILL || s->mode == MODE_MENU) {
        draw_cell_outline(s->cursor_x, s->cursor_y, CLR_CURSOR);
    }

    // mode label top-right
    const char *mode_lbl = "MENU";
    if (s->mode == MODE_DRILL) {
        mode_lbl = (s->drill.status == DSTATE_COMPLETE) ? "DONE" :
                   (s->drill.status == DSTATE_RUNNING)  ? "DRILL" : "IDLE";
    }
    draw_text(tbuf, 340, 4, 0.55f, CLR_WHITE, mode_lbl);

    // drill name top-left
    static const char *drill_names[DRILL_COUNT] = {
        "90 Turn", "Cone", "L-Shape", "Free"
    };
    if (s->selected_drill >= 0 && s->selected_drill < DRILL_COUNT) {
        draw_text(tbuf, 4, 4, 0.5f, CLR_GOLD, drill_names[s->selected_drill]);
    }

    // step progress bar (bottom of top screen)
    if (s->mode == MODE_DRILL && s->drill.total_steps > 0) {
        char prog[32];
        snprintf(prog, sizeof(prog), "Step %d / %d",
                 s->drill.current_step + 1, s->drill.total_steps);
        draw_text(tbuf, 4, 222, 0.5f, CLR_WHITE, prog);

        // progress rect
        float bar_w = (float)SCREEN_TOP_WIDTH * s->drill.current_step
                      / s->drill.total_steps;
        C2D_DrawRectSolid(0, 236, 0, bar_w, 4, CLR_GREEN);
    }

    // completion banner
    if (s->mode == MODE_DRILL && s->drill.status == DSTATE_COMPLETE) {
        C2D_DrawRectSolid(80, 90, 0, 240, 60, C2D_Color32(0,0,0,0xCC));
        draw_text(tbuf, 120, 100, 0.8f, CLR_GOLD,  "COMPLETE!");
        char bt[32];
        timer_format(&s->drill_timer, bt, sizeof(bt));
        draw_text(tbuf, 130, 125, 0.6f, CLR_WHITE, bt);
    }
}

// ---- bottom screen: HUD -----------------------------------------------------
#define BOT_PAD 8

void render_bottom_screen(AppState *s, C2D_TextBuf tbuf) {
    // header bar
    C2D_DrawRectSolid(0, 0, 0, 320, 24, CLR_HUD);
    draw_text(tbuf, BOT_PAD, 4, 0.6f, CLR_WHITE, "BuildNow3DS");

    // timer box
    char timer_str[16];
    timer_format(&s->drill_timer, timer_str, sizeof(timer_str));
    C2D_DrawRectSolid(BOT_PAD, 32, 0, 148, 36, C2D_Color32(0,0,0,0x88));
    draw_text(tbuf, BOT_PAD + 4, 36, 0.5f, CLR_DIM, "TIME");
    draw_text(tbuf, BOT_PAD + 4, 50, 0.75f,
              s->drill_timer.running ? CLR_GREEN : CLR_WHITE, timer_str);

    // best time
    if (s->best_time != 0xFFFFFFFF) {
        DrillTimer fake = {0, s->best_time, 0};
        char best_str[16];
        timer_format(&fake, best_str, sizeof(best_str));
        C2D_DrawRectSolid(164, 32, 0, 148, 36, C2D_Color32(0,0,0,0x88));
        draw_text(tbuf, 168, 36, 0.5f, CLR_DIM,  "BEST");
        draw_text(tbuf, 168, 50, 0.75f, CLR_GOLD, best_str);
    } else {
        C2D_DrawRectSolid(164, 32, 0, 148, 36, C2D_Color32(0,0,0,0x88));
        draw_text(tbuf, 168, 36, 0.5f, CLR_DIM,  "BEST");
        draw_text(tbuf, 168, 50, 0.75f, CLR_DIM, "--:--.--");
    }

    // score
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "SCORE  %06u", s->score);
    draw_text(tbuf, BOT_PAD, 78, 0.6f, CLR_WHITE, score_str);

    // attempts / completions
    char sess_str[64];
    snprintf(sess_str, sizeof(sess_str), "Attempts %u  |  Done %u",
             s->session_attempts, s->session_completions);
    draw_text(tbuf, BOT_PAD, 100, 0.5f, CLR_DIM, sess_str);

    // mistakes
    if (s->mode == MODE_DRILL) {
        char mis_str[32];
        snprintf(mis_str, sizeof(mis_str), "Mistakes: %u", s->drill.mistakes);
        draw_text(tbuf, BOT_PAD, 118, 0.5f,
                  s->drill.mistakes > 0 ? CLR_RED : CLR_GREEN, mis_str);
    }

    // ---- controls legend (bottom of bottom screen) ----
    C2D_DrawRectSolid(0, 170, 0, 320, 70, C2D_Color32(0,0,0,0x99));

    if (s->mode == MODE_MENU) {
        draw_text(tbuf, BOT_PAD, 174, 0.45f, CLR_WHITE, "[A] Start drill");
        draw_text(tbuf, BOT_PAD, 190, 0.45f, CLR_WHITE, "[B] Stats");
        draw_text(tbuf, BOT_PAD, 206, 0.45f, CLR_WHITE, "[</>] Select drill");
        draw_text(tbuf, BOT_PAD, 222, 0.45f, CLR_WHITE, "[START] Quit");
    } else if (s->mode == MODE_DRILL) {
        draw_text(tbuf, BOT_PAD, 174, 0.45f, CLR_WHITE, "[A] Place  [B] Rotate piece");
        draw_text(tbuf, BOT_PAD, 190, 0.45f, CLR_WHITE, "[X] Reset  [Y] Edit toggle");
        draw_text(tbuf, BOT_PAD, 206, 0.45f, CLR_WHITE, "[Circle] Move cursor");
        draw_text(tbuf, BOT_PAD, 222, 0.45f, CLR_WHITE, "[SELECT] Menu  [START] Quit");
    }

    // piece selector pill
    if (s->mode == MODE_DRILL) {
        static const char *pnames[] = {"", "Floor", "Wall", "Ramp", ""};
        int pt = (int)s->place_type;
        if (pt >= 1 && pt <= 3) {
            char pill[32];
            snprintf(pill, sizeof(pill), "Piece: %s", pnames[pt]);
            uint32_t pc = (pt == 1) ? CLR_FLOOR : (pt == 2) ? CLR_WALL : CLR_RAMP;
            C2D_DrawRectSolid(200, 100, 0, 112, 20, C2D_Color32(0,0,0,0x99));
            draw_text(tbuf, 204, 103, 0.5f, pc, pill);
        }
    }
}
