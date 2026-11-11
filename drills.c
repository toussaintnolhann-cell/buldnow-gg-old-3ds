/*
 * drills.c - drill definitions and step logic
 * Lang: C99 | File: drills.c | Runtime: libctru
 */
#include "drills.h"
#include "game.h"
#include <string.h>
#include <stdlib.h>

// ---- drill definitions (hardcoded step sequences) ---------------------------

// 90° turn: place wall, ramp facing right, floor, wall, ramp facing forward
static const DrillStep k_90turn[] = {
    {4, 5, 0, CELL_FLOOR},
    {5, 5, 0, CELL_FLOOR},
    {5, 5, 1, CELL_WALL},
    {5, 4, 1, CELL_RAMP},
    {5, 4, 1, CELL_FLOOR},
    {6, 4, 1, CELL_WALL},
    {6, 4, 2, CELL_RAMP},
};

// Cone: 3×3 base, 2×2 mid, 1×1 top (9+4+1 = 14 steps)
static const DrillStep k_cone[] = {
    // base ring
    {3,3,0,CELL_FLOOR},{4,3,0,CELL_FLOOR},{5,3,0,CELL_FLOOR},
    {3,4,0,CELL_FLOOR},{5,4,0,CELL_FLOOR},
    {3,5,0,CELL_FLOOR},{4,5,0,CELL_FLOOR},{5,5,0,CELL_FLOOR},
    // center base
    {4,4,0,CELL_FLOOR},
    // mid ring
    {3,3,1,CELL_WALL}, {5,3,1,CELL_WALL},
    {3,5,1,CELL_WALL}, {5,5,1,CELL_WALL},
    // top
    {4,4,2,CELL_WALL},
};

// L-shape: horizontal + vertical arm
static const DrillStep k_lshape[] = {
    {2,5,0,CELL_FLOOR},{3,5,0,CELL_FLOOR},{4,5,0,CELL_FLOOR},{5,5,0,CELL_FLOOR},
    {5,4,0,CELL_FLOOR},{5,3,0,CELL_FLOOR},{5,2,0,CELL_FLOOR},
};

typedef struct {
    const DrillStep *steps;
    int              count;
    const char      *name;
} DrillDef;

static const DrillDef k_defs[DRILL_COUNT] = {
    [DRILL_90TURN]  = { k_90turn,  7,  "90 Turn"  },
    [DRILL_CONE]    = { k_cone,   14,  "Cone"     },
    [DRILL_LSHAPE]  = { k_lshape,  7,  "L-Shape"  },
    [DRILL_FREEMODE]= { NULL,       0, "Free Mode" },
};

// ---- public API -------------------------------------------------------------

void drill_init(AppState *s) {
    s->selected_drill = DRILL_90TURN;
    memset(&s->drill, 0, sizeof(DrillState));
    s->drill.status = DSTATE_IDLE;
}

void drill_start(AppState *s) {
    int idx = s->selected_drill;
    DrillState *d = &s->drill;

    memset(d, 0, sizeof(DrillState));
    d->type        = (DrillType)idx;
    d->status      = DSTATE_RUNNING;
    d->total_steps = k_defs[idx].count;

    if (k_defs[idx].steps && k_defs[idx].count > 0) {
        for (int i = 0; i < k_defs[idx].count && i < MAX_STEPS; i++) {
            d->steps[i] = k_defs[idx].steps[i];
        }
    }

    // reset grid
    memset(s->grid, 0, sizeof(s->grid));

    // cursor at first target
    if (d->total_steps > 0) {
        s->cursor_x = d->steps[0].gx;
        s->cursor_y = d->steps[0].gy;
    } else {
        s->cursor_x = 4;
        s->cursor_y = 4;
    }

    s->place_type = CELL_FLOOR;
    s->session_attempts++;

    timer_start(&s->drill_timer);
}

static int cursor_in_bounds(int x, int y) {
    return x >= 0 && x < GRID_W && y >= 0 && y < GRID_H;
}

void drill_input(AppState *s, int event) {
    DrillState *d = &s->drill;
    if (d->status != DSTATE_RUNNING && event != INPUT_TOGGLE_EDIT) return;

    switch (event) {
        case INPUT_LEFT:
            if (cursor_in_bounds(s->cursor_x - 1, s->cursor_y))
                s->cursor_x--;
            break;
        case INPUT_RIGHT:
            if (cursor_in_bounds(s->cursor_x + 1, s->cursor_y))
                s->cursor_x++;
            break;
        case INPUT_UP:
            if (cursor_in_bounds(s->cursor_x, s->cursor_y - 1))
                s->cursor_y--;
            break;
        case INPUT_DOWN:
            if (cursor_in_bounds(s->cursor_x, s->cursor_y + 1))
                s->cursor_y++;
            break;

        case INPUT_ROTATE: {
            // cycle piece type: FLOOR → WALL → RAMP → FLOOR
            CellType cycle[] = { CELL_FLOOR, CELL_WALL, CELL_RAMP };
            for (int i = 0; i < 3; i++) {
                if (s->place_type == cycle[i]) {
                    s->place_type = cycle[(i + 1) % 3];
                    break;
                }
            }
            break;
        }

        case INPUT_TOGGLE_EDIT:
            s->edit_mode ^= 1;
            break;

        case INPUT_CONFIRM: {
            // place piece at cursor, check if it matches the current step
            if (!cursor_in_bounds(s->cursor_x, s->cursor_y)) break;

            if (d->current_step < d->total_steps) {
                const DrillStep *step = &d->steps[d->current_step];
                GridCell *cell = &s->grid[s->cursor_y][s->cursor_x];

                // place it regardless
                cell->type   = s->place_type;
                cell->height = (uint8_t)(cell->height + 1);

                // check correctness
                if (s->cursor_x == step->gx &&
                    s->cursor_y == step->gy &&
                    s->place_type == step->piece) {
                    // correct step
                    d->current_step++;
                    s->score += 10;

                    if (d->current_step >= d->total_steps) {
                        d->status = DSTATE_COMPLETE;
                        timer_stop(&s->drill_timer);
                        s->session_completions++;

                        if (s->drill_timer.elapsed_ms < s->best_time)
                            s->best_time = s->drill_timer.elapsed_ms;
                    } else {
                        // advance cursor to next target
                        s->cursor_x = d->steps[d->current_step].gx;
                        s->cursor_y = d->steps[d->current_step].gy;
                    }
                } else {
                    // wrong piece or position
                    d->mistakes++;
                    if (s->score >= 5) s->score -= 5;
                }
            }
            break;
        }

        default: break;
    }
}

int drill_check_step(AppState *s) {
    return s->drill.current_step;
}
