#ifndef MYY_SRC_NODES_H
#define MYY_SRC_NODES_H 1

#include <stdint.h>
#include <myy/helpers/struct.h>
#include <myy/current/opengl.h>
#include <myy/helpers/opengl/quads_structures.h>

#define CORNER_SIZE 10 // pixels
#define TITLE_HEIGHT 24

// Data types ----------------------------------------------------------

struct node_container_metadata {
	int16_t x, y, width, height; int32_t buffer_offset;
	int16_t handler_func_id;
};

struct node_contents_metadata {
	int16_t x, y; GLuint buffer_offset, quads;
};
struct nodes_containers {
	GLuint buffer_id, buffer_offset;
	uint16_t quads, n_nodes;
	struct node_container_metadata metadata[128];
};
struct nodes_contents {
	GLuint buffer_id, buffer_offset;
	struct node_contents_metadata metadata[128];
};
struct nodes_display_data {
	int16_t selected_node_id;
	int16_t selected_node_movable;
	// Relative to the upper-left corner
	int16_t selection_x_offset, selection_y_offset;
	void (*click_handlers[4])();
	struct dropdown_menus * current_menus;
	struct nodes_containers containers;
	struct nodes_contents contents;
	void * associated_data[256];

};
static unsigned int is_any_node_selected
(struct nodes_display_data const * __restrict const nodes)
{
	return nodes->selected_node_id > -1;
}
static unsigned int is_node_selected
(struct nodes_display_data const * __restrict const nodes,
 unsigned int const id)
{
	return nodes->selected_node_id == id;
}

struct round_borders_rectangle {
	uint16_t rel_x, rel_y, width, height;
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

struct generated_quads generate_and_store_nodes_containers_in_gpu
(struct nodes_display_data * __restrict const nodes,
 uint8_t * __restrict const cpu_buffer);

void draw_nodes
(struct nodes_display_data * __restrict const nodes,
 GLuint const * __restrict const programs,
 int16_t const global_offset_x, int16_t const global_offset_y);

void set_node_position
(struct nodes_display_data * __restrict const nodes,
 int const id, int const x, int const y,
 uint8_t * __restrict const buffer);

void nodes_set_selected_node_position
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y, uint8_t * __restrict const buffer);

void set_nodes_buffers
(struct nodes_display_data * __restrict const nodes,
 GLuint const containers_buffer_id,
 GLuint const containers_buffer_offset,
 GLuint const contents_buffer_id,
 GLuint const contents_buffer_offset);

int nodes_at
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y);

int nodes_selected_at
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y);

void nodes_set_handler_func
(struct nodes_display_data * __restrict const nodes,
 unsigned int func_id,
 void (*func_address)());

int nodes_handle_click
(struct nodes_display_data * __restrict const nodes,
 int const x, int const y);

int nodes_handle_move
(struct nodes_display_data * __restrict const nodes,
 int x, int y, uint8_t * __restrict const buffer);

#endif
