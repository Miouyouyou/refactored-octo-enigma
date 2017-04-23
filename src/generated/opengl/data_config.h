#ifndef MYY_SRC_GENERATED_DATAINFOS
#define MYY_SRC_GENERATED_DATAINFOS 1

#include <myy/current/opengl.h>
struct glsl_shader {
	GLuint type;
	uint32_t str_pos;
};
enum glsl_shader_name {
	lines_vsh,
	lines_fsh,
	node_vsh,
	node_fsh,
	fixed_widgets_vsh,
	fixed_widgets_fsh,
	color_node_vsh,
	color_node_fsh,
	glsl_shaders_count
};

enum glsl_program_name {
	glsl_program_lines,
	glsl_program_node,
	glsl_program_fixed_widgets,
	glsl_program_color_node,
	glsl_programs_count
};

struct glsl_programs_shared_data {
	GLuint programs[glsl_programs_count];
	struct glsl_shader shaders[glsl_shaders_count];
	GLchar strings[512];
};

enum lines_shader_attributes {
	lines_shader_attr_xyz,
	n_attrs
};

enum lines_shader_uniforms {
	lines_shader_unif_offset,
	lines_shader_unif_color,
	n_uniforms
};

enum color_node_shader_attributes {
	color_node_shader_attr_xy,
	color_node_shader_attr_rgba
};

enum color_node_shader_uniforms {
	color_node_shader_unif_unused,
	color_node_shader_unif_px_offset,
	color_node_shader_unif_projection,
	color_node_shader_unif_layer
};

enum node_shader_attributes {
	node_shader_attr_xyz,
	node_shader_attr_st,
};

enum node_shader_uniforms {
	node_shader_unif_sampler,
	node_shader_unif_px_offset,
	node_shader_unif_px_to_norm,
	node_shader_unif_layer,
};

enum myy_current_textures_id {
	glsl_texture_fonts,
	glsl_texture_cursor,
	glsl_texture_menus,
	n_textures_id
};

void glhShadersPackLoader
(struct glsl_programs_shared_data * __restrict const data);

#endif
