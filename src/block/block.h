#pragma once

#include "block/block-defs.h"
#include "scrns/scrn-game/scrn-game-theme.h"

void block_upd(struct block *block, i16 block_size);
void block_drw(struct block *block, enum game_theme theme, i32 x, i32 y, i16 block_size);
