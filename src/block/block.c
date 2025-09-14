#include "block.h"
#include "block/block-defs.h"
#include "block/block-type.h"
#include "board/board-defs.h"
#include "engine/assets/assets.h"
#include "engine/gfx/gfx.h"
#include "globals/g-gfx.h"
#include "globals/g-tex-refs.h"

void
block_drw(struct block *block, u8 block_size)
{
	i32 w     = block_size;
	i32 h     = block_size;
	i32 x     = block->x * block_size;
	i32 y     = block->y * block_size;
	i32 cx    = x + (w * 0.5f);
	i32 cy    = y + (h * 0.5f);
	rec_i32 r = {.x = x + 1, .y = y + 1, .w = w - 2, .h = h - 2};

#if BOARD_USE_SHAPES
	i32 id = g_tex_refs_id_get(G_TEX_SHAPES);
#else
	i32 id = g_tex_refs_id_get(G_TEX_SEEDS);
#endif
	struct tex t           = asset_tex(id);
	struct tex_rec tex_rec = asset_tex_rec(id, 0, 0, t.h, t.h);

#if BOARD_USE_SHAPES
	if(block->type != 0) {
		g_pat(gfx_pattern_50());
	}
	g_color(PRIM_MODE_WHITE);
	g_cir(cx, cy, r.w - 1);
	g_pat(gfx_pattern_100());
#endif

	i32 d = w - 18;
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
	tex_rec = asset_tex_rec(id, (block->type - 1) * 28, 0, 28, t.h);
	g_spr_mode(SPR_MODE_COPY);
	g_spr_piv(tex_rec, cx, cy, 0, (v2){0.5f, 0.5f});
	g_spr_mode(SPR_MODE_COPY);
}
