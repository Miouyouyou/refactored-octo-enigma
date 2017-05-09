#include <myy/helpers/hitbox_action.h>
#include <myy/helpers/memory.h>

#include <tests/common.h>

static volatile uint32_t test = 0;
uint8_t set_test_to_2(position_S rel, position_S abs)
{
	test = 2;
	return 1;
}

uint8_t set_test_to_4(position_S rel, position_S abs)
{
	test = 4;
	return 1;
}

uint8_t set_test_to_rel_add(position_S rel, position_S abs)
{
	test = rel.x + rel.y;
	return 1;
}

uint8_t set_test_to_abs_add(position_S rel, position_S abs)
{
	test = abs.x + abs.y;
	return 1;
}

void test_hitboxes_action_add()
{
	hitboxes_S_t hitboxes = hitboxes_struct(1);
	assert_equal(hitboxes.count, 0);
	assert_equal(hitboxes.max,   1);
	assert_not_null((uint8_t const *) hitboxes.data);
	
	assert_true(hitboxes_S_add(&hitboxes, 10,100,10,100, set_test_to_2));
	assert_equal(hitboxes.count, 1);
	assert_equal(hitboxes.max,   1);
	assert_box_coords_s16(hitboxes.data[0].coords, 10, 100, 10, 100);
	assert_same_inst_addr(
		(void (*)()) hitboxes.data[0].action, (void (*)()) set_test_to_2
	);
	
	assert_true(hitboxes_S_add(&hitboxes, 20,20,20,20, set_test_to_4));
	assert_equal(hitboxes.count, 2);
	assert_equal(hitboxes.max,   2);
	assert_box_coords_s16(hitboxes.data[1].coords, 20, 20, 20, 20);
	assert_same_inst_addr(
		(void (*)()) hitboxes.data[1].action, (void (*)()) set_test_to_4
	);
	
	// When deleting a hitbox from the hitboxes, the hitboxes are
	// repacked together.
	assert_true(
		hitboxes_S_delete(&hitboxes, 10,100,10,100, set_test_to_2)
	);
	assert_equal(hitboxes.count, 1);
	assert_equal(hitboxes.max,   2);
	assert_box_coords_s16(hitboxes.data[0].coords, 20, 20, 20, 20);
	assert_same_inst_addr(
		(void (*)()) hitboxes.data[0].action, (void (*)()) set_test_to_4
	);
	
	// Trying to delete an unknown hitbox should not work.
	assert_false(
		hitboxes_S_delete(&hitboxes, 10,100,10,110, set_test_to_2)
	);
	assert_false(
		hitboxes_S_delete(&hitboxes,  20,20,20,20, set_test_to_2)
	);
	assert_false(
		hitboxes_S_delete(&hitboxes,  20,20,20,19, set_test_to_4)
	);
	assert_false(
		hitboxes_S_delete(&hitboxes, 20,20,-20,20, set_test_to_4)
	);
	assert_false(
		hitboxes_S_delete(&hitboxes,  20,21,20,20, set_test_to_4)
	);
	assert_false(
		hitboxes_S_delete(&hitboxes, -20,20,20,20, set_test_to_4)
	);
	// Still unchanged
	assert_equal(hitboxes.count, 1);
	assert_equal(hitboxes.max,   2);
	assert_box_coords_s16(hitboxes.data[0].coords, 20, 20, 20, 20);
	assert_same_inst_addr(
		(void (*)()) hitboxes.data[0].action, (void (*)()) set_test_to_4
	);
	free_durable_memory(hitboxes.data);
}

/* TODO : Regénération des hitboxes en fonction de la taille de l'écran
 * 
 *  dimensions_hitbox * dimensions_ecran / dimensions_ecran_initiales
 *
 * TODO : Gérer les clics sur les menus
 * TODO : Basculer le tout en OpenGL ES 2.x
 * TODO : Saisie de texte
 * TODO : Parser le code assembleur saisi
 * TODO : Utiliser un assembleur tel que BFD pour convertir le code
 *        assembleur saisi en code machine
 *        OU
 *        Se débrouiller pour que notre propre assembleur génère ce code
 */

void test_hitboxes_action_click
(hitboxes_S_t * __restrict const hitboxes)
{

	assert_equal(hitboxes->count, 0);
	assert_not_null(hitboxes->data);
	test = 0;
	assert_equal(test, 0);
	assert_true(hitboxes_S_add(hitboxes, 20,40,20,40, set_test_to_2));
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(25,25));
	assert_equal(test, 2);
	test = 0;
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(19,19));
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(41,41));
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(19,41));
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(41,19));
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(-25,25));
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(25,-25));
	assert_equal(test, 0);
	
	assert_true(hitboxes_S_add(hitboxes, 0,400,0,400, set_test_to_4));
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(5,5));
	assert_equal(test, 4);
	test = 0;
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(25,25));
	assert_equal(test, 2);
	test = 0;
	assert_equal(test, 0);
	assert_true(hitboxes_S_delete(hitboxes, 20,40,20,40, set_test_to_2));
	hitboxes_action_react_on_click_at(hitboxes, position_S_struct(25,25));
	assert_equal(test, 4);

	assert_true(hitboxes_S_delete(hitboxes, 0, 400, 0, 400, set_test_to_4));
	assert_equal(hitboxes->count, 0);
}

inline static void box_coords_S_rebase_from_1280_720
(box_coords_S_t * coords, unsigned int x_base, unsigned int y_base)
{
	coords->left   = (uint16_t) (coords->left   * x_base / 1280);
	coords->right  = (uint16_t) (coords->right  * x_base / 1280);
	coords->top    = (uint16_t) (coords->top    * y_base / 720);
	coords->bottom = (uint16_t) (coords->bottom * y_base / 720);
}

void test_readapt_hitbox() {
	hitboxes_S_t hitboxes = hitboxes_struct(1);
	hitbox_action_S_t test_hitbox = {{0,1280,0,720},set_test_to_abs_add};
	box_coords_S_rebase_from_1280_720(&test_hitbox.coords, 1920,1080);
	assert_equal(test_hitbox.coords.right,  1920);
	assert_equal(test_hitbox.coords.bottom, 1080);
	assert_true(hitboxes_S_add_copy(&hitboxes, &test_hitbox));
	test = 0;
	assert_equal(test, 0);
	hitboxes_action_react_on_click_at(&hitboxes, position_S_struct(1919,1079));
	assert_equal(test, 1919+1079);
	
}
