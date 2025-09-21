#pragma once

#include "base/str.h"
#include "base/types.h"
#include "base/utils.h"
#include "engine/assets/assets.h"

// G_ASSET_TEX
enum g_tex_id {
	G_TEX_NONE,

	G_TEX_HUD,
	G_TEX_SEEDS,
	G_TEX_SEEDS_BG,
	G_TEX_SHAPES,
	G_TEX_ACORN,
	G_TEX_ABSTRACT_SEEDS,
	G_TEX_FLOWER_01,
	G_TEX_FLOWER_02,
	G_TEX_FLOWER_03,
	G_TEX_FLOWER_04,
	G_TEX_FLOWER_05,
	G_TEX_FLOWER_06,

	G_TEX_NUM_COUNT,
};

static const str8 G_TEX_PATH_MAP[G_TEX_NUM_COUNT] = {
	[G_TEX_HUD]            = str8_lit_comp("assets/imgs/hud.tex"),
	[G_TEX_SEEDS_BG]       = str8_lit_comp("assets/imgs/seeds-bg.tex"),
	[G_TEX_SEEDS]          = str8_lit_comp("assets/imgs/seeds.tex"),
	[G_TEX_SHAPES]         = str8_lit_comp("assets/imgs/shapes.tex"),
	[G_TEX_ACORN]          = str8_lit_comp("assets/imgs/avellana.tex"),
	[G_TEX_ABSTRACT_SEEDS] = str8_lit_comp("assets/imgs/abstract-seeds.tex"),
	[G_TEX_FLOWER_01]      = str8_lit_comp("assets/imgs/flor-1.tex"),
	[G_TEX_FLOWER_02]      = str8_lit_comp("assets/imgs/flor-2.tex"),
	[G_TEX_FLOWER_03]      = str8_lit_comp("assets/imgs/flor-3.tex"),
	[G_TEX_FLOWER_04]      = str8_lit_comp("assets/imgs/flor-4.tex"),
	[G_TEX_FLOWER_05]      = str8_lit_comp("assets/imgs/flor-5.tex"),
	[G_TEX_FLOWER_06]      = str8_lit_comp("assets/imgs/flor-6.tex"),
};

static i32 G_TEX_IDS[G_TEX_NUM_COUNT];
static struct asset_tex_info G_TEX_INFO[G_TEX_NUM_COUNT];

static inline void
g_tex_refs_id_set(enum g_tex_id index, usize id)
{
	G_TEX_IDS[index] = id;
}

static inline void
g_tex_refs_id_path_set(enum g_tex_id index, str8 path)
{
	i32 id           = asset_tex_get_id(path);
	G_TEX_IDS[index] = id;
}

static inline i32
g_tex_refs_id_get(enum g_tex_id index)
{
	return G_TEX_IDS[index];
}

static inline void
g_tex_refs_info_set(enum g_tex_id index, str8 path)
{
	struct asset_handle handle =
		asset_db_handle_from_path(path, ASSET_TYPE_TEXTURE_INFO);
	struct asset_tex_info info = asset_db_tex_info_get(&ASSETS.db, handle);
	G_TEX_INFO[index]          = info;
}

static inline struct asset_tex_info
g_tex_refs_info_get(enum g_tex_id index)
{
	return G_TEX_INFO[index];
}

static inline void
g_tex_refs_ini(void)
{
	for(usize i = 0; i < ARRLEN(G_TEX_PATH_MAP); ++i) {
		str8 path = G_TEX_PATH_MAP[i];
		g_tex_refs_id_path_set(i, path);
		g_tex_refs_info_set(i, path);
	}
}
