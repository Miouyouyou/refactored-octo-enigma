#ifndef MYY_MENUS_H
#define MYY_MENUS_H 1

#include <myy/current/opengl.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/fonts/packed_fonts_display.h>

struct dropdown_menus_infos {
	unsigned int buffer_start;
	uint16_t buffer_size;
	uint16_t buffer_quads;
	uint8_t const * const * strings;
	unsigned int n_strings;
	void (*hitbox_func)(unsigned int hit_index);
};

enum ga_dropdown_menu {
	ga_dropdown_menu_instructions,
	ga_dropdown_menu_registers,
	ga_dropdown_menu_conditions,
	ga_dropdown_menu_frame_names,
	n_ga_menus
};

struct swap_menu_infos {
	unsigned int 
		title_size, title_quads,
		columns_size, columns_quads,
		left_n_options, right_n_options;
};

void prepare_static_menu_parts();

void prepare_context_menu_with
(GLuint const * __restrict const context_menu_text_buffer,
 struct glyph_infos const * __restrict const myy_glyph_infos,
 uint8_t const * __restrict const * __restrict strings,
 unsigned int n_strings,
 unsigned int strings_vertical_separation_px);

void draw_context_menu
(struct dropdown_menus_infos const * __restrict const menus,
 enum ga_dropdown_menu menu,
 GLuint const * __restrict const programs,
 GLuint const static_parts_buffer,
 GLuint const dropdowns_contents_buffer);

void draw_swap_menu
(struct swap_menu_infos const * __restrict const menu_infos,
 GLuint const * __restrict const programs,
 GLuint const dynamic_parts_buffer,
 GLuint const static_parts_buffer);

void draw_current_context_menu
(struct dropdown_menus_infos const * __restrict const menus,
 GLuint const * __restrict const programs,
 GLuint const static_parts_buffer,
 GLuint const dropdowns_contents_buffer);

void regenerate_menus
(struct dropdown_menus_infos * __restrict const menus,
 GLuint const gpu_buffer_id,
 struct glyph_infos const * __restrict const myy_glyph_infos);

void setup_menu
(struct dropdown_menus_infos * __restrict const menus,
 enum ga_dropdown_menu menu,
 uint8_t const * const * const strings,
 unsigned int const n_strings,
 void (* const hitbox_func)(unsigned int hit_index));

void enable_context_menu();
void disable_context_menu();
void enable_swap_menu();
void disable_swap_menu();
void set_current_dropdown_menu
(enum ga_dropdown_menu current_menu_id);
unsigned int manage_current_menu_click
(struct dropdown_menus_infos const * __restrict const menus,
 int const x, int const win_y);

void set_swap_menu_title
(struct swap_menu_infos * __restrict const swap_menu_infos,
 GLuint const buffer_id, uint8_t const * __restrict const title,
 struct glyph_infos const * __restrict const glyph_infos);

void set_swap_menu_listings
(struct swap_menu_infos * __restrict const swap_menu_infos,
 GLuint const buffer_id,
 unsigned int const n_strings_left,
 unsigned int const n_strings_right,
 uint8_t const * const * __restrict const left_column_strings,
 uint8_t const * const * __restrict const right_column_strings,
 struct glyph_infos const * __restrict const glyph_infos);

#endif
