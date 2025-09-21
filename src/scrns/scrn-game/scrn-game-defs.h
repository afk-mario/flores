#pragma once

#include "block/block-type.h"
#include "board/board-defs.h"
#include "frame/frame.h"
#include "garden/garden-defs.h"
#include "piece/piece-defs.h"
#include "scrns/scrn-game/scrn-game-state.h"
#include "scrns/scrn-game/scrn-game-theme.h"
#include "scrns/scrn-type.h"
#include "vfx/vfx-defs.h"

struct scrn_game_editor {
	f32 timestamp[BOARD_COLUMNS];
	enum block_type type;
};

struct piece_blocks {
	enum block_type types[2];
};

struct piece_blocks_list {
	struct piece_blocks piece_blocks[(BLOCK_TYPE_NUM_COUNT - 1) * (BLOCK_TYPE_NUM_COUNT - 1)];
};

struct scrn_game_piece_lists {
	size piece_idx;
	u8 list_idx;
	struct piece_blocks_list lists[2];
};

struct scrn_game {
	f32 t;
	u32 score;
	u32 score_ui;
	u32 chain;
	struct scrn_game_piece_lists piece_lists;
	enum scrn_type exit_to;
	enum scrn_game_state prev_state;
	enum scrn_game_state state;
	struct frame_info frame;
	struct scrn_game_editor editor;
	enum game_theme theme;
	struct garden garden;
	struct board board;
	struct piece piece;
	struct vfxs vfxs;
	struct vfxs matches_vfx;
};
