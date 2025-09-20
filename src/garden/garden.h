#pragma once

#include "frame/frame.h"
#include "garden/garden-defs.h"
#include "scrns/scrn-game/scrn-game-theme.h"

void garden_ini(struct garden *garden, struct alloc alloc, f32 timestamp);
void garden_upd(struct garden *garden, struct frame_info frame);
void garden_drw(struct garden *garden, rec_i32 layout, enum game_theme theme);
