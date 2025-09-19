#pragma once

#include "base/dbg.h"
#include "base/mathfunc.h"
#include "base/types.h"
#include "engine/assets/assets.h"
#include "engine/audio/audio.h"
#include "lib/rndm.h"

enum sfx_sequence_type {
	SFX_SEQUENCE_TYPE_NONE,
	SFX_SEQUENCE_TYPE_RNDM,
	SFX_SEQUENCE_TYPE_LOOP,
	SFX_SEQUENCE_TYPE_HOLD,
};

struct sfx_sequence {
	enum sfx_sequence_type type;
	f32 reset_time;
	f32 reset_timestamp;
	f32 vol_min;
	f32 vol_max;
	f32 pitch_min;
	f32 pitch_max;
	usize clip_index;
	size clips_len;
	i32 *clips;
};

u32
sfx_sequence_play(struct sfx_sequence *sequence, f32 timestamp)
{
	if(sequence->reset_timestamp && sequence->reset_timestamp < timestamp) {
		sequence->clip_index = 0;
	}
	switch(sequence->type) {
	case SFX_SEQUENCE_TYPE_RNDM: {
		sequence->clip_index = rndm_next_i32(NULL) % sequence->clips_len;
	} break;
	case SFX_SEQUENCE_TYPE_LOOP: {
		sequence->clip_index = (sequence->clip_index + 1) % sequence->clips_len;
	} break;
	case SFX_SEQUENCE_TYPE_HOLD: {
		sequence->clip_index = min_i32(sequence->clip_index + 1, sequence->clips_len - 1);
	} break;
	default: {
		dbg_sentinel("sfx");
	} break;
	}

	if(sequence->reset_time > 0) {
		sequence->reset_timestamp = timestamp + sequence->reset_time;
	}

	struct snd sfx = asset_snd(sequence->clips[sequence->clip_index]);
	f32 vol        = rndm_range_f32(NULL, sequence->vol_min, sequence->vol_max);
	f32 pitch      = rndm_range_f32(NULL, sequence->pitch_min, sequence->pitch_max);
	return snd_instance_play(sfx, vol, pitch, 1);

error:
	return 0;
}
