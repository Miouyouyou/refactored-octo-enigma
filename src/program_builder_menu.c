#include <src/program_builder_menu.h>
#include <myy/helpers/arrays.h>
#include <myy/helpers/memory.h>

#include <myy/helpers/log.h>

#include <lib/Assembler/dumbelflib.h>

#include <stdint.h>

void program_builder_menu_refresh
(program_builder_menu_t * __restrict const menu)
{
	LOG("[program_builder_menu_refresh]\n  Refreshing menu...\n");
	unsigned int const n_frames_indices =
		program_builder_menu_total_elements(menu);
	uint8_t ** strings = allocate_temporary_memory(
	  sizeof(uint8_t *) * n_frames_indices
	);
	
	for (unsigned int s = 0; s < n_frames_indices; s++) {
		unsigned int const frame_i = menu->split_indices->i[s];
		strings[s] = menu->frames->data[frame_i]->metadata.name;
	}

	unsigned int const n_left_strings = 
		program_builder_menu_get_left_count(menu);
	unsigned int const n_right_strings =
		program_builder_menu_get_right_count(menu);
	
	set_swap_menu_listings(
		menu->swap_menu_template, n_left_strings, n_right_strings,
		strings, strings+n_left_strings
	);
}

void program_builder_move_selected_right_to_left
(program_builder_menu_t * __restrict const menu)
{
	if (!program_builder_menu_is_selection_valid(menu, split_array_right))
		goto invalid_selection;

	unsigned int const current_right_start = 
		program_builder_menu_get_left_count(menu);
	unsigned int const new_last_left = current_right_start;
	unsigned int const new_left_end  = current_right_start+1;
	unsigned int const old_right_count =
		program_builder_menu_get_right_count(menu);
	unsigned int const abs_selected_index = 
		current_right_start + program_builder_menu_get_right_selection(menu);
	unsigned int const far_right = current_right_start + old_right_count;
	
	uint16_t * __restrict const split_indices = 
		menu->split_indices->i;
	
	/* Premièrement - Sauvegarde de l'index à faire basculer,
	   retrait de cet index et tassement des données dans la zone
	   droite
	 */
	uint16_t index       = split_indices[abs_selected_index];
	uint16_t after_right_selection = abs_selected_index+1;
	uint16_t first_move_size = 
		(far_right - after_right_selection) * sizeof(uint16_t);
	
	recopy_inside_memory_space(
		split_indices+abs_selected_index,
		split_indices+after_right_selection,
		first_move_size
	);
	
	/* Deuxièmement - 
	   La zone gauche se trouve avant la zone droite, et les deux zones
	   sont contigües.
	   Donc, pour insérer un élément à la fin de la zone gauche, il est
	   nécessaire de redéplacer tous les éléments à droite.
	   Ceci ne peut être fait à la base, car l'espace mémoire est
	   calculé pour ne contenir que les éléments nécessaires.
	   (Et puis, cela ne paraît pas vraiment utile de sur-allouer juste
	   pour cette opération)
	 */
	
	recopy_inside_memory_space(
		split_indices+new_left_end, // current_right_start+1
		split_indices+current_right_start,
		old_right_count
	);
	
	/* Troisièmement -
	   Placer l'élément de droite, sauvegardé préalablement,
	   dans l'emplacement libéré à la fin de la zone gauche, juste avant
	   la zone droite
	 */
	split_indices[new_last_left] = index;
	
	/* Enfin -
	   Mise à jour des compteurs et rafraichissement du menu
	*/
	
	menu->split_indices->elements.count[split_array_left]  += 1;
	menu->split_indices->elements.count[split_array_right] -= 1;
	
	program_builder_menu_refresh(menu);
	
invalid_selection:
	return;
}

void program_builder_move_selected_left_to_right
(program_builder_menu_t * __restrict const menu)
{
	if (!program_builder_menu_is_selection_valid(menu, split_array_left))
		goto invalid_selection;
	
	/* Premièrement : Retirer l'élément à déplacer et retasser jusqu'à
	                  l'index de destination */
	
	uint_fast16_t const src_i  =
		program_builder_menu_get_left_selection(menu);
	uint_fast16_t const dst_i =
		menu->split_indices->elements.total - 1;
	uint_fast16_t const index_to_move = menu->split_indices->i[src_i];
	uint_fast16_t const next_to_src_i = src_i + 1;
	uint_fast32_t const recopy_size =
		(next_to_src_i + dst_i + 1) * sizeof(uint16_t);
		
	recopy_inside_memory_space(
		menu->split_indices->i+src_i,
		menu->split_indices->i+next_to_src_i,
		recopy_size
	);
	
	/* Deuxièmement : Écrire l'élement à déplacer dans la zone de
	 *                destination */
	
	menu->split_indices->i[dst_i] = index_to_move;
	
	/* Enfin : Mise à jour des compteurs et rafraichissement du menu */
	menu->split_indices->elements.count[split_array_left]  -= 1;
	menu->split_indices->elements.count[split_array_right] += 1;
	
	program_builder_menu_refresh(menu);
	
invalid_selection:
	return;
}


void program_builder_menu_set_left_indices
(program_builder_menu_t * __restrict const menu,
 uint_fast32_t count)
{
	for (uint_fast32_t i = 0; i < count; i++)
		menu->split_indices->i[i] = i;

	menu->split_indices->elements.count[split_array_left] = count;
	menu->split_indices->elements.count[split_array_right] = 0;
}

/* TODO Program frames should be retrieved automatically... */
/* TODO Remove the glyph_infos requirement ! */
void program_builder_menu_show_with
(struct program_builder_menu * __restrict const menu,
 struct armv7_text_frames * __restrict const program_frames)
{
	menu->frames = program_frames;
	menu->selection_index[split_array_left] = -1;
	menu->selection_index[split_array_right] = -1;
	
	free_temporary_memory(menu->split_indices);
	menu->split_indices = allocate_temporary_memory(
		sizeof(menu->split_indices)
	);
	
	split_array_init(
		(SplitArray *) menu->split_indices,
		program_frames->count,
		sizeof(uint16_t)
	);
	program_builder_menu_set_left_indices(menu, program_frames->count);
	program_builder_menu_refresh(menu);
	enable_swap_menu(menu->swap_menu_template);
}

void program_builder_menu_first_init
(struct program_builder_menu * __restrict const menu,
 swap_menus * __restrict const swap_menu_template)
{
	menu->frames = NULL;
	menu->selection_index[split_array_left]  = -1;
	menu->selection_index[split_array_right] = -1;
	
	menu->split_indices = allocate_temporary_memory(
		sizeof(menu->split_indices)
	);
	menu->swap_menu_template = swap_menu_template;
}

inline static uint_fast16_t
program_builder_menu_chosen_frames_count
(struct program_builder_menu const * __restrict const menu)
{
	return menu->split_indices->elements.count[split_array_right];
}

inline static uint16_t * __restrict
program_builder_menu_chosen_frames_indices
(struct program_builder_menu const * __restrict const menu)
{
	uint_fast16_t right_start =
		menu->split_indices->elements.count[split_array_left];
	return menu->split_indices->i+right_start;
}

inline static void
program_builder_menu_add_chosen_frames_to_text_section
(struct program_builder_menu const * __restrict const menu,
 struct armv7_text_section * __restrict const section)
{
	uint16_t * __restrict const right_frames_indices =
		program_builder_menu_chosen_frames_indices(menu);
	
	uint_fast16_t n_chosen_frames =
		program_builder_menu_chosen_frames_count(menu);
		
	for (uint_fast16_t i = 0; i < n_chosen_frames; i++) {
		armv7_text_section_add_frame(
			section, menu->frames->data[right_frames_indices[i]]
		);
	}
	
	
}

struct fake_string { 
	uint8_t const size;
	uint8_t * __restrict const name;
	uint8_t * __restrict const content;
} const fake_strings[2] = {
	[0] = {
		.size    = sizeof("Hello world\n"),
		.name    = (uint8_t *) "Hello",
		.content = (uint8_t *) "Hello world\n"
	},
	[1] = {
		.size    = sizeof("Meow world\n"),
		.name    = (uint8_t *) "Meow",
		.content = (uint8_t *) "Meow world\n"
	},
};

inline static void prepare_fake_data
(struct data_section * __restrict const data_section)
{
	
	data_section_add(
		data_section, 4, 
		fake_strings[0].size, fake_strings[0].name, fake_strings[0].content
	);
	data_section_add(
		data_section, 4,
		fake_strings[1].size, fake_strings[1].name, fake_strings[1].content
	);
}

void program_builder_menu_generate_executable
(struct program_builder_menu * __restrict const menu)
{
	struct data_section * __restrict const data_section =
		generate_data_section();
	struct armv7_text_section * __restrict const text_section = 
		generate_armv7_text_section();
	
	prepare_fake_data(data_section);
	program_builder_menu_add_chosen_frames_to_text_section(
		menu, text_section
	);
	
	dumbelflib_build_armv7_program(data_section, text_section, "exec");
}
