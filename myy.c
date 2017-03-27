#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>

#include <myy/helpers/matrices.h>
#include <myy/helpers/struct.h>
#include <myy/helpers/log.h>
#include <myy.h>

#include <stddef.h> // offsetof

// ----- Variables ----------------------------------------


struct glsl_programs_shared_data glsl_shared_data = {
	.programs = {0},
	.strings = {
	  "shaders/standard.vert\0shaders/standard.frag\0"
	  "shaders/node.vert\0shaders/node.frag\0"
	},
	.shaders = {
	  [standard_vsh] = {
	    .type = GL_VERTEX_SHADER,
	    .str_pos = 0,
	  },
	  [standard_fsh] = {
	    .type = GL_FRAGMENT_SHADER,
	    .str_pos = 22,
	  },
	  [node_vsh] = {
	    .type = GL_VERTEX_SHADER,
	    .str_pos = 44,
	  },
	  [node_fsh] = {
	    .type = GL_FRAGMENT_SHADER,
	    .str_pos = 62,
	  }
	}
};

#define MANAGED_CODEPOINTS 512
struct myy_packed_fonts_codepoints codepoints[MANAGED_CODEPOINTS] = {0};
struct myy_packed_fonts_glyphdata   glyphdata[MANAGED_CODEPOINTS] = {0};

struct glyph_infos myy_glyph_infos = {
	.stored_codepoints = 0,
	.codepoints_addr = codepoints,
	.glyphdata_addr  = glyphdata
};

int32_t string[]        = L"mov r0, #0";
int32_t second_string[] = L"add r0, #14";
uint32_t string_size = sizeof(string)/sizeof(uint32_t);
uint32_t second_string_size = sizeof(string)/sizeof(uint32_t);
US_two_tris_quad_3D quads[30];


// ----- Code ---------------------------------------------

static void prepare_string
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 US_two_tris_quad_3D * __restrict const quads) {
	myy_codepoints_to_glyph_twotris_quads_window_coords(
	  glyph_infos, string, n_characters, quads
	);
}

void myy_init() {}

struct myy_gl_segment {
	float startx, starty, endx, endy;
};

static void generer_segment_chaque_n_pixels
(struct myy_gl_segment * const segments, unsigned int n_pixels,
 unsigned int screen_width, unsigned int screen_height)
{
	unsigned int const
	  n_lines_per_height = screen_height / n_pixels + 2,
	  n_lines_per_width  = screen_width  / n_pixels + 2;
	GLfloat
	  half_width = (screen_width >> 1),
	  half_height = (screen_height >> 1);

	unsigned int l = 0;
	/* [2.0,-2.0] ranges are useful when offseting the lines */
	for (unsigned int i = 0; i < n_lines_per_width; i++, l++) {
		GLfloat x_pos = (i * n_pixels)/half_width - 1;
		segments[l].startx = x_pos;
		segments[l].starty = 2.0;
		segments[l].endx   = x_pos;
		segments[l].endy   = -2.0;
	}
	for (unsigned int i = 0; i < n_lines_per_height; i++, l++) {
		GLfloat y_pos = (i * n_pixels)/half_height - 1;
		segments[l].startx = -2.0;
		segments[l].starty = y_pos;
		segments[l].endx   = 2.0;
		segments[l].endy   = y_pos;
	}
}

struct myy_gl_segment segments[80] = {0};
GLuint segment_buffer;
GLuint screen_width = 0, screen_height = 0;
void myy_display_initialised(unsigned int width, unsigned int height) {
	generer_segment_chaque_n_pixels(segments, 50, width, height);
	screen_width = width / 2;
	screen_height = height / 2;
	glGenBuffers(1, &segment_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, segment_buffer);
	glBufferData(GL_ARRAY_BUFFER, 80 * sizeof(struct myy_gl_segment),
	             segments, GL_STATIC_DRAW);
	glUseProgram(glsl_shared_data.programs[glsl_node_program]);
	union myy_4x4_matrix px_to_norm;
	myy_matrix_4x4_ortho_layered_window_coords(
		&px_to_norm, width, height, 1024
	);
	glUniformMatrix4fv(
		node_shader_unif_px_to_norm, 1, GL_FALSE, px_to_norm.raw_data
	);
}

void myy_generate_new_state() {}

#define CORNER_SIZE 10 // pixels
struct box_metadata {
	int32_t x, y;
	int16_t width, height;
} box_minimal_size = {
	.width  = CORNER_SIZE * 2 + 1, // pixels
	.height = CORNER_SIZE * 2 + 1 // pixels
};

struct box_metadata boxes[1] = {
	[0] = {
		.x = 1280/2-100,
		.y = 720/2-50,
		.width  = 200,
		.height = 100
	}
};

struct gpu_box_part { uint16_t rel_x, rel_y, width, height; };

struct UIS_2D_point {
	int32_t x, y;
	uint16_t s, t;
} __PALIGN__;
struct UIS_2D_triangle {
	struct UIS_2D_point a, b, c;
} __PALIGN__;
union UIS_2D_two_tris_quad {
	int32_t xyST[18];
	struct UIS_2D_point points[6];
	struct UIS_2D_triangle triangles[2];
} __PALIGN__;

static void gpu_box_part
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

enum gpu_box_corners { 
	gpu_box_corner_topleft, gpu_box_corner_topright,
	gpu_box_corner_bottomright, gpu_box_corner_bottomleft,
	gpu_box_n_corners
};
enum gpu_box_borders {
	gpu_box_left_border, gpu_box_right_border, gpu_box_n_borders
};
struct gpu_box_metadata {
	union UIS_2D_two_tris_quad
		main_body, lateral_borders[gpu_box_n_borders];
	union UIS_2D_two_tris_quad corners[gpu_box_n_corners];
};

static void gpu_box_corner
(int32_t x, int32_t y, int32_t corner_size,
 union UIS_2D_two_tris_quad * __restrict const corner) {
	gpu_box_part(x, y, corner_size, corner_size, corner);
}
void boxes_to_gpu_boxes
(struct box_metadata const * __restrict const boxes,
 unsigned int const n_boxes,
 struct gpu_box_metadata * __restrict const output)
{
	unsigned int corner_size = CORNER_SIZE;
	
	memset(output, 0, n_boxes * sizeof(struct gpu_box_metadata));
	for (unsigned int i = 0; i < n_boxes; i++) {
		int 
			x = boxes[i].x,
			y = boxes[i].y,
			main_body_height = boxes[i].height,
			main_body_width = boxes[i].width - corner_size - corner_size,
			lateral_height  = boxes[i].height - corner_size - corner_size;
		
		struct gpu_box_metadata * __restrict const current_gpu_box = 
			output+i;
		LOG("x : %d, y : %d\n", x, y);
		gpu_box_part(
			x+corner_size, y,
			main_body_width, main_body_height, 
			&current_gpu_box->main_body
		);
		gpu_box_part(
			x, y+corner_size,
			corner_size, lateral_height,
			current_gpu_box->lateral_borders+gpu_box_left_border
		);
		gpu_box_part(
			x+corner_size+main_body_width, y+corner_size, 
			corner_size, lateral_height,
			current_gpu_box->lateral_borders+gpu_box_right_border
		);
		gpu_box_corner(
			x, y, corner_size,
			current_gpu_box->corners+gpu_box_corner_topleft
		);
		gpu_box_corner(
			x+corner_size+main_body_width, y, corner_size, 
			current_gpu_box->corners+gpu_box_corner_topright
		);
		gpu_box_corner(
			x+corner_size+main_body_width, y+corner_size+lateral_height,
			corner_size, 
			current_gpu_box->corners+gpu_box_corner_bottomright
		);
		gpu_box_corner(
			x, y+corner_size+lateral_height, corner_size,
			current_gpu_box->corners+gpu_box_corner_bottomleft
		);

	}
}
struct gpu_box_metadata gpu_boxes[2];
void myy_init_drawing() {
	
	glhBuildAndSaveSimpleProgram(
	  &glsl_shared_data,
	  node_vsh, node_fsh,
	  glsl_node_program
	);
	glhBuildAndSaveSimpleProgram(
	  &glsl_shared_data,
	  standard_vsh, standard_fsh,
	  glsl_standard_program
	);

	GLuint textures_id[n_textures_id];
	glhUploadMyyRawTextures(
	  "textures/fonts.raw\0"
	  "textures/cursor.raw",
	  n_textures_id,
	  textures_id
	);
	glhActiveTextures(textures_id, 2);

	struct box_metadata box[2] = {
		[0] = {
			.x = 1280/2 - 100,
			.y = 720/2 - 50,
			.width = 400,
			.height = 100
		},
		[1] = {
			.x = -100,
			.y = -100,
			.width = 300,
			.height = 700
		}
	};
	
	boxes_to_gpu_boxes(box, 2, gpu_boxes);
	myy_parse_packed_fonts(&myy_glyph_infos, "data/codepoints.dat");
	prepare_string(&myy_glyph_infos, string, string_size, quads);
	prepare_string(
		&myy_glyph_infos, second_string, second_string_size,
		quads+string_size
	);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	LOG("After boboxes\n");
}

int offset[4] = {0};
struct norm_offset { GLfloat x, y; };
struct norm_offset normalised_offset() {
	struct norm_offset norm_offset = {
		.x = (float) offset[0] / screen_width,
		.y = (float) offset[1] / screen_height
	};
	return norm_offset;
};

void myy_draw() {

	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(0.1f, 0.3f, 0.5f, 1.0f);

	GLuint * glsl_programs = glsl_shared_data.programs;
	struct norm_offset norm_offset = normalised_offset();
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/* Nodes */
	glUseProgram(glsl_programs[glsl_node_program]);
	glUniform1f(node_shader_unif_layer, 0.4f);
	glUniform4f(node_shader_unif_px_offset, offset[2], offset[3],0,0);
	glEnableVertexAttribArray(node_shader_attr_xyz);
	glEnableVertexAttribArray(node_shader_attr_st);
	glVertexAttribPointer(
		node_shader_attr_xyz, 2, GL_INT, GL_FALSE,
		sizeof(struct UIS_2D_point),
		(uint8_t *) gpu_boxes+offsetof(struct UIS_2D_point, x)
	);
	glVertexAttribPointer(
		node_shader_attr_st, 2, GL_UNSIGNED_SHORT, GL_TRUE,
		sizeof(struct UIS_2D_point),
		(uint8_t *) gpu_boxes+offsetof(struct UIS_2D_point, s)
	);
	glDrawArrays(GL_TRIANGLES, 0, 42*2);

	/* Text */
	glUniform1i(node_shader_unif_sampler, 0);
	glUniform1f(node_shader_unif_layer, 0.3f);
	glUniform4f(node_shader_unif_px_offset, offset[2], offset[3],-80,-80);
	glVertexAttribPointer(
		node_shader_attr_xyz, 3, GL_SHORT, GL_FALSE,
		sizeof(struct US_textured_point_3D),
		(uint8_t *) quads+offsetof(struct US_textured_point_3D, x)
	);
	glVertexAttribPointer(
		node_shader_attr_st, 2, GL_UNSIGNED_SHORT, GL_TRUE,
		sizeof(struct US_textured_point_3D),
		(uint8_t *) (quads)+offsetof(struct US_textured_point_3D, s)
	);
	glDrawArrays(GL_TRIANGLES, 0, 6*string_size);
	
	glUniform4f(node_shader_unif_px_offset, offset[2], offset[3],-80,-60);
	glDrawArrays(GL_TRIANGLES, 6*string_size, 6*second_string_size);
	
	glUseProgram(glsl_programs[glsl_standard_program]);
	glEnableVertexAttribArray(attr_xyz);
	glUniform2f(unif_offset, norm_offset.x, norm_offset.y);
	glBindBuffer(GL_ARRAY_BUFFER, segment_buffer);
	glVertexAttribPointer(attr_xyz, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glLineWidth(2);
	glDrawArrays(GL_LINES, 0, 320);
}

void myy_rel_mouse_move(int x, int y) {
}

void myy_mouse_action(enum mouse_action_type type, int value) {
}

void myy_save_state(struct myy_game_state * const state) {}

void myy_resume_state(struct myy_game_state * const state) {}

void myy_cleanup_drawing() {}

void myy_stop() {}

int16_t last_x, last_y;
void myy_click(int x, int y, unsigned int button) {
	last_x = x;
	last_y = y;
}
void myy_doubleclick(int x, int y, unsigned int button) {}
void myy_move(int x, int y, int start_x, int start_y) {
	int32_t delta_x = x - last_x;
	int32_t delta_y = y - last_y;
	int32_t new_offset_x = (offset[0] + delta_x) % 50;
	int32_t new_offset_y = (offset[1] + delta_y) % 50;
  offset[0] = new_offset_x;
	offset[1] = new_offset_y;
	offset[2] += delta_x;
	offset[3] -= delta_y;
	last_x = x;
	last_y = y;
}
void myy_hover(int x, int y) {
}

#define MYY_KP_7 79
#define MYY_KP_8 80
#define MYY_KP_9 81
#define MYY_KP_MINUS 82
#define MYY_KP_4 83
#define MYY_KP_5 84
#define MYY_KP_6 85
#define MYY_KP_PLUS 86
#define MYY_KP_1 87
#define MYY_KP_2 88
#define MYY_KP_3 89

void myy_key(unsigned int keycode) {
}
