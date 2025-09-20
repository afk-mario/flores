#include "garden.h"
#include "base/dbg.h"
#include "base/marena.h"
#include "base/mathfunc.h"
#include "base/mem.h"
#include "base/rec.h"
#include "base/str.h"
#include "base/types.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/gfx/gfx-defs.h"
#include "engine/gfx/gfx.h"
#include "engine/input.h"
#include "garden/garden-data.h"
#include "globals/g-gfx.h"
#include "lib/tex/tex.h"
#include "sys/sys.h"

#define GARDEN_ITERATIONS  1
#define GARDEN_LENGTH      5
#define GARDEN_ANGLE_DELTA 25
#define GARDEN_ANGLE_START -90
#define GARDEN_TEX_W       40

static inline str8 garden_generate(struct alloc alloc, struct garden_rules *rules, struct str8 axiom);
static inline void garden_cell_reevaluate(struct garden_cell *cell);
static inline void garden_cell_drw(struct garden_cell *cell, f32 t);

void
garden_ini(struct garden *garden, struct alloc alloc, f32 timestamp)
{
	size mem_size = MKILOBYTE(50);
	for(size i = 0; i < (size)ARRLEN(garden->cells); ++i) {
		struct garden_cell *cell = garden->cells + i;
		void *mem                = alloc.allocf(alloc.ctx, mem_size);
		mclr(mem, mem_size);
		marena_init(&cell->marena, mem, mem_size);
		enum block_type type = (i % 5) + 1;
		cell->alloc          = marena_allocator(&cell->marena);
		cell->axiom          = GARDEN_RULES[type].axiom;
		cell->angle_delta    = GARDEN_RULES[type].angle_delta * DEG_TO_TURN;
		cell->length         = GARDEN_RULES[type].length;
		cell->iterations     = GARDEN_RULES[type].iterations_max;
		cell->type           = type;
		cell->timestamp      = timestamp;
		cell->start_angle    = GARDEN_ANGLE_START * DEG_TO_TURN;
		garden_cell_reevaluate(cell);
	}
}

void
garden_upd(struct garden *garden, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	if(inp_key_just_pressed('m')) {
		for(size i = 0; i < (size)ARRLEN(garden->cells); ++i) {
			struct garden_cell *cell   = garden->cells + i;
			struct garden_rules *rules = GARDEN_RULES + cell->type;
			cell->iterations           = MIN(rules->iterations_max, cell->iterations + 1);
			cell->timestamp            = timestamp;
			garden_cell_reevaluate(cell);
		}
	}
	if(inp_key_just_pressed('n')) {
		for(size i = 0; i < (size)ARRLEN(garden->cells); ++i) {
			struct garden_cell *cell = garden->cells + i;
			cell->iterations         = MAX(0, cell->iterations - 1);
			cell->timestamp          = timestamp;
			garden_cell_reevaluate(cell);
		}
	}
#if 1
	for(size i = 0; i < (size)ARRLEN(garden->cells); ++i) {
		struct garden_cell *cell   = garden->cells + i;
		struct garden_rules *rules = GARDEN_RULES + cell->type;
		f32 dur                    = (timestamp - cell->timestamp) / 1.0f;
		f32 t                      = clamp_f32(dur, 0.0f, 1.0f);
		f32 wind                   = sin_f32(timestamp - cell->timestamp) * (12 * DEG_TO_TURN);
		f32 angle_start            = GARDEN_ANGLE_START * DEG_TO_TURN;
		cell->angle_delta          = lerp(0, rules->angle_delta, t) * DEG_TO_TURN;
		cell->length               = lerp(0, rules->length, t);
		cell->start_angle          = angle_start + wind;
		garden_cell_drw(cell, t);
	}
#endif
}

void
garden_drw(struct garden *garden, rec_i32 rec, enum game_theme theme)
{
	rec_i32 root    = rec;
	size cell_count = ARRLEN(garden->cells);
	size cell_size  = root.w / cell_count;

	for(size i = 0; i < cell_count; ++i) {
		struct garden_cell *cell = garden->cells + i;
		rec_i32 layout           = rec_i32_cut_left(&root, cell_size);
		layout.x += cell->offset.x;
		debug_draw_rec(REC_UNPACK(layout));
		struct tex t           = cell->ctx.dst;
		struct tex_rec tex_rec = {.t = t, .r = {0, 0, t.w, t.h}};
		g_spr_mode(SPR_MODE_WHITE_ONLY);
		g_spr_piv(tex_rec, layout.x + (layout.w * 0.5f), layout.y + layout.h, 0, (v2){0.5, 1.0f});
		debug_draw_rec(layout.x, layout.y, t.w, t.h);
	}
}

static inline void
garden_cell_reevaluate(struct garden_cell *cell)
{
	marena_reset(&cell->marena);
	str8 next                  = cell->axiom;
	struct garden_rules *rules = GARDEN_RULES + cell->type;
	for(size i = 0; i < cell->iterations; ++i) {
		next = garden_generate(cell->alloc, rules, next);
	}
	log_info("garden", "Rem: %$$u Used: %$$u type: %s", (uint)cell->marena.rem, (uint)cell->marena.buf_size - cell->marena.rem, BLOCK_TYPE_LABELS[cell->type]);
	cell->next = next;

	i32 w        = GARDEN_TEX_W;
	i32 h        = SYS_DISPLAY_H;
	struct tex t = tex_create(w, h, cell->alloc);
	cell->ctx    = gfx_ctx_default(t);
	garden_cell_drw(cell, 1.0f);
}

static inline str8
garden_generate(struct alloc alloc, struct garden_rules *rules, struct str8 axiom)
{
	struct str8 res       = {0};
	struct str8_list list = {0};

	for(size i = 0; i < (size)axiom.size; ++i) {
		u8 c     = axiom.str[i];
		str8 res = rules->rules[c];
		if(res.size > 0) {
			str8_list_push(alloc, &list, res);
		} else {
			str8_list_pushf(alloc, &list, "%c", c);
		}
	}
	res = str8_list_join(alloc, &list, NULL);
	return res;
}

static inline void
garden_cell_drw(struct garden_cell *cell, f32 t)
{
	struct gfx_ctx old = g_drw_ctx(cell->ctx);
	tex_clr(cell->ctx.dst, GFX_COL_CLEAR);
	i32 length                     = cell->length;
	f32 angle_delta                = cell->angle_delta;
	rec_i32 layout                 = {0, 0, cell->ctx.dst.w, cell->ctx.dst.h};
	struct str8 next               = cell->next;
	struct garden_state stack[100] = {0};
	i32 stack_ptr                  = 0;
	stack[stack_ptr]               = (struct garden_state){
					  .p.x   = layout.x + (layout.w * 0.5f),
					  .p.y   = layout.y + (layout.h),
					  .angle = cell->start_angle,
    };

	g_pat(gfx_pattern_100());
	g_color(PRIM_MODE_WHITE);
	i32 count = next.size * t;
	for(size i = 0; i < count; ++i) {
		struct garden_state *state = stack + stack_ptr;
		u8 c                       = next.str[i];
		u8 nc                      = (i + 1 < (size)next.size) ? next.str[i + 1] : 0;
		switch(c) {
		case 'F': {
			i32 xb = state->p.x + (length * state->rot.c);
			i32 yb = state->p.y + (length * state->rot.s);
			g_lin(state->p.x, state->p.y, xb, yb);
			state->p.x = xb;
			state->p.y = yb;
			if(nc != 'F' && nc != 'G' && nc != '[' && nc != 'X') {
			}

		} break;
		case 'f':
		case 'G': {
			i32 xb     = state->p.x + (length * state->rot.c);
			i32 yb     = state->p.y + (length * state->rot.s);
			state->p.x = xb;
			state->p.y = yb;
		} break;
		case '+': {
			state->angle += angle_delta;
			state->rot.s = sin_f32(state->angle * TURN_TO_RAD);
			state->rot.c = cos_f32(state->angle * TURN_TO_RAD);
		} break;
		case '-': {
			state->angle -= angle_delta;
			state->rot.s = sin_f32(state->angle * TURN_TO_RAD);
			state->rot.c = cos_f32(state->angle * TURN_TO_RAD);
		} break;
		case '&': {
			state->angle = -state->angle;
			state->rot.s = sin_f32(state->angle * TURN_TO_RAD);
			state->rot.c = cos_f32(state->angle * TURN_TO_RAD);
		} break;
		case 'x': {
			state->angle += 2 * DEG_TO_TURN;
			state->rot.s = sin_f32(state->angle * TURN_TO_RAD);
			state->rot.c = cos_f32(state->angle * TURN_TO_RAD);
		} break;
		case 'y': {
			state->angle -= 1.5f * DEG_TO_TURN;
			state->rot.s = sin_f32(state->angle * TURN_TO_RAD);
			state->rot.c = cos_f32(state->angle * TURN_TO_RAD);
		} break;
		case '[': {
			dbg_assert(stack_ptr < (size)ARRLEN(stack));
			stack[++stack_ptr] = *state;
		} break;
		case ']': {
			dbg_assert(stack_ptr > 0);
			// g_cir_fill(state->p.x, state->p.y, 3); // tip of branch
			// debug_draw_cir_fill(state->p.x, state->p.y, 3);
			stack_ptr--;
		} break;
		case 'c': {
			// g_pat(gfx_pattern_100());
		} break;
		case 'd': {
			// g_pat(gfx_pattern_50());
		} break;
		case 'e': {
			// g_pat(gfx_pattern_50());
		} break;
		default: {
		} break;
		}
	}
	g_drw_ctx(old);
}
