#ifndef MYY_SRC_TEXT_H
#define MYY_SRC_TEXT_H 1

#include <myy/current/opengl.h>

void draw_character_quads
(GLuint const buffer_id,
 GLuint const xyz_attr_id, GLuint const st_attr_id,
 GLuint const offset,
 unsigned int n_char_quads);

#endif
