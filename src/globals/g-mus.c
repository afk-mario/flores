#include "g-mus.h"

#include "base/mathfunc.h"
#include "lib/easing.h"

#include "base/utils.h"

#include "g-mus-data.h"

static i32 MUSIC_CHANNEL;
static f32 G_AUD_TIMESTAMP = 0;
static enum g_mus_id MUSIC_ID;

static void g_mus_fade(struct mus_fade *fade, f32 timestamp);

enum mus_channel_id
g_mus_play(enum g_mus_id ref, b32 fade_in, b32 loop)
{
	// if(!g_settings_mus_get()) { return 0; }
	if(ref == MUSIC_ID) { return MUSIC_CHANNEL; }

	MUSIC_CHANNEL = MUSIC_CHANNEL == 1 ? 2 : 1;
	MUSIC_ID      = ref;

	f32 vol                    = fade_in ? 0 : G_MUS_VOL;
	struct asset_handle handle = g_mus_refs_handle_get(ref);
	mus_play(handle, MUSIC_CHANNEL, vol, loop);
	if(fade_in) {
		G_MUS_FADE[MUSIC_CHANNEL] = (struct mus_fade){
			.channel_id      = MUSIC_CHANNEL,
			.duration        = G_MUS_FADE_IN_DURATION,
			.timestamp_start = G_AUD_TIMESTAMP,
			.vol_i           = 0.0f,
			.vol_f           = G_MUS_VOL,
		};
	}

	return MUSIC_CHANNEL;
}

void
g_mus_vol_set(f32 vol)
{
	mus_set_vol(MUSIC_CHANNEL, vol);
}

void
g_mus_stop(b32 fade_out)
{
	MUSIC_ID = 0;
	if(MUSIC_CHANNEL != 0) {
		if(fade_out) {
			G_MUS_FADE[MUSIC_CHANNEL] = (struct mus_fade){
				.channel_id      = MUSIC_CHANNEL,
				.duration        = G_MUS_FADE_OUT_DURATION,
				.timestamp_start = G_AUD_TIMESTAMP,
				.vol_i           = G_MUS_VOL,
				.vol_f           = 0.0f,
				.cmd             = FADE_CMD_STOP,
			};
		} else {
			mus_stop(MUSIC_CHANNEL);
		}
	}
}

void
g_mus_upd(f32 dt)
{
	G_AUD_TIMESTAMP += dt;
	f32 timestamp = G_AUD_TIMESTAMP;

	for(usize i = 1; i < ARRLEN(G_MUS_FADE); i++) {
		g_mus_fade(G_MUS_FADE + i, timestamp);
	}
}

static void
g_mus_fade(struct mus_fade *fade, f32 timestamp)
{
	if(fade->channel_id == AUD_MUS_CHANNEL_NONE) { return; }
	if(fade->timestamp_start > timestamp) { return; }

	if(mus_is_playing(fade->channel_id)) {
		f32 t = min_f32(1.0, (timestamp - fade->timestamp_start) / fade->duration);
		if(fade->vol_i < fade->vol_f) {
			t = ease_expo_in(t);
		} else {
			t = ease_expo_out(t);
		}
		f32 vol = lerp(fade->vol_i, fade->vol_f, t);
		mus_set_vol(fade->channel_id, vol);
		if(t == 1.0f) {
			if(fade->cmd == FADE_CMD_STOP) {
				mus_stop(fade->channel_id);
			}
			fade->channel_id = 0;
		}
	}
}

void
g_mus_stop_all(void)
{
	MUSIC_ID = 0;
	for(usize i = AUD_MUS_CHANNEL_1; i < AUD_MUS_CHANNEL_NUM_COUNT; ++i) {
		mus_stop(i);
	}
}
