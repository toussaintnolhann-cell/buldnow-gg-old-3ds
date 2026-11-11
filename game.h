/*
 * game.h - shared state for BuildNow3DS
 */
#pragma once
#include <stdint.h>
#include "timer.h"
#include "drills.h"

// ---- modes ------------------------------------------------------------------
typedef enum {
    MODE_MENU = 0,
    MODE_DRILL,
    MODE_STATS,
    MODE_SETTINGS
} AppMode;

// ---- grid -------------------------------------------------------------------
#define GRID_W 10
#define GRID_H 10

typedef enum {
    CELL_EMPTY = 0,
    CELL_FLOOR,
    CELL_WALL,
    CELL_RAMP,
    CELL_TARGET
} CellType;

typedef struct {
    CellType type;
    uint8_t  height;    // 0-3
} GridCell;

// ---- input events -----------------------------------------------------------
typedef enum {
    INPUT_CONFIRM,
    INPUT_ROTATE,
    INPUT_TOGGLE_EDIT,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_UP,
    INPUT_DOWN
} InputEvent;

// ---- app state --------------------------------------------------------------
typedef struct {
    AppMode    mode;

    // drill selection
    int        selected_drill;      // index into drill list

    // drill runtime
    DrillState drill;
    DrillTimer drill_timer;

    // stats
    uint32_t   score;
    uint32_t   best_time;           // milliseconds; 0xFFFFFFFF = no record
    uint32_t   session_attempts;
    uint32_t   session_completions;

    // grid (top screen)
    GridCell   grid[GRID_H][GRID_W];
    int        cursor_x, cursor_y;
    CellType   place_type;          // what player is placing
    int        edit_mode;           // 0=build, 1=edit
} AppState;
