#pragma once

#include "base/str.h"
#include "block/block-type.h"
#include "garden/garden-defs.h"
#include "globals/g-tex-refs.h"

static enum g_tex_id FLOWERS_TEX_MAP[BLOCK_TYPE_NUM_COUNT] = {
	[BLOCK_TYPE_A] = G_TEX_FLOWER_01,
	[BLOCK_TYPE_B] = G_TEX_FLOWER_02,
	[BLOCK_TYPE_C] = G_TEX_FLOWER_03,
	[BLOCK_TYPE_D] = G_TEX_FLOWER_04,
	[BLOCK_TYPE_E] = G_TEX_FLOWER_05,
	[BLOCK_TYPE_F] = G_TEX_FLOWER_06,
};

static struct flower_rules FLOWERS_RULES[BLOCK_TYPE_NUM_COUNT] = {
	[BLOCK_TYPE_A] = {
		.iterations_max = 3,
		.length         = 5,
		.angle_delta    = 25,
		.axiom          = str8_lit_comp("X"),
		.rules          = {
            ['F'] = str8_lit_comp("FF"),
            ['X'] = str8_lit_comp("F[+X][-X]F[+X]-FXB"),
        },
	},
	[BLOCK_TYPE_B] = {
		.iterations_max = 3,
		.length         = 5,
		.angle_delta    = 20,
		.axiom          = str8_lit_comp("F"),
		.rules          = {
            ['F'] = str8_lit_comp("FF+[+F-F]-[-F+F]"),
        },
	},
	[BLOCK_TYPE_C] = {
		.iterations_max = 2,
		.length         = 15,
		.angle_delta    = 322,
		.axiom          = str8_lit_comp("F"),
		.rules          = {
            ['F'] = str8_lit_comp("Fe[+cFF]Fd[-FF]cFB"),
        },
	},
	[BLOCK_TYPE_D] = {
		.iterations_max = 5,
		.length         = 5,
		.angle_delta    = 20,
		.axiom          = str8_lit_comp("X"),
		.rules          = {
            ['X'] = str8_lit_comp("F[+X][-X]B"),
            ['F'] = str8_lit_comp("FF"),
        },
	},
	[BLOCK_TYPE_E] = {
		.iterations_max = 3,
		.length         = 5,
		.angle_delta    = 27,
		.axiom          = str8_lit_comp("F"),
		.rules          = {
            ['F'] = str8_lit_comp("FMNxQRyQROP"),
            ['M'] = str8_lit_comp("d[++FF+FF+]"),
            ['N'] = str8_lit_comp("d[--FF-FF-]"),
            ['O'] = str8_lit_comp("e[-F++F++]"),
            ['P'] = str8_lit_comp("e[+F--F--]"),
            ['Q'] = str8_lit_comp("c++F--F"),
            ['R'] = str8_lit_comp("c--F++F"),
        },
	},
	[BLOCK_TYPE_F] = {
		.iterations_max = 4,
		.length         = 10,
		.angle_delta    = 20,
		.axiom          = str8_lit_comp("VZFFF"),
		.rules          = {
            ['V'] = str8_lit_comp("[+++W][---W]YV"),
            ['W'] = str8_lit_comp("+X[-W]Z"),
            ['X'] = str8_lit_comp("-W[+X]Z"),
            ['Y'] = str8_lit_comp("YZ"),
            ['Z'] = str8_lit_comp("[-FcFF][+FdFF]F"),
        },
	},
};
