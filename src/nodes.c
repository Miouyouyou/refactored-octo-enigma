#include <src/nodes.h>

#include <stdint.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/log.h>

#include <strings.h>


static void gpu_node_part
(int32_t const x, int32_t const y,
 int32_t width, int32_t height,
 union UIS_2D_two_tris_quad * __restrict const two_tris_quad)
{
	two_tris_quad->points[upleft_corner].x            = x;
	two_tris_quad->points[upleft_corner].y            = y;
	two_tris_quad->points[upleft_corner].s            = 0;
	two_tris_quad->points[upleft_corner].t            = 0;

	two_tris_quad->points[downleft_corner].x          = x;
	two_tris_quad->points[downleft_corner].y          = y + height;
	two_tris_quad->points[downleft_corner].s          = 0;
	two_tris_quad->points[downleft_corner].t          = 0;

	two_tris_quad->points[upright_corner].x           = x + width;
	two_tris_quad->points[upright_corner].y           = y;
	two_tris_quad->points[upright_corner].s           = 0;
	two_tris_quad->points[upright_corner].t           = 0;

	two_tris_quad->points[downright_corner].x         = x + width;
	two_tris_quad->points[downright_corner].y         = y + height;
	two_tris_quad->points[downright_corner].s         = 0;
	two_tris_quad->points[downright_corner].t         = 0;

	two_tris_quad->points[repeated_upright_corner].x  = x + width;
	two_tris_quad->points[repeated_upright_corner].y  = y;
	two_tris_quad->points[repeated_upright_corner].s  = 0;
	two_tris_quad->points[repeated_upright_corner].t  = 0;

	two_tris_quad->points[repeated_downleft_corner].x = x;
	two_tris_quad->points[repeated_downleft_corner].y = y + height;
	two_tris_quad->points[repeated_downleft_corner].s = 0;
	two_tris_quad->points[repeated_downleft_corner].t = 0;
}


static void gpu_node_corner
(int32_t x, int32_t y, int32_t corner_size,
 union UIS_2D_two_tris_quad * __restrict const corner) {
	gpu_node_part(x, y, corner_size, corner_size, corner);
}


void generate_nodes_contents
(struct graphical_node_metadata const * __restrict const nodes,
 unsigned int n_nodes,
 gpu_node_representation * __restrict const output)
{
	
}

void generate_nodes_containers
(struct graphical_node_metadata const * __restrict const nodes,
 unsigned int const n_nodes,
 gpu_node_representation * __restrict const output)
{
	unsigned int corner_size = CORNER_SIZE;
	
	memset(output, 0, n_nodes * sizeof(gpu_node_representation));
	for (unsigned int i = 0; i < n_nodes; i++) {
		int 
			x = nodes[i].x,
			y = nodes[i].y,
			main_body_height = nodes[i].height,
			main_body_width  = nodes[i].width  - corner_size - corner_size,
			lateral_height   = nodes[i].height - corner_size - corner_size;
		
		gpu_node_representation * __restrict const current_gpu_node = 
			output+i;
		LOG("x : %d, y : %d\n", x, y);
		gpu_node_part(
			x+corner_size, y,
			main_body_width, main_body_height, 
			&current_gpu_node->main_body
		);
		gpu_node_part(
			x, y+corner_size,
			corner_size, lateral_height,
			current_gpu_node->lateral_borders+rect_left_border
		);
		gpu_node_part(
			x+corner_size+main_body_width, y+corner_size, 
			corner_size, lateral_height,
			current_gpu_node->lateral_borders+rect_right_border
		);
		gpu_node_corner(
			x, y, corner_size,
			current_gpu_node->corners+rect_round_corner_topleft
		);
		gpu_node_corner(
			x+corner_size+main_body_width, y, corner_size, 
			current_gpu_node->corners+rect_round_corner_topright
		);
		gpu_node_corner(
			x+corner_size+main_body_width, y+corner_size+lateral_height,
			corner_size, 
			current_gpu_node->corners+rect_round_corner_bottomright
		);
		gpu_node_corner(
			x, y+corner_size+lateral_height, corner_size,
			current_gpu_node->corners+rect_round_corner_bottomleft
		);

	}
}
