#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>

#include <myy/helpers/matrices.h>
#include <myy/helpers/struct.h>
#include <myy/helpers/log.h>
#include <myy.h>

#include <lib/Assembler/armv7-arm.h>
#include <lib/Assembler/helpers/memory.h>
#include <assert.h>

#include <src/nodes.h>
#include <src/nodes_asm.h>
#include <src/menus.h>

#include <stddef.h> // offsetof



// ----- Variables ----------------------------------------
struct nodes_display_data nodes_display_data = {0};

struct glsl_programs_shared_data glsl_shared_data = {0};

#define MANAGED_CODEPOINTS 512
struct myy_packed_fonts_codepoints codepoints[MANAGED_CODEPOINTS] = {0};
struct myy_packed_fonts_glyphdata   glyphdata[MANAGED_CODEPOINTS] = {0};
GLuint static_menu_parts_buffer[1];
GLuint context_menu_text_buffer[2];
struct glyph_infos myy_glyph_infos = {
	.stored_codepoints = 0,
	.codepoints_addr = codepoints,
	.glyphdata_addr  = glyphdata
};

/*------- Assembler ---*/

GLuint test_frame_gpu_buffer[1] = {0};

// Background lines

struct myy_gl_segment { float startx, starty, endx, endy; };

struct myy_gl_segment segments[80] = {0};
GLuint segment_buffer;
GLuint screen_width = 0, screen_height = 0;

uint8_t scratch_buffer[0x100000] __attribute__((aligned(32)));

struct dropdown_menus ga_menus = {0};
// ----- Code ---------------------------------------------

static void add_inst
(struct armv7_text_frame * __restrict const frame,
 enum known_instructions mnemonic_id,
 enum argument_type arg_type1,
 int32_t arg1,
 enum argument_type arg_type2,
 int32_t arg2,
 enum argument_type arg_type3,
 int32_t arg3)
{
	struct armv7_add_instruction_status status =
		frame_add_instruction(frame);
	assert(status.added);
	struct instruction_representation * const inst = status.address;
	
	instruction_mnemonic_id(inst, mnemonic_id);
	instruction_arg(inst, 0, arg_type1, arg1);
	instruction_arg(inst, 1, arg_type2, arg2);
	instruction_arg(inst, 2, arg_type3, arg3);
}

static uint32_t frame_id = 0;
static uint32_t id_gen() {
	uint32_t returned_id = frame_id;
	frame_id++;
	return returned_id;
}

struct armv7_text_frame * __restrict test_frames[2];
static struct armv7_text_frame ** prepare_test_frame_insts() {
	struct armv7_text_frame * __restrict const test_frame =
		generate_armv7_text_frame(id_gen);
	
	uint32_t const hamster_id = 9;
	test_frame->metadata.name = "Meow";
	add_inst(
		test_frame,
		inst_mov_immediate, arg_register, r0, arg_immediate, 0, 0, 0
	);
	
	add_inst(
		test_frame,
		inst_mov_immediate,
		arg_register, r1,
		arg_data_symbol_address_bottom16, hamster_id,
		0, 0
	);
	
	add_inst(
		test_frame,
		inst_movt_immediate,
		arg_register, r1,
		arg_data_symbol_address_top16, hamster_id,
		0, 0
	);
	
	add_inst(
		test_frame,
		inst_mov_immediate,
		arg_register, r2,
		arg_data_symbol_size, hamster_id,
		0, 0
	);
	
	add_inst(
		test_frame,
		inst_mov_immediate,
		arg_register, r7,
		arg_immediate, 4,
		0, 0
	);
	
	add_inst(
		test_frame,
		inst_svc_immediate,
		arg_immediate, 0,
		0, 0,
		0, 0
	);
		
	add_inst(
		test_frame,
		inst_bx_register,
		arg_condition, cond_al,
		arg_register, reg_lr,
		0, 0
	);
	
	test_frames[0] = test_frame;
	
	struct armv7_text_frame * __restrict const exit_frame =
		generate_armv7_text_frame(id_gen);
	exit_frame->metadata.name = "Exit";
	add_inst(
		exit_frame,
		inst_mov_immediate, arg_register, r0, arg_immediate, 0, 0, 0
	);
	
	add_inst(
		exit_frame,
		inst_mov_immediate,
		arg_register, r7,
		arg_immediate, 1,
		0, 0
	);
	
	add_inst(
		exit_frame,
		inst_svc_immediate,
		arg_immediate, 0,
		0, 0,
		0, 0
	);
	
	test_frames[1] = exit_frame;
	
	
	return test_frames;
}


void myy_init() {}

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

void myy_display_initialised
(unsigned int width, unsigned int height)
{
	LOG(
		"[myy_display_initialised]\n  width : %u, height : %u\n",
		width, height
	);
	glViewport(0, 0, width, height);
	generer_segment_chaque_n_pixels(segments, 50, width, height);
	screen_width = width / 2;
	screen_height = height / 2;
	glGenBuffers(1, &segment_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, segment_buffer);
	glBufferData(
		GL_ARRAY_BUFFER, 80 * sizeof(struct myy_gl_segment),
		segments, GL_STATIC_DRAW
	);
	glUseProgram(glsl_shared_data.programs[glsl_program_node]);
	union myy_4x4_matrix px_to_norm;
	myy_matrix_4x4_ortho_layered_window_coords(
		&px_to_norm, width, height, 1024
	);
	glUniformMatrix4fv(
		node_shader_unif_px_to_norm, 1, GL_FALSE, px_to_norm.raw_data
	);
	glUseProgram(glsl_shared_data.programs[glsl_program_fixed_widgets]);
	glUniformMatrix4fv(
		node_shader_unif_px_to_norm, 1, GL_FALSE, px_to_norm.raw_data
	);
	glUseProgram(glsl_shared_data.programs[glsl_program_color_node]);
	glUniformMatrix4fv(
		color_node_shader_unif_projection, 1, GL_FALSE, px_to_norm.raw_data
	);
}

void myy_generate_new_state() {}



struct swap_menu_infos swap_menu_infos = {0};

#define SWAP_MENUS_BUFFER_OFFSET 0x80000

void myy_init_drawing() {
	
	glhShadersPackLoader(&glsl_shared_data);
	
	glhBuildAndSaveSimpleProgram(
	  &glsl_shared_data,
	  lines_vsh, lines_fsh,
	  glsl_program_lines
	);
	glhBuildAndSaveSimpleProgram(
	  &glsl_shared_data,
	  node_vsh, node_fsh,
	  glsl_program_node
	);
	glhBuildAndSaveSimpleProgram(
		&glsl_shared_data,
		fixed_widgets_vsh, fixed_widgets_fsh,
		glsl_program_fixed_widgets
	);
	glhBuildAndSaveSimpleProgram(
		&glsl_shared_data,
		color_node_vsh, color_node_fsh,
		glsl_program_color_node
	);

	GLuint textures_id[n_textures_id];
	glhUploadMyyRawTextures(
	  "textures/fonts.raw\0"
	  "textures/cursor.raw\0"
		"textures/menus.raw",
	  n_textures_id,
	  textures_id
	);
	glhActiveTextures(textures_id, n_textures_id);

	myy_parse_packed_fonts(&myy_glyph_infos, "data/codepoints.dat");

	// 0x2000 and 0x4000 are arbitrary sizes (8 KB and 16 KB)
	// (8 KiB and 16 KiB using SI notation)
	glGenBuffers(1, static_menu_parts_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, static_menu_parts_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 0x2000, (uint8_t *) 0, GL_STATIC_DRAW);
	glGenBuffers(1, context_menu_text_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, context_menu_text_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 0xf0000, (uint8_t *) 0, GL_DYNAMIC_DRAW);

	set_menu_buffers_and_offsets(
		&ga_menus.gl_infos,
		context_menu_text_buffer[0],
		0x0000,
		static_menu_parts_buffer[0],
		0x0000
	);
	set_menu_buffers_and_offsets(
		&swap_menu_infos.gl_infos,
		context_menu_text_buffer[0],
		0x80000,
		static_menu_parts_buffer[0],
		0x0000
	);
	
	nodes_asm_setup_dropdowns_menus(&nodes_display_data, &ga_menus);
	regenerate_menus(&ga_menus, &myy_glyph_infos);
	
	/*set_swap_menu_title(
		&swap_menu_infos,
		"Hello", &myy_glyph_infos
	);

	set_swap_menu_listings(
		&swap_menu_infos,
		n_known_instructions, 0,
		mnemonics_full_desc, conditions_strings,
		&myy_glyph_infos
	);*/
	
	prepare_static_menu_parts(&ga_menus.gl_infos);

	glGenBuffers(1, test_frame_gpu_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, test_frame_gpu_buffer[0]);
	glBufferData(
		GL_ARRAY_BUFFER, 0xf0000, (uint8_t *) 0, GL_DYNAMIC_DRAW
	);
	
	prepare_test_frame_insts();
	set_nodes_buffers(
		&nodes_display_data,
		test_frame_gpu_buffer[0],
		0x0000,
		test_frame_gpu_buffer[0],
		0x4000
	);
	nodes_set_handler_func(&nodes_display_data, 0, node_frame_click);
	generate_frames_quads(
		test_frames, 2, &nodes_display_data, &myy_glyph_infos
	);
	generate_and_store_nodes_containers_in_gpu(
		&nodes_display_data, scratch_buffer
	);
	set_node_position(&nodes_display_data, 0,   0,   0, scratch_buffer);
	set_node_position(&nodes_display_data, 1, 400, 400, scratch_buffer);
	glEnable(GL_DEPTH_TEST);

	LOG("After nodes to GPU\n");
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

void set_test_frame_mnemonic(unsigned int const id) {
	/*instruction_mnemonic_id(
		test_frame->instructions+current_inst_id, id
	);
	generate_test_frame_quads(
		&myy_glyph_infos, test_frame, 24, test_frame_gpu_buffer[0], 0
	);*/
}

void myy_draw() {

	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(0.616f, 0.639f, 0.667f, 1.0f);

	GLuint * glsl_programs = glsl_shared_data.programs;
	struct norm_offset norm_offset = normalised_offset();

	// Context menu
	draw_current_context_menu(&ga_menus, glsl_programs);
	draw_swap_menu(&swap_menu_infos, glsl_programs);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/* Nodes */
	draw_nodes(&nodes_display_data, glsl_programs, offset[2], offset[3]);

	// Lines
	glUseProgram(glsl_programs[glsl_program_lines]);
	glUniform2f(lines_shader_unif_offset, norm_offset.x, norm_offset.y);
	glUniform3f(
		lines_shader_unif_color,
		0.900, 0.900, 0.900
	);
	glEnableVertexAttribArray(lines_shader_attr_xyz);
	glBindBuffer(GL_ARRAY_BUFFER, segment_buffer);
	glVertexAttribPointer(
		lines_shader_attr_xyz, 2, GL_FLOAT, GL_FALSE, 0, 0
	);
	glLineWidth(2);
	glDrawArrays(GL_LINES, 0, 320);
}

void myy_after_draw() {}

void myy_rel_mouse_move(int x, int y) {
}

void myy_mouse_action(enum mouse_action_type type, int value) {
}

void myy_save_state(struct myy_game_state * const state) {}

void myy_resume_state(struct myy_game_state * const state) {}

void myy_cleanup_drawing() {}

void myy_stop() {}

uint8_t moving_node = 0;
int16_t last_x, last_y;
void myy_click(int x, int y, unsigned int button) {
	last_x = x;
	last_y = y;
	//display_context_menu ^= 1;
	int offseted_x = x - offset[2];
	int offseted_y = y - offset[3];
	
	LOG("coords : %d, %d\noffsets : %d, %d\nresults : %d, %d\n",
	    x, y, offset[2], offset[3], offseted_x, offseted_y);
	
	if (!manage_current_menu_click(&ga_menus, x, y))
		nodes_handle_click(&nodes_display_data, offseted_x, offseted_y);

}

void myy_doubleclick(int x, int y, unsigned int button) {}

void myy_move(int x, int y, int start_x, int start_y) {
	unsigned int has_moved_node = nodes_handle_move(
		&nodes_display_data, x-offset[2], y-offset[3], scratch_buffer
	);
	if (!has_moved_node) {
		int32_t delta_x = x - last_x;
		int32_t delta_y = y - last_y;
		int32_t new_offset_x = (offset[0] + delta_x) % 50;
		int32_t new_offset_y = (offset[1] - delta_y) % 50;
		offset[0] = new_offset_x;
		offset[1] = new_offset_y;
		offset[2] += delta_x;
		offset[3] += delta_y;
		last_x = x;
		last_y = y;
	}
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
	switch (keycode) {
		case MYY_KP_1:
			//show_registers_context_menu();
			set_current_dropdown_menu(&ga_menus, ga_dropdown_menu_registers);
			enable_context_menu();
		break;
		case MYY_KP_2:
			set_current_dropdown_menu(&ga_menus, ga_dropdown_menu_conditions);
			enable_context_menu();
		break;
		case MYY_KP_3:
			enable_swap_menu();
		break;
		case MYY_KP_4:
			disable_swap_menu();
		break;
		/*case MYY_KP_5:
			set_swap_menu_listings(
				&swap_menu_infos,
				16, 14,
				register_names, conditions_strings,
				&myy_glyph_infos
			);
		break;
		case MYY_KP_6:
			set_swap_menu_listings(
				&swap_menu_infos,
				14, n_known_instructions,
				conditions_strings, mnemonics_full_desc,
				&myy_glyph_infos
			);*/
		default: break;
	}
}
void myy_key_release(unsigned int keycode) {
}


