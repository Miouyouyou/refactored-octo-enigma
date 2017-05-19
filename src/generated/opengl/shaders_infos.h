#ifndef MYY_GENERATED_OPENGL_ENUMS_H
#define MYY_GENERATED_OPENGL_ENUMS_H 1
#include <myy/current/opengl.h>
#include <stdint.h>
enum glsl_shader_name {
	lines_vsh,
	lines_fsh,
	node_vsh,
	node_fsh,
	fixed_widgets_vsh,
	fixed_widgets_fsh,
	color_node_vsh,
	color_node_fsh,
	color_static_vsh,
	color_static_fsh,
	n_glsl_shaders,
};

enum glsl_program_name {
	glsl_program_lines,
	glsl_program_node,
	glsl_program_fixed_widgets,
	glsl_program_color_node,
	glsl_program_color_static,
	n_glsl_programs,
};

enum glsl_program_uniform {
	lines_shader_unif_offset,
	lines_shader_unif_color,
	node_shader_unif_px_offset,
	node_shader_unif_px_to_norm,
	node_shader_unif_layer,
	node_shader_unif_sampler,
	fixed_widgets_shader_unif_px_offset,
	fixed_widgets_shader_unif_px_to_norm,
	color_node_shader_unif_px_offset,
	color_node_shader_unif_projection,
	color_node_shader_unif_layer,
	color_static_shader_unif_projection,
	color_static_shader_unif_layer,
	n_glsl_program_uniforms,
};

enum lines_attribute {
	lines_shader_attr_xyz,
	n_lines_attributes,
};

enum node_attribute {
	node_shader_attr_xyz,
	node_shader_attr_in_st,
	n_node_attributes,
};

enum fixed_widgets_attribute {
	fixed_widgets_shader_attr_xyz,
	fixed_widgets_shader_attr_in_rgba,
	n_fixed_widgets_attributes,
};

enum color_node_attribute {
	color_node_shader_attr_xy,
	color_node_shader_attr_in_rgba,
	n_color_node_attributes,
};

enum color_static_attribute {
	color_static_shader_attr_xyz,
	color_static_shader_attr_in_rgba,
	n_color_static_attributes,
};

struct glsl_elements {
	struct { uint16_t n, pos; } attributes, uniforms;
};

struct glsl_shader {
	GLuint type;
	uint32_t str_pos;
};

struct glsl_programs_shared_data {
	GLuint programs[n_glsl_programs];
	GLint  unifs[n_glsl_program_uniforms];
	struct glsl_shader shaders[n_glsl_shaders];
	struct glsl_elements metadata[n_glsl_programs];
	uint8_t strings[228];
	uint8_t identifiers[162];
};


#endif
