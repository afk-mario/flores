#include "block.h"
#include "block/block-defs.h"
#include "block/block-type.h"
#include "globals/g-gfx.h"

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

	// g_color(PRIM_MODE_WHITE);
	// g_rrec_fill(r.x - 1, r.y - 1, r.w + 2, r.h + 2, 3);
	// g_color(PRIM_MODE_BLACK);
	// g_rec_fill(r.x - 1, r.y - 1, r.w + 2, r.h + 2);
	// g_rrec_fill(r.x, r.y, r.w, r.h, 3);
	g_color(PRIM_MODE_WHITE);
	g_cir(cx, cy, r.w - 1);

	i32 d = w - 18;
	g_color(PRIM_MODE_WHITE);
	switch(block->type) {
	case BLOCK_TYPE_A: {
		g_cir_fill(cx, cy, d);
	} break;
	case BLOCK_TYPE_B: {
		g_rec_fill(cx - (d * 0.5f), cy - (d * 0.5f), d, d);
	} break;
	case BLOCK_TYPE_C: {
		i32 margin = ((w - d) * 0.5f);
		i32 tx     = x - 1;
		i32 tcx    = cx - 1;
		g_tri_fill(
			tcx,
			y + margin,
			tx + margin,
			y + block_size - margin,
			tx + block_size - margin,
			y + block_size - margin);
	} break;
	default: {
	} break;
	}
}
