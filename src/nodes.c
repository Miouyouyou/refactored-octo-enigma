#include <src/nodes.h>
#include <src/text.h>

#include <stdint.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/log.h>
#include <src/generated/opengl/data_config.h>

#include <strings.h>

void set_nodes_buffers
(struct nodes_display_data * __restrict const nodes,
 GLuint const containers_buffer_id,
 GLuint const containers_buffer_offset,
 GLuint const contents_buffer_id,
 GLuint const contents_buffer_offset)
{
	nodes->containers.buffer_id     = containers_buffer_id;
	nodes->containers.buffer_offset = containers_buffer_offset;
	nodes->contents.buffer_id       = contents_buffer_id;
	nodes->contents.buffer_offset   = contents_buffer_offset;
}
static void gpu_node_part
(int32_t const x, int32_t const y,
 int32_t const width, int32_t const height,
 SuB_2t_colored_quad * __restrict
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

struct width_height {
	uint16_t width, height;
};

static struct width_height decorated_container_dimensions
(struct node_container_metadata const * __restrict const container_infos)
{
	int const margins = 12;
	struct width_height dimensions = {
		.width  = container_infos->width + margins,
		.height = container_infos->height + margins
	};
	return dimensions;
}
 

static struct generated_quads generate_node_container
(struct nodes_display_data * __restrict const nodes,
 gpu_node_representation * __restrict const current_node_buffer,
 GLuint const node_index)
{
	int const title_height = 24;
	
	struct node_container_metadata * __restrict const
		current_container = nodes->containers.metadata+node_index;
	struct node_contents_metadata const * __restrict const
		current_content = nodes->contents.metadata+node_index;
	
	struct width_height container_dimensions =
		decorated_container_dimensions(current_container);

	// TODO : Compute the container height without the title before
	//        hand...
	container_dimensions.height -= title_height;
	int16_t const x = current_content->x;
	int16_t const y = current_content->y;

	gpu_node_part(
		x, y, container_dimensions.width, title_height,
		&current_node_buffer->title_bar,
		175,20,75,255
	);
	gpu_node_part(
		x, y+title_height,
		container_dimensions.width, container_dimensions.height,
		&current_node_buffer->main_body,
		0,0,0,255
	);
	
	struct generated_quads quads = {
		.count = 2,
		.size  = sizeof(gpu_node_representation)
	};
	
	return quads;
}

static struct generated_quads generate_nodes_containers
(struct nodes_display_data * __restrict const nodes,
 uint8_t * __restrict const cpu_buffer)
{
	//unsigned int corner_size = CORNER_SIZE;
	gpu_node_representation * __restrict const casted_cpu_buffer =
		(gpu_node_representation *) cpu_buffer;
	unsigned int const n_nodes = nodes->containers.n_nodes;
	memset(
		cpu_buffer, 0, n_nodes * sizeof(gpu_node_representation)
	);
	uint32_t cpu_offset = 0;
	GLuint base_gpu_offset = nodes->containers.buffer_offset;
	unsigned int quads_count = 0;
	for (unsigned int n = 0; n < n_nodes; n++) {

		struct generated_quads container_quads = 
			generate_node_container(nodes, casted_cpu_buffer+n, n);
		
		nodes->containers.metadata[n].buffer_offset =
			cpu_offset + base_gpu_offset;
		quads_count += container_quads.count;
		cpu_offset += container_quads.size;
	}
	
	struct generated_quads generated_quads = {
		.size = cpu_offset,
		.count = quads_count
	};
	
	return generated_quads;
}

struct generated_quads generate_and_store_nodes_containers_in_gpu
(struct nodes_display_data * __restrict const nodes,
 uint8_t * __restrict const cpu_buffer)
{

	struct generated_quads generated_quads = 
		generate_nodes_containers(nodes, cpu_buffer);

	GLuint const gpu_buffer_id =
		nodes->containers.buffer_id;
	GLuint const gpu_buffer_offset = 
		nodes->containers.buffer_offset;
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

static void generate_and_store_node_container_in_gpu
(struct nodes_display_data * __restrict const nodes,
 uint8_t * __restrict const cpu_buffer,
 unsigned int const id)
{
	gpu_node_representation * __restrict const casted_buffer =
		(gpu_node_representation *) cpu_buffer;
	struct generated_quads container_quads = generate_node_container(
		nodes, casted_buffer, id
	);
	glBindBuffer(GL_ARRAY_BUFFER, nodes->containers.buffer_id);
	glBufferSubData(
		GL_ARRAY_BUFFER, nodes->containers.metadata[id].buffer_offset,
		container_quads.size, casted_buffer
	);
}

void draw_nodes
(struct nodes_display_data * __restrict const nodes,
 GLuint const * __restrict const programs,
 int16_t const global_offset_x, int16_t const global_offset_y)
{
	unsigned int const n_nodes = nodes->containers.n_nodes;
	unsigned int const title_height = 24;
	unsigned int const margin = 6;
	int const added_y_px = margin;
	int const added_x_px = margin;
	
	// Containers -- 
	//   Background should be drawn last to avoid overdraw
	//   however, dealing with transparent text is kind of
	//   pain when doing this...
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
	
	// Nodes contents
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		
		draw_character_quads(
			nodes_contents_gpu_buffer_id,
			node_shader_attr_xyz,
			node_shader_attr_st,
			current_contents.buffer_offset,
			current_contents.quads
		);
		
	}
	glDisable(GL_BLEND);
	
}

void set_node_position
(struct nodes_display_data * __restrict const nodes,
 int const id, int const x, int const y,
 uint8_t * __restrict const buffer)
{

	nodes->containers.metadata[id].x = x;
	nodes->containers.metadata[id].y = y;
	nodes->contents.metadata[id].x = x;
	nodes->contents.metadata[id].y = y;
	generate_and_store_node_container_in_gpu(nodes, buffer, id);
}

void nodes_set_selected_node_position
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y, uint8_t * __restrict const buffer)
{
	int selected_node_id = nodes->selected_node_id;
	/* All this circus so that, when grabing the title bar, the cursor
	   stays at the same relative position from the title bar, instead of
	   having the upper-left corner of the title bar jump to the cursor
	   position.
	*/
	int rel_x = x - nodes->selection_x_offset;
	int rel_y = y - nodes->selection_y_offset;
	set_node_position(nodes, selected_node_id, rel_x, rel_y, buffer);
}

int nodes_at
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y)
{
	unsigned int n_nodes = nodes->containers.n_nodes;
	unsigned int index = -1;
	
	for (unsigned int n = 0; n < n_nodes; n++) {
		struct node_container_metadata const * __restrict const
			node_position = nodes->containers.metadata+n;
		struct width_height dimensions =
			decorated_container_dimensions(node_position);
		int node_x = node_position->x;
		int node_y = node_position->y;
		if (x > node_x &&
		    x < node_x + dimensions.width &&
		    y > node_y &&
		    y < node_y + dimensions.height) {
			index = n;
			break;
		}
	}
	
	return index;
}

int nodes_selected_at
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y)
{
	int id = nodes_at(nodes, x, y);
	nodes->selected_node_id = id;
	return id;
}

void nodes_set_handler_func
(struct nodes_display_data * __restrict const nodes,
 unsigned int func_id,
 void (*func_address)())
{
	nodes->click_handlers[func_id] = func_address;
}

int nodes_handle_click
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y)
{
	LOG("[nodes_handle_click]\n  %d, %d\n", x, y);
	int id = nodes_selected_at(nodes, x, y);
	if (id > -1) {
		struct node_container_metadata current_node =
			nodes->containers.metadata[id];
		int rel_x = x - current_node.x;
		int rel_y = y - current_node.y;
		LOG("rel_x : %d, rel_y : %d\n", rel_x, rel_y);
		if (rel_y < TITLE_HEIGHT) {
			nodes->selected_node_movable = 1;
			nodes->selection_x_offset = rel_x;
			nodes->selection_y_offset = rel_y;
		}
		else {
			nodes->selected_node_movable = 0;
			nodes->click_handlers[current_node.handler_func_id](
				nodes, id, rel_x, rel_y - TITLE_HEIGHT
			);
		}
		
	}
	else { 
		nodes->selected_node_movable = 0;
	}
	return id;
}

// TODO : Remove the last 'buffer' argument !
//        It's only used to regenerate the containers on change
int nodes_handle_move
(struct nodes_display_data * __restrict const nodes,
 int x, int y, uint8_t * __restrict const buffer)
{
	unsigned int moving_a_node = 
		nodes->selected_node_id > -1 && nodes->selected_node_movable;
	if (moving_a_node)
		nodes_set_selected_node_position(nodes, x, y, buffer);
	return moving_a_node; // move consumed
}
