#include "flower.h"
#include "base/mathfunc.h"
#include "base/types.h"
#include "engine/gfx/gfx-defs.h"
#include "garden/flower-data.h"
#include "garden/garden-data.h"
#include "globals/g-gfx.h"
#include "lib/rndm.h"

#define FLOWER_ANGLE_DELTA 25
#define FLOWER_ANGLE_START -90

static inline str8
flower_generate(struct alloc alloc, struct flower_rules *rules, struct str8 axiom);

void
flower_ini(struct flower *flower, struct alloc alloc, struct tex t)
{
	size mem_size = MKILOBYTE(50);
	void *mem     = alloc.allocf(alloc.ctx, mem_size);
	marena_init(&flower->marena, mem, mem_size);
	enum block_type type = flower->type;
	flower->alloc        = marena_allocator(&flower->marena);
	flower->ctx          = gfx_ctx_default(t);
}

void
flower_upd(struct flower *flower, struct frame_info frame)
{
	if(flower->type == BLOCK_TYPE_NONE) { return; }

	f32 timestamp              = frame.timestamp;
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	f32 dur                    = (timestamp - flower->timestamp) / 1.0f;
	f32 t                      = clamp_f32(dur, 0.0f, 1.0f);
	f32 wind_force             = (5 * DEG_TO_TURN);
	f32 wind                   = sin_f32(timestamp - flower->timestamp) * wind_force;
	f32 angle_start            = flower->start_angle_og;
	flower->start_angle        = angle_start + wind;
	// flower->length             = lerp(0, rules->length, t);
	// flower->angle_delta          = lerp(0, rules->angle_delta, t) * DEG_TO_TURN;
	tex_clr(flower->ctx.dst, GFX_COL_CLEAR);
	flower_upd_tex(flower, t);
}

void
flower_drw(struct flower *flower, i32 x, i32 y)
{
	if(flower->type == BLOCK_TYPE_NONE) { return; }
	struct tex t           = flower->ctx.dst;
	struct tex_rec tex_rec = {.t = t, .r = {0, 0, t.w, t.h}};
	g_spr_piv(tex_rec, flower->p.x + x, flower->p.y + y, 0, (v2){0.5f, 0.0f});
}

void
flower_type_set(struct flower *flower, enum block_type type, f32 timestamp)
{
	struct flower_rules *rules = FLOWERS_RULES + type;
	f32 len_rndm               = rndm_range_f32(NULL, -3.0, 3.0f);
	f32 angle_delta_rndm       = rndm_range_f32(NULL, -10.0f, 10.0f);
	f32 angle_start_rndm       = rndm_range_f32(NULL, -10.0f, 10.0f);
	flower->type               = type;
	flower->iterations         = 1;
	flower->axiom              = rules->axiom;
	flower->angle_delta        = (rules->angle_delta + angle_delta_rndm) * DEG_TO_TURN;
	flower->length             = rules->length + len_rndm;
	flower->start_angle_og     = (FLOWER_ANGLE_START + angle_start_rndm) * DEG_TO_TURN;
	flower->start_angle        = flower->start_angle_og;
	flower->timestamp          = timestamp;
	flower_reevaluate(flower);
}

void
flower_iterations_add(struct flower *flower, i32 value, f32 timestamp)
{
	flower_iterations_set(flower, flower->iterations + 1, timestamp);
}

void
flower_iterations_sub(struct flower *flower, i32 value, f32 timestamp)
{
	flower_iterations_set(flower, flower->iterations - 1, timestamp);
}

void
flower_iterations_max(struct flower *flower, f32 timestamp)
{
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	flower_iterations_set(flower, rules->iterations_max, timestamp);
}

void
flower_iterations_set(struct flower *flower, i32 value, f32 timestamp)
{
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	flower->iterations         = clamp_i32(value, 0, rules->iterations_max);
	flower_reevaluate(flower);
}

void
flower_reevaluate(struct flower *flower)
{
	marena_reset(&flower->marena);
	str8 next                  = flower->axiom;
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	for(size i = 0; i < flower->iterations; ++i) {
		next = flower_generate(flower->alloc, rules, next);
	}
	log_info("garden", "Rem: %$$u Used: %$$u type: %s", (uint)flower->marena.rem, (uint)flower->marena.buf_size - flower->marena.rem, BLOCK_TYPE_LABELS[flower->type]);
	flower->next = next;

	tex_clr(flower->ctx.dst, GFX_COL_CLEAR);
	flower_upd_tex(flower, 1.0f);
}

static inline str8
flower_generate(struct alloc alloc, struct flower_rules *rules, struct str8 axiom)
{
	struct str8 res       = {0};
	struct str8_list list = {0};

	for(size i = 0; i < (size)axiom.size; ++i) {
		u8 c      = axiom.str[i];
		str8 rule = rules->rules[c];
		if(rule.size > 0) {
			str8_list_push(alloc, &list, rule);
		} else {
			str8_list_pushf(alloc, &list, "%c", c);
		}
	}
	res = str8_list_join(alloc, &list, NULL);
	return res;
}

void
flower_upd_tex(struct flower *flower, f32 t)
{
	struct gfx_ctx old             = g_drw_ctx(flower->ctx);
	i32 length                     = flower->length;
	f32 angle_delta                = flower->angle_delta;
	rec_i32 layout                 = {0, 0, flower->ctx.dst.w, flower->ctx.dst.h};
	struct str8 next               = flower->next;
	struct flower_state stack[100] = {0};
	i32 stack_ptr                  = 0;
	stack[stack_ptr]               = (struct flower_state){
					  .p.x   = layout.x + (layout.w * 0.5f),
					  .p.y   = layout.y + (layout.h),
					  .angle = flower->start_angle,
    };

	g_pat(gfx_pattern_100());
	g_color(PRIM_MODE_WHITE);
	i32 count = next.size * t;
	for(size i = 0; i < count; ++i) {
		struct flower_state *state = stack + stack_ptr;
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
