#pragma once

#include "block/block-type.h"
#include "board/board-defs.h"
#include "frame/frame.h"
#include "piece/piece-defs.h"
#include "scrns/scrn-game/scrn-game-state.h"
#include "scrns/scrn-game/scrn-game-theme.h"
#include "scrns/scrn-type.h"
#include "vfx/vfx-defs.h"

struct scrn_game_editor {
	f32 timestamp[BOARD_COLUMNS];
	enum block_type type;
};

struct scrn_game {
	f32 t;
	u32 score;
	u32 score_ui;
	enum scrn_type exit_to;
	enum scrn_game_state prev_state;
	enum scrn_game_state state;
	struct frame_info frame;
	struct scrn_game_editor editor;
	enum game_theme theme;
	struct board board;
	struct piece piece;
	struct vfxs vfxs;
	struct vfxs matches_vfx;
};
