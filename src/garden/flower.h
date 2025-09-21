#pragma once

#include "frame/frame.h"
#include "garden/flower-defs.h"

void flower_ini(struct flower *flower, struct alloc alloc, struct tex t);
void flower_upd(struct flower *flower, struct frame_info frame);
void flower_drw(struct flower *flower, i32 x, i32 y);
void flower_iterations_max(struct flower *flower, struct frame_info frame);
void flower_iterations_set(struct flower *flower, i32 value, struct frame_info frame);
void flower_iterations_add(struct flower *flower, i32 value, struct frame_info frame);
void flower_iterations_sub(struct flower *flower, i32 value, struct frame_info frame);

void flower_type_set(struct flower *flower, enum block_type type, struct frame_info frame);
void flower_reevaluate(struct flower *flower, struct frame_info frame);
void flower_upd_tex(struct flower *flower, struct alloc scratch, f32 t);
