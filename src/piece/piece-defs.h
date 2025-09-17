#pragma once

#include "base/types.h"
#include "block/block-type.h"
#include "board/board-defs.h"
#include "frame/frame.h"

struct piece {
	i32 btn_buffer;
	b16 fast_drop;
	v2_i32 p;
	v2_i32 so;
	v2_i32 o;
	f32 ani_timestamp;
	f32 ani_duration;
	f32 timestamp;
	enum block_type types[2];
	void (*upd)(struct piece *piece, struct board *board, struct frame_info frame);
};
