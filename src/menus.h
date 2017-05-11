#ifndef MYY_MENUS_H
#define MYY_MENUS_H 1

#include <myy/current/opengl.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/hitbox_action.h>

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
typedef struct dropdown_menus dropdown_menus;

enum swap_menu_hitbox_name {
	swap_hitbox_left_listing,
	swap_hitbox_right_listing,
	swap_hitbox_swap_ltr,
	swap_hitbox_swap_rtl,
	swap_hitbox_right_button_up,
	swap_hitbox_right_button_down,
	swap_hitbox_close_button,
	n_swap_hitboxes
};
struct swap_menu_infos;
typedef struct swap_menu_infos swap_menus;

enum swap_menu_listing_position {
	swap_menu_listing_left,
	swap_menu_listing_right,
	n_swap_menu_listings
};
struct swap_menu_infos {
	struct menu_gl_metadata gl_infos;
	struct myy_common_data const * common_graphics_data;
	hitboxes_S_t hitboxes;
	int16_t
		title_size, title_quads,
		columns_size, columns_quads,
		left_n_options, right_n_options;
	uint16_t selected[n_swap_menu_listings];
};

struct menu_button_configuration {
	enum swap_menu_hitbox_name button;
	void * data;
	uint8_t (*action)(HITBOX_ACTION_SIG);
};
struct swap_menu_buttons {
	uint32_t n_buttons;
	struct menu_button_configuration buttons_settings[];
};
typedef struct swap_menu_buttons swap_menu_buttons;


struct menus {
	dropdown_menus dropdown;
	swap_menus swap;
};

#define SWAP_MENU_LISTING_VERTICAL_SPACING 24

/**
 * Recalculate the dimensions of the static menus parts after resizing
 * the window
 * 
 * The 'viewport' is the surface on which the program is displayed.
 * It's either its window, in window mode, or the screen, in fullscreen
 * mode.
 * 
 * @param menus Currently ignored
 * @param width  The new viewport width
 * @param height The new viewport height
 */
void menus_recalculate_dimensions
(struct menus * __restrict const menus,
 uint16_t const width, uint16_t const height);

void menus_regen_static_parts
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

void dropdown_menus_draw
(dropdown_menus const * __restrict const menus,
 enum ga_dropdown_menu menu,
 GLuint const * __restrict const programs);

void swap_menus_init
(swap_menus * __restrict const menu_infos,
 struct myy_common_data const * __restrict const common_data);

void swap_menus_draw
(swap_menus const * __restrict const menu_infos,
 GLuint const * __restrict const programs);

void dropdown_menus_draw_current
(dropdown_menus const * __restrict const menus,
 GLuint const * __restrict const programs);

void regenerate_menus
(dropdown_menus * __restrict const menus,
 struct glyph_infos const * __restrict const myy_glyph_infos);

void dropdowns_menu_setup_menu
(dropdown_menus * __restrict const menus,
 enum ga_dropdown_menu menu,
 uint8_t const * const * const strings,
 unsigned int const n_strings,
 void (* const hitbox_func)());

void enable_context_menu();
void disable_context_menu();

void enable_swap_menu
(swap_menus  * __restrict const swap_menus, void * menu_data);

void disable_swap_menu
(swap_menus * __restrict const swap_menus);

void menus_refresh
(struct menus * __restrict const menus,
 uint16_t width, uint16_t height);

void dropdown_menus_set_current
(dropdown_menus * __restrict const menus,
 enum ga_dropdown_menu current_menu_id);

void dropdown_menus_set_current_callback
(dropdown_menus * __restrict const menus,
 void (*callback)(void *, unsigned int id),
 void * __restrict const data);

unsigned int manage_current_menu_click
(dropdown_menus const * __restrict const menus,
 int const x, int const win_y);

void set_swap_menu_title
(swap_menus * __restrict const swap_menu_infos,
 uint8_t const * __restrict const title);

void set_swap_menu_listings
(swap_menus * __restrict const swap_menu_infos,
 unsigned int const n_strings_left,
 unsigned int const n_strings_right,
 uint8_t const * const * __restrict const left_column_strings,
 uint8_t const * const * __restrict const right_column_strings);


void set_swap_menu_buttons
(swap_menus * __restrict const swap_menu_infos,
 swap_menu_buttons const * __restrict const buttons_configuration);
 


#endif
