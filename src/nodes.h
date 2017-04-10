#ifndef MYY_SRC_NODES_H
#define MYY_SRC_NODES_H 1

#include <stdint.h>
#include <myy/helpers/struct.h>

#define CORNER_SIZE 10 // pixels

// Data types ----------------------------------------------------------

struct graphical_node_metadata {
	int32_t x, y;
	int16_t width, height;
};

struct UIS_2D_point {
	int32_t x, y;
	uint16_t s, t;
} __PALIGN__;
struct UIS_2D_triangle {
	struct UIS_2D_point a, b, c;
} __PALIGN__;
union UIS_2D_two_tris_quad {
	int32_t xyST[18]; // x : int32, y : int32, s : uint16, t : uint16
	struct UIS_2D_point points[6];
	struct UIS_2D_triangle triangles[2];
} __PALIGN__;

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
	union UIS_2D_two_tris_quad
		main_body, lateral_borders[n_lateral_borders_in_rectangle];
	union UIS_2D_two_tris_quad corners[n_round_corners_in_rectangle];
};

typedef struct gpu_round_corners_rect gpu_node_representation;

// Procedures definitions ----------------------------------------------

void generate_nodes_containers
(struct graphical_node_metadata const * __restrict const nodes,
 unsigned int const n_nodes,
 gpu_node_representation * __restrict const output);

#endif
