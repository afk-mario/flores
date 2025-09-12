#pragma once

#include "engine/audio/audio.h"
#include "g-mus-refs.h"

#define G_MAX_MUS_FADE 3

enum fade_cmd {
	FADE_CMD_NONE,

	FADE_CMD_STOP,
};

struct mus_fade {
	enum mus_channel_id channel_id;
	enum fade_cmd cmd;
	f32 vol_i;
	f32 vol_f;
	f32 timestamp_start;
	f32 duration;
};

static struct mus_fade G_MUS_FADE[G_MAX_MUS_FADE];
enum mus_channel_id g_mus_play(enum g_mus_id id, b32 fade_in, b32 loop);
void g_mus_vol_set(f32 vol);
void g_mus_stop(b32 fade_out);
void g_mus_upd(f32 dt);
void g_mus_stop_all(void);
