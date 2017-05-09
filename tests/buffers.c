#include <assert.h>
#include <myy/myy.h>
#include <myy/helpers/opengl/buffers.h>
#include <myy/helpers/buffers.h>

#include <tests/common.h>

gpu_dumb_3buffs_t test_gpu_buffers;

static void assert_current_id
(gpu_dumb_3buffs_t const * __restrict const buffers,
 uint8_t index)
{
	GLuint bound_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (void *) &bound_buffer);
	assert(buffers->current_index == index);
	assert(
		bound_buffer == gpu_dumb_3buffs_current_buffer_id(buffers)
	);
	assert( bound_buffer == buffers->id[index] );
	
}

inline static void assert_current_buffer_size(GLuint expected_size)
{
	GLuint actual_size;
	glGetBufferParameteriv(
		GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (void *) &actual_size
	);
	assert(actual_size == expected_size);
	
}

static void print_gpu_dumb_3buffs
(gpu_dumb_3buffs_t const * __restrict const buffers)
{
	LOG(
		"[print_buffers]\n  "
		"  id     : %u, %u, %u\n"
		"  points : %d, %d, %d\n"
		"  cindex : %d\n"
		"  max    : %d, %d, %d\n",
		buffers->id[0], buffers->id[1], buffers->id[2],
		buffers->points[0], buffers->points[1], buffers->points[2],
		buffers->current_index,
		buffers->size.max[0], buffers->size.max[1], buffers->size.max[2]
	);
}

struct { uint32_t current, max; } size = {
	.current = 4096, .max = 0xffffff
};

static void store_more_data_in_current_buffer
(gpu_dumb_3buffs_t * __restrict const buffers)
{
	uint32_t new_store_size = size.current * 6 / 4;
	if (new_store_size > size.max) new_store_size = size.max;

	GLuint current_gpu_buffer_size;
	glGetBufferParameteriv(
		GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (void *) &current_gpu_buffer_size
	);
	gpu_dumb_3buffs_store(buffers, new_store_size, scratch_buffer);
	
	GLuint new_gpu_buffer_size;
	glGetBufferParameteriv(
		GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (void *) &new_gpu_buffer_size
	);
	if (new_store_size > current_gpu_buffer_size) {
		GLuint const expected_new_gpu_buffer_size = new_store_size * 2;
		assert(new_gpu_buffer_size == expected_new_gpu_buffer_size);
	}
	else {
		assert(new_gpu_buffer_size == current_gpu_buffer_size);
	}
	size.current = new_store_size;
}


void test_gpu_dumb_3buffs()
{

	assert(test_gpu_buffers.current_index == 0);
	assert(test_gpu_buffers.points[0] == 0);
	assert(test_gpu_buffers.points[1] == 0);
	assert(test_gpu_buffers.points[2] == 0);
	assert(test_gpu_buffers.size.max[0] == 0);
	assert(test_gpu_buffers.size.max[1] == 0);
	assert(test_gpu_buffers.size.max[2] == 0);
	assert(test_gpu_buffers.id[0] == 0);
	assert(test_gpu_buffers.id[1] == 0);
	assert(test_gpu_buffers.id[2] == 0);
	
	gpu_dumb_3buffs_init(&test_gpu_buffers, 4096, GL_DYNAMIC_DRAW);
	assert(test_gpu_buffers.size.max[0] == 4096);
	assert(test_gpu_buffers.size.max[1] == 4096);
	assert(test_gpu_buffers.size.max[2] == 4096);
	assert(test_gpu_buffers.points[0] == 0);
	assert(test_gpu_buffers.points[1] == 0);
	assert(test_gpu_buffers.points[2] == 0);
	assert(test_gpu_buffers.id[0] != 0);
	assert(test_gpu_buffers.id[1] != 0);
	assert(test_gpu_buffers.id[2] != 0);
	assert(test_gpu_buffers.current_index == 0);
	
	gpu_dumb_3buffs_bind(&test_gpu_buffers);
	assert_current_id(&test_gpu_buffers, 0);
	assert_current_buffer_size(4096);
	gpu_dumb_3buffs_bind_next_buffer(&test_gpu_buffers);
	assert_current_id(&test_gpu_buffers, 1);
	assert_current_buffer_size(4096);
	gpu_dumb_3buffs_bind_next_buffer(&test_gpu_buffers);
	assert_current_id(&test_gpu_buffers, 2);
	assert_current_buffer_size(4096);
	gpu_dumb_3buffs_bind_next_buffer(&test_gpu_buffers);
	assert_current_id(&test_gpu_buffers, 0);
	assert_current_buffer_size(4096);
	
}

void test_gpu_dumb_3buffs_store_more()
{
	gpu_dumb_3buffs_bind_next_buffer(&test_gpu_buffers);
	store_more_data_in_current_buffer(&test_gpu_buffers);
}
