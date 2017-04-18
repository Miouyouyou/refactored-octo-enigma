#ifndef MYY_SRC_NODES_H
#define MYY_SRC_NODES_H 1

#include <stdint.h>
#include <myy/helpers/struct.h>
#include <myy/current/opengl.h>
#include <myy/helpers/opengl/quads_structures.h>

#define CORNER_SIZE 10 // pixels

// Data types ----------------------------------------------------------

struct node_container_metadata {
	uint16_t width, height; int32_t buffer_offset;
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
	GLuint buffer_id;
	struct node_contents_metadata metadata[128];
};
struct nodes_display_data {
	struct nodes_containers containers;
	struct nodes_contents contents;
};

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
 uint8_t * __restrict const cpu_buffer,
 GLuint const buffer_id, GLuint const buffer_offset);

void draw_nodes
(struct nodes_display_data * __restrict const nodes,
 GLuint const * __restrict const programs,
 int16_t const global_offset_x, int16_t const global_offset_y);

#endif
