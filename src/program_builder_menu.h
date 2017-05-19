#ifndef MYY_SRC_PROGRAM_BUILDER_MENU_H
#define MYY_SRC_PROGRAM_BUILDER_MENU_H 1

#include <myy/helpers/arrays.h>

#include <lib/Assembler/armv7-arm.h>

#include <src/menus.h>

#include <stdint.h>

struct split_array_data_addr {
	uint8_t ** data;
	struct split_array_metadata elements;
};

struct split_array_indices {
	uint16_t * i;
	struct split_array_metadata elements;
};

struct program_builder_menu {
	struct armv7_text_frames   * frames;
	struct split_array_indices * split_indices;
	swap_menus                 * swap_menu_template;
	uint16_t                     selection_index[n_split_array_sections];
};
typedef struct program_builder_menu program_builder_menu_t;

void program_builder_menu_first_init
(struct program_builder_menu * __restrict const menu,
 swap_menus * __restrict const swap_menu_template);

void program_builder_menu_show_with
(program_builder_menu_t * __restrict const menu,
 struct armv7_text_frames * __restrict const program_frames);

uint8_t program_builder_move_selected_left_to_right
(program_builder_menu_t * __restrict const menu);

uint8_t program_builder_move_selected_right_to_left
(program_builder_menu_t * __restrict const menu);

void program_builder_menu_generate_executable
(program_builder_menu_t * __restrict const menu);

void program_builder_menu_refresh
(program_builder_menu_t * __restrict const menu);

void program_builder_menu_select
(program_builder_menu_t * __restrict const menu,
 enum split_array_section section, uint16_t index);

inline static uint_fast16_t program_builder_menu_get_left_count
(program_builder_menu_t * __restrict const menu)
{
	return split_array_get_count(
		(SplitArray *) menu->split_indices, split_array_left
	);
}

inline static uint_fast16_t program_builder_menu_get_right_count
(program_builder_menu_t * __restrict const menu)
{
	return split_array_get_count(
		(SplitArray *) menu->split_indices, split_array_right
	);
}

inline static uint_fast16_t program_builder_menu_get_count
(program_builder_menu_t * __restrict const menu,
 enum split_array_section section)
{
	return split_array_get_count(
		(SplitArray *) menu->split_indices, section
	);
}

inline static uint_fast16_t program_builder_menu_get_selection
(program_builder_menu_t * __restrict const menu,
 enum split_array_section section)
{
	return menu->selection_index[section];
}

inline static uint_fast16_t program_builder_menu_get_left_selection
(program_builder_menu_t * __restrict const menu)
{
	return program_builder_menu_get_selection(menu, split_array_left);
}

inline static uint_fast16_t program_builder_menu_get_right_selection
(program_builder_menu_t * __restrict const menu)
{
	return program_builder_menu_get_selection(menu, split_array_right);
}

inline static uint_fast16_t program_builder_menu_total_elements
(program_builder_menu_t * __restrict const menu)
{
	return menu->split_indices->elements.total;
}

inline static uint8_t program_builder_menu_is_selection_valid
(program_builder_menu_t * __restrict const menu,
 enum split_array_section section)
{
	uint_fast16_t n_section_indices =
		program_builder_menu_get_count(menu, section);
	uint_fast16_t selection_index = menu->selection_index[section];
	return selection_index < n_section_indices;
}




#endif
