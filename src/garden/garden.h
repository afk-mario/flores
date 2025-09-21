#pragma once

#include "frame/frame.h"
#include "garden/garden-defs.h"
#include "scrns/scrn-game/scrn-game-theme.h"

void garden_load(struct garden *garden, v2_i32 size, struct alloc alloc);
void garden_ini(struct garden *garden, struct alloc alloc, struct frame_info frame);
void garden_upd(struct garden *garden, struct frame_info frame);
void garden_drw(struct garden *garden, rec_i32 layout, enum game_theme theme);
b32 garden_seed_add(struct garden *garden, i32 column, enum block_type type, struct frame_info frame);
void garden_time_adv(struct garden *garden, struct frame_info frame);
void garden_water_add(struct garden *garden, i32 column, i32 value, struct frame_info frame);
u16 garden_flower_spawn(struct garden *garden, i32 column, enum block_type type, struct frame_info frame);
