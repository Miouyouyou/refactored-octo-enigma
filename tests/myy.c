#include <myy/myy.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/log.h>
#include <myy/helpers/opengl/buffers.h>

#include <src/program_builder_menu.h>

#include <tests/tests_defs.h>

#include <assert.h>
#include <GLES3/gl3.h>

uint8_t scratch_buffer[0x4ffffff] __attribute__((aligned(32)));
struct myy_packed_fonts_codepoints codepoints[512] = {0};
struct myy_packed_fonts_glyphdata   glyphdata[512] = {0};
GLuint static_menu_parts_buffer[1];
GLuint context_menu_text_buffer[2];
struct glyph_infos myy_glyph_infos = {
	.stored_codepoints = 0,
	.codepoints_addr = codepoints,
	.glyphdata_addr  = glyphdata
};
struct myy_common_data common_graphics_elements;

void myy_generate_new_state() {}

struct myy_platform_handlers platform = {
	.stop = NULL,
	.stop_data = NULL
};

struct myy_platform_handlers * myy_get_platform_handlers() {
	return &platform;
}
void myy_init() {}

void myy_display_initialised
(unsigned int const width, unsigned int const height)
{

}

//---------------------------------




//---------------------------------

gpu_dumb_3buffs_t test_gpu_buffers;
hitboxes_S_t hitboxes;
void myy_init_drawing()
{
	myy_parse_packed_fonts(&myy_glyph_infos, "data/codepoints.dat");
	common_graphics_elements.fonts_glyphs = &myy_glyph_infos;

	test_gpu_dumb_3buffs();
	hitboxes_S_init(&hitboxes, 1);
}

int i = 0;
void myy_draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.1f,0.2f,0.3f,1.0f);

	test_gpu_dumb_3buffs_store_more();
	test_split_array();
	test_program_generator_menu();
	test_hitboxes_action_add();
	test_hitboxes_action_click(&hitboxes);
	test_readapt_hitbox();
	if (i < 600) i++; else myy_stop();
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

void myy_user_quit() {
	platform.stop(platform.stop_data);
}

void myy_click(int x, int y, unsigned int button) {}
void myy_doubleclick(int x, int y, unsigned int button) {}
void myy_move(int x, int y, int start_x, int start_y) {
}
void myy_hover(int x, int y) {
}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}
void myy_key_release(unsigned int keycode) {}


