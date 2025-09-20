#pragma once

#include "base/types.h"
#include "board/board-defs.h"
#include "globals/g-tex-refs.h"
#include "scrns/scrn-game/scrn-game-theme.h"

#define GAME_THEME         GAME_THEME_ABSTRACT
#define SCORE_VFX_LIFETIME 0.5f
#define SCORE_VFX_Y_OFFSET -16

#if DEBUG
// #define GAME_SHOW_COLS
#endif

static const enum prim_mode GAME_BOARD_BG[GAME_THEME_NUM_COUNT] = {
	[GAME_THEME_SHAPES]   = PRIM_MODE_WHITE,
	[GAME_THEME_SEEDS]    = PRIM_MODE_WHITE,
	[GAME_THEME_ABSTRACT] = PRIM_MODE_WHITE,
};

static const enum prim_mode GAME_BLOCK_BG[GAME_THEME_NUM_COUNT] = {
	[GAME_THEME_SHAPES]   = PRIM_MODE_BLACK,
	[GAME_THEME_SEEDS]    = PRIM_MODE_WHITE,
	[GAME_THEME_ABSTRACT] = -1,
};

static const enum prim_mode GAME_BLOCK_LINK[GAME_THEME_NUM_COUNT] = {
	[GAME_THEME_SHAPES]   = PRIM_MODE_BLACK,
	[GAME_THEME_SEEDS]    = PRIM_MODE_WHITE,
	[GAME_THEME_ABSTRACT] = PRIM_MODE_BLACK,
};

static const enum prim_mode GAME_BLOCK_FG[GAME_THEME_NUM_COUNT] = {
	[GAME_THEME_SHAPES]   = PRIM_MODE_WHITE,
	[GAME_THEME_SEEDS]    = PRIM_MODE_BLACK,
	[GAME_THEME_ABSTRACT] = -1,
};

static enum g_tex_id GAME_THEME_REFS[GAME_THEME_NUM_COUNT] = {
	[GAME_THEME_SHAPES]   = G_TEX_SHAPES,
	[GAME_THEME_SEEDS]    = G_TEX_SEEDS,
	[GAME_THEME_ABSTRACT] = G_TEX_ABSTRACT_SEEDS,
};

#define NUM_BOARDS 10
// clang-format off
static u8 GAME_PRESET_BOARDS[NUM_BOARDS][BOARD_COLUMNS * BOARD_ROWS]= {
  [0] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
  },
  [1] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 5, 5, 4, 0,
		0, 5, 5, 3, 5, 0,
		0, 5, 2, 5, 5, 0,
		1, 2, 3, 4, 1, 1,
		1, 2, 3, 4, 1, 5,
		1, 2, 3, 4, 1, 5,
  },
  [2] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 3, 4, 5, 0,
		0, 2, 1, 3, 4, 5,
		0, 2, 1, 3, 4, 5,
		2, 2, 1, 3, 4, 5,
  },
  [3] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 3, 4, 5, 0,
		0, 2, 1, 3, 4, 5,
		0, 2, 1, 3, 4, 5,
		2, 2, 1, 3, 4, 5,
		0, 0, 0, 0, 0, 0,
  },
};
// clang-format on
