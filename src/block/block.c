#include "block.h"
#include "base/v2.h"
#include "block/block-defs.h"
#include "block/block-type.h"
#include "engine/assets/assets.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/gfx/gfx.h"
#include "globals/g-gfx.h"
#include "globals/g-tex-refs.h"
#include "scrns/scrn-game/scrn-game-data.h"
#include "scrns/scrn-game/scrn-game-theme.h"

void
block_upd(struct block *block, i16 block_size)
{
	// if(block->type != BLOCK_TYPE_NONE) {
	// 	block->bx = block->x / block_size;
	// 	block->by = block->y / block_size;
	// }
}

void
block_drw(
	struct block *block,
	enum game_theme theme,
	i32 x,
	i32 y,
	i16 block_size)
{
	v2 p   = {x, y};
	i32 d  = 26;
	i32 r  = (block_size - 2) * 0.5f;
	i32 bg = GAME_BLOCK_BG[theme];
	i32 fg = GAME_BLOCK_FG[theme];

	if(bg > -1) {
		g_color(bg);
		g_cir_fill((p.x + 1) + r, (p.y + 1) + r, d);
	}

	g_color(PRIM_MODE_WHITE);
	switch(block->type) {
	case BLOCK_TYPE_A: {
	} break;
	case BLOCK_TYPE_B: {
	} break;
	case BLOCK_TYPE_C: {
	} break;
	default: {
	} break;
	}

	enum g_tex_id ref      = GAME_THEME_REFS[theme];
	i32 id                 = g_tex_refs_id_get(ref);
	struct tex t           = asset_tex(id);
	struct tex_rec tex_rec = asset_tex_rec(id, (block->type - 1) * t.h, 0, t.h, t.h);
	g_spr_mode(SPR_MODE_COPY);
	g_spr(tex_rec, p.x, p.y, 0);
	if(block->state == 1) {
		if(fg > -1) {
			g_color(fg);
			g_cir((p.x + 1) + r, (p.y + 1) + r, d - 2);
		}
	}
}

void
block_link_drw(
	enum game_theme theme,
	i32 x,
	i32 y,
	i32 rx,
	i32 ry,
	i16 block_size)
{
	i32 d              = BLOCK_LINK_D;
	i32 r              = d / 2;
	i32 block_size_h   = block_size / 2;
	i32 bg             = GAME_BLOCK_LINK[theme];
	v2_i32 link_offset = {
		rx * (block_size_h - r),
		-ry * (block_size_h + r),
	};

	g_color(bg);
	{
		i32 cx = (x + block_size_h) + link_offset.x;
		i32 cy = (y + block_size_h) + link_offset.y;
		{
			g_cir_fill(cx, cy, d);
			debug_draw_cir(cx, cy, d);
		}
	}
	{
		x      = x + block_size * rx;
		y      = y + block_size * -ry;
		i32 cx = (x + block_size_h) + -link_offset.x;
		i32 cy = (y + block_size_h) + -link_offset.y;
		{
			g_cir_fill(cx, cy, d);
			debug_draw_cir(cx, cy, d);
		}
	}
}
