/*
 * timer.h - millisecond drill timer using 3DS TickCounter
 */
#pragma once
#include <stdint.h>
#include <3ds.h>

typedef struct {
    uint64_t start_tick;    // osGetTime() tick at start
    uint32_t elapsed_ms;
    int      running;
} DrillTimer;

static inline void timer_start(DrillTimer *t) {
    t->start_tick  = osGetTime();
    t->elapsed_ms  = 0;
    t->running     = 1;
}

static inline void timer_stop(DrillTimer *t) {
    if (t->running) {
        t->elapsed_ms = (uint32_t)(osGetTime() - t->start_tick);
        t->running    = 0;
    }
}

static inline void timer_reset(DrillTimer *t) {
    t->start_tick  = 0;
    t->elapsed_ms  = 0;
    t->running     = 0;
}

static inline void timer_tick(DrillTimer *t) {
    if (t->running) {
        t->elapsed_ms = (uint32_t)(osGetTime() - t->start_tick);
    }
}

// format mm:ss.cs into buf (at least 10 chars)
static inline void timer_format(const DrillTimer *t, char *buf, int bufsz) {
    uint32_t ms  = t->elapsed_ms;
    uint32_t cs  = (ms % 1000) / 10;
    uint32_t sec = (ms / 1000) % 60;
    uint32_t min = (ms / 60000);
    snprintf(buf, bufsz, "%02u:%02u.%02u", min, sec, cs);
}
