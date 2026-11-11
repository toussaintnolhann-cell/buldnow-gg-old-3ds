/*
 * drills.h - drill definitions and runtime state
 */
#pragma once
#include <stdint.h>

#define DRILL_COUNT 4

typedef enum {
    DRILL_90TURN = 0,   // classic 90° ramp+wall
    DRILL_CONE,         // pyramid structure
    DRILL_LSHAPE,       // L-shaped platform
    DRILL_FREEMODE      // no target, free build
} DrillType;

typedef enum {
    DSTATE_IDLE = 0,
    DSTATE_RUNNING,
    DSTATE_COMPLETE,
    DSTATE_FAILED
} DrillStatus;

// A step = one piece placed in the right cell at the right height
typedef struct {
    int8_t  gx, gy;     // grid position
    uint8_t height;
    uint8_t piece;      // CellType expected
} DrillStep;

#define MAX_STEPS 32

typedef struct {
    DrillType    type;
    DrillStatus  status;
    int          current_step;
    int          total_steps;
    DrillStep    steps[MAX_STEPS];
    uint32_t     mistakes;
} DrillState;

// forward declarations (implemented in drills.c)
struct AppState;
void drill_init(struct AppState *s);
void drill_start(struct AppState *s);
void drill_input(struct AppState *s, int event);
int  drill_check_step(struct AppState *s);
