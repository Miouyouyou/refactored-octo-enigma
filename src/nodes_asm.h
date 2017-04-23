#ifndef MYY_SRC_NODES_ASM_H
#define MYY_SRC_NODES_ASM_H 1

#include <src/nodes.h>
#include <src/menus.h>
#include <lib/Assembler/armv7-arm.h>
#include <myy/helpers/fonts/packed_fonts_display.h>

struct mnemonic_info {
	uint8_t n_args;
	uint8_t * name;
};

struct quads_and_size {
	struct generated_quads quads;
	struct text_offset size;
};

void generate_frames_quads
(struct armv7_text_frame const * const * __restrict const text_frames,
 unsigned int const n_frames,
 struct nodes_display_data * __restrict const display_data,
 struct glyph_infos const * __restrict const glyphs);

void nodes_asm_setup_dropdowns_menus
(struct nodes_display_data * __restrict const nodes,
 struct dropdown_menus * __restrict const menus);

void node_frame_click
(struct nodes_display_data * __restrict const nodes,
 unsigned int index,
 int const x, int const y);

#endif
