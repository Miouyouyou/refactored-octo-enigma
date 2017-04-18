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

struct mnemonic_info {
	uint8_t n_args;
	uint8_t * name;
};

enum { 
	arg_cat_invalid,
	arg_cat_condition,
	arg_cat_register,
	arg_cat_immediate,
	arg_cat_negative_immediate,
	arg_cat_immediate_or_address,
	arg_cat_data_address,
	arg_cat_frame_address_pc_relative,
	arg_cat_regmask
};

static uint8_t const mnemonics_args_type[n_known_instructions][MAX_ARGS] =
{
	[inst_add_immediate]  =
		{arg_cat_register, arg_cat_register, arg_cat_immediate},
	[inst_b_address]      =
		{arg_cat_condition, arg_cat_frame_address_pc_relative, 0},
	[inst_bl_address]     =
		{arg_cat_condition, arg_cat_frame_address_pc_relative, 0},
	[inst_blx_address]    =
		{arg_cat_condition, arg_cat_frame_address_pc_relative, 0},
	[inst_blx_register]   = {arg_cat_condition, arg_cat_register, 0},
	[inst_bx_register]    = {arg_cat_condition, arg_cat_register, 0},
	[inst_mov_immediate]  = {arg_cat_register, arg_cat_immediate, 0},
	[inst_mov_register]   = {arg_cat_register, arg_cat_register, 0},
	[inst_movt_immediate] = {arg_cat_register, arg_cat_immediate, 0},
	[inst_movw_immediate] = 
		{arg_cat_register, arg_cat_immediate_or_address, 0},
	[inst_mvn_immediate]  = 
		{arg_cat_register, arg_cat_negative_immediate, 0},
	[inst_pop_regmask]    = {arg_cat_regmask, 0, 0},
	[inst_push_regmask]   = {arg_cat_regmask, 0, 0},
	[inst_sub_immediate]  =
		{arg_cat_register, arg_cat_register, arg_cat_immediate},
	[inst_svc_immediate]  = {arg_cat_immediate, 0, 0},
};

static uint8_t const * const mnemonics[n_known_instructions] = {
	[inst_add_immediate]  = "add",
	[inst_b_address]      = "b",
	[inst_bl_address]     = "bl",
	[inst_blx_address]    = "blx",
	[inst_blx_register]   = "blx",
	[inst_bx_register]    = "bx",
	[inst_mov_immediate]  = "mov",
	[inst_mov_register]   = "mov",
	[inst_movt_immediate] = "movt",
	[inst_movw_immediate] = "movw",
	[inst_mvn_immediate]  = "mvn",
	[inst_pop_regmask]    = "pop",
	[inst_push_regmask]   = "push",
	[inst_sub_immediate]  = "sub",
	[inst_svc_immediate]  = "svc"
};

static uint8_t const * const
mnemonics_full_desc[n_known_instructions] = {
	[inst_add_immediate]  = "ADD (Immediate)",
	[inst_b_address]      = "B",
	[inst_bl_address]     = "BL",
	[inst_blx_address]    = "BLX",
	[inst_blx_register]   = "BLX (Register)",
	[inst_bx_register]    = "BX (Register)",
	[inst_mov_immediate]  = "MOV",
	[inst_mov_register]   = "MOV (Register)",
	[inst_movt_immediate] = "MOVT",
	[inst_movw_immediate] = "MOVW",
	[inst_mvn_immediate]  = "MVN",
	[inst_pop_regmask]    = "POP",
	[inst_push_regmask]   = "PUSH",
	[inst_sub_immediate]  = "SUB (Immediate)",
	[inst_svc_immediate]  = "SVC"
};

static uint8_t mnemonic_args[n_known_instructions] = {
	[inst_add_immediate]  = 3,
	[inst_b_address]      = 2,
	[inst_bl_address]     = 2,
	[inst_blx_address]    = 2,
	[inst_blx_register]   = 2,
	[inst_bx_register]    = 2,
	[inst_mov_immediate]  = 2,
	[inst_mov_register]   = 2,
	[inst_movt_immediate] = 2,
	[inst_movw_immediate] = 2,
	[inst_mvn_immediate]  = 2,
	[inst_pop_regmask]    = 1,
	[inst_push_regmask]   = 1,
	[inst_sub_immediate]  = 3,
	[inst_svc_immediate]  = 1,
};

static uint8_t const * conditions_strings[15] = {
	[cond_eq] = "eq",
	[cond_ne] = "ne",
	[cond_cs] = "cs",
	[cond_cc] = "cc",
	[cond_mi] = "mi",
	[cond_pl] = "pl",
	[cond_vs] = "vs",
	[cond_vc] = "vc",
	[cond_hi] = "hi",
	[cond_ls] = "ls",
	[cond_ge] = "ge",
	[cond_lt] = "lt",
	[cond_gt] = "gt",
	[cond_le] = "le",
	[cond_al] = "al"
};

uint8_t const * register_names[] = {
	[r0]  = "r0",
	[r1]  = "r1",
	[r2]  = "r2",
	[r3]  = "r3",
	[r4]  = "r4",
	[r5]  = "r5",
	[r6]  = "r6",
	[r7]  = "r7",
	[r8]  = "r8",
	[r9]  = "r9",
	[r10] = "r10",
	[r11] = "r11",
	[r12] = "ip",
	[r13] = "sp",
	[r14] = "lr",
	[r15] = "pc"
};

GLuint test_frame_gpu_buffer[1] = {0};
unsigned int current_inst_id = 0;
unsigned int current_arg_id = 0;

// Hitboxes

struct frame_hitbox {
	int16_t x_left, x_right, y_up, y_down;
};
struct frame_hitboxes {
	uint32_t n_hitboxes;
	uint32_t max_hitboxes;
	struct frame_hitbox * hitboxes;
};
struct frame_hitboxes test_frame_hitboxes;
// fixed hitboxes number by element
// hitboxes / 4 = index - hitboxes % 4 = arg

// Background lines

struct myy_gl_segment { float startx, starty, endx, endy; };

struct myy_gl_segment segments[80] = {0};
GLuint segment_buffer;
GLuint screen_width = 0, screen_height = 0;

// Menu
uint8_t scratch_buffer[0x4000] __attribute__((aligned(32)));


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

struct armv7_text_frame * __restrict test_frame;
static struct armv7_text_frame * prepare_test_frame_insts() {
	test_frame = generate_armv7_text_frame(id_gen);
	
	uint32_t const hamster_id = 9;
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
	
	return test_frame;
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

void myy_display_initialised(unsigned int width, unsigned int height) {
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

#include <myy/helpers/strings.h>
#include <stdio.h>

static unsigned int offset_between
(uintptr_t const begin, uintptr_t const end)
{
	return (unsigned int) (end - begin);
}



static void add_hitbox
(int32_t const x_left, int32_t const x_right,
 int32_t const y_up,   int32_t const y_down)
{
	struct frame_hitbox * const new_hitbox = 
		test_frame_hitboxes.hitboxes+test_frame_hitboxes.n_hitboxes;
	
	new_hitbox->x_left = x_left;
	new_hitbox->x_right = x_right;
	new_hitbox->y_up = y_up;
	new_hitbox->y_down = y_down;
	
	test_frame_hitboxes.n_hitboxes++;
}

struct hitcheck_result {
	unsigned int hit;
	unsigned int id;
};

static struct hitcheck_result got_hitbox
(struct frame_hitboxes * hitboxes, int x, int y)
{
	unsigned int h = 0; 
	unsigned int n_hitboxes = hitboxes->n_hitboxes;
	while (h < n_hitboxes) {
		struct frame_hitbox current_hitbox = hitboxes->hitboxes[h];
		if (x < current_hitbox.x_left || x > current_hitbox.x_right ||
			  y < current_hitbox.y_up || y > current_hitbox.y_down)
			h++;
		else
			break;
	}
	
	struct hitcheck_result result = {
		.hit = (h != n_hitboxes),
		.id = h
	};
	
	return result;
}

static inline void add_invalid_hitbox() {
	add_hitbox(0xffff,0xffff,0xffff,0xffff);
}

struct instruction_from_hitbox {
	unsigned int i;
	unsigned int arg_i;
};
static struct instruction_from_hitbox infer_instruction_from_hitbox_id
(unsigned int hitbox_index)
{
	struct instruction_from_hitbox result = {
		.i = hitbox_index / 4,
		.arg_i = hitbox_index % 4
	};
	
	return result;
}
static void react_on_hitbox(unsigned int const id)
{
	struct instruction_from_hitbox inst_infos =
		infer_instruction_from_hitbox_id(id);
	if (id % 4 == 0) enable_context_menu();
	LOG(
		"%04x: %s (arg : %d)\n", id * 4,
		mnemonics[test_frame->instructions[inst_infos.i].mnemonic_id],
		inst_infos.arg_i
	);
	current_inst_id = inst_infos.i;
	current_arg_id = inst_infos.arg_i;
}

struct quads_and_size {
	struct generated_quads quads;
	struct text_offset size;
};

struct quads_and_size generate_instruction_quads
(struct glyph_infos const * __restrict const glyph_infos,
 struct instruction_representation const * __restrict const instruction,
 uint8_t * __restrict const cpu_buffer, int16_t const y_offset)
{

	uint8_t * __restrict buffer = cpu_buffer;
	struct quads_and_size total = {0};
	struct generated_quads string_quads;
	
	struct text_offset text_offset = {
		.x_offset = 0,
		.y_offset = y_offset
	};
	
	// Arbitrary defined. Ugly but should do the trick for the PoC.
	unsigned int const line_height = 20; // pixels. 
	uint8_t const * __restrict const mnemonic =
		mnemonics[instruction->mnemonic_id];
	unsigned int n_mnemonic_args =
		mnemonic_args[instruction->mnemonic_id];

	
	string_quads = myy_single_string_to_quads(
		glyph_infos, mnemonic, buffer, &text_offset
	);
	total.quads.count += string_quads.count;
	total.quads.size  += string_quads.size;
	buffer += string_quads.size;
	
	/*add_hitbox(
		-10, text_offset.x_offset+10, y_offset, y_offset+line_height
	);*/
	text_offset.x_offset = 60;
	
	uint8_t empty_buffer = '\0';
	uint8_t to_string_buffer[64];
	uint8_t const * current_string;
	unsigned int a = 0;
	while (a < n_mnemonic_args) {
		struct instruction_args_infos arg = instruction->args[a];
		switch(arg.type) {
			case arg_invalid: 
				current_string = &empty_buffer; break;
			case arg_condition:
				current_string = conditions_strings[arg.value];
				break;
			case arg_register:
				current_string = register_names[arg.value];
				break;
			case arg_immediate:
				snprintf(to_string_buffer, 31, "%d", arg.value);
				current_string = &to_string_buffer;
				break;
			case arg_address:
				snprintf(to_string_buffer, 11, "0x%08x", arg.value);
				current_string = &to_string_buffer;
				break;
			case arg_data_symbol_address:
			case arg_data_symbol_address_top16:
			case arg_data_symbol_address_bottom16:
			case arg_data_symbol_size:
			case arg_frame_address:
			case arg_frame_address_pc_relative:
				snprintf(to_string_buffer, 31, "%d", arg.value);
				current_string = &to_string_buffer;
				break;
			case arg_regmask:
				snprintf(to_string_buffer, 34, "%b", arg.value);
				current_string = &to_string_buffer;
				break;
		}
		int current_x = text_offset.x_offset;
		string_quads = myy_single_string_to_quads(
			glyph_infos, current_string, buffer, &text_offset
		);
		total.quads.count += string_quads.count;
		total.quads.size  += string_quads.size;
		buffer += string_quads.size;
		
		/*add_hitbox(
			current_x - 6, text_offset.x_offset + 6, y_offset, y_offset+line_height
		);*/
		text_offset.x_offset += 12;
		a++;
	}
	while (a < MAX_ARGS) {
		/*add_invalid_hitbox();*/
		a++;
	}
	
	total.size = text_offset;

	return total;
}

unsigned int test_frame_quads = 0;
void prepare_hitboxes
(struct frame_hitboxes * __restrict const hitboxes)
{
	hitboxes->n_hitboxes = 0;
	hitboxes->hitboxes = allocate_durable_memory(
		sizeof(struct frame_hitbox) * 128
	);
	hitboxes->max_hitboxes = 0;
}

struct quads_and_size generate_frame_quads
(struct armv7_text_frame const * __restrict const text_frame,
 uint8_t * __restrict const cpu_buffer,
 unsigned int cpu_buffer_offset)
{
	unsigned int const y_offset = 20;
	unsigned int n_instructions =
		text_frame->metadata.stored_instructions;
	uint32_t text_pos_y = 0;
	struct quads_and_size total = {0};
	struct quads_and_size instruction_quads;

	for (unsigned int i = 0; i < n_instructions; i++) {
		
		instruction_quads = generate_instruction_quads(
			&myy_glyph_infos, text_frame->instructions+i,
			cpu_buffer+cpu_buffer_offset, text_pos_y
		);
		total.quads.count += instruction_quads.quads.count;
		total.quads.size  += instruction_quads.quads.size;
		if (instruction_quads.size.x_offset > total.size.x_offset)
			total.size.x_offset = instruction_quads.size.x_offset;
		cpu_buffer_offset += instruction_quads.quads.size;
		text_pos_y += y_offset;
	}
	total.size.y_offset = text_pos_y;
	return total;
}

static void prepare_string
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 US_two_tris_quad_3D * __restrict const quads)
{
	myy_codepoints_to_glyph_twotris_quads_window_coords(
	  glyph_infos, string, n_characters, quads
	);
}

struct dropdown_menus_infos ga_menus[n_ga_menus] = {0};

void dropdown_menu_hit_func(unsigned int i) {
  LOG("%d\n", i);
}

struct swap_menu_infos swap_menu_infos = {0};

void generate_frames_quads
(struct armv7_text_frame const * __restrict const text_frames,
 unsigned int n_frames,
 GLuint const gpu_buffer_id,
 GLuint const gpu_buffer_offset,
 struct nodes_display_data * __restrict const display_data)
{
	unsigned int cpu_buffer_offset = 0;
	for (unsigned int f = 0; f < n_frames; f++) {
		struct node_container_metadata * const current_container_info =
			display_data->containers.metadata+f;
		struct node_contents_metadata * const current_content_info =
			display_data->contents.metadata+f;
		struct armv7_text_frame const * __restrict const current_frame =
			text_frames+f;
		
		struct quads_and_size frame_quads = generate_frame_quads(
			current_frame, scratch_buffer, cpu_buffer_offset
		);

		current_container_info->width  = frame_quads.size.x_offset;
		current_container_info->height = frame_quads.size.y_offset;

		current_content_info->buffer_offset =
			gpu_buffer_offset + cpu_buffer_offset;
		current_content_info->quads = frame_quads.quads.count;
		
		cpu_buffer_offset  += frame_quads.quads.size;
	}
	
	display_data->containers.n_nodes += n_frames;
	display_data->contents.buffer_id = gpu_buffer_id;
	glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer_id);
	glBufferSubData(
		GL_ARRAY_BUFFER, gpu_buffer_offset, cpu_buffer_offset,
		scratch_buffer
	);
}

void myy_init_drawing() {
	
	glhShadersPackLoader(&glsl_shared_data);
	
	glhBuildAndSaveSimpleProgram(
	  &glsl_shared_data,
	  standard_vsh, standard_fsh,
	  glsl_program_standard
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
	glBufferData(GL_ARRAY_BUFFER, 0x2000, (uint8_t *) 0, GL_DYNAMIC_DRAW);

	prepare_static_menu_parts();

	glGenBuffers(2, context_menu_text_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, context_menu_text_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 0x4000, (uint8_t *) 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, context_menu_text_buffer[1]);
	glBufferData(GL_ARRAY_BUFFER, 0x4000, (uint8_t *) 0, GL_DYNAMIC_DRAW);

	/*prepare_context_menu_with(
		context_menu_text_buffer, &myy_glyph_infos, mnemonics_full_desc,
		n_known_instructions, 32
	);*/
	setup_menu(
		ga_menus, ga_dropdown_menu_instructions, mnemonics_full_desc,
    n_known_instructions, dropdown_menu_hit_func
	);
	setup_menu(
		ga_menus, ga_dropdown_menu_registers, register_names,
		16, dropdown_menu_hit_func
	);
	setup_menu(
		ga_menus, ga_dropdown_menu_frame_names, register_names, 0,
		dropdown_menu_hit_func
	);
	setup_menu(
		ga_menus, ga_dropdown_menu_conditions, conditions_strings, 15,
		dropdown_menu_hit_func
	);
	regenerate_menus(
		ga_menus, context_menu_text_buffer[0], &myy_glyph_infos
	);
	
	set_swap_menu_title(
		&swap_menu_infos, context_menu_text_buffer[0],
		"Hello", &myy_glyph_infos
	);

	set_swap_menu_listings(
		&swap_menu_infos, context_menu_text_buffer[0],
		n_known_instructions, 0,
		mnemonics_full_desc, conditions_strings,
		&myy_glyph_infos
	);
	

	glGenBuffers(1, test_frame_gpu_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, test_frame_gpu_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 0x2000, (uint8_t *) 0, GL_DYNAMIC_DRAW);
	
	prepare_test_frame_insts();
	generate_frames_quads(
		test_frame,
		1,
		test_frame_gpu_buffer[0],
		0, &nodes_display_data
	);
	generate_and_store_nodes_containers_in_gpu(
		&nodes_display_data, scratch_buffer,
		test_frame_gpu_buffer[0], 0x1000
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
	glClearColor(0.1f, 0.3f, 0.5f, 1.0f);

	GLuint * glsl_programs = glsl_shared_data.programs;
	struct norm_offset norm_offset = normalised_offset();

	// Context menu
	draw_current_context_menu(
		ga_menus, glsl_programs,
		static_menu_parts_buffer[0], context_menu_text_buffer[0]
	);
	draw_swap_menu(
		&swap_menu_infos, glsl_programs,
		context_menu_text_buffer[0], 
		static_menu_parts_buffer[0]
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/* Nodes */
	draw_nodes(&nodes_display_data, glsl_programs, offset[2], offset[3]);

	// Lines
	glUseProgram(glsl_programs[glsl_program_standard]);
	glEnableVertexAttribArray(attr_xyz);
	glUniform2f(unif_offset, norm_offset.x, norm_offset.y);
	glBindBuffer(GL_ARRAY_BUFFER, segment_buffer);
	glVertexAttribPointer(attr_xyz, 2, GL_FLOAT, GL_FALSE, 0, 0);
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

int16_t last_x, last_y;
void myy_click(int x, int y, unsigned int button) {
	last_x = x;
	last_y = y;
	//display_context_menu ^= 1;
	int offseted_x = x - offset[2];
	int offseted_y = y - offset[3];
	
	LOG("coords : %d, %d\noffsets : %d, %d\nresults : %d, %d\n",
	    x, y, offset[2], offset[3], offseted_x, offseted_y);
	
	if (offseted_x > -100 && offseted_x < 200 &&
		  offseted_y > -100 && offseted_y < 600)
	{
		struct hitcheck_result hitbox_check =
			got_hitbox(&test_frame_hitboxes, offseted_x+80, offseted_y+80);
			
		if (hitbox_check.hit) react_on_hitbox(hitbox_check.id);
	}
		
	
	manage_current_menu_click(ga_menus, x, y);
}

void myy_doubleclick(int x, int y, unsigned int button) {}
void myy_move(int x, int y, int start_x, int start_y) {
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
			set_current_dropdown_menu(ga_dropdown_menu_registers);
			enable_context_menu();
		break;
		case MYY_KP_2:
			set_current_dropdown_menu(ga_dropdown_menu_conditions);
			enable_context_menu();
		break;
		case MYY_KP_3:
			enable_swap_menu();
		break;
		case MYY_KP_4:
			disable_swap_menu();
		break;
		case MYY_KP_5:
			set_swap_menu_listings(
				&swap_menu_infos, context_menu_text_buffer[0],
				16, 0,
				register_names, mnemonics_full_desc,
				&myy_glyph_infos
			);
		break;
		default: break;
	}
}
void myy_key_release(unsigned int keycode) {
}


