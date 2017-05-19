#include <src/generated/opengl/data_config.h>
#include <myy/helpers/file.h>
#include <myy/helpers/strings.h>
#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/log.h>

void glhShadersPackLoader
(struct glsl_programs_shared_data * __restrict const data)
{
	fh_WholeFileToBuffer("data/shaders.pack", data);
}

#define MYY_GLES2_MAX_SHADERS 2
void glhShadersPackCompileAndLink
(struct glsl_programs_shared_data * __restrict const data)
{
	uint_fast8_t ul = 0;
	for (unsigned int p = 0; p < n_glsl_programs; p++) {
		LOG("---------- PROGRAM %d ----------\n", p);
		enum glsl_shader_name shaders[MYY_GLES2_MAX_SHADERS] = {
			p*MYY_GLES2_MAX_SHADERS, p*MYY_GLES2_MAX_SHADERS+1
		};
		GLuint program = 
			glhCompileProgram(data, MYY_GLES2_MAX_SHADERS, shaders);
		
		struct glsl_elements program_elements = data->metadata[p];
		char const * __restrict current_identifier =
				(char const * __restrict)
				data->identifiers+program_elements.attributes.pos;
		for (unsigned int a = 0; a < program_elements.attributes.n; a++) {
			glBindAttribLocation(program, a, current_identifier);
			sh_pointToNextString(current_identifier);
		}
		if (glhLinkAndSaveProgram(data, p, program)) {
			for (
				uint_fast8_t u = 0; u < program_elements.uniforms.n; u++, ul++
			) {
				data->unifs[ul] = 
					glGetUniformLocation(program, current_identifier);
				sh_pointToNextString(current_identifier);
			}
			glUseProgram(program);
			for (unsigned int a = 0; a < program_elements.attributes.n; a++)
				glEnableVertexAttribArray(a);
		}
		else LOG("Could not link program %u...\n", p);
		LOG("========== PROGRAM %d ==========\n", p);
	}
}
