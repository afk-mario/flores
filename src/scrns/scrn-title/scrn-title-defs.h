#include "frame/frame.h"
#include "garden/garden-defs.h"

struct scrn_title {
	f32 timestamp;
	struct frame_info frame;
	struct garden garden;
	b32 transition_in;
	f32 transition_in_t;
};
