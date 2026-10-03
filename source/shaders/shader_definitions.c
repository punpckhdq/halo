/*
SHADER_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "shader_definitions.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "light_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct shader_effect global_shader_effect_additive =
{
	{
		{0},
		{0},
		_shader_type_effect,
		0
	},
	{
		0,
		_shader_framebuffer_blend_function_add,
		_shader_framebuffer_fade_mode_none,
		0,
		{0},
		{
			BITMAP_GROUP_TAG,
			"",
			0,
			NONE
		}
	}
};

struct shader_effect global_shader_effect_alpha_blended =
{
	{
		{0},
		{0},
		_shader_type_effect,
		0
	},
	{
		0,
		_shader_framebuffer_blend_function_alpha_blend,
		_shader_framebuffer_fade_mode_none,
		0,
		{0},
		{
			BITMAP_GROUP_TAG,
			"",
			0,
			NONE
		}
	}
};

/* ---------- public code */

void *shader_get_and_verify_type(
	struct shader const *shader,
	short shader_type)
{
	match_assert("c:\\halo\\SOURCE\\shaders\\shader_definitions.c", 2140, shader);
	match_assert("c:\\halo\\SOURCE\\shaders\\shader_definitions.c", 2141, shader->base.type==shader_type);

	return (void *)shader;
}

/* ---------- private code */
