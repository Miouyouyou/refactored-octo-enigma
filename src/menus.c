#include <myy.h>
#include <src/menus.h>
#include <myy/generated/opengl/data_config.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/log.h>

#include <src/nodes.h>

unsigned int quads_in_context_menu;

uint8_t current_context_text_buffer = 0;
uint8_t display_context_menu = 0;

extern uint8_t scratch_buffer[];

static inline unsigned int toggle
(unsigned int boolean)
{
	return boolean ^ 1;
}

static inline unsigned int buffer_switch
(unsigned int const buffer)
{
	return buffer ^ 1;
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

	unsigned int size_used = myy_strings_to_quads_va(
		myy_glyph_infos, n_strings, strings, scratch_buffer,
		strings_vertical_separation_px
	);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size_used, scratch_buffer);
	quads_in_context_menu = size_used / sizeof(US_two_tris_quad_3D);
	current_context_text_buffer = other_text_buffer;
}

void draw_context_menu
(GLuint const * __restrict const programs,
 GLuint const * __restrict const menu_parts_buffers,
 GLuint const * __restrict const menu_content_buffer)
{
	if (display_context_menu) {
		glUseProgram(programs[glsl_program_node]);

		glUniform1i(node_shader_unif_sampler, glsl_texture_fonts);
		glUniform4f(node_shader_unif_px_offset, 0, 0, 1100, 40);

		glBindBuffer(
			GL_ARRAY_BUFFER, menu_content_buffer[current_context_text_buffer]
		);
		glUniform1f(node_shader_unif_layer, 0.20f);
		glVertexAttribPointer(
			node_shader_attr_xyz, 3, GL_SHORT, GL_FALSE,
			sizeof(struct US_textured_point_3D),
			(uint8_t *) offsetof(struct US_textured_point_3D, x)
		);
		glVertexAttribPointer(
			node_shader_attr_st, 2, GL_UNSIGNED_SHORT, GL_TRUE,
			sizeof(struct US_textured_point_3D),
			(uint8_t *) offsetof(struct US_textured_point_3D, s)
		);
		glDrawArrays(GL_TRIANGLES, 0, 6*quads_in_context_menu);

		glUseProgram(programs[glsl_program_fixed_widgets]);
		glUniform4f(node_shader_unif_px_offset, 1080, 0, 256, 0.0f); 
		glUniform1i(node_shader_unif_sampler, glsl_texture_menus);
		glBindBuffer(GL_ARRAY_BUFFER, menu_parts_buffers[0]);
		glVertexAttribPointer(
			node_shader_attr_xyz, 3, GL_SHORT, GL_FALSE,
			sizeof(struct US_textured_point_3D),
			(uint8_t *) offsetof(struct US_textured_point_3D, x)
		);
		glVertexAttribPointer(
			node_shader_attr_st, 2, GL_UNSIGNED_SHORT, GL_TRUE,
			sizeof(struct US_textured_point_3D),
			(uint8_t *) offsetof(struct US_textured_point_3D, s)
		);
		glDrawArrays(GL_TRIANGLES, 0, 12);
	}
}

void enable_context_menu()
{
	display_context_menu = 1;
}

void disable_context_menu()
{
	display_context_menu = 0;
}

unsigned int const menu_x_position = 1080;
static unsigned clicked_on_context_menu(int x, int win_y)
{
	return (display_context_menu && x > menu_x_position);
}

unsigned int manage_current_menu_click(int x, int win_y) 
{
	unsigned int inside_x = x - menu_x_position;
	unsigned int inside = clicked_on_context_menu(x, win_y);
	if (inside && inside_x > 170 && win_y < 30)
		display_context_menu = 0;
	else if (inside && win_y > 40) {
		unsigned int const element = (win_y - 40) / 32;
		LOG("Clicked on %d element.\n", element);
		set_test_frame_mnemonic(element);
		display_context_menu = 0;
	}
	return inside;
}
