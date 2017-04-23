#ifndef MYY_MENUS_H
#define MYY_MENUS_H 1

#include <myy/current/opengl.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/fonts/packed_fonts_display.h>

struct menu_gl_metadata {
	GLuint 
		content_buffer_id, content_buffer_offset,
		static_elements_buffer_id, static_elements_buffer_offset;
};
struct dropdown_menu_infos {
	GLuint offset;
	uint16_t content_quads;
	uint16_t n_strings;
	uint8_t const * const * strings;
	void (*hitbox_func)();
};

enum ga_dropdown_menu {
	ga_dropdown_menu_instructions,
	ga_dropdown_menu_registers,
	ga_dropdown_menu_conditions,
	ga_dropdown_menu_frame_names,
	n_ga_menus
};

struct dropdown_menus {
	struct menu_gl_metadata gl_infos;
	enum ga_dropdown_menu current_dropdown_menu;
	void (*current_dropdown_callback)(void *, unsigned int id);
	void * current_dropdown_callback_data;
	struct dropdown_menu_infos data[n_ga_menus];
};

struct swap_menu_infos {
	struct menu_gl_metadata gl_infos;
	int16_t
		title_size, title_quads,
		columns_size, columns_quads,
		left_n_options, right_n_options;
};

void prepare_static_menu_parts
(struct menu_gl_metadata const * __restrict const metadata);

void set_menu_buffers_and_offsets
(struct menu_gl_metadata * __restrict const metadata,
 GLuint const content_buffer_id, GLuint const content_buffer_offset,
 GLuint const static_elements_buffer_id,
 GLuint const static_elements_buffer_offset);



void prepare_context_menu_with
(GLuint const * __restrict const context_menu_text_buffer,
 struct glyph_infos const * __restrict const myy_glyph_infos,
 uint8_t const * __restrict const * __restrict strings,
 unsigned int n_strings,
 unsigned int strings_vertical_separation_px);

void draw_context_menu
(struct dropdown_menus const * __restrict const menus,
 enum ga_dropdown_menu menu,
 GLuint const * __restrict const programs);

void draw_swap_menu
(struct swap_menu_infos const * __restrict const menu_infos,
 GLuint const * __restrict const programs);

void draw_current_context_menu
(struct dropdown_menus const * __restrict const menus,
 GLuint const * __restrict const programs);

void regenerate_menus
(struct dropdown_menus * __restrict const menus,
 struct glyph_infos const * __restrict const myy_glyph_infos);

void setup_dropdown_menu
(struct dropdown_menus * __restrict const menus,
 enum ga_dropdown_menu menu,
 uint8_t const * const * const strings,
 unsigned int const n_strings,
 void (* const hitbox_func)());

void enable_context_menu();
void disable_context_menu();
void enable_swap_menu();
void disable_swap_menu();
void set_current_dropdown_menu
(struct dropdown_menus * __restrict const menus,
 enum ga_dropdown_menu current_menu_id);

void set_current_dropdown_menu_callback
(struct dropdown_menus * __restrict const menus,
 void (*callback)(void *, unsigned int id),
 void * __restrict const data);

unsigned int manage_current_menu_click
(struct dropdown_menus const * __restrict const menus,
 int const x, int const win_y);

void set_swap_menu_title
(struct swap_menu_infos * __restrict const swap_menu_infos,
 uint8_t const * __restrict const title,
 struct glyph_infos const * __restrict const glyph_infos);

void set_swap_menu_listings
(struct swap_menu_infos * __restrict const swap_menu_infos,
 unsigned int const n_strings_left,
 unsigned int const n_strings_right,
 uint8_t const * const * __restrict const left_column_strings,
 uint8_t const * const * __restrict const right_column_strings,
 struct glyph_infos const * __restrict const glyph_infos);

#endif
