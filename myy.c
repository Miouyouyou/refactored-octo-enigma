#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/arrays.h>
#include <myy/helpers/matrices.h>
#include <myy/helpers/memory.h>
#include <myy/helpers/struct.h>
#include <myy/helpers/log.h>
#include <myy.h>

#include <lib/Assembler/armv7-arm.h>

#include <assert.h>

#include <src/nodes.h>
#include <src/nodes_asm.h>
#include <src/menus.h>
#include <src/program_builder_menu.h>

#include <stddef.h> // offsetof

// ----- Variables ----------------------------------------
struct myy_common_data common_display_data = {0};
hitboxes_S_t hitboxes;
nodes nodes_display_data = {0};
struct glsl_programs_shared_data glsl_shared_data = {0};
struct myy_platform_handlers platform_handlers = {0};
struct myy_platform_handlers * myy_get_platform_handlers()
{
	return &platform_handlers;
}
void myy_user_quit() {
	platform_handlers.stop(platform_handlers.stop_data);
}

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

GLuint test_frame_gpu_buffer[2] = {0};

// Background lines

struct myy_gl_segment { float startx, starty, endx, endy; };

struct myy_gl_segment segments[80] = {0};
GLuint segment_buffer;
GLuint screen_width = 0, screen_height = 0;

uint8_t scratch_buffer[0x100000] __attribute__((aligned(32)));

struct menus menus = {0};
//struct dropdown_menus ga_menus = {0};
//struct swap_menu_infos swap_menu_infos = {0};
program_builder_menu_t program_builder_menu;
// ----- Code ---------------------------------------------

static void add_inst
(struct armv7_text_frame * __restrict const frame,
 enum known_instructions mnemonic_id,
 enum argument_type arg_type1, int32_t const arg1,
 enum argument_type arg_type2, int32_t const arg2,
 enum argument_type arg_type3, int32_t const arg3)
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

struct armv7_text_frames text_frames;
static void prepare_test_frame_insts
(struct armv7_text_frames * __restrict const assembly_frames)
{
	dyn_array_data_pointers_init(
		(DynPointers_u16_t *) assembly_frames, 2
	);
	dyn_array_data_pointers_append_u16(
		(DynPointers_u16_t *) assembly_frames,
		generate_armv7_text_frame(id_gen)
	);
	dyn_array_data_pointers_append_u16(
		(DynPointers_u16_t *) assembly_frames,
		generate_armv7_text_frame(id_gen)
	);
	dyn_array_data_pointers_append_u16(
		(DynPointers_u16_t *) assembly_frames,
		generate_armv7_text_frame(id_gen)
	);
	struct armv7_text_frame * __restrict const print_frame =
		assembly_frames->data[0];
	struct armv7_text_frame * __restrict const exit_frame =
		assembly_frames->data[1];
	struct armv7_text_frame * __restrict const start_frame =
		assembly_frames->data[2];
	print_frame->metadata.name = (uint8_t *) "Print";
	exit_frame->metadata.name  = (uint8_t *) "Exit";
	start_frame->metadata.name = (uint8_t *) "Meow";
	
	uint32_t meow_world_id = 1;
	// Start
	add_inst(
		start_frame,
		inst_bl_address,
		arg_condition, cond_al,
		arg_frame_address_pc_relative, print_frame->metadata.id,
		arg_invalid, 0
	);
	add_inst(
		start_frame,
		inst_b_address,
		arg_condition, cond_al,
		arg_frame_address_pc_relative, exit_frame->metadata.id,
		arg_invalid, 0
	);
	// Print
	add_inst(
		print_frame,
		inst_mov_immediate, arg_register, r0, arg_immediate, 0,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_mov_immediate,
		arg_register, r1,
		arg_data_symbol_address_bottom16, meow_world_id,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_movt_immediate,
		arg_register, r1,
		arg_data_symbol_address_top16, meow_world_id,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_mov_immediate,
		arg_register, r2,
		arg_data_symbol_size, meow_world_id,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_mov_immediate,
		arg_register, r7,
		arg_immediate, 4,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_svc_immediate,
		arg_immediate, 0,
		arg_invalid, 0,
		arg_invalid, 0
	);
		
	add_inst(
		print_frame,
		inst_bx_register,
		arg_condition, cond_al,
		arg_register, reg_lr,
		arg_invalid, 0
	);
	
	// Exit
	add_inst(
		exit_frame,
		inst_mov_immediate, 
		arg_register, r0, 
		arg_immediate, 0,
		arg_invalid, 0
	);
	
	add_inst(
		exit_frame,
		inst_mov_immediate,
		arg_register, r7,
		arg_immediate, 1,
		arg_invalid, 0
	);
	
	add_inst(
		exit_frame,
		inst_svc_immediate,
		arg_immediate, 0,
		arg_invalid, 0,
		arg_invalid, 0
	);
}


void myy_init() {}

static void generer_segment_chaque_n_pixels
(struct myy_gl_segment * const segments, unsigned int n_pixels,
 unsigned int screen_width, unsigned int screen_height)
{
	unsigned int const
	  n_lines_per_height = screen_height / n_pixels + 2,
	  n_lines_per_width  = screen_width  / n_pixels + 2;
	GLfloat const
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


void myy_display_initialised(unsigned int width, unsigned int height)
{
	LOG(
		"[myy_display_initialised]\n  width : %u, height : %u\n",
		width, height
	);
	glViewport(0, 0, width, height);
	generer_segment_chaque_n_pixels(segments, 50, width, height);
	//menus_recalculate_dimensions(&menus.swap, width, height);
	//menus_regen_static_parts(&menus.dropdown.gl_infos);
	menus_refresh(&menus, width, height);
	program_builder_menu_refresh(&program_builder_menu);
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




#define SWAP_MENUS_BUFFER_OFFSET 0x80000

void myy_init_drawing() {
	
	glhShadersPackLoader(&glsl_shared_data);
	
	glhBuildAndSaveSimpleProgram(
	  &glsl_shared_data, lines_vsh, lines_fsh, glsl_program_lines
	);
	glhBuildAndSaveSimpleProgram(
	  &glsl_shared_data, node_vsh, node_fsh, glsl_program_node
	);
	glhBuildAndSaveSimpleProgram(
		&glsl_shared_data,
		fixed_widgets_vsh,
		fixed_widgets_fsh,
		glsl_program_fixed_widgets
	);
	glhBuildAndSaveSimpleProgram(
		&glsl_shared_data,
		color_node_vsh,
		color_node_fsh,
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
	hitboxes_S_init(&hitboxes, 32);
	common_display_data.fonts_glyphs = &myy_glyph_infos;
	common_display_data.hitboxes = &hitboxes;

	// Menus
	// 0x2000 and 0x4000 are arbitrary sizes (8 KB and 16 KB)
	// (8 KiB and 16 KiB using SI notation)
	glGenBuffers(1, static_menu_parts_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, static_menu_parts_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 0x2000, (uint8_t *) 0, GL_STATIC_DRAW);
	glGenBuffers(1, context_menu_text_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, context_menu_text_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 0xf0000, (uint8_t *) 0, GL_DYNAMIC_DRAW);

	set_menu_buffers_and_offsets(
		&menus.dropdown.gl_infos,
		context_menu_text_buffer[0], 0x0000,
		static_menu_parts_buffer[0], 0x0000
	);
	set_menu_buffers_and_offsets(
		&menus.swap.gl_infos,
		context_menu_text_buffer[0], 0x10000,
		static_menu_parts_buffer[0], 0x0000
	);
	swap_menus_init(&menus.swap, &common_display_data);
	nodes_asm_setup_dropdowns_menus(&nodes_display_data, &menus.dropdown);
	regenerate_menus(&menus.dropdown, &myy_glyph_infos);
	
	prepare_test_frame_insts(&text_frames);
	
	program_builder_menu_first_init(&program_builder_menu, &menus.swap);
	program_builder_menu_show_with(&program_builder_menu, &text_frames);
	
		// Nodes
	
	nodes_init(&nodes_display_data, 0x1000, 0x2000, &common_display_data);
	nodes_set_handler_func(
		&nodes_display_data, 0, nodes_asm_onclick_handler
	);
	nodes_add(
		&nodes_display_data,
		(void * __restrict * __restrict) text_frames.data,
		text_frames.count,
		scratch_buffer,
		node_asm_generate_title, node_asm_generate_content
	);
	node_set_position(
		&nodes_display_data, 0,
		position_S_struct(0,0), scratch_buffer
	);
	node_set_position(
		&nodes_display_data, 1,
		position_S_struct(400, 400), scratch_buffer
	);
	node_set_position(
		&nodes_display_data, 2,
		position_S_struct(400, 0), scratch_buffer
	);
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
	dropdown_menus_draw_current(&menus.dropdown, glsl_programs);
	swap_menus_draw(&menus.swap, glsl_programs);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/* Nodes */
	nodes_draw(&nodes_display_data, glsl_programs, offset[2], offset[3]);

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
	
	/*if (!manage_current_menu_click(&ga_menus, x, y))
		nodes_try_to_handle_click(
			&nodes_display_data,
			position_S_struct(offseted_x, offseted_y)
		);
	*/
	hitboxes_action_react_on_click_at(
		&hitboxes, position_S_struct(x, y)
	);
}

void myy_doubleclick(int x, int y, unsigned int button) {}

void myy_move(int x, int y, int start_x, int start_y) {
	unsigned int has_moved_node = nodes_handle_move(
		&nodes_display_data,
		position_S_struct(x-offset[2], y-offset[3]),
		scratch_buffer
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
#define MYY_KP_4 75
#define MYY_KP_5 76
#define MYY_KP_6 77
#define MYY_KP_PLUS 86
#define MYY_KP_1 87
#define MYY_KP_2 88
#define MYY_KP_3 89

void myy_key(unsigned int keycode) {
	LOG("Received : %u\n", keycode);
	switch (keycode) {
		case MYY_KP_1:
			//show_registers_context_menu();
			dropdown_menus_set_current(
				&menus.dropdown, ga_dropdown_menu_registers
			);
			enable_context_menu();
		break;
		case MYY_KP_2:
			dropdown_menus_set_current(
				&menus.dropdown, ga_dropdown_menu_conditions
			);
			enable_context_menu();
		break;
		case MYY_KP_3:
		break;
		case MYY_KP_4:
			program_builder_menu_select(
				&program_builder_menu, split_array_right, 0
			);
			program_builder_move_selected_right_to_left(
				&program_builder_menu
			);
		break;
		case MYY_KP_5:
			program_builder_menu_generate_executable(&program_builder_menu);
		break;
		case MYY_KP_6:
			program_builder_menu_select(
				&program_builder_menu, split_array_left, 0
			);
			program_builder_move_selected_left_to_right(
				&program_builder_menu
			);
		break;
		default: break;
	}
}
void myy_key_release(unsigned int keycode) {
}


