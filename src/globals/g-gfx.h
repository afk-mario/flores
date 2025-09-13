#pragma once

#include "base/mathfunc.h"
#include "engine/gfx/gfx-spr.h"
#include "engine/gfx/gfx-txt.h"
#include "engine/gfx/gfx.h"

enum g_txt_style {
	G_TXT_STYLE_NONE,

	G_TXT_STYLE_DEBUG,
	G_TXT_STYLE_DEBUG_SML,
	G_TXT_STYLE_HUD,

	G_TXT_STYLE_NUM_COUNT,
};

struct txt_style {
	i32 tracking;
	i32 leading;
	enum spr_mode mode;
	struct fnt fnt;
};

struct g_gfx {
	struct gfx_ctx ctx;
	v2_i32 offset;
	enum prim_mode prim_mode; // color
	enum spr_mode spr_mode;
	struct txt_style txt_styles[G_TXT_STYLE_NUM_COUNT];
};

static struct g_gfx G_GFX = {0};

struct gfx_ctx
g_drw_ctx(struct gfx_ctx value)
{
	struct gfx_ctx old = G_GFX.ctx;
	if(value.dst.px != NULL) {
		G_GFX.ctx = value;
	}
	return old;
}

v2_i32
g_drw_offset(i32 x, i32 y)
{
	v2_i32 old   = G_GFX.offset;
	G_GFX.offset = (v2_i32){x, y};
	return old;
}

enum prim_mode
g_color(enum prim_mode mode)
{
	enum prim_mode old = G_GFX.prim_mode;
	G_GFX.prim_mode    = mode;
	return old;
}

struct gfx_pattern
g_pat(struct gfx_pattern pat)
{
	struct gfx_pattern old = G_GFX.ctx.pat;
	G_GFX.ctx.pat          = pat;
	return old;
}

struct txt_style
g_txt_style(enum g_txt_style style, struct txt_style value)
{
	struct txt_style old = G_GFX.txt_styles[style];
	if(value.fnt.t.px != 0) {
		G_GFX.txt_styles[style] = value;
	}
	return old;
}

enum spr_mode
g_spr_mode(enum spr_mode mode)
{
	enum spr_mode old = G_GFX.spr_mode;
	G_GFX.spr_mode    = mode;
	return old;
}

void
g_rec(i32 x, i32 y, i32 w, i32 h)
{
	gfx_rec(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, w, h, G_GFX.prim_mode);
}

void
g_rec_fill(i32 x, i32 y, i32 w, i32 h)
{
	gfx_rec_fill(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, w, h, G_GFX.prim_mode);
}

void
g_rrec_fill(i32 x, i32 y, i32 w, i32 h, i32 r)
{
	gfx_rrec_fill(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, w, h, r, G_GFX.prim_mode);
}

void
g_cir(i32 x, i32 y, i32 d)
{
	gfx_cir(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, d, G_GFX.prim_mode);
}

void
g_cir_fill(i32 x, i32 y, i32 d)
{
	gfx_cir_fill(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, d, G_GFX.prim_mode);
}

void
g_ellipsis(i32 x, i32 y, i32 rx, i32 ry)
{
	gfx_ellipsis(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, rx, ry, G_GFX.prim_mode);
}

void
g_arc(i32 x, i32 y, f32 start_angle, f32 end_angle, i32 r)
{
	gfx_arc(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, 255 * mod_euc_f32(-start_angle, 1.0f), 255 * mod_euc_f32(-end_angle, 1.0f), r, G_GFX.prim_mode);
}

void
g_arc_thick(i32 x, i32 y, f32 start_angle, f32 end_angle, i32 r, i32 thick)
{
	gfx_arc_thick(G_GFX.ctx, x + G_GFX.offset.x, y + G_GFX.offset.y, 255 * mod_euc_f32(-start_angle, 1.0f), 255 * mod_euc_f32(-end_angle, 1.0f), r, thick, G_GFX.prim_mode);
}

void
g_lin(i32 ax, i32 ay, i32 bx, i32 by)
{
	gfx_lin(G_GFX.ctx, ax + G_GFX.offset.x, ay + G_GFX.offset.y, bx + G_GFX.offset.x, by + G_GFX.offset.y, G_GFX.prim_mode);
}

void
g_lin_thick(i32 ax, i32 ay, i32 bx, i32 by, i32 r)
{
	gfx_lin_thick(G_GFX.ctx, ax + G_GFX.offset.x, ay + G_GFX.offset.y, bx + G_GFX.offset.x, by + G_GFX.offset.y, r, G_GFX.prim_mode);
}

void
g_spr(struct tex_rec src, i32 x, i32 y, enum spr_flip flip)
{
	gfx_spr(G_GFX.ctx, src, x + G_GFX.offset.x, y + G_GFX.offset.y, flip, G_GFX.spr_mode);
}

void
g_spr_piv(struct tex_rec src, i32 x, i32 y, enum spr_flip flip, v2 piv)
{
	i32 px = x + G_GFX.offset.x;
	i32 py = y + G_GFX.offset.y;
	gfx_spr(G_GFX.ctx, src, px - (src.r.w * piv.x), py - (src.r.h * piv.y), flip, G_GFX.spr_mode);
}

void
g_txt(str8 str, i32 x, i32 y, enum g_txt_style g_style)
{
	enum g_txt_style styl =
		g_style == G_TXT_STYLE_NONE ? G_TXT_STYLE_DEBUG : g_style;
	struct txt_style style     = G_GFX.txt_styles[styl];
	enum spr_mode old_spr_mode = g_spr_mode(0);

	if(style.fnt.t.px != NULL) {
		g_spr_mode(style.mode);
		fnt_draw_str(G_GFX.ctx, style.fnt, str, x + G_GFX.offset.x, y + G_GFX.offset.y, style.tracking, style.leading, style.mode);
	}
	g_spr_mode(old_spr_mode);
}

v2_i32
g_txt_size(str8 str, enum g_txt_style g_style)
{
	v2_i32 res = {0};
	enum g_txt_style styl =
		g_style == G_TXT_STYLE_NONE ? G_TXT_STYLE_DEBUG : g_style;
	struct txt_style style = G_GFX.txt_styles[styl];
	if(style.fnt.t.px != NULL) {
		res = fnt_size_px(style.fnt, str, style.tracking, style.leading);
	}
	return res;
}

rec_i32
g_txt_pivot(str8 str, i32 x, i32 y, v2 pivot, enum g_txt_style g_style)
{
	enum g_txt_style styl =
		g_style == G_TXT_STYLE_NONE ? G_TXT_STYLE_DEBUG : g_style;
	struct txt_style style = G_GFX.txt_styles[styl];
	if(style.fnt.t.px != NULL) {
		v2_i32 text_size =
			fnt_size_px(style.fnt, str, style.tracking, style.leading);
		i32 txt_x = x - (i32)(text_size.x * pivot.x);
		i32 txt_y = y - (i32)(text_size.y * pivot.y);
		g_txt(str, txt_x, txt_y, g_style);
		return (rec_i32){txt_x, txt_y, text_size.x, text_size.y};
	}
	return (rec_i32){0};
}
