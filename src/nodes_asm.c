#include <src/nodes_asm.h>
#include <myy/helpers/memory.h>
#include <myy/helpers/log.h>

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

extern uint8_t scratch_buffer[];

struct quads_and_size node_asm_generate_title
(nodes const * __restrict const nodes,
 void const * __restrict const uncasted_frame,
 struct glyph_infos const * __restrict const fonts_glyphs,
 buffer_t buffer)
{
	struct quads_and_size text_quads_and_size = {
		.quads = {.size     = 0, .count    = 0},
		.size  = {.x_offset = 0, .y_offset = -24}
	};
	
	struct armv7_text_frame const * __restrict const armv7_frame =
		(struct armv7_text_frame const * __restrict) uncasted_frame;

	struct generated_quads quads = myy_single_string_to_quads(
		fonts_glyphs, armv7_frame->metadata.name, buffer, &text_quads_and_size.size
	);
	text_quads_and_size.quads.size  = quads.size;
	text_quads_and_size.quads.count = quads.count;
	return text_quads_and_size;
}

static struct quads_and_size generate_instruction_quads
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
				snprintf((char *) to_string_buffer, 31, "%d", arg.value);
				current_string = to_string_buffer;
				break;
			case arg_address:
				snprintf((char *) to_string_buffer, 11, "0x%08x", arg.value);
				current_string = to_string_buffer;
				break;
			case arg_data_symbol_address:
			case arg_data_symbol_address_top16:
			case arg_data_symbol_address_bottom16:
			case arg_data_symbol_size:
			case arg_frame_address:
			case arg_frame_address_pc_relative:
				snprintf((char *) to_string_buffer, 31, "%d", arg.value);
				current_string = to_string_buffer;
				break;
			case arg_regmask:
				snprintf((char *) to_string_buffer, 34, "%b", arg.value);
				current_string = to_string_buffer;
				break;
		}
		int current_x = text_offset.x_offset;
		string_quads = myy_single_string_to_quads(
			glyph_infos, current_string, buffer, &text_offset
		);
		total.quads.count += string_quads.count;
		total.quads.size  += string_quads.size;
		buffer += string_quads.size;
		
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

struct quads_and_size node_asm_generate_content
(nodes const * __restrict const nodes,
 void const * __restrict const uncasted_frame,
 struct glyph_infos const * __restrict const fonts_glyphs,
 buffer_t buffer)
{
	struct armv7_text_frame const * __restrict const text_frame =
		(struct armv7_text_frame const * __restrict) uncasted_frame;
	
	unsigned int const y_offset = 20;
	unsigned int n_instructions =
		text_frame->metadata.stored_instructions;
	uint32_t text_pos_y = 0; // Start after the title
	struct quads_and_size total = {0};
	struct quads_and_size instruction_quads;

	unsigned int cpu_buffer_offset = 0;
	for (unsigned int i = 0; i < n_instructions; i++) {
		
		instruction_quads = generate_instruction_quads(
			fonts_glyphs, text_frame->instructions+i,
			buffer+cpu_buffer_offset, text_pos_y
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

static void dropdown_menu_hit_func
(struct dropdown_menus const * __restrict const menus,
 unsigned int i)
{
	menus->current_dropdown_callback(
		menus->current_dropdown_callback_data,
		i
	);
  LOG("%d\n", i);
}

void nodes_asm_setup_dropdowns_menus
(nodes * __restrict const nodes,
 struct dropdown_menus * __restrict const menus)
{
	nodes->current_menus = menus;
	dropdowns_menu_setup_menu(
		menus, ga_dropdown_menu_instructions, mnemonics_full_desc,
    n_known_instructions, dropdown_menu_hit_func
	);
	dropdowns_menu_setup_menu(
		menus, ga_dropdown_menu_registers, register_names,
		16, dropdown_menu_hit_func
	);
	dropdowns_menu_setup_menu(
		menus, ga_dropdown_menu_frame_names, register_names, 0,
		dropdown_menu_hit_func
	);
	dropdowns_menu_setup_menu(
		menus, ga_dropdown_menu_conditions, conditions_strings, 15,
		dropdown_menu_hit_func
	);
}

unsigned int selected_line = 0;
extern struct glyph_infos myy_glyph_infos;
void nodes_set_selected_instruction
(void * __restrict const uncasted_nodes, unsigned int instruction_id)
{
	/*struct nodes_display_data * __restrict const nodes =
		(struct nodes_display_data *) uncasted_nodes;
	
	unsigned int node_id = nodes_get_selected_id(nodes);
	struct armv7_text_frame * current_frame =
		(struct armv7_text_frame *) nodes->associated_data[node_id];
	struct instruction_representation * instruction = 
		current_frame->instructions+selected_line;
	instruction_mnemonic_id(instruction, instruction_id);
	generate_frames_quads(
		(struct armv7_text_frame **) nodes->associated_data, 2, nodes,
		&myy_glyph_infos
	);
	generate_and_store_nodes_containers_in_gpu(nodes, scratch_buffer);*/
	LOG("Meow\n");
}

void nodes_asm_onclick_handler
(nodes const * __restrict const nodes,
 unsigned int index, position_S const pos)
{
	struct dropdown_menus * __restrict const menus =
		nodes->current_menus;
	dropdown_menus_set_current(menus, ga_dropdown_menu_instructions);
	dropdown_menus_set_current_callback(
		menus, nodes_set_selected_instruction, (void *) nodes
	);
	enable_context_menu();
	LOG(
		"Meep : %d - rel_x: %d, rel_y: %d (%d)\n",
		index, pos.x, pos.y, (pos.y-6)/20
	);
	selected_line = (pos.y-6)/20;
}
