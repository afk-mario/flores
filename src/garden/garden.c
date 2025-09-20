#include "garden.h"
#include "base/dbg.h"
#include "base/marena.h"
#include "base/str.h"
#include "base/types.h"
#include "engine/input.h"
#include "globals/g-gfx.h"

#define GARDEN_ITERATIONS  1
#define GARDEN_LENGTH      5
#define GARDEN_ANGLE_DELTA 25
#define GARDEN_ANGLE_START -90
#define GARDEN_START_AXIOM "F"

static inline str8 garden_generate(struct garden *garden, struct str8 axiom);

void
garden_ini(struct garden *garden, struct alloc alloc, f32 timestamp)
{
	size size = MMEGABYTE(1);
	void *mem = alloc.allocf(alloc.ctx, size);
	mclr(mem, size);
	marena_init(&garden->marena, mem, size);
	garden->alloc       = marena_allocator(&garden->marena);
	garden->axiom       = str8_lit(GARDEN_START_AXIOM);
	garden->angle_delta = GARDEN_ANGLE_DELTA * DEG_TO_TURN;
	garden->length      = GARDEN_LENGTH;
	garden->iterations  = GARDEN_ITERATIONS;
}

void
garden_upd(struct garden *garden, struct frame_info frame)
{
	marena_reset(&garden->marena);
	if(inp_key_just_pressed('n')) {
		garden->iterations++;
	}
	if(inp_key_just_pressed('b')) {
		garden->iterations = MAX(0, garden->iterations - 1);
	}
	str8 next = garden->axiom;
	for(size i = 0; i < garden->iterations; ++i) {
		next = garden_generate(garden, next);
	}
	garden->next = next;
}

void
garden_drw(struct garden *garden, rec_i32 layout, enum game_theme theme)
{
	i32 length                     = garden->length;
	f32 angle_delta                = garden->angle_delta;
	struct str8 next               = garden->next;
	struct garden_state stack[100] = {0};
	i32 stack_ptr                  = 0;
	stack[stack_ptr]               = (struct garden_state){
					  .p.x   = layout.x + (layout.w * 0.5f),
					  .p.y   = layout.y + (layout.h),
					  .angle = GARDEN_ANGLE_START * DEG_TO_TURN,
    };

	g_color(PRIM_MODE_WHITE);
	for(size i = 0; i < (size)next.size; ++i) {
		u8 c                       = next.str[i];
		struct garden_state *state = stack + stack_ptr;
		switch(c) {
		case 'F': {
			i32 xb = state->p.x + (i32)(length * cos_f32(state->angle * TURN_TO_RAD));
			i32 yb = state->p.y + (i32)(length * sin_f32(state->angle * TURN_TO_RAD));
			g_lin(state->p.x, state->p.y, xb, yb);
			state->p.x = xb;
			state->p.y = yb;
		} break;
		case 'G': {
			i32 xb     = state->p.x + (i32)(length * cos_f32(state->angle * TURN_TO_RAD));
			i32 yb     = state->p.y + (i32)(length * sin_f32(state->angle * TURN_TO_RAD));
			state->p.x = xb;
			state->p.y = yb;
		} break;
		case '+': {
			state->angle += angle_delta;
		} break;
		case '-': {
			state->angle -= angle_delta;
		} break;
		case '[': {
			dbg_assert(stack_ptr < (size)ARRLEN(stack));
			stack[++stack_ptr] = *state;
		} break;
		case ']': {
			dbg_assert(stack_ptr > 0);
			stack_ptr--;
		} break;
		default: {
		} break;
		}
	}
}

static inline str8
garden_generate(struct garden *garden, struct str8 axiom)
{
	struct str8 res       = {0};
	struct str8_list list = {0};
	struct alloc alloc    = garden->alloc;
	// F → FF + [+ F – F – F] – [– F + F + F]

	for(size i = 0; i < (size)axiom.size; ++i) {
		u8 c = axiom.str[i];
		switch(c) {
		case 'X': {
			// str8_list_push(alloc, &list, str8_lit("F[+F][-F]"));
		} break;
		case 'F': {
			// str8_list_push(alloc, &list, str8_lit("FF+[+F–F–F]–[–F+F+F]"));
			str8_list_push(alloc, &list, str8_lit("FF+[+F-F-F]-[-F+F+F]"));
		} break;
		default: {
		} break;
		}
	}
	res = str8_list_join(alloc, &list, NULL);
	return res;
}
