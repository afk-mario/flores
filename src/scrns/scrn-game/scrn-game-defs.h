#pragma once

#include "block/block-type.h"
#include "board/board-defs.h"
#include "frame/frame.h"
#include "piece/piece-defs.h"
#include "scrns/scrn-game/scrn-game-state.h"
#include "scrns/scrn-game/scrn-game-theme.h"

struct scrn_game_editor {
	enum block_type type;
};

struct scrn_game {
	enum scrn_game_state state;
	struct frame_info frame;
	struct scrn_game_editor editor;
	enum game_theme theme;
	struct board board;
	struct piece piece;
};
