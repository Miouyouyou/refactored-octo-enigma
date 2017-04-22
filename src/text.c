#include <src/text.h>
#include <myy/helpers/opengl/quads_structures.h>

void draw_character_quads
(GLuint const buffer_id,
 GLuint const xyz_attr_id, GLuint const st_attr_id,
 GLuint const offset,
 unsigned int n_char_quads)
{
		US_two_tris_quad_3D_draw_pixelscoords(
			buffer_id,
			xyz_attr_id,
			st_attr_id,
			offset,
			n_char_quads
		);
}
