#pragma once

#include <assert.h>
#include <myy/myy.h>
#include <myy/helpers/buffers.h>

#include <myy/helpers/hitbox_action.h> // box coords

extern buffer_t scratch_buffer[];
extern struct myy_common_data common_graphics_elements;

inline static void assert_not_equal(unsigned int a, unsigned int b)
{
	assert(a != b);
}

inline static void assert_not_null
(uint8_t const * __restrict const addr)
{
	assert(addr != NULL);
}

inline static void assert_equal(unsigned int a, unsigned int b)
{
	assert(a == b);
}

inline static void assert_equal_s32(int32_t a, int32_t b) {
	assert(a == b);
}
inline static void assert_equal_s16(int16_t a, int16_t b) {
	assert(a == b);
}

inline static void assert_true(uint_fast8_t value)
{
	assert(value != 0);
}

inline static void assert_false(uint_fast8_t value)
{
	assert(value == 0);
}

inline static void assert_same_inst_addr(void (*a)(), void (*b)()) {
	assert(a == b);
}

inline static void assert_box_coords_s16
(box_coords_S_t coords,
 int16_t left, int16_t right, int16_t top, int16_t bottom)
{
	assert(coords.left   == left);
	assert(coords.right  == right);
	assert(coords.top    == top);
	assert(coords.bottom == bottom);
}
