#pragma once

#include "app/app-data.h"
#include "base/marena.h"
#include "base/mem.h"
#include "base/rec.h"
#include "base/str.h"
#include "globals/g-gfx.h"

struct g_dbg {
	b32 is_ini;
	struct marena marena;
	struct alloc alloc;
	struct str8_list str_list;
};

static struct g_dbg G_DBG = {0};

void
g_dbg_ini(struct alloc alloc)
{
	size size = MKILOBYTE(1);
	void *mem = alloc.allocf(alloc.ctx, size);
	marena_init(&G_DBG.marena, mem, size);
	G_DBG.alloc  = marena_allocator(&G_DBG.marena);
	G_DBG.is_ini = true;
}

void
g_dbg_clr(void)
{
	if(!G_DBG.is_ini) { return; }
	marena_reset(&G_DBG.marena);
	G_DBG.str_list = (struct str8_list){0};
}

void
g_dbg_drw(void)
{
	if(!G_DBG.is_ini) { return; }
	if(G_DBG.str_list.node_count == 0) { return; }
	struct str_join join   = {.sep = str8_lit("\n")};
	enum g_txt_style style = G_TXT_STYLE_DEBUG_SML;
	str8 str               = str8_list_join(G_DBG.alloc, &G_DBG.str_list, &join);
	v2_i32 old_offset      = g_drw_offset(0, 0);
	v2_i32 size            = g_txt_size(str, style);
	rec_i32 layout         = rec_i32_anchor(APP_SCRN_REC, rec_i32_expand((rec_i32){SYS_DISPLAY_W, 0, size.x, size.y}, 1), (v2){1.0f, 0.0f});

	g_color(PRIM_MODE_BLACK);
	g_rec_fill(REC_UNPACK(layout));
	g_spr_mode(SPR_MODE_WHITE);
	g_txt(str, layout.x + 1, layout.y + 1, style);

	g_drw_offset(old_offset.x, old_offset.y);
}

void
g_dbg_str(str8 str)
{
	if(!G_DBG.is_ini) { return; }
	str8_list_push(G_DBG.alloc, &G_DBG.str_list, str);
}
