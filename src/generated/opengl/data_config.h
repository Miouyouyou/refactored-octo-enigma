#ifndef MYY_SRC_GENERATED_DATAINFOS
#define MYY_SRC_GENERATED_DATAINFOS 1

#include <myy/current/opengl.h>
#include <src/generated/opengl/shaders_infos.h>

enum myy_current_textures_id {
	glsl_texture_fonts,
	glsl_texture_cursor,
	glsl_texture_menus,
	n_textures_id
};

inline static void glhMatrix4fv
(GLint const id, GLsizei const size, GLboolean const transpose,
 GLfloat * __restrict const values,
 struct glsl_programs_shared_data const * __restrict const programs)
{
	glUniformMatrix4fv(programs->unifs[id], size, transpose, values);
}

inline static void glhUnif1i
(GLint const id, GLint value,
 struct glsl_programs_shared_data const * __restrict const programs)
{
	glUniform1i(programs->unifs[id], value);
}

inline static void glhUnif1f
(GLint const id, GLfloat const val1,
 struct glsl_programs_shared_data const * __restrict const programs)
{
	glUniform1f(programs->unifs[id], val1);
}

inline static void glhUnif2f
(GLint const id, GLfloat const val1, GLfloat const val2,
 struct glsl_programs_shared_data const * __restrict const programs)
{
	glUniform2f(programs->unifs[id], val1, val2);
}

inline static void glhUnif3f
(GLint const id, GLfloat const val1, GLfloat const val2,
 GLfloat const val3,
 struct glsl_programs_shared_data const * __restrict const programs)
{
	glUniform3f(programs->unifs[id], val1, val2, val3);
}

inline static void glhUnif4f
(GLint const id, GLfloat const val1, GLfloat const val2,
 GLfloat const val3, GLfloat const val4,
 struct glsl_programs_shared_data const * __restrict const programs)
{
	glUniform4f(programs->unifs[id], val1, val2, val3, val4);
}

inline static void glhUnif4fv
(GLint const id, GLuint const count,
 GLfloat * __restrict const values,
 struct glsl_programs_shared_data const * __restrict const programs)
{
	glUniform4fv(programs->unifs[id], count, values);
}

void glhShadersPackLoader
(struct glsl_programs_shared_data * __restrict const data);

void glhShadersPackCompileAndLink
(struct glsl_programs_shared_data * __restrict const data);

#endif
