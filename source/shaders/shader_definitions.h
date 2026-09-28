/*
SHADER_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __SHADER_DEFINITIONS_H
#define __SHADER_DEFINITIONS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_shader_framebuffer_blend_function_alpha_blend = 0,
	_shader_framebuffer_blend_function_multiply,
	_shader_framebuffer_blend_function_double_multiply,
	_shader_framebuffer_blend_function_add,
	_shader_framebuffer_blend_function_reverse_subtract,
	_shader_framebuffer_blend_function_min,
	_shader_framebuffer_blend_function_max,
	_shader_framebuffer_blend_function_alpha_multiply_add,
	NUMBER_OF_SHADER_FRAMEBUFFER_BLEND_FUNCTIONS
};

/* ---------- macros */

/* ---------- structures */

struct shader_radiosity_properties
{
	unsigned short flags;
	short detail_level;
	real power;
	real_rgb_color color;
	real_rgb_color tint_color;
};

struct shader_physics_properties
{
	unsigned short flags;
	short material_type;
};

struct _shader
{
	struct shader_radiosity_properties radiosity;
	struct shader_physics_properties physics;
	short type;
	unsigned short pad;
};

struct _shader_decal
{
	unsigned short flags;
	short type;
	short framebuffer_blend_function;
	unsigned short pad1;
	long unused1[5];
	struct tag_reference map;
	long unused2[5];
};

struct shader_decal
{
	struct _shader shader;
	struct _shader_decal decal;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __SHADER_DEFINITIONS_H
