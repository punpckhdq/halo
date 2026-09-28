/*
SHADERS.H

header included in hcex build.
*/

#ifndef __SHADERS_H
#define __SHADERS_H
#pragma once

/* ---------- headers */

#include "shader_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/SHADERS.C */

short shader_get_vertex_shader_permutation(const struct shader *shader);
boolean shader_is_mirror(struct shader *shader);
boolean shader_is_destructable(struct shader *shader);
boolean shader_is_double_sided(struct shader *shader);
boolean shader_is_decal(struct shader *shader);
boolean shader_is_water_decal(struct shader *shader);
boolean shader_ignores_effect(struct shader *shader);
boolean shader_type_is_transparent(short shader_type);
boolean shader_type_is_lightmapped(short shader_type);
boolean shader_type_is_vertex_lit(short shader_type);
boolean shader_type_is_valid_for_environment(short shader_type);
boolean shader_type_is_valid_for_model(short shader_type);
boolean shader_type_is_valid_for_modifier(short shader_type);
void shader_texture_animation_evaluate(const struct shader_texture_animation *texture_animation, const struct render_animation *render_animation, real u_scale, real v_scale, real u_offset, real v_offset, real r_offset, real time_value, real_vector4d *u_transform_reference, real_vector4d *v_transform_reference);
void shader_environment_texture_animation_evaluate(const struct shader *shader, double time_value, real *u_offset, real *v_offset);
void numeric_countdown_timer_set(long milliseconds, boolean auto_start);
short numeric_countdown_timer_get(short digit_index);
void numeric_countdown_timer_stop(void);
void numeric_countdown_timer_restart(void);
void numeric_countdown_timer_update(void);

/* ---------- globals */

/* ---------- public code */

#endif // __SHADERS_H
