#pragma once

#include "board/board-defs.h"
#include "falling/falling-defs.h"

b32 falling_upd(struct falling *falling, struct board *board, f32 timestamp);
