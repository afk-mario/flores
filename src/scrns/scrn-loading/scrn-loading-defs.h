#include "base/types.h"

struct loading_state {
	b32 is_finished;
	usize total;
	usize loaded;
	usize loaded_sub;
	b32 (*load)(struct loading_state *state, f32 time_start, f32 time_max, void *args);
};

struct scrn_loading {
	f32 timestamp;
	struct loading_state loading_state;
	struct loading_state *loadings;
};
