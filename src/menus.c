#include <myy/myy.h>
#include <src/menus.h>
#include <src/text.h>
#include <src/generated/opengl/data_config.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/log.h>

#include <src/nodes.h>

unsigned int quads_in_context_menu;

uint8_t current_context_text_buffer = 0;
uint8_t display_context_menu = 0;
uint8_t display_swap_menu = 0;

unsigned int const default_vertical_space = 32; // pixels
struct { int16_t x, y; } const default_context_menu_content_pos = {
  .x = 1080, .y = 0
};
struct { uint8_t id; uint16_t y_pos; } current_dropdown_menu = {
  .id = 0, .y_pos = 40
};

extern uint8_t scratch_buffer[];

enum { 
	context_menu_background,
	context_menu_close_button,
	swap_menu_left_part,
	swap_menu_right_part,
	swap_menu_swap_button_ltr,
	swap_menu_swap_button_rtl,
	swap_menu_right_button_up,
	swap_menu_right_button_down,
	swap_menu_close_button,
	swap_menu_background,
	n_context_menu_elements
};

generated_quads_uS ui_text_listing_generate_text_quads
(ui_text_listing * __restrict const listings, unsigned int n_listings,
 buffer_t quads_buffer,
 struct glyph_infos * __restrict const font_glyphs)
{
	generated_quads_uS listings_quads = generated_quads_uS_struct(0,0);
	
	for (unsigned int l = 0; l < n_listings; l++) {
		ui_text_listing * __restrict const listing = listings+l;
		
		position_S text_position =
			position_S_box_coords_S_top_left(listing->metadata.coords);
		generated_quads_uS_add(&listings_quads, myy_strings_to_quads_va(
			font_glyphs, listing->n_strings, listing->strings,
			quads_buffer+listings_quads.size, listing->vertical_spacing,
			&text_position
		));
		
	}
	
	return listings_quads;
	
}

#define DARK_BLUE 10,20,75,255

box_coords_S_t ui_text_listing_determine_selection_box_size
(ui_text_listing const * __restrict const listing)
{
	uint_fast16_t selected_index   = listing->selected_index;
	uint_fast16_t vertical_spacing = listing->vertical_spacing;
	box_coords_S_t box = {
		.left   = listing->metadata.coords.left,
		.right  = listing->metadata.coords.right,
		.top    = (uint16_t) 
			(listing->metadata.coords.top
			 + selected_index * vertical_spacing),
		.bottom = (uint16_t)
			(listing->metadata.coords.top 
			 + (selected_index+1) * vertical_spacing)
	};
	
	return box;
}

uint_fast8_t ui_text_listing_is_selection_valid
(ui_text_listing const * __restrict const listing)
{
	return listing->selected_index < listing->n_strings;
}

generated_quads_uS ui_text_listing_generate_selection_quads
(ui_text_listing * __restrict const listings, unsigned int n_listings,
 buffer_t quads_buffer,
 struct glyph_infos * __restrict const font_glyphs)
{
	SuB_2t_colored_quad * __restrict const casted_quads_buffer =
		(SuB_2t_colored_quad * __restrict) quads_buffer;
	
	generated_quads_uS quads = generated_quads_uS_struct(0,0);
	
	for (unsigned int l = 0; l < n_listings; l++) {
		ui_text_listing * __restrict const listing = listings+l;
		
		if (ui_text_listing_is_selection_valid(listing)) {
			box_coords_S_t selection_coords =
				ui_text_listing_determine_selection_box_size(listing);
			SuB_2t_colored_quad_store_box(
				selection_coords, DARK_BLUE, casted_quads_buffer+quads.count
			);
			quads.count += 1;
			quads.size  += sizeof(SuB_2t_colored_quad);
		}
		
	}
	
	return quads;
}

void set_menu_buffers_and_offsets
(struct menu_gl_metadata * __restrict const metadata,
 GLuint const content_buffer_id, GLuint const content_buffer_offset,
 GLuint const static_elements_buffer_id,
 GLuint const static_elements_buffer_offset)
{
	metadata->content_buffer_id = content_buffer_id;
	metadata->content_buffer_offset = content_buffer_offset;
	metadata->static_elements_buffer_id = static_elements_buffer_id;
	metadata->static_elements_buffer_offset =
		static_elements_buffer_offset;
}

#define CLOSE_BUTTON_TEX \
  {.left = 0, .right = 8191, .top = 8191, .bottom = 256}
#define DOWN_BUTTON_TEX \
  {.left = 8320, .right = 16256, .top = 8191, .bottom = 256}
#define UP_BUTTON_TEX \
  {.left = 16512, .right = 24448, .top = 8191, .bottom = 256}
#define LTR_BUTTON_TEX \
  {.left = 24704, .right = 32640, .top = 3967, .bottom = 256}
#define RTL_BUTTON_TEX \
  {.left = 24704, .right = 32640, .top = 4223, .bottom = 8063}
  
#define BLACK     {0,  0, 0,255}
#define DARK_GREY {30,30,30,255}
#define DARK_RED  {80, 0, 0,255}

struct menu_gl_coords {
	box_coords_S_t coords;
	uint16_t depth;
	rgba_t color;
} const original_menu_coords[n_context_menu_elements] = {
	[context_menu_background] = {
		.coords =
			{.left = 0, .right = 200, .top = 0,   .bottom = 720},
		.depth = 1,
		.color = BLACK
	},
	[context_menu_close_button] = {
		.coords = 
			{.left = 200-32, .right = 200, .top = 0,   .bottom = 32},
		.depth = 0,
		.color = DARK_RED
	},
	[swap_menu_left_part] = {
		.coords =
			{.left = 120,    .right = 446, .top = 150, .bottom = 620},
		.depth = 0,
		.color = DARK_GREY
	},
	[swap_menu_right_part] = {
		.coords =
			{.left = 534,    .right = 810, .top = 150, .bottom = 620},
		.depth = 0,
		.color = DARK_GREY
	},
	[swap_menu_swap_button_ltr] = {
		.coords =
			{.left = 466,    .right = 514, .top = 190, .bottom = 214},
		.depth = 0,
		.color = {DARK_BLUE}
  },
	[swap_menu_swap_button_rtl] = {
		.coords =
			{.left = 466,    .right = 514, .top = 556, .bottom = 580},
		.depth = 0,
		.color = {DARK_BLUE}
	},
	[swap_menu_right_button_up] = {
		.coords =
			{.left = 820,    .right = 870, .top = 190, .bottom = 214},
		.depth = 0,
		.color = {DARK_BLUE}
	},
	[swap_menu_right_button_down] = {
		.coords =
			{.left = 820,    .right = 870, .top = 556, .bottom = 580},
		.depth = 0,
		.color = {DARK_BLUE}
	},
	[swap_menu_close_button] = {
		.coords =
			{.left = 880-32,    .right = 880, .top = 50, .bottom = 50+32},
		.depth = 0,
		.color = DARK_RED
	},
	[swap_menu_background] = {
		.coords =
			{.left = 100,    .right = 880, .top = 50,  .bottom = 670},
		.depth = 32,
		.color = BLACK
	}
};

struct menu_gl_coords actual_menu_coords[n_context_menu_elements];

inline static box_coords_S_t box_coords_S_rebase_from_1280_720
(box_coords_S_t const src,
 unsigned int const width, unsigned int const height)
{
	box_coords_S_t returned_box = {
		.left   = (uint16_t) (src.left   * width / 1280),
		.right  = (uint16_t) (src.right  * width / 1280),
		.top    = (uint16_t) (src.top    * height / 720),
		.bottom = (uint16_t) (src.bottom * height / 720)
	};
	
	return returned_box;
}

static void swap_menu_update_widgets_pos
(swap_menus * __restrict const swap_menu,
 struct menu_gl_coords * __restrict const actual_coords,
 uint16_t const width, uint16_t height)
{
	box_coords_S_t original_menu_title_coords = {
		.left = 120, .right = 860, .top = 70, .bottom = 130
	};
	
	swap_menu->title.metadata.coords = box_coords_S_rebase_from_1280_720(
		original_menu_title_coords, width, height
	);

	uint16_t vertical_spacing_px = 24 * height / 720;
	swap_menu->listings[swap_menu_listing_left].metadata.coords =
		actual_coords[swap_menu_left_part].coords;
	swap_menu->listings[swap_menu_listing_left].vertical_spacing =
		vertical_spacing_px;
	swap_menu->listings[swap_menu_listing_right].metadata.coords =
		actual_coords[swap_menu_right_part].coords;
	swap_menu->listings[swap_menu_listing_right].vertical_spacing =
		vertical_spacing_px;
}

static void swap_menu_update_hitboxes
(swap_menus * __restrict const swap_menu,
 struct menu_gl_coords * __restrict const actual_coords)
{
	hitbox_action_S_change_coords(
		swap_menu->hitboxes.data+swap_hitbox_left_listing,
		actual_coords[swap_menu_left_part].coords
	);
	hitbox_action_S_change_coords(
		swap_menu->hitboxes.data+swap_hitbox_right_listing,
		actual_coords[swap_menu_right_part].coords
	);
	hitbox_action_S_change_coords(
		swap_menu->hitboxes.data+swap_hitbox_swap_ltr,
		actual_coords[swap_menu_swap_button_ltr].coords
	);
	hitbox_action_S_change_coords(
		swap_menu->hitboxes.data+swap_hitbox_swap_rtl,
		actual_coords[swap_menu_swap_button_rtl].coords
	);
	hitbox_action_S_change_coords(
		swap_menu->hitboxes.data+swap_hitbox_right_button_up,
		actual_coords[swap_menu_right_button_up].coords
	);
	hitbox_action_S_change_coords(
		swap_menu->hitboxes.data+swap_hitbox_right_button_down,
		actual_coords[swap_menu_right_button_down].coords
	);
	hitbox_action_S_change_coords(
		swap_menu->hitboxes.data+swap_hitbox_close_button,
		actual_coords[swap_menu_close_button].coords
	);
	swap_menu->hitboxes.count = n_swap_hitboxes;
}

void menus_recalculate_dimensions
(struct menus * __restrict const menus,
 uint16_t const width, uint16_t const height)
{
	for (unsigned int b = 0; b < n_context_menu_elements; b++) {
		actual_menu_coords[b].coords = box_coords_S_rebase_from_1280_720(
			original_menu_coords[b].coords, width, height
		);
		actual_menu_coords[b].depth = original_menu_coords[b].depth;
		actual_menu_coords[b].color = original_menu_coords[b].color;
	}
	hitboxes_S_quick_reset(&menus->swap.hitboxes);
	swap_menu_update_widgets_pos(
		&menus->swap, actual_menu_coords, width, height
	);
	swap_menu_update_hitboxes(&menus->swap, actual_menu_coords);
}

void menus_regen_static_parts
(struct menu_gl_metadata const * __restrict const metadata)
{
	
	SuB_2t_colored_quad_3D context_menu_elements[n_context_menu_elements];
	
	for (unsigned int i = 0; i < n_context_menu_elements; i++) {
		SuB_2t_colored_quad_store_box_rgba_3D(
			actual_menu_coords[i].coords,
			actual_menu_coords[i].color,
			actual_menu_coords[i].depth,
			context_menu_elements+i
		);
	}
	
	glBindBuffer(
		GL_ARRAY_BUFFER,
		metadata->static_elements_buffer_id
	);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		metadata->static_elements_buffer_offset,
		sizeof(SuB_2t_colored_quad_3D)*n_context_menu_elements,
		context_menu_elements
	);
}

static uint8_t hitbox_dummy_action
(HITBOX_ACTION_FULL_SIG(data,rel, abs))
{
	LOG("I'm a dummy action !\n");
	return 0;
}

static void reset_swap_hitboxes_action
(swap_menus * __restrict swap_menu)
{
	hitboxes_S_reset(&swap_menu->hitboxes);
	for (unsigned int i = 0; i < n_swap_hitboxes; i++)
		swap_menu->hitboxes.data[i].action = hitbox_dummy_action;
	swap_menu->hitboxes.data[swap_hitbox_close_button].action_data =
		swap_menu;
	swap_menu->hitboxes.data[swap_hitbox_close_button].action =
		disable_swap_menu;
}

void swap_menus_init
(swap_menus * __restrict const swap_menu,
 struct myy_common_data const * __restrict const common_data)
{
	swap_menu->common_graphics_data = common_data;
	hitboxes_S_init(&swap_menu->hitboxes, n_swap_hitboxes);
	reset_swap_hitboxes_action(swap_menu);
}


void dropdown_menus_draw
(dropdown_menus const * __restrict const menus,
 enum ga_dropdown_menu menu,
 struct glsl_programs_shared_data const * __restrict const glsl_data)
{
	if (display_context_menu) {
		glUseProgram(glsl_data->programs[glsl_program_fixed_widgets]);
		glUniform4f(
			fixed_widgets_shader_unif_px_offset, 1080, 0, 256, 0.0f
		); 

		SuB_2t_colored_quad_3D_draw_pixel_coords(
			menus->gl_infos.static_elements_buffer_id,
			fixed_widgets_shader_attr_xyz,
			fixed_widgets_shader_attr_in_rgba,
			menus->gl_infos.static_elements_buffer_offset,
			2 // quads
		);
		
		uint16_t context_menu_dropdown_x =
			default_context_menu_content_pos.x + 20;
		uint16_t context_menu_dropdown_y = current_dropdown_menu.y_pos;

		glUseProgram(glsl_data->programs[glsl_program_node]);
		glhUnif1i(node_shader_unif_sampler, glsl_texture_fonts, glsl_data);
		glhUnif4f(
			node_shader_unif_px_offset, 0, 0,
			context_menu_dropdown_x, context_menu_dropdown_y,
			glsl_data
		);

		glhUnif1f(node_shader_unif_layer, 0.20f, glsl_data);
		struct dropdown_menu_infos const * __restrict const
			current_menu = menus->data+menu;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		draw_character_quads(
			menus->gl_infos.content_buffer_id,
			node_shader_attr_xyz,
			node_shader_attr_in_st,
			current_menu->offset,
			current_menu->content_quads
		);
		glDisable(GL_BLEND);
	}
}

void swap_menus_draw
(swap_menus const * __restrict const menu_infos,
 struct glsl_programs_shared_data const * __restrict const glsl_data)
{
	if (display_swap_menu) {
		glUseProgram(glsl_data->programs[glsl_program_fixed_widgets]);
		
		struct { GLfloat x, y, z, w; } __PALIGN__
			menu_elements_offset = { 0, 0, 256, 0 };
		

		glhUnif4fv(
			fixed_widgets_shader_unif_px_offset,
			1, (GLfloat *) &menu_elements_offset,
			glsl_data
		);
		SuB_2t_colored_quad_3D_draw_pixel_coords(
			menu_infos->gl_infos.static_elements_buffer_id, 
			fixed_widgets_shader_attr_xyz,
			fixed_widgets_shader_attr_in_rgba,
			menu_infos->gl_infos.static_elements_buffer_offset +
			swap_menu_left_part*sizeof(SuB_2t_colored_quad_3D),
			n_context_menu_elements - swap_menu_left_part // quads
		);
		
		/* Rappels :
		 * Lors de l'ajout d'un nouveau shader :
		 * - Rajouter les informations dans generated/data_config.h
		 *   (Qui devrait être généré automatiquement !)
		 * - Vérifier de bien régler la matrice de projection
		 * - Vérifier de bien activer les Vertex Buffers
		 *   (Devrait être fait qu'une seule fois, cependant)
		 */
		// Draw selections boxes
		
		glUseProgram(glsl_data->programs[glsl_program_color_static]);
		glhUnif1f(color_node_shader_unif_layer, 0.2f, glsl_data);

		SuB_2t_colored_quad_draw_pixel_coords(
			menu_infos->gl_infos.content_buffer_id,
			color_node_shader_attr_xy,
			color_node_shader_attr_in_rgba,
			menu_infos->gl_infos.content_buffer_offset,
			menu_infos->quads.selections.count
		);
		
		
		// Draw text
		glUseProgram(glsl_data->programs[glsl_program_node]);
		glhUnif1i(node_shader_unif_sampler, glsl_texture_fonts, glsl_data);
		glhUnif1f(node_shader_unif_layer, 0.19f, glsl_data);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLuint const dynamic_parts_buffer =
			menu_infos->gl_infos.content_buffer_id;
		GLuint const dynamic_parts_offset =
			menu_infos->gl_infos.content_buffer_offset;
		glhUnif4f(
      node_shader_unif_px_offset, 0, 0, 0, 0, glsl_data
    );
		draw_character_quads(
			dynamic_parts_buffer,
			node_shader_attr_xyz,
			node_shader_attr_in_st,
			dynamic_parts_offset+menu_infos->quads.selections.size,
			menu_infos->quads.columns.count
		);
		glDisable(GL_BLEND);
	}
}

void dropdown_menus_draw_current
(struct dropdown_menus const * __restrict const menus,
 struct glsl_programs_shared_data const * __restrict const programs)
{
  dropdown_menus_draw(menus, menus->current_dropdown_menu, programs);
}

inline static GLuint buffer_switch(GLuint buffer_index)
{
	return buffer_index ^ 1;
}

void prepare_context_menu_with
(GLuint const * __restrict const context_menu_text_buffer,
 struct glyph_infos const * __restrict const myy_glyph_infos,
 uint8_t const * __restrict const * __restrict strings,
 unsigned int n_strings,
 unsigned int strings_vertical_separation_px)
{
	unsigned int other_text_buffer = 
		buffer_switch(current_context_text_buffer);

	glBindBuffer(
		GL_ARRAY_BUFFER, context_menu_text_buffer[other_text_buffer]
	);

	position_S text_rel_position_px = position_S_struct(0,0);
	struct generated_quads char_quads = myy_strings_to_quads_va(
		myy_glyph_infos, n_strings, strings, scratch_buffer,
		strings_vertical_separation_px, &text_rel_position_px
	);
	glBufferSubData(GL_ARRAY_BUFFER, 0, char_quads.size, scratch_buffer);
	quads_in_context_menu = char_quads.count;
	current_context_text_buffer = other_text_buffer;
}

static unsigned int generate_and_store_menu_in_gpu
(struct dropdown_menus * __restrict const menus,
 enum ga_dropdown_menu menu,
 GLuint const offset,
 struct glyph_infos const * __restrict const myy_glyph_infos)
{
	GLuint const gpu_buffer_id =
		menus->gl_infos.content_buffer_id;
	GLuint const gpu_offset =
		menus->gl_infos.content_buffer_offset + offset;
	struct dropdown_menu_infos * __restrict const current_menu
		= menus->data+menu;
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer_id);

	position_S text_rel_position_px = position_S_struct(0,0);

	struct generated_quads char_quads = myy_strings_to_quads_va(
		myy_glyph_infos,
		current_menu->n_strings, current_menu->strings,
		scratch_buffer, default_vertical_space, &text_rel_position_px
	);
	
	glBufferSubData(
		GL_ARRAY_BUFFER, gpu_offset,
		char_quads.size, scratch_buffer
	);
	
	current_menu->content_quads = char_quads.count;
	current_menu->offset = offset;
	
	
	LOG(
		"[generate_and_store_menu_in_gpu]  \n"
		"char_quads -- count : %d - size %d\n",
		 char_quads.count, char_quads.size
		);
	
	return char_quads.size;
}

void regenerate_menus
(struct dropdown_menus * __restrict const menus,
 struct glyph_infos const * __restrict const myy_glyph_infos)
{
	unsigned int offset = 0;
	
	for (enum ga_dropdown_menu current_menu = 0;
	     current_menu < n_ga_menus;
	     current_menu++) {
		offset += generate_and_store_menu_in_gpu(
			menus, current_menu, offset, myy_glyph_infos
		);
	}
}

void dropdowns_menu_setup_menu
(struct dropdown_menus * __restrict const menus,
 enum ga_dropdown_menu menu,
 uint8_t const * const * const strings,
 unsigned int const n_strings, 
 void (* const hitbox_func)())
{
	menus->data[menu].strings = strings;
	menus->data[menu].n_strings = n_strings;
	menus->data[menu].hitbox_func = hitbox_func;
}


void enable_context_menu() { display_context_menu = 1; }
void disable_context_menu() { display_context_menu = 0; }

extern swap_menus swap_menu_infos;

static uint8_t meep
(void * __restrict action_data, position_S rel, position_S abs)
{
	LOG("Meep ! (%d, %d) - (%d, %d)\n", rel.x, rel.y, abs.x, abs.y);
	
	swap_menus * __restrict const swap_menu = 
		(swap_menus * __restrict) action_data;
	
	return hitboxes_action_react_on_click_at(&swap_menu->hitboxes, abs);

}

void enable_swap_menu
(swap_menus * __restrict const swap_menus)
{ 
	hitboxes_S_add_box_action(
		swap_menus->common_graphics_data->hitboxes,
		&actual_menu_coords[swap_menu_background].coords,
		meep,
		swap_menus
	);

	display_swap_menu = 1;
}
void disable_swap_menu
(swap_menus * __restrict const swap_menus)
{
	hitboxes_S_delete_box_action(
		swap_menus->common_graphics_data->hitboxes,
		&actual_menu_coords[swap_menu_background].coords,
		meep
	);
	display_swap_menu = 0;
}
void menus_refresh
(struct menus * __restrict const menus,
 uint16_t width, uint16_t height)
{
	uint8_t swap_menus_currently_shown = display_swap_menu;
	uint8_t context_menu_currently_shown = display_context_menu;
	swap_menus * __restrict const swap_menus = &menus->swap;
	disable_swap_menu(swap_menus);
	disable_context_menu();
	menus_recalculate_dimensions(menus, width, height);
	menus_regen_static_parts(&swap_menus->gl_infos);
	
	if (swap_menus_currently_shown)
		enable_swap_menu(swap_menus);
	if (context_menu_currently_shown) enable_context_menu();
}

void dropdown_menus_set_current
(dropdown_menus * __restrict const menus,
 enum ga_dropdown_menu current_menu_id)
{
	menus->current_dropdown_menu = current_menu_id;
}
void dropdown_menus_set_current_callback
(dropdown_menus * __restrict const menus,
 void (*callback)(),
 void * __restrict const data)
{
	menus->current_dropdown_callback = callback;
	menus->current_dropdown_callback_data = data;
}


unsigned int const menu_x_position = 1080;
static unsigned clicked_on_context_menu(int x, int win_y)
{
	return (display_context_menu && x > menu_x_position);
}

static void close_context_menu() {
	display_context_menu = 0;
}

unsigned int manage_current_menu_click
(struct dropdown_menus const * __restrict const menus,
 int const x, int const win_y)
{
	unsigned int inside_x = x - menu_x_position;
	unsigned int clicked_inside = clicked_on_context_menu(x, win_y);

	if (clicked_inside && inside_x > 170 && win_y < 30)
		close_context_menu();

	else if (clicked_inside && win_y > 40) {
		unsigned int const element = (win_y - 40) / 32;
		enum ga_dropdown_menu current_menu = menus->current_dropdown_menu;
		struct dropdown_menu_infos const * __restrict const
			current_menu_infos = menus->data+current_menu;
		if (element < current_menu_infos->n_strings)
			current_menu_infos->hitbox_func(menus, element);
	}

	return clicked_inside;
}

void set_swap_menu_title
(struct swap_menu_infos * __restrict const swap_menu_infos,
 uint8_t const * __restrict const title)
{

	position_S text_offset = position_S_struct(0,0); 
	struct glyph_infos * __restrict const glyph_infos =
		swap_menu_infos->common_graphics_data->fonts_glyphs;
	
	struct generated_quads quads = myy_single_string_to_quads(
		glyph_infos, title, scratch_buffer, &text_offset
	);
	
	glBindBuffer(
		GL_ARRAY_BUFFER,
		swap_menu_infos->gl_infos.content_buffer_id
	);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		swap_menu_infos->gl_infos.content_buffer_offset,
		quads.size, scratch_buffer
	);
	
	swap_menu_infos->quads.title = quads;
	
}

void set_swap_menu_listings
(struct swap_menu_infos * __restrict const swap_menu_infos,
 unsigned int const n_strings_left,
 unsigned int const n_strings_right,
 uint8_t const * const * __restrict const left_column_strings,
 uint8_t const * const * __restrict const right_column_strings)
{
	GLuint const buffer_offset =
		swap_menu_infos->gl_infos.content_buffer_offset;
	unsigned int const gpu_offset = buffer_offset;

	swap_menu_infos->listings[swap_menu_listing_left].n_strings =
		n_strings_left;
	swap_menu_infos->listings[swap_menu_listing_left].strings =
		left_column_strings;
	swap_menu_infos->listings[swap_menu_listing_right].n_strings =
		n_strings_right;
	swap_menu_infos->listings[swap_menu_listing_right].strings =
		right_column_strings;
	
	struct glyph_infos * __restrict const glyph_infos = 
		swap_menu_infos->common_graphics_data->fonts_glyphs;
	generated_quads_uS listing_selections_quads =
		ui_text_listing_generate_selection_quads(
			swap_menu_infos->listings, 2, scratch_buffer, glyph_infos
		);
	generated_quads_uS listings_quads = 
		ui_text_listing_generate_text_quads(
			swap_menu_infos->listings, 2,
			scratch_buffer+listing_selections_quads.size, glyph_infos
		);
	
	glGetError();
	glBindBuffer(
		GL_ARRAY_BUFFER, swap_menu_infos->gl_infos.content_buffer_id
	);
	glBufferSubData(
		GL_ARRAY_BUFFER, gpu_offset,
		listings_quads.size + listing_selections_quads.size, scratch_buffer
	);
	
	swap_menu_infos->quads.selections = listing_selections_quads;
	swap_menu_infos->quads.columns = listings_quads;
}

void set_swap_menu_buttons
(swap_menus * __restrict const swap_menu_infos,
 swap_menu_buttons const * __restrict const buttons_configuration)
{
	uint_fast32_t n_buttons = buttons_configuration->n_buttons;
	hitboxes_S_t const hitboxes = swap_menu_infos->hitboxes;
	
	for (uint_fast32_t i = 0; i < n_buttons; i++) {
		struct menu_button_configuration button_config =
			buttons_configuration->buttons_settings[i];
		struct hitbox_action_S * button_hitbox =
			hitboxes.data+button_config.button;
			
		button_hitbox->action      = button_config.action;
		button_hitbox->action_data = button_config.data;
	}
}

