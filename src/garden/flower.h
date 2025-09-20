#pragma once

#include "frame/frame.h"
#include "garden/flower-defs.h"

void flower_ini(struct flower *flower, struct alloc alloc, struct tex t);
void flower_upd(struct flower *flower, struct frame_info frame);
void flower_drw(struct flower *flower, i32 x, i32 y);
void flower_iterations_max(struct flower *flower, f32 timestamp);
void flower_iterations_set(struct flower *flower, i32 value, f32 timestamp);
void flower_iterations_add(struct flower *flower, i32 value, f32 timestamp);
void flower_iterations_sub(struct flower *flower, i32 value, f32 timestamp);

void flower_type_set(struct flower *flower, enum block_type type, f32 timestamp);
void flower_reevaluate(struct flower *flower);
void flower_upd_tex(struct flower *flower, f32 t);
