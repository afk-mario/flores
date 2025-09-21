#include "flower.h"
#include "base/mathfunc.h"
#include "base/str.h"
#include "base/types.h"
#include "base/v2.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/gfx/gfx-defs.h"
#include "engine/gfx/gfx.h"
#include "garden/flower-data.h"
#include "garden/garden-data.h"
#include "globals/g-gfx.h"
#include "globals/g-tex-refs.h"
#include "lib/rndm.h"

#define FLOWER_ANGLE_DELTA      25
#define FLOWER_ANGLE_START      -90
#define FLOWER_WATER_FOR_GROWTH 10

static inline void flower_bud_drw(struct flower *flower, i32 stage, i32 x, i32 y);

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
	flower->start_angle        = angle_start - wind;
	// flower->length             = lerp(0, rules->length, t);
	// flower->angle_delta          = lerp(0, rules->angle_delta, t) * DEG_TO_TURN;
	flower_upd_tex(flower, frame.alloc, 1.0f);
}

void
flower_drw(struct flower *flower, i32 x, i32 y, i32 day)
{
	if(flower->type == BLOCK_TYPE_NONE) { return; }
	struct tex t           = flower->ctx.dst;
	struct tex_rec tex_rec = {.t = t, .r = {0, 0, t.w, t.h}};
	if(day) {
		g_spr_mode(SPR_MODE_COPY);
	} else {
		g_spr_mode(SPR_MODE_INV);
	}
	g_spr_piv(tex_rec, flower->p.x + x, flower->p.y + y, 0, (v2){0.5f, 0.0f});
}

void
flower_type_set(struct flower *flower, enum block_type type, struct frame_info frame)
{
	struct flower_rules *rules = FLOWERS_RULES + type;
	f32 len_rndm               = rndm_range_f32(NULL, -1.0, 2.0f);
	f32 angle_delta_rndm       = rndm_range_f32(NULL, -2.0f, 2.0f);
	f32 angle_start_rndm       = rndm_range_f32(NULL, -2.0f, 2.0f);
	angle_start_rndm           = 0;
	angle_delta_rndm           = 0;
	flower->type               = type;
	flower->iterations         = 1;
	flower->axiom              = rules->axiom;
	flower->angle_delta        = (rules->angle_delta + angle_delta_rndm) * DEG_TO_TURN;
	flower->length             = rules->length + len_rndm;
	flower->start_angle_og     = (FLOWER_ANGLE_START + angle_start_rndm) * DEG_TO_TURN;
	flower->start_angle        = flower->start_angle_og;
	flower->water              = 0;
	flower->timestamp          = frame.timestamp;
	flower_reevaluate(flower, frame);
}

void
flower_water_add(struct flower *flower, i32 value, struct frame_info frame)
{
	i32 water                  = flower->water + value;
	flower->water              = water;
	i32 iterations_to_add      = 0;
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	while(flower->water > FLOWER_WATER_FOR_GROWTH) {
		flower->water = flower->water - 10;
		iterations_to_add++;
		if(flower->iterations + iterations_to_add >= rules->iterations_max) {
			break;
		}
	}
	if(iterations_to_add > 0) {
		flower_iterations_add(flower, iterations_to_add, frame);
	} else {
		flower_upd_tex(flower, frame.alloc, 1.0f);
	}
}

void
flower_iterations_add(struct flower *flower, i32 value, struct frame_info frame)
{
	flower_iterations_set(flower, flower->iterations + 1, frame);
}

void
flower_iterations_sub(struct flower *flower, i32 value, struct frame_info frame)
{
	flower_iterations_set(flower, flower->iterations - 1, frame);
}

void
flower_iterations_max(struct flower *flower, struct frame_info frame)
{
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	flower_iterations_set(flower, rules->iterations_max, frame);
}

void
flower_iterations_set(struct flower *flower, i32 value, struct frame_info frame)
{
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	flower->iterations         = clamp_i32(value, 0, rules->iterations_max);
	flower_reevaluate(flower, frame);
}

void
flower_reevaluate(struct flower *flower, struct frame_info frame)
{
	marena_reset(&flower->marena);
	str8 next                  = flower->axiom;
	struct flower_rules *rules = FLOWERS_RULES + flower->type;
	for(size i = 0; i < flower->iterations; ++i) {
		next = flower_generate(flower->alloc, rules, next);
	}
	log_info("garden", "Rem: %$$u Used: %$$u type: %s", (uint)flower->marena.rem, (uint)flower->marena.buf_size - flower->marena.rem, BLOCK_TYPE_LABELS[flower->type]);
	flower->next = next;

	flower_upd_tex(flower, frame.alloc, 1.0f);
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
flower_upd_tex(struct flower *flower, struct alloc scratch, f32 t)
{
	tex_clr(flower->ctx.dst, GFX_COL_CLEAR);
	struct gfx_ctx old             = g_drw_ctx(flower->ctx);
	i32 length                     = flower->length;
	f32 angle_delta                = flower->angle_delta;
	rec_i32 layout                 = {0, 0, flower->ctx.dst.w, flower->ctx.dst.h};
	struct rndm *rndm              = &flower->rndm;
	struct str8 next               = flower->next;
	struct flower_state stack[100] = {0};
	struct flower_bud buds[100]    = {0};
	i32 bud_idx                    = 0;
	i32 stack_ptr                  = 0;
	v2_i32 start_p                 = {layout.x + (layout.w * 0.5f), .y = layout.y + layout.h};
	stack[stack_ptr]               = (struct flower_state){
					  .p.x   = start_p.x,
					  .p.y   = start_p.y,
					  .angle = flower->start_angle,
					  .rot.s = sin_f32(flower->start_angle * TURN_TO_RAD),
					  .rot.c = cos_f32(flower->start_angle * TURN_TO_RAD),

    };

	rndm_seed(rndm, flower->timestamp);

	g_pat(gfx_pattern_100());
	g_color(PRIM_MODE_BLACK);
	g_spr_mode(SPR_MODE_COPY);
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
			// mark as a tip if next char is branch close or end of string
			if(nc == ']' || nc == 0) {
				buds[bud_idx] = (struct flower_bud){.water = 1, .x = state->p.x, .y = state->p.y};
				bud_idx       = (bud_idx + 1) % ARRLEN(buds);
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
			stack_ptr--;
			buds[bud_idx] = (struct flower_bud){.water = 1, .x = state->p.x, .y = state->p.y};
			bud_idx       = (bud_idx + 1) % ARRLEN(buds);
		} break;
		case 'B': {
			buds[bud_idx] = (struct flower_bud){.water = 1, .x = state->p.x, .y = state->p.y};
			bud_idx       = (bud_idx + 1) % ARRLEN(buds);
		} break;
		case 'c': {
		} break;
		case 'd': {
		} break;
		case 'e': {
		} break;
		default: {
		} break;
		}
	}
	{
		i32 water_total = flower->water;
		if(water_total > 0) {
			while(water_total > 0) {
				b32 one_empty = false;
				for(size i = ARRLEN(buds) - 1; i > 0; --i) {
					struct flower_bud *bud = buds + i;
					if(bud->water == 0) { continue; }
					if(bud->water > 3) { continue; }
					if(water_total == 0) { break; }
					i32 rnmd_water  = rndm_range_i32(rndm, 0, 3);
					i32 water_need  = 4 - bud->water;
					i32 water_value = min_i32(min_i32(rnmd_water, water_total), water_need);
					bud->water += water_value;
					water_total -= water_value;
					if(bud->water < 4) {
						one_empty = true;
					}
				}
				if(!one_empty && water_total > 0) {
					flower->is_full = true;
					break;
				}
			}
			for(size i = 0; i < (size)ARRLEN(buds); ++i) {
				struct flower_bud *bud = buds + i;
				if(bud->water < 1) { continue; }
				flower_bud_drw(
					flower,
					bud->water - 1,
					bud->x,
					bud->y);
			}
		}
	}
	g_drw_ctx(old);
}

static inline void
flower_bud_drw(struct flower *flower, i32 stage, i32 x, i32 y)
{
	i32 ref      = FLOWERS_TEX_MAP[flower->type];
	i32 id       = g_tex_refs_id_get(ref);
	struct tex t = asset_tex(id);
	i32 cells    = t.w / t.h;
	if(stage == 0) {
		g_cir_fill(x, y, 3 + stage);
	} else {
		i32 frame              = (stage - 1) % (cells - 1);
		struct tex_rec tex_rec = asset_tex_rec(id, t.h * frame, 0, t.h, t.h);
		g_spr_piv(tex_rec, x, y, 0, (v2){0.5f, 0.5f});
	}
}
