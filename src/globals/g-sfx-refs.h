
#pragma once

#include "base/str.h"
#include "base/types.h"
#include "sfx/sfx.h"

enum g_sfx_id {
	G_SFX_NONE,

	G_SFX_BUMP_01,
	G_SFX_BUMP_02,
	G_SFX_BUMP_03,
	G_SFX_CHAIN_01,
	G_SFX_CHAIN_02,
	G_SFX_CHAIN_03,
	G_SFX_CLICK_01,
	G_SFX_CLICK_02,
	G_SFX_COLLIDE_01,
	G_SFX_COLLIDE_02,
	G_SFX_COLLIDE_03,
	G_SFX_COLLIDE_04,
	G_SFX_COLLIDE_05,
	G_SFX_DROP_01,
	G_SFX_DROP_02,
	G_SFX_MATCH_01,
	G_SFX_MATCH_02,
	G_SFX_MATCH_03,
	G_SFX_MATCH_04,
	G_SFX_ROTATE_01,
	G_SFX_MOVE_01,
	G_SFX_SPAWN_01,

	G_SFX_NUM_COUNT,
};

static const str8 G_SFX_PATH_MAP[G_SFX_NUM_COUNT] = {
	[G_SFX_BUMP_01]    = str8_lit_comp("assets/sfx/bump-01.snd"),
	[G_SFX_BUMP_02]    = str8_lit_comp("assets/sfx/bump-02.snd"),
	[G_SFX_BUMP_03]    = str8_lit_comp("assets/sfx/bump-03.snd"),
	[G_SFX_CHAIN_01]   = str8_lit_comp("assets/sfx/chain-01.snd"),
	[G_SFX_CHAIN_02]   = str8_lit_comp("assets/sfx/chain-02.snd"),
	[G_SFX_CHAIN_03]   = str8_lit_comp("assets/sfx/chain-03.snd"),
	[G_SFX_CLICK_01]   = str8_lit_comp("assets/sfx/click-01.snd"),
	[G_SFX_CLICK_02]   = str8_lit_comp("assets/sfx/click-02.snd"),
	[G_SFX_COLLIDE_01] = str8_lit_comp("assets/sfx/collide-01.snd"),
	[G_SFX_COLLIDE_02] = str8_lit_comp("assets/sfx/collide-02.snd"),
	[G_SFX_COLLIDE_03] = str8_lit_comp("assets/sfx/collide-03.snd"),
	[G_SFX_COLLIDE_04] = str8_lit_comp("assets/sfx/collide-04.snd"),
	[G_SFX_COLLIDE_05] = str8_lit_comp("assets/sfx/collide-05.snd"),
	[G_SFX_DROP_01]    = str8_lit_comp("assets/sfx/drop-01.snd"),
	[G_SFX_DROP_02]    = str8_lit_comp("assets/sfx/drop-02.snd"),
	[G_SFX_MATCH_01]   = str8_lit_comp("assets/sfx/match-01.snd"),
	[G_SFX_MATCH_02]   = str8_lit_comp("assets/sfx/match-02.snd"),
	[G_SFX_MATCH_03]   = str8_lit_comp("assets/sfx/match-03.snd"),
	[G_SFX_MATCH_04]   = str8_lit_comp("assets/sfx/match-04.snd"),
	[G_SFX_ROTATE_01]  = str8_lit_comp("assets/sfx/rotatate-01.snd"),
	[G_SFX_MOVE_01]    = str8_lit_comp("assets/sfx/move-01.snd"),
	[G_SFX_SPAWN_01]   = str8_lit_comp("assets/sfx/spawn-01.snd"),
};

enum g_sfx_seq_id {
	G_SFX_SEQ_NONE,

	G_SFX_SEQ_BUMP,
	G_SFX_SEQ_CHAIN,
	G_SFX_SEQ_CLICK,
	G_SFX_SEQ_COLLIDE,
	G_SFX_SEQ_DROP,
	G_SFX_SEQ_MATCH,

	G_SFX_SEQ_NUM_COUNT,
};

static const struct sfx_sequence G_SFX_SEQ_MAP[G_SFX_NUM_COUNT] = {
	[G_SFX_SEQ_BUMP] = {
		.type      = SFX_SEQUENCE_TYPE_RNDM,
		.vol_min   = 1.0f,
		.vol_max   = 1.0f,
		.pitch_min = 0.5f,
		.pitch_max = 1.0f,
	},
	[G_SFX_SEQ_CHAIN]   = {0},
	[G_SFX_SEQ_CLICK]   = {0},
	[G_SFX_SEQ_COLLIDE] = {0},
	[G_SFX_SEQ_DROP]    = {0},
	[G_SFX_SEQ_MATCH]   = {0},

};

static i32 G_SFX_IDS[G_SFX_NUM_COUNT];

static inline void
g_sfx_set_id(enum g_sfx_id index, usize id)
{
	G_SFX_IDS[index] = id;
}

static inline void
g_sfx_set_id_path(enum g_sfx_id index, str8 path)
{
	i32 id = asset_snd_get_id(path);
	dbg_assert(id > 0);
	G_SFX_IDS[index] = id;
}

static inline i32
g_sfx_get_id(enum g_sfx_id id)
{
	return G_SFX_IDS[id];
}

static inline void
g_sfx_refs_ini(void)
{
	for(usize i = 0; i < ARRLEN(G_SFX_PATH_MAP); ++i) {
		str8 path = G_SFX_PATH_MAP[i];
		if(path.size > 0) {
			g_sfx_set_id_path(i, path);
		}
	}
}

u32
g_sfx(enum g_sfx_id id, u16 repeat_count)
{
	// if(!g_settings_sfx_get()) { return 0; }

	struct snd sfx = asset_snd(g_sfx_get_id(id));
	f32 vol        = 1.0f;
	f32 pitch      = 1.0f;
	return snd_instance_play(sfx, vol, pitch, repeat_count);
}
