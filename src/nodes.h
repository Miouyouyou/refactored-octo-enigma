#ifndef MYY_SRC_NODES_H
#define MYY_SRC_NODES_H 1

#include <stdint.h>
#include <myy/helpers/struct.h>
#include <myy/current/opengl.h>
#include <myy/helpers/opengl/buffers.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/position.h>
#include <myy/helpers/dimensions.h>
#include <myy/helpers/buffers.h>

#include <myy.h>

#define CORNER_SIZE 10 // pixels
#define TITLE_HEIGHT 24

// Data types ----------------------------------------------------------

struct quads_and_size {
	struct generated_quads quads;
	struct text_offset size;
};


struct node_container_metadata {
	position_S pos;
	dimensions_uS dimensions;
	int32_t buffer_offset;
	int16_t handler_func_id;
};
typedef struct node_container_metadata node_container;

struct node_contents_metadata {
	position_S pos;
	GLuint buffer_offset, quads;
};
typedef struct node_contents_metadata node_content;

struct nodes_containers {
	gpu_dumb_3buffs_t buffer;
	uint16_t          quads;
	uint16_t          n_nodes;
	node_container  * data;
};
typedef struct nodes_containers nodes_containers;

struct myy_buffer_size { uint32_t used, max; };
struct nodes_contents {
	gpu_dumb_3buffs_t buffer;
	node_content    * data;
};
typedef struct nodes_contents nodes_contents;

typedef void * node_related_data;

typedef int16_t node_id;

struct nodes_display_data;
typedef struct nodes_display_data nodes;

#define NODES_ON_CLICK_SIG \
	nodes const * __restrict, \
	unsigned int, \
	position_S const

struct nodes_display_data {
	struct selection_infos { 
		node_id id; uint8_t movable;
		position_S grabbed_at;
	} selection;
	void                 ( * click_handlers[4])(NODES_ON_CLICK_SIG);
	struct myy_common_data * common_display_elements;
	struct dropdown_menus  * current_menus;
	uint16_t                 count;
	uint16_t                 max;
	nodes_containers         containers;
	nodes_contents           contents;
	node_related_data      * associated_data;
};

struct round_borders_rectangle {
	position_S rel_x, rel_y;
	dimensions_uS width, height;
};

enum rectangle_round_corners { 
	rect_round_corner_topleft, rect_round_corner_topright,
	rect_round_corner_bottomright, rect_round_corner_bottomleft,
	n_round_corners_in_rectangle
};
enum rectangle_lateral_borders {
	rect_left_border, rect_right_border, 
	n_lateral_borders_in_rectangle
};

struct gpu_round_corners_rect {
	SuB_2t_colored_quad title_bar, main_body;
};

typedef struct gpu_round_corners_rect gpu_node_representation;

// Procedures definitions ----------------------------------------------

// Inline procedures ------------

static inline uint8_t nodes_selected
(nodes const * __restrict const nodes)
{
	return nodes->selection.id > -1;
}
static inline uint8_t nodes_is_node_selected
(nodes const * __restrict const nodes,
 node_id const id)
{
	return nodes->selection.id == id;
}
static inline uint8_t nodes_is_selection_movable
(nodes const * __restrict const nodes)
{
	return nodes->selection.movable == 1;
}

static inline node_id nodes_get_selected_id
(nodes const * __restrict const nodes)
{
	return nodes->selection.id;
}

static inline node_container * nodes_get_container
(nodes const * __restrict const nodes, node_id const id)
{
	return nodes->containers.data+id;
}

static inline node_container * nodes_get_container_at
(nodes const * __restrict const nodes,
 unsigned int const index)
{
	return nodes->containers.data+index;
}

static inline node_container nodes_get_container_copy
(nodes const * __restrict const nodes, node_id const id)
{
	return nodes->containers.data[id];
}

static inline node_container nodes_get_container_copy_at
(nodes const * __restrict const nodes,
 unsigned int const index)
{
	return nodes->containers.data[index];
}

static inline node_content * nodes_get_content
(nodes const * __restrict const nodes,
 node_id const id)
{
	return nodes->contents.data+id;
}

static inline node_content * nodes_get_content_at
(nodes const * __restrict const nodes,
 node_id const index)
{
	return nodes->contents.data+index;
}

static inline node_content nodes_get_content_copy_at
(nodes const * __restrict const nodes,
 node_id const index)
{
	return nodes->contents.data[index];
}

// Standard procedures ------------

void nodes_init(
	nodes * __restrict const these_nodes,
	GLuint const containers_buffer_offset,
	GLuint const contents_buffer_offset,
	struct myy_common_data * display_data
);

#define NODE_QUADS_GENERATOR_FUNC_SIG \
 nodes const * __restrict, \
 void const * __restrict, \
 struct glyph_infos const * __restrict, \
 buffer_t
struct status_and_amount nodes_add
(nodes * __restrict const these_nodes,
 void * __restrict * __restrict data, uint16_t amount,
 buffer_t buffer,
 struct quads_and_size
	(*title_gen_func)(NODE_QUADS_GENERATOR_FUNC_SIG),
 struct quads_and_size 
	(*content_gen_func)(NODE_QUADS_GENERATOR_FUNC_SIG)
);

struct generated_quads nodes_generate_and_store_containers_in_gpu
(nodes * __restrict const nodes, buffer_t cpu_buffer);

void nodes_draw
(nodes * __restrict const nodes,
 GLuint const * __restrict const programs,
 int16_t const global_offset_x, int16_t const global_offset_y);

/*node_id nodes_id_at_position
(nodes * __restrict const nodes, position_S position);*/

struct status_and_amount { uint8_t status; uint16_t amount; };



void node_set_position
(nodes * __restrict const nodes, node_id const id,
 position_S const abs_position, buffer_t buffer);

node_id nodes_try_to_select_node_at
(nodes * __restrict const nodes, position_S position);

void nodes_set_handler_func
(nodes * __restrict const these_nodes, unsigned int const func_id,
 void (*func_address)(NODES_ON_CLICK_SIG));

uint8_t nodes_try_to_handle_click
(nodes * __restrict const nodes, position_S const abs_click_position);

uint8_t nodes_handle_move
(nodes * __restrict const nodes, position_S move_abs_position,
 buffer_t buffer);

unsigned int inline static nodes_last_index
(nodes const * __restrict const nodes)
{
	return nodes->count - 1;
}

#endif
