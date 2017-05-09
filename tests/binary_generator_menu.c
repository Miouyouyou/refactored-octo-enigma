
#include <myy/helpers/arrays.h>
#include <src/program_builder_menu.h>

#include "common.h"

SplitArray a_split_array;

#include <myy/helpers/memory.h>


static int frame_id = 0;
static uint32_t id_generator() {
	return frame_id += 1;
}

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



void init_frames
(struct armv7_text_frames * __restrict const assembly_frames)
{
	dyn_array_data_pointers_init(
		(DynPointers_u16_t *) assembly_frames, 2
	);
	assert_not_null(assembly_frames->data);
	assert_equal(assembly_frames->count, 0);
	assert_equal(assembly_frames->max, 2);
	dyn_array_data_pointers_append_u16(
		(DynPointers_u16_t *) assembly_frames,
		generate_armv7_text_frame(id_generator)
	);
	dyn_array_data_pointers_append_u16(
		(DynPointers_u16_t *) assembly_frames,
		generate_armv7_text_frame(id_generator)
	);
	struct armv7_text_frame * __restrict const print_frame =
		assembly_frames->data[0];
	struct armv7_text_frame * __restrict const exit_frame =
		assembly_frames->data[1];
		
	print_frame->metadata.name = (uint8_t *) "Print";
	exit_frame->metadata.name = (uint8_t *) "Exit";
	
	// Print
	uint32_t hamster_id = 9;
	add_inst(
		print_frame,
		inst_mov_immediate, arg_register, r0, arg_immediate, 0,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_mov_immediate,
		arg_register, r1,
		arg_data_symbol_address_bottom16, hamster_id,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_movt_immediate,
		arg_register, r1,
		arg_data_symbol_address_top16, hamster_id,
		arg_invalid, 0
	);
	
	add_inst(
		print_frame,
		inst_mov_immediate,
		arg_register, r2,
		arg_data_symbol_size, hamster_id,
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

#include <string.h>
static inline void assert_strings_equal_u8
(uint8_t const * __restrict const string_a,
 uint8_t const * __restrict const string_b,
 unsigned int size)
{
	assert_equal(0, memcmp(string_a, string_b, size));
}

void test_split_array()
{
	split_array_init(&a_split_array, 10, sizeof(uint16_t));
	assert_not_null(a_split_array.data);
	assert_equal(a_split_array.elements.total, 10);
	
	a_split_array.elements.count[split_array_left]  = 7;
	a_split_array.elements.count[split_array_right] = 3;
	
	assert_equal(
		split_array_get_count(&a_split_array, split_array_left), 7
	);
	assert_equal(
		split_array_get_count(&a_split_array, split_array_right), 3
	);
	
	GLuint buffers[2];
	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, 0x2000, (uint8_t *) 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, 0x8000, (uint8_t *) 0, GL_DYNAMIC_DRAW);
	
	swap_menus swap_menus;
	set_menu_buffers_and_offsets(
		&swap_menus.gl_infos, buffers[0], 0x0000, buffers[1], 0x0000
	);
	swap_menus_set_common_data(&swap_menus, &common_graphics_elements);
	prepare_static_menu_parts(&swap_menus.gl_infos);
	
	program_builder_menu_t menu;
	struct armv7_text_frames assembly_frames;
	init_frames(&assembly_frames);
	assert_strings_equal_u8(
		(uint8_t *) "Print",
		assembly_frames.data[0]->metadata.name,
		sizeof("Print")
	);
	assert_strings_equal_u8(
		(uint8_t *) "Exit",
		assembly_frames.data[1]->metadata.name,
		sizeof("Exit")
	);
	
	program_builder_menu_first_init(&menu, &swap_menus);
	program_builder_menu_show_with(&menu, &assembly_frames);
	assert_equal(menu.frames, &assembly_frames);
	
	assert_equal(menu.split_indices->elements.count[split_array_left], 2);
	assert_equal(menu.split_indices->elements.count[split_array_right], 0);
	assert_equal(menu.split_indices->elements.total, 2);
	assert_equal(
		program_builder_menu_get_count(&menu, split_array_left), 2
	);
	assert_equal(
		program_builder_menu_get_count(&menu, split_array_right), 0
	);
	assert_equal(program_builder_menu_get_left_count(&menu), 2);
	assert_equal(program_builder_menu_get_right_count(&menu), 0);
	assert_equal_s16(program_builder_menu_get_left_selection(&menu), -1);
	assert_equal_s16(program_builder_menu_get_right_selection(&menu), -1);
	assert_equal_s16(menu.split_indices->i[0], 0);
	assert_equal_s16(menu.split_indices->i[1], 1);
	
	assert_equal_s16(swap_menus.left_n_options, 2);
	assert_equal_s16(swap_menus.right_n_options, 0);
	
	program_builder_menu_select(&menu, split_array_left, 0);
	assert_equal(menu.selection_index[split_array_left], 0);
	assert_true(
		program_builder_menu_is_selection_valid(&menu, split_array_left)
	);
	
	program_builder_move_selected_left_to_right(&menu);
	assert_equal_s16(program_builder_menu_get_left_count(&menu), 1);
	assert_equal_s16(program_builder_menu_get_right_count(&menu), 1);
	assert_equal_s16(menu.split_indices->i[0], 1);
	assert_equal_s16(menu.split_indices->i[1], 0);
	program_builder_move_selected_left_to_right(&menu);
	assert_equal_s16(program_builder_menu_get_left_count(&menu), 0);
	assert_equal_s16(program_builder_menu_get_right_count(&menu), 2);
	assert_equal_s16(menu.split_indices->i[0], 0);
	assert_equal_s16(menu.split_indices->i[1], 1);
	assert_false(
		program_builder_menu_is_selection_valid(&menu, split_array_left)
	);
	
	unsigned int loop = 10;
	while(loop--) {
		assert_false(
			program_builder_menu_is_selection_valid(&menu, split_array_left)
		);
		program_builder_move_selected_left_to_right(&menu);
		assert_equal_s16(program_builder_menu_get_left_count(&menu), 0);
		assert_equal_s16(program_builder_menu_get_right_count(&menu), 2);
		assert_equal_s16(menu.split_indices->i[0], 0);
		assert_equal_s16(menu.split_indices->i[1], 1);
	}
	loop = 50;
	while(loop--) {
		program_builder_menu_select(&menu, split_array_left, loop);
		assert_false(
			program_builder_menu_is_selection_valid(&menu, split_array_left)
		);
		program_builder_move_selected_left_to_right(&menu);
		assert_equal_s16(program_builder_menu_get_left_count(&menu), 0);
		assert_equal_s16(program_builder_menu_get_right_count(&menu), 2);
		assert_equal_s16(menu.split_indices->i[0], 0);
		assert_equal_s16(menu.split_indices->i[1], 1);
	}
	
	program_builder_menu_select(&menu, split_array_right, 0);
	program_builder_move_selected_right_to_left(&menu);
	assert_equal_s16(program_builder_menu_get_left_count(&menu), 1);
	assert_equal_s16(program_builder_menu_get_right_count(&menu), 1);
	assert_equal_s16(menu.split_indices->i[0], 0);
	assert_equal_s16(menu.split_indices->i[1], 1);
	program_builder_move_selected_right_to_left(&menu);
	assert_equal_s16(program_builder_menu_get_left_count(&menu), 2);
	assert_equal_s16(program_builder_menu_get_right_count(&menu), 0);
	assert_equal_s16(menu.split_indices->i[0], 0);
	assert_equal_s16(menu.split_indices->i[1], 1);
	assert_false(
		program_builder_menu_is_selection_valid(&menu, split_array_right)
	);
	
}

void test_program_generator_menu() {
	
}
