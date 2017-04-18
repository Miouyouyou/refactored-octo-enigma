#include <myy.h>
#include <src/menus.h>
#include <src/generated/opengl/data_config.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/log.h>

#include <src/nodes.h>

unsigned int quads_in_context_menu;

uint8_t current_context_text_buffer = 0;
uint8_t display_context_menu = 0;
uint8_t display_swap_menu = 0;

unsigned int const default_vertical_space = 32; // pixels
struct {
  int16_t x, y;
} const default_context_menu_content_pos = {
  .x = 1080,
  .y = 0
};
struct {
  uint8_t id;
  uint16_t y_pos;
} current_dropdown_menu = {
  .id = 0,
  .y_pos = 40
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

void prepare_static_menu_parts()
{

	US_two_tris_quad_3D context_menu_elements[n_context_menu_elements] = {
		[context_menu_background] =
		  /*         pixels      layer (1/1024)  UV (norm Unsigned Short) */
			STXYZ_QUAD(0, 200, 0, 720, 1, 40000, 40000, 40000, 40000),
		[context_menu_close_button] =
			STXYZ_QUAD(200-32, 200, 0, 32, 0, 0, 8191, 256, 8191),
		[swap_menu_left_part] =
			STXYZ_QUAD(120,446, 150, 620, 0, 30000, 30000, 30000, 30000),
		[swap_menu_right_part] =
			STXYZ_QUAD(534, 860, 150, 620, 0, 30000, 30000, 30000, 30000),
		[swap_menu_swap_button_up] =
			STXYZ_QUAD(466, 514, 190, 214, 0, 4096, 4096, 4096, 4096),
		[swap_menu_swap_button_down] =
			STXYZ_QUAD(466, 514, 556, 580, 0, 4096, 4096, 4096, 4096),
		[swap_menu_background] =
			STXYZ_QUAD(100, 880, 50, 670, 32, 30000,30000,30000,30000),
	};
	
	glBufferSubData(
		GL_ARRAY_BUFFER, 0, sizeof(US_two_tris_quad_3D)*n_context_menu_elements,
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

void draw_context_menu
(struct dropdown_menus_infos const * __restrict const menus,
 enum ga_dropdown_menu menu,
 GLuint const * __restrict const programs,
 GLuint const static_parts_buffer,
 GLuint const dropdowns_contents_buffer)
{
	if (display_context_menu) {
		glUseProgram(programs[glsl_program_node]);

		glUniform1i(node_shader_unif_sampler, glsl_texture_fonts);
    uint16_t context_menu_dropdown_x =
      default_context_menu_content_pos.x + 20;
    uint16_t context_menu_dropdown_y =
      current_dropdown_menu.y_pos;
		glUniform4f(
      node_shader_unif_px_offset, 0, 0,
      context_menu_dropdown_x,
      context_menu_dropdown_y
    );

		glUniform1f(node_shader_unif_layer, 0.20f);
		struct dropdown_menus_infos const * __restrict const menu_infos =
			menus+menu;
		US_two_tris_quad_3D_draw_pixelscoords(
			dropdowns_contents_buffer,
			node_shader_attr_xyz,
			node_shader_attr_st,
			menu_infos->buffer_start,
			menu_infos->buffer_quads
		);

		glUseProgram(programs[glsl_program_fixed_widgets]);
		glUniform4f(node_shader_unif_px_offset, 1080, 0, 256, 0.0f); 
		glUniform1i(node_shader_unif_sampler, glsl_texture_menus);
		US_two_tris_quad_3D_draw_pixelscoords(
			static_parts_buffer, node_shader_attr_xyz,
			node_shader_attr_st, 0, 2
		);
	}
}

void draw_swap_menu
(struct swap_menu_infos const * __restrict const menu_infos,
 GLuint const * __restrict const programs,
 GLuint const dynamic_parts_buffer,
 GLuint const static_parts_buffer)
{
	if (display_swap_menu) {
		glUseProgram(programs[glsl_program_node]);
		glUniform1i(node_shader_unif_sampler, glsl_texture_fonts);
		glUniform4f(
      node_shader_unif_px_offset, 0, 0,
      120,70
    );
		glUniform1f(node_shader_unif_layer, 0.19f);
		US_two_tris_quad_3D_draw_pixelscoords(
			dynamic_parts_buffer,
			node_shader_attr_xyz,
			node_shader_attr_st,
			0x1000,
			menu_infos->title_quads
		);
		glUniform4f(
      node_shader_unif_px_offset, 0, 0,
      120,150
    );
		US_two_tris_quad_3D_draw_pixelscoords(
			dynamic_parts_buffer,
			node_shader_attr_xyz,
			node_shader_attr_st,
			0x1000+menu_infos->title_size,
			menu_infos->columns_quads
		);
		glUseProgram(programs[glsl_program_fixed_widgets]);
		glUniform4f(node_shader_unif_px_offset, 0, 0, 256, 0.0f); 
		glUniform1i(node_shader_unif_sampler, glsl_texture_menus);
		US_two_tris_quad_3D_draw_pixelscoords(
			static_parts_buffer, node_shader_attr_xyz,
			node_shader_attr_st,
			swap_menu_left_part*sizeof(US_two_tris_quad_3D), 5
		);
	}
}

void draw_current_context_menu
(struct dropdown_menus_infos const * __restrict const menus,
 GLuint const * __restrict const programs,
 GLuint const static_parts_buffer,
 GLuint const dropdowns_contents_buffer) {
  draw_context_menu(
    menus, current_dropdown_menu.id, programs, static_parts_buffer,
    dropdowns_contents_buffer
  );
}

static unsigned int get_buffer_start_of
(struct dropdown_menus_infos const * __restrict const menus,
 enum ga_dropdown_menu menu)
{
	unsigned int offset = 0;
	for (
		enum ga_dropdown_menu previous_menu =
			ga_dropdown_menu_instructions;
		previous_menu < menu;
		previous_menu++
	)
	{
		offset += menus[previous_menu].buffer_start;
		offset += menus[previous_menu].buffer_size;
	}
	return offset;
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

	struct generated_quads char_quads = myy_strings_to_quads_va(
		myy_glyph_infos, n_strings, strings, scratch_buffer,
		strings_vertical_separation_px
	);
	glBufferSubData(GL_ARRAY_BUFFER, 0, char_quads.size, scratch_buffer);
	quads_in_context_menu = char_quads.count;
	current_context_text_buffer = other_text_buffer;
}

static unsigned int generate_and_store_menu_in_gpu
(struct dropdown_menus_infos * __restrict const menus,
 enum ga_dropdown_menu menu,
 GLuint const gpu_buffer_id,
 unsigned int offset,
 struct glyph_infos const * __restrict const myy_glyph_infos)
{
	struct dropdown_menus_infos const menu_infos = menus[menu];
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer_id);

	struct generated_quads char_quads = myy_strings_to_quads_va(
		myy_glyph_infos, menu_infos.n_strings, menu_infos.strings,
		scratch_buffer, default_vertical_space
	);
	
	glBufferSubData(
		GL_ARRAY_BUFFER, offset, char_quads.size, scratch_buffer
	);
	
	menus[menu].buffer_start = offset;
	menus[menu].buffer_size  = char_quads.size;
	menus[menu].buffer_quads = char_quads.count;
	
	return char_quads.size;
}

void draw_menu
(struct dropdown_menus_infos * __restrict const menus,
 GLuint const gpu_buffer_id)
{
	
}

void regenerate_menus
(struct dropdown_menus_infos * __restrict const menus,
 GLuint const gpu_buffer_id,
 struct glyph_infos const * __restrict const myy_glyph_infos)
{
	unsigned int offset = 0;
	
	for (enum ga_dropdown_menu current_menu = 0;
	     current_menu < n_ga_menus;
	     current_menu++) {
		offset += generate_and_store_menu_in_gpu(
			menus, current_menu, gpu_buffer_id, offset, myy_glyph_infos
		);
	}
}

void setup_menu
(struct dropdown_menus_infos * __restrict const menus,
 enum ga_dropdown_menu menu,
 uint8_t const * const * const strings,
 unsigned int const n_strings,
 void (* const hitbox_func)(unsigned int hit_index))
{
	menus[menu].strings = strings;
	menus[menu].n_strings = n_strings;
	menus[menu].hitbox_func = hitbox_func;
}


void enable_context_menu() { display_context_menu = 1; }
void disable_context_menu() { display_context_menu = 0; }
void enable_swap_menu() { display_swap_menu = 1; }
void disable_swap_menu() { display_swap_menu = 0; }
void set_current_dropdown_menu(enum ga_dropdown_menu current_menu_id) {
	current_dropdown_menu.id = current_menu_id;
}

unsigned int const menu_x_position = 1080;
static unsigned clicked_on_context_menu(int x, int win_y)
{
	return (display_context_menu && x > menu_x_position);
}

unsigned int manage_current_menu_click
(struct dropdown_menus_infos const * __restrict const menus,
 int const x, int const win_y)
{
	unsigned int inside_x = x - menu_x_position;
	unsigned int inside = clicked_on_context_menu(x, win_y);
	if (inside && inside_x > 170 && win_y < 30)
		display_context_menu = 0;
	else if (inside && win_y > 40) {
		unsigned int const element = (win_y - 40) / 32;
		struct dropdown_menus_infos const * __restrict const current_menu_infos =
			menus+current_dropdown_menu.id;
		if (element < current_menu_infos->n_strings)
			menus[current_dropdown_menu.id].hitbox_func(element);
	}
	return inside;
}

void set_swap_menu_title
(struct swap_menu_infos * __restrict const swap_menu_infos,
 GLuint const buffer_id, uint8_t const * __restrict const title,
 struct glyph_infos const * __restrict const glyph_infos)
{
	unsigned int const default_gpu_offset = 0x1000;
	struct text_offset text_offset = {
		.x_offset = 0,
		.y_offset = 0
	};
	struct generated_quads quads = myy_single_string_to_quads(
		glyph_infos, title, scratch_buffer, &text_offset
	);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
	glBufferSubData(
		GL_ARRAY_BUFFER, default_gpu_offset, quads.size, scratch_buffer
	);
	
	swap_menu_infos->title_size  = quads.size;
	swap_menu_infos->title_quads = quads.count;
}

void set_swap_menu_listings
(struct swap_menu_infos * __restrict const swap_menu_infos,
 GLuint const buffer_id,
 unsigned int const n_strings_left,
 unsigned int const n_strings_right,
 uint8_t const * const * __restrict const left_column_strings,
 uint8_t const * const * __restrict const right_column_strings,
 struct glyph_infos const * __restrict const glyph_infos)
{
	unsigned int const gpu_offset = 0x1000 + swap_menu_infos->title_size;
	
	struct generated_quads generated_quads = myy_strings_to_quads_va(
		glyph_infos, n_strings_left, left_column_strings,
		scratch_buffer, 24
	);
	unsigned int const left_quads = generated_quads.count;
	unsigned int const left_size  = generated_quads.size;

	generated_quads = myy_strings_to_quads_va(
		glyph_infos, n_strings_right, right_column_strings,
		scratch_buffer+left_size, 24
	);
	unsigned int const total_size  = left_size + generated_quads.size;
	unsigned int const total_quads = left_quads + generated_quads.count;

	glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
	glBufferSubData(
		GL_ARRAY_BUFFER, gpu_offset, total_size, scratch_buffer
	);
	
	swap_menu_infos->columns_quads   = total_quads;
	swap_menu_infos->columns_size    = total_size;
	swap_menu_infos->left_n_options  = n_strings_left;
	swap_menu_infos->right_n_options = n_strings_right;
}
