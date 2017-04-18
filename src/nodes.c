#include <src/nodes.h>

#include <stdint.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/log.h>
#include <src/generated/opengl/data_config.h>

#include <strings.h>


static void gpu_node_part
(int32_t const x, int32_t const y,
 int32_t const width, int32_t const height,
 union SuB_two_triangles_colored_quad_2D_representations * __restrict
 const quad,
 uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a)
{
	SuB_2t_colored_quad_store(
		x, y, x+width, y+height, quad,
		r, g, b, a
	);
}

static unsigned int offset_between
(uintptr_t const begin, uintptr_t const end)
{
	return (unsigned int) (end - begin);
}
static struct generated_quads generate_nodes_containers
(struct nodes_display_data * __restrict const nodes,
 uint8_t * __restrict const cpu_buffer,
 uint32_t base_gpu_offset)
{
	//unsigned int corner_size = CORNER_SIZE;
	gpu_node_representation * __restrict const casted_cpu_buffer =
		(gpu_node_representation *) cpu_buffer;
	unsigned int const n_nodes = nodes->containers.n_nodes;
	memset(
		cpu_buffer, 0, n_nodes * sizeof(gpu_node_representation)
	);
	uint32_t cpu_offset = 0;
	for (unsigned int n = 0; n < n_nodes; n++) {
		
		int const title_height = 24;
		int const margins = 12;
		struct node_container_metadata * __restrict const
			current_container = nodes->containers.metadata+n;
		struct node_contents_metadata const * __restrict const
			current_content = nodes->contents.metadata+n;
		
		uint16_t const width = current_container->width + margins;
		uint16_t const content_height = current_container->height + margins;
		int16_t const x = current_content->x;
		int16_t const y = current_content->y;
		
		gpu_node_representation * __restrict const current_gpu_node = 
			casted_cpu_buffer+n;

		gpu_node_part(
			x, y, width, title_height,
			&current_gpu_node->title_bar,
			20,75,175,255
		);
		gpu_node_part(
			x, y+title_height, width, content_height,
			&current_gpu_node->main_body,
			0,0,0,255
		);
		
		current_container->buffer_offset = 
			cpu_offset + base_gpu_offset;
		cpu_offset += 2*sizeof(gpu_node_representation);

	}
	
	unsigned int const quads_count = 2*n_nodes;
	
	struct generated_quads generated_quads = {
		.size = cpu_offset,
		.count = quads_count
	};
	
	return generated_quads;
}

struct generated_quads generate_and_store_nodes_containers_in_gpu
(struct nodes_display_data * __restrict const nodes,
 uint8_t * __restrict const cpu_buffer,
 GLuint const gpu_buffer_id, GLuint const gpu_buffer_offset)
{

	struct generated_quads generated_quads = 
		generate_nodes_containers(nodes, cpu_buffer, gpu_buffer_offset);

	glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer_id);
	glBufferSubData(
		GL_ARRAY_BUFFER, gpu_buffer_offset,
		generated_quads.size, cpu_buffer
	);
	
	nodes->containers.buffer_id = gpu_buffer_id;
	nodes->containers.buffer_offset = gpu_buffer_offset;
	nodes->containers.quads = generated_quads.count;
	
	return generated_quads;
}

void draw_nodes
(struct nodes_display_data * __restrict const nodes,
 GLuint const * __restrict const programs,
 int16_t const global_offset_x, int16_t const global_offset_y)
{
	unsigned int const n_nodes = nodes->containers.n_nodes;
	unsigned int const title_height = 24;
	unsigned int const margin = 6;
	unsigned int const added_y_px = title_height + margin;
	unsigned int const added_x_px = margin;
	
	glUseProgram(programs[glsl_program_node]);
	glEnableVertexAttribArray(node_shader_attr_st);
	glEnableVertexAttribArray(node_shader_attr_xyz);
	glUniform1i(node_shader_unif_sampler, glsl_texture_fonts);
	glUniform1f(node_shader_unif_layer, 0.4f);
	GLuint const nodes_contents_gpu_buffer_id =
		nodes->contents.buffer_id;
	for (unsigned int n = 0; n < n_nodes; n++) {
		struct node_contents_metadata current_contents =
			nodes->contents.metadata[n];
		
		glUniform4f(
			node_shader_unif_px_offset,
			global_offset_x, global_offset_y,
			current_contents.x+added_x_px, current_contents.y+added_y_px
		);
		
		US_two_tris_quad_3D_draw_pixelscoords(
			nodes_contents_gpu_buffer_id,
			node_shader_attr_xyz,
			node_shader_attr_st,
			current_contents.buffer_offset,
			current_contents.quads
		);
		
	}
	
	// Containers -- Background must be drawn last to avoid overdraw
	glUseProgram(programs[glsl_program_color_node]);
	glEnableVertexAttribArray(color_node_shader_attr_xy);
	glEnableVertexAttribArray(color_node_shader_attr_rgba);
	glUniform4f(
		color_node_shader_unif_px_offset,
		global_offset_x, global_offset_y,
		0,0
	);
	glUniform1f(node_shader_unif_layer, 0.5f);

	SuB_2t_colored_quad_draw_pixel_coords(
		nodes->containers.buffer_id,
		color_node_shader_attr_xy,
		color_node_shader_attr_rgba,
		nodes->containers.buffer_offset,
		nodes->containers.quads
	);
	
}


