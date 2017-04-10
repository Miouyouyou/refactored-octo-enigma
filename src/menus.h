#ifndef MYY_MENUS_H
#define MYY_MENUS_H 1

#include <myy/current/opengl.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/fonts/packed_fonts_display.h>

void prepare_context_menu_with
(GLuint const * __restrict const context_menu_text_buffer,
 struct glyph_infos const * __restrict const myy_glyph_infos,
 uint8_t const * __restrict const * __restrict strings,
 unsigned int n_strings,
 unsigned int strings_vertical_separation_px);

void draw_context_menu
(GLuint const * __restrict const programs,
 GLuint const * __restrict const menu_parts_buffers,
 GLuint const * __restrict const menu_content_buffer);

void enable_context_menu();
void disable_context_menu();
unsigned int manage_current_menu_click(int x, int win_y);

#endif
