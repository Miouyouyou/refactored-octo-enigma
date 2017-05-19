#include <src/nodes.h>
#include <src/text.h>

#include <stdint.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/log.h>
#include <src/generated/opengl/data_config.h>
#include <src/nodes_mem.h>

#include <strings.h>

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

static dimensions_uS node_container_decorated_dimensions
(node_container const * __restrict const container_infos)
{
	int const margins = 12;
	return dimensions_uS_abs_scale(container_infos->dimensions, margins);
}

static struct generated_quads node_container_generate
(node_container const * __restrict const container,
 gpu_node_representation * __restrict const current_node_buffer)
{
	int const title_height = 24;
	
	dimensions_uS container_dimensions =
		node_container_decorated_dimensions(container);

	// TODO : Compute the container height without the title before
	//        hand...
	position_S container_position = container->pos;

	LOG(
		"[node_container_generate]\n"
		"Title height : %d\n"
		"Dimensions : %d, %d\n"
		"Position : %d, %d\n",
		 title_height, 
		 container->dimensions.height, container->dimensions.width,
		 container_position.x, container_position.y
	);
	gpu_node_part(
		container_position.x, container_position.y-title_height,
		container_dimensions.width, title_height,
		&current_node_buffer->title_bar,
		175,20,75,255
	);
	gpu_node_part(
		container_position.x, container_position.y,
		container_dimensions.width, container_dimensions.height,
		&current_node_buffer->main_body,
		0,0,0,255
	);
	
	return generated_quads_uS_struct(2, sizeof(gpu_node_representation));
}

static struct generated_quads nodes_generate_containers
(nodes * __restrict const these_nodes, buffer_t cpu_buffer)
{
	nodes_containers * __restrict const containers =
		&these_nodes->containers;
	//unsigned int corner_size = CORNER_SIZE;
	gpu_node_representation * __restrict const casted_cpu_buffer =
		(gpu_node_representation *) cpu_buffer;
	
	unsigned int const n_nodes = these_nodes->count;
	
	memset(
		cpu_buffer, 0, n_nodes * sizeof(gpu_node_representation)
	);
	
	uint32_t cpu_offset = 0;
	unsigned int quads_count = 0;
	
	for (unsigned int n = 0; n < n_nodes; n++) {

		node_container * __restrict const container =
			containers->data+n;
		struct generated_quads container_quads = 
			node_container_generate(container, casted_cpu_buffer+n);
		
		container->buffer_offset = cpu_offset;

		LOG(
			"[node_generate_containers]\n  quads : %u, size : %u\n",
			container_quads.count, container_quads.size
		);
		quads_count += container_quads.count;
		cpu_offset  += container_quads.size;
	}
	
	return generated_quads_uS_struct(quads_count, cpu_offset);
}

inline static void nodes_set_buffers
(struct nodes_display_data * __restrict const nodes,
 GLuint const containers_buffer_size,
 GLuint const contents_buffer_size)
{
	gpu_dumb_3buffs_init(
		&nodes->containers.buffer, containers_buffer_size, GL_DYNAMIC_DRAW
	);
	gpu_dumb_3buffs_init(
		&nodes->contents.buffer, contents_buffer_size, GL_DYNAMIC_DRAW
	);
	
}

void nodes_init
(nodes * __restrict const these_nodes,
 GLuint const containers_buffer_size,
 GLuint const contents_buffer_size,
 struct myy_common_data * display_data)
{
	nodes_storage_create(these_nodes, 1);
	nodes_set_buffers(
		these_nodes, containers_buffer_size, contents_buffer_size
	);
	these_nodes->common_display_elements = display_data;
}



static struct generated_quads nodes_generate_containers_and_store_in_gpu
(struct nodes_display_data * __restrict const nodes,
 buffer_t cpu_buffer)
{

	struct generated_quads generated_quads = 
		nodes_generate_containers(nodes, cpu_buffer);

	LOG(
		"[nodes_generate_containers_and_store_in_gpu]\n"
		"  quads : %d, size : %d\n",
		 generated_quads.count, generated_quads.size
	);
	gpu_dumb_3buffs_bind_next_buffer(&nodes->containers.buffer);
	gpu_dumb_3buffs_store(
		&nodes->containers.buffer, generated_quads.size, cpu_buffer
	);
	
	nodes->containers.quads = generated_quads.count;
	
	return generated_quads;
}

/* TODO : Il est impossible de mettre à jour un composant uniquement
 *        avec un système de multibuffering sans journal de
 *        modifications.
 *        Il faut, soit :
 *        - Changer de système de multibuffering juste pour pour les
 *          conteneurs, avec journal de modifications et garantie de
 *          synchronisation des différents tampons mémoires.
 *        - Basculer sur une architecture élément sélectionné / reste
 *          non sélectionné, avec mise à jour complète de l'élément
 *          sélectionné lors d'une modification sur la sélection, et
 *          mise à jour complète des éléments non sélectionnés lors de
 *          la désélection.
 */
static void node_container_regenerate_gpu_quads
(nodes * __restrict const nodes, node_id const node_id,
 buffer_t cpu_buffer)
{
	nodes_generate_containers_and_store_in_gpu(nodes, cpu_buffer);
	/*
	node_container const * __restrict const container =
		nodes_get_container(nodes, node_id);
	
	gpu_node_representation * __restrict const casted_buffer =
		(gpu_node_representation *) cpu_buffer;
	
	struct generated_quads container_quads = node_container_generate(
		container, casted_buffer
	);
	gpu_buffer containers_gpu_buffer = nodes->containers.buffer;
	gpu_buffer_bind(containers_gpu_buffer);
	LOG(
		"[node_container_regenerate_gpu_quads]\n"
		"Offset : %u\n"
		"Position x, y : %d, %d\n"
		"Size : %u\n",
		container->buffer_offset, container->pos.x, container->pos.y,
		container_quads.size
	);
	gpu_buffer_store_at(
		container_quads.size, container->buffer_offset, casted_buffer
	);*/
}

void nodes_draw
(struct nodes_display_data * __restrict const nodes,
 struct glsl_programs_shared_data const * __restrict const glsl_data,
 int16_t const global_offset_x, int16_t const global_offset_y)
{
	unsigned int const n_nodes = nodes->count;
	unsigned int const margin = 6;
	int const added_y_px = margin;
	int const added_x_px = margin;
	
	// Containers -- 
	//   Background should be drawn last to avoid overdraw
	//   however, dealing with transparent text is kind of
	//   pain when doing this...
	glUseProgram(glsl_data->programs[glsl_program_color_node]);

	glhUnif4f(
		color_node_shader_unif_px_offset,
		global_offset_x, global_offset_y,
		0,0, glsl_data
	);
	glhUnif1f(node_shader_unif_layer, 0.5f, glsl_data);

	SuB_2t_colored_quad_draw_pixel_coords(
		gpu_dumb_3buffs_current_buffer_id(&nodes->containers.buffer),
		color_node_shader_attr_xy,
		color_node_shader_attr_in_rgba,
		0,
		nodes->containers.quads
	);
	
	// Nodes contents
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(glsl_data->programs[glsl_program_node]);

	glhUnif1i(node_shader_unif_sampler, glsl_texture_fonts, glsl_data);
	glhUnif1f(node_shader_unif_layer, 0.4f, glsl_data);
	GLuint const nodes_contents_gpu_buffer_id =
		gpu_dumb_3buffs_current_buffer_id(&nodes->contents.buffer);
	for (unsigned int n = 0; n < n_nodes; n++) {
		node_content current_contents = 
			nodes_get_content_copy_at(nodes, n);
		
		glhUnif4f(
			node_shader_unif_px_offset,
			global_offset_x, global_offset_y,
			current_contents.pos.x+added_x_px,
			current_contents.pos.y+added_y_px,
			glsl_data
		);
		
		draw_character_quads(
			nodes_contents_gpu_buffer_id,
			node_shader_attr_xyz,
			node_shader_attr_in_st,
			current_contents.buffer_offset,
			current_contents.quads
		);
		
	}
	glDisable(GL_BLEND);
	
}

void node_set_position
(nodes * __restrict const nodes,
 node_id id, position_S const abs_position, buffer_t buffer)
{
	position_S_copy_position(
		&nodes_get_container(nodes, id)->pos, abs_position
	);
	position_S_copy_position(
		&nodes_get_content(nodes, id)->pos, abs_position
	);

	node_container_regenerate_gpu_quads(nodes, id, buffer);
}

static void nodes_set_selected_node_position
(nodes * __restrict const nodes, position_S abs_position,
 buffer_t buffer)
{
	node_id selected_node_id = nodes_get_selected_id(nodes);
	/* All this circus so that, when grabing the title bar, the cursor
	   stays at the same relative position from the title bar, instead of
	   having the upper-left corner of the title bar jump to the cursor
	   position.
	*/
	position_S grabbed_at = nodes->selection.grabbed_at;
	position_S grab_rel_position = position_S_relative_to_window_coords(
		abs_position, grabbed_at
	);
	node_set_position(
		nodes, selected_node_id, grab_rel_position, buffer
	);
}

static node_id nodes_id_at_position
(nodes * __restrict const nodes, position_S absolute_position)
{
	unsigned int n_nodes = nodes->count;
	node_id id = -1;
	
	int const x = absolute_position.x;
	int const y = absolute_position.y;
	for (unsigned int n = 0; n < n_nodes; n++) {
		node_container const * __restrict const container = 
			nodes_get_container_at(nodes, n);
		
		dimensions_uS dimensions =
			node_container_decorated_dimensions(container);
		
		int const 
			node_left   = container->pos.x,
			node_right  = container->pos.x + dimensions.width,
			node_top    = container->pos.y - TITLE_HEIGHT,
			node_bottom = container->pos.y + dimensions.height;
		
		if (
			x > node_left && x < node_right &&
			y > node_top && y < node_bottom
		)
		{ id = n; break; }
	}
	
	return id;
}

node_id nodes_try_to_select_node_at
(nodes * __restrict const nodes, position_S const absolute_position)
{
	int id = nodes_id_at_position(nodes, absolute_position);
	nodes->selection.id = id;
	return id;
}

// Check usage !
void nodes_set_handler_func
(nodes * __restrict const these_nodes,
 unsigned int const func_id,
 void (*func_address)(NODES_ON_CLICK_SIG))
{
	these_nodes->click_handlers[func_id] = func_address;
}

inline static uint8_t nodes_clicked_on_title
(position_S pos_rel_to_container_upper_left)
{
	return pos_rel_to_container_upper_left.y < 0;
}

uint8_t nodes_try_to_handle_click
(nodes * __restrict const nodes, position_S const abs_click_position)
{
	LOG(
		"[nodes_handle_click]\n  %d, %d\n",
		 abs_click_position.x, abs_click_position.y
	);
	int id = nodes_try_to_select_node_at(nodes, abs_click_position);
	if (id > -1) {
		node_container current_node = nodes_get_container_copy(nodes, id);

		position_S rel_position =
			position_S_relative_to_window_coords(
				abs_click_position, current_node.pos
			);

		LOG("rel_x : %d, rel_y : %d\n", rel_position.x, rel_position.y);
		if (nodes_clicked_on_title(rel_position)) {
			nodes->selection.movable = 1;
			nodes->selection.grabbed_at.x = rel_position.x;
			nodes->selection.grabbed_at.y = rel_position.y;
		}
		else {
			nodes->selection.movable = 0;
			nodes->click_handlers[current_node.handler_func_id](
				nodes, id, rel_position
			);
		}
		
	}
	else { 
		nodes->selection.movable = 0;
		//nodes_close_associated_menu(nodes);
	}
	return id;
}

// TODO : Remove the last 'buffer' argument !
//        It's only used to regenerate the containers on change
uint8_t nodes_handle_move
(nodes * __restrict const nodes,
 position_S move_abs_position, buffer_t buffer)
{
	uint8_t moving_a_node = 
		nodes_selected(nodes) && nodes_is_selection_movable(nodes);
	if (moving_a_node)
		nodes_set_selected_node_position(nodes, move_abs_position, buffer);
	return moving_a_node; // move consumed
}



struct status_and_amount nodes_add
(nodes * __restrict const these_nodes,
 void * __restrict * __restrict data, uint16_t amount,
 buffer_t buffer,
 struct quads_and_size
	(*title_gen_func)(NODE_QUADS_GENERATOR_FUNC_SIG),
 struct quads_and_size 
	(*content_gen_func)(NODE_QUADS_GENERATOR_FUNC_SIG)
)
{
	struct status_and_amount status_and_amount = {
		 .status = 0, .amount = 0
	};
	
	uint32_t new_total = these_nodes->count + amount;
	if (new_total > these_nodes->max
			&& !nodes_storage_reallocate(these_nodes, new_total))
		goto no_more_storage_space_for_nodes;
	
	uint16_t buffer_size = 0;
	GLuint global_offset = 0;
	generated_quads_uS total_quads = {0,0};
	
	for (unsigned int d = 0; d < amount; d++) {
		node_content * __restrict const current_content =
			nodes_get_content_at(these_nodes, d);
		node_container * __restrict const current_container =
			nodes_get_container_at(these_nodes, d);
		
		/* Set the current content gpu buffer start address of this content
		   points.
		   The offset is absolute.
		   Relative offsets are useless with OpenGL ES 2.x.
		   TODO : Remember to review this when creating an OpenGL ES 3.x
		          renderer.
		*/
		current_content->buffer_offset = global_offset + buffer_size;
		
		/* Generate the quads points to a CPU buffer */
		void * __restrict current_data = data[d];
		struct quads_and_size title = title_gen_func(
			these_nodes, current_data,
			these_nodes->common_display_elements->fonts_glyphs,
			buffer+buffer_size
		);
		
		buffer_size += title.quads.size;
		generated_quads_uS_add(&total_quads, title.quads);
		
		struct quads_and_size content = content_gen_func(
			these_nodes, current_data,
			these_nodes->common_display_elements->fonts_glyphs,
			buffer+buffer_size
		);
		
		buffer_size += content.quads.size;
		generated_quads_uS_add(&total_quads, content.quads);
		//unsigned int const content_width = content.size.x_offset;
		
		/* Store the number of quads generated.
		 * FIXME : That should be the number of points. We should not care
		 *         about the content formation */
		current_content->quads = title.quads.count + content.quads.count;
		
		// Set the width and the height of the container
		// The title dimensions are not stored. The title dimensions will
		// be computed when generating the container.
		
		current_container->dimensions.width  = content.size.x;
		current_container->dimensions.height = content.size.y;
	 }
	
	/* TODO : Pas de mise à jour précise avec des tampons simples
	 *        non synchronisés */
	/*
	GLuint gpu_end_offset = 
		these_nodes->contents.buffer.offset +
		these_nodes->contents.buffer.size.used;
	
	gpu_buffer_bind(these_nodes->contents.buffer);
	gpu_buffer_store_at(buffer_size, gpu_end_offset, buffer);*/
	
	these_nodes->count += amount;
	
	status_and_amount.amount = amount;
	status_and_amount.status = 1;
	
	gpu_dumb_3buffs_bind_next_buffer(&these_nodes->contents.buffer);
	gpu_dumb_3buffs_store(
		&these_nodes->contents.buffer, buffer_size, buffer
	);
	nodes_generate_containers_and_store_in_gpu(these_nodes, buffer);
	
no_more_storage_space_for_nodes:
	return status_and_amount;
}
