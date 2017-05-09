#include <myy.h>
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
	swap_menu_swap_button_up,
	swap_menu_swap_button_down,
	swap_menu_background,
	n_context_menu_elements
};

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

struct menu_gl_coords {
	box_coords_S_t coords;
	uint16_t depth;
	struct { uint16_t left, right, top, bottom; } tex_coords;
} const original_menu_coords[n_context_menu_elements] = {
	[context_menu_background] = {
		.coords =
			{.left = 0, .right = 200, .top = 0,   .bottom = 720},
		.depth = 1,
		.tex_coords =
			{.left = 40000,  .right = 40000, .top = 40000, .bottom = 40000}
	},
	[context_menu_close_button] = {
		.coords = 
			{.left = 200-32, .right = 200, .top = 0,   .bottom = 32},
		.depth = 0,
		.tex_coords =
			{.left = 0, .right = 8191, .top = 8191, .bottom = 256}
	},
	[swap_menu_left_part] = {
		.coords =
			{.left = 120,    .right = 446, .top = 150, .bottom = 620},
		.depth = 0,
		.tex_coords = {30000,30000,30000,30000}
	},
	[swap_menu_right_part] = {
		.coords =
			{.left = 534,    .right = 860, .top = 150, .bottom = 620},
		.depth = 0,
		.tex_coords = {30000,30000,30000,30000}
	},
	[swap_menu_swap_button_up] = {
		.coords =
			{.left = 466,    .right = 514, .top = 190, .bottom = 214},
		.depth = 0,
		.tex_coords = {4096,4096,4096,4096}
  },
	[swap_menu_swap_button_down] = {
		.coords =
			{.left = 466,    .right = 514, .top = 556, .bottom = 580},
		.depth = 0,
		.tex_coords = {4096,4096,4096,4096}
	},
	[swap_menu_background] = {
		.coords =
			{.left = 100,    .right = 880, .top = 50,  .bottom = 670},
		.depth = 32,
		.tex_coords = {30000,30000,30000,30000}
	}
};

struct menu_gl_coords actual_menu_coords[n_context_menu_elements];

inline static void box_coords_S_rebase_from_1280_720
(box_coords_S_t * __restrict const dst,
 box_coords_S_t const * __restrict const src,
 unsigned int width, unsigned int height)
{
	dst->left   = (uint16_t) (src->left   * width / 1280);
	dst->right  = (uint16_t) (src->right  * width / 1280);
	dst->top    = (uint16_t) (src->top    * height / 720);
	dst->bottom = (uint16_t) (src->bottom * height / 720);
}

void menus_recalculate_dimensions
(uint16_t width, uint16_t height)
{
	for (unsigned int b = 0; b < n_context_menu_elements; b++) {
		box_coords_S_rebase_from_1280_720(
			&actual_menu_coords[b].coords,
			&original_menu_coords[b].coords,
			width, height
		);
	}
}

void menus_regen_static_parts
(struct menu_gl_metadata const * __restrict const metadata)
{
	
	US_two_tris_quad_3D context_menu_elements[n_context_menu_elements];
	
	for (unsigned int i = 0; i < n_context_menu_elements; i++) {
		US_two_tris_quad_3D_store(
			context_menu_elements+i,
			actual_menu_coords[i].coords.left,
			actual_menu_coords[i].coords.right,
			actual_menu_coords[i].coords.top,
			actual_menu_coords[i].coords.bottom,
			actual_menu_coords[i].depth,
			actual_menu_coords[i].tex_coords.left,
			actual_menu_coords[i].tex_coords.right,
			actual_menu_coords[i].tex_coords.top,
			actual_menu_coords[i].tex_coords.bottom
		);
		
	}
	
	glBindBuffer(
		GL_ARRAY_BUFFER,
		metadata->static_elements_buffer_id
	);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		metadata->static_elements_buffer_offset,
		sizeof(US_two_tris_quad_3D)*n_context_menu_elements,
		context_menu_elements
	);
}

static inline unsigned int toggle(unsigned int boolean)
{
	return boolean ^ 1;
}

static inline unsigned int buffer_switch(unsigned int const buffer)
{
	return buffer ^ 1;
}

void dropdown_menus_draw
(dropdown_menus const * __restrict const menus,
 enum ga_dropdown_menu menu,
 GLuint const * __restrict const programs)
{
	if (display_context_menu) {
		glUseProgram(programs[glsl_program_fixed_widgets]);
		glUniform4f(node_shader_unif_px_offset, 1080, 0, 256, 0.0f); 
		glUniform1i(node_shader_unif_sampler, glsl_texture_menus);
		US_two_tris_quad_3D_draw_pixelscoords(
			menus->gl_infos.static_elements_buffer_id,
			node_shader_attr_xyz,
			node_shader_attr_st,
			menus->gl_infos.static_elements_buffer_offset,
			2 // quads
		);
		
		uint16_t context_menu_dropdown_x =
			default_context_menu_content_pos.x + 20;
		uint16_t context_menu_dropdown_y = current_dropdown_menu.y_pos;

		glUseProgram(programs[glsl_program_node]);
		glUniform1i(node_shader_unif_sampler, glsl_texture_fonts);
		glUniform4f(
			node_shader_unif_px_offset, 0, 0,
			context_menu_dropdown_x, context_menu_dropdown_y
		);

		glUniform1f(node_shader_unif_layer, 0.20f);
		struct dropdown_menu_infos const * __restrict const
			current_menu = menus->data+menu;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		draw_character_quads(
			menus->gl_infos.content_buffer_id,
			node_shader_attr_xyz,
			node_shader_attr_st,
			current_menu->offset,
			current_menu->content_quads
		);
		glDisable(GL_BLEND);
	}
}

void swap_menus_draw
(swap_menus const * __restrict const menu_infos,
 GLuint const * __restrict const programs)
{
	if (display_swap_menu) {
		glUseProgram(programs[glsl_program_fixed_widgets]);
		glUniform4f(node_shader_unif_px_offset, 0, 0, 256, 0.0f); 
		glUniform1i(node_shader_unif_sampler, glsl_texture_menus);
		US_two_tris_quad_3D_draw_pixelscoords(
			menu_infos->gl_infos.static_elements_buffer_id, 
			node_shader_attr_xyz,
			node_shader_attr_st,
			menu_infos->gl_infos.static_elements_buffer_offset +
			swap_menu_left_part*sizeof(US_two_tris_quad_3D),
			5 // quads
		);
		glUseProgram(programs[glsl_program_node]);
		glUniform1i(node_shader_unif_sampler, glsl_texture_fonts);
		glUniform4f(
      node_shader_unif_px_offset, 0, 0,
      120,70
    );
		glUniform1f(node_shader_unif_layer, 0.19f);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLuint const dynamic_parts_buffer =
			menu_infos->gl_infos.content_buffer_id;
		GLuint const dynamic_parts_offset =
			menu_infos->gl_infos.content_buffer_offset;
		glUniform4f(
      node_shader_unif_px_offset, 0, 0, 0, 0
    );
		draw_character_quads(
			dynamic_parts_buffer,
			node_shader_attr_xyz,
			node_shader_attr_st,
			dynamic_parts_offset+menu_infos->title_size,
			menu_infos->columns_quads
		);
		glDisable(GL_BLEND);
	}
}

void dropdown_menus_draw_current
(struct dropdown_menus const * __restrict const menus,
 GLuint const * __restrict const programs)
{
  dropdown_menus_draw(
    menus, menus->current_dropdown_menu, programs
  );
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

	struct text_offset text_rel_position_px = {
		.x_offset = 0,
		.y_offset = 0
	};
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

	struct text_offset text_rel_position_px = {
		.x_offset = 0, .y_offset = 0
	};
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
static hitbox_action_S_t swap_menu_hitbox() {
	
}

static uint8_t meep(position_S rel, position_S abs)
{
	LOG(
		"Meep ! (%d, %d) - (%d, %d)\n",
		rel.x, rel.y, abs.x, abs.y
	);
	return 1;
}

void enable_swap_menu
(swap_menus * __restrict const swap_menus)
{ 
	hitboxes_S_add_box_action(
		swap_menus->common_graphics_data->hitboxes,
		&actual_menu_coords[swap_menu_background].coords,
		meep
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
void swap_menus_refresh
(swap_menus * __restrict const swap_menus,
 uint16_t width, uint16_t height)
{
	uint8_t swap_menus_currently_shown = display_swap_menu;
	uint8_t context_menu_currently_shown = display_context_menu;
	disable_swap_menu(swap_menus);
	disable_context_menu();
	menus_recalculate_dimensions(width, height);
	menus_regen_static_parts(&swap_menus->gl_infos);
	
	if (swap_menus_currently_shown)   enable_swap_menu(swap_menus);
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

	struct text_offset text_offset = {
		.x_offset = 0,
		.y_offset = 0
	};
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
	
	swap_menu_infos->title_size  = quads.size;
	swap_menu_infos->title_quads = quads.count;
	
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

	struct text_offset text_position_px = {
		.x_offset = actual_menu_coords[swap_menu_left_part].coords.left,
		.y_offset = actual_menu_coords[swap_menu_left_part].coords.top
	};
	struct glyph_infos * __restrict const glyph_infos = 
		swap_menu_infos->common_graphics_data->fonts_glyphs;
	struct generated_quads generated_quads = myy_strings_to_quads_va(
		glyph_infos, n_strings_left, left_column_strings,
		scratch_buffer, 24, &text_position_px
	);
	text_position_px.x_offset = 
		actual_menu_coords[swap_menu_right_part].coords.left;
	text_position_px.y_offset =
		actual_menu_coords[swap_menu_right_part].coords.top;
	unsigned int const left_quads = generated_quads.count;
	unsigned int const left_size  = generated_quads.size;
	generated_quads = myy_strings_to_quads_va(
		glyph_infos, n_strings_right, right_column_strings,
		scratch_buffer+left_size, 24, &text_position_px
	);
	unsigned int const total_size  = left_size + generated_quads.size;
	unsigned int const total_quads = left_quads + generated_quads.count;

	glBindBuffer(
		GL_ARRAY_BUFFER,
		swap_menu_infos->gl_infos.content_buffer_id
	);
	glBufferSubData(
		GL_ARRAY_BUFFER, gpu_offset, total_size, scratch_buffer
	);
	
	swap_menu_infos->columns_quads   = total_quads;
	swap_menu_infos->columns_size    = total_size;
	swap_menu_infos->left_n_options  = n_strings_left;
	swap_menu_infos->right_n_options = n_strings_right;
}
