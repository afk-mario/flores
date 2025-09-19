#include "falling.h"
#include "base/v2.h"
#include "block/block.h"
#include "board/board.h"
#include "engine/debug-draw/debug-draw.h"
#include "globals/g-gfx.h"
#include "piece/piece-data.h"

b32
falling_upd(struct falling *falling, struct board *board, f32 timestamp)
{
	b32 res        = false;
	i32 ody        = FALLING_SPD;
	i32 block_size = board->block_size;
	v2_i32 dest    = {falling->p.x, falling->p.y - 1};
	b32 col_block  = board_block_has(board, dest.x, dest.y);
	b32 col_wall   = board_is_wall_y(board, dest.y);
	b32 col        = col_block || col_wall;

	if(col) { return true; }

	falling->dy        = min_f32(falling->dy + 0.4f, 5.0f);
	falling->o.y       = falling->o.y + (FALLING_SPD * falling->dy);
	falling->timestamp = timestamp + PIECE_WAIT;

	if(falling->o.y < block_size) { return res; }

	falling->o.y = 0;

	if(col) {
		return true;
	}

	falling->p = dest;

	return false;
}

void
falling_drw(struct falling *falling, struct board *board, enum game_theme theme)
{
	i32 block_size   = board->block_size;
	f32 block_size_h = block_size / 2;
	v2_i32 px        = board_coords_to_px(board, falling->p.x, falling->p.y);
	v2_i32 px_o      = v2_add_i32(px, falling->o);
	struct block a   = {.type = falling->type};

#if defined(GAME_SHOW_COLS)
	g_pat(gfx_pattern_50());
	block_drw(&a, theme, px.x, px.y, block_size);
#endif

	g_pat(gfx_pattern_100());

	block_drw(&a, theme, px_o.x, px_o.y, block_size);
	debug_draw_cir_fill(px_o.x, px_o.y, 3);
}
