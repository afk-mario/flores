#pragma once
#include "base/marena.h"
#include "base/mem.h"
#include "base/types.h"
#include "block/block-type.h"
#include "board/board-defs.h"
#include "piece/piece-defs.h"
#include "scrns/scrn-game/scrn-game-state.h"

struct frame_info {
	f32 dt;
	f32 dt_inv;
	f32 timestamp;
	struct alloc alloc;
	struct marena marena;
};

struct scrn_game_editor {
	enum block_type type;
};

struct scrn_game {
	enum scrn_game_state state;
	struct frame_info frame;
	struct scrn_game_editor editor;
	struct board board;
	struct piece piece;
};
