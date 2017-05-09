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

struct quads_and_size node_asm_generate_content
(nodes const * __restrict const nodes,
 void const * __restrict const uncasted_frame,
 struct glyph_infos const * __restrict const fonts_glyphs,
 buffer_t buffer);

struct quads_and_size node_asm_generate_title
(nodes const * __restrict const nodes,
 void const * __restrict const uncasted_frame,
 struct glyph_infos const * __restrict const fonts_glyphs,
 buffer_t buffer);

void nodes_asm_setup_dropdowns_menus
(struct nodes_display_data * __restrict const nodes,
 struct dropdown_menus * __restrict const menus);

void nodes_asm_onclick_handler
(nodes const * __restrict const nodes,
 unsigned int index, position_S const pos);

#endif
