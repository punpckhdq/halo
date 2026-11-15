/*
RASTERIZER_XBOX_WIDGETS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "widgets.h"
#include "rasterizer_widgets.h"

/* ---------- public code */

static boolean rasterizer_widget_project_billboard(
	real_point3d const *point,
	real radius,
	real_point3d *projected_center,
	real_vector2d *projected_axes)
{
	boolean result = FALSE;

	if (radius>0.f)
	{
		real_point3d p_view;
		real x;
		real y;
		real z;
		real w;
		real axis_x;
		real axis_y;
		short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
		short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;

		matrix4x3_transform_point(&global_window_parameters.frustum.world_to_view, point, &p_view);
		x = global_window_parameters.frustum.projection_matrix[0][0] * p_view.x + global_window_parameters.frustum.projection_matrix[1][0] * p_view.y + global_window_parameters.frustum.projection_matrix[2][0] * p_view.z + global_window_parameters.frustum.projection_matrix[3][0];
		y = global_window_parameters.frustum.projection_matrix[0][1] * p_view.x + global_window_parameters.frustum.projection_matrix[1][1] * p_view.y + global_window_parameters.frustum.projection_matrix[2][1] * p_view.z + global_window_parameters.frustum.projection_matrix[3][1];
		z = global_window_parameters.frustum.projection_matrix[0][2] * p_view.x + global_window_parameters.frustum.projection_matrix[1][2] * p_view.y + global_window_parameters.frustum.projection_matrix[2][2] * p_view.z + global_window_parameters.frustum.projection_matrix[3][2];
		w = global_window_parameters.frustum.projection_matrix[0][3] * p_view.x + global_window_parameters.frustum.projection_matrix[1][3] * p_view.y + global_window_parameters.frustum.projection_matrix[2][3] * p_view.z + global_window_parameters.frustum.projection_matrix[3][3];
		axis_x = global_window_parameters.frustum.projection_matrix[0][0] * radius;
		axis_y = global_window_parameters.frustum.projection_matrix[1][1] * radius;

		if (z>0.f)
		{
			real one_over_w = 1.f / w;

			projected_center->x = ((x * one_over_w + 1.f) * viewport_width - 1.f) * 0.5f;
			projected_center->y = ((1.f - y * one_over_w) * viewport_height - 1.f) * 0.5f;
			projected_center->z = MIN(1.f, z * one_over_w);
			projected_axes->i = 0.5f * axis_x * one_over_w * viewport_width;
			projected_axes->j = 0.5f * axis_y * one_over_w * viewport_height;
			result = TRUE;
		}
	}

	return result;
}

void _rasterizer_widget_submit(
	long object_index,
	long widget_index,
	real_point3d const *centroid,
	void (*render_proc)(long, long))
{
	static boolean warned = FALSE;

	if (render_proc)
	{
		struct transparent_geometry_group *group = rasterizer_transparent_geometry_new_group();

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 88, centroid);

		if (group)
		{
			real_plane3d zero_plane;
			real_vector3d forward;

			zero_plane.n.i = 0.f;
			zero_plane.n.j = 0.f;
			zero_plane.n.k = 0.f;
			zero_plane.d = 0.f;

			vector_from_points3d(&global_window_parameters.camera.position, centroid, &forward);
			group->geometry_flags = 0;
			group->object_index = 0;
			group->source_object_index = 0;
			group->shader = NULL;
			group->shader_permutation_index = 0;
			group->effect.type = _render_model_effect_type_none;
			group->dynamic_triangle_buffer_index = NONE;
			group->triangle_buffer = (struct triangle_buffer const *)render_proc;
			group->first_triangle_index = object_index;
			group->triangle_count = widget_index;
			group->dynamic_vertex_buffer_index = NONE;
			group->vertex_buffers = NULL;
			group->lightmap = NULL;
			group->z_sort = -dot_product3d(&global_window_parameters.camera.forward, &forward);
			group->centroid = *centroid;
			group->plane = zero_plane;
			group->model_base_map_scale.j = 1.f;
			group->model_base_map_scale.i = 1.f;
			group->prev_group_presorted_index = NONE;
			group->next_group_presorted_index = NONE;
			group->active_camouflage_transparent_source_object_index = 0;
			group->cortana_hack = FALSE;
			group->node_matrices = NULL;
			group->node_matrix_count = 0;
			group->lighting = NULL;
			group->animation = NULL;
		}
		else if (!warned)
		{
			error(_error_silent, "### ERROR too many transparent geometry groups");
			warned = TRUE;
		}
	}

	return;
}

void _rasterizer_widget_begin(
	short type,
	word flags)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 154, global_d3d_device);

	switch (type)
	{
	case _widget_type_internal_sprite:
		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, TEST_FLAG(flags, _widget_internal_zbuffer_bit));
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, TEST_FLAG(flags, _widget_internal_zbuffer_write_bit));
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_widget_set_tint_factor(1.f);
		rasterizer_set_vertex_shader_permutation(_vertex_shader_lens_flare, _rasterizer_vertex_type_dynamic_unlit, _vertex_shader_permutation_default);

		{
			short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
			short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
			real vsh_constants__screenproj[VSH_CONSTANTS__SCREENPROJ_COUNT][4] =
			{
				{2.f / viewport_width, 0.f, 0.f, -1.f - 1.f / viewport_width},
				{0.f, -2.f / viewport_height, 0.f, 1.f / viewport_height + 1.f},
				{0.f, 0.f, 1.f, 0.f},
				{0.f, 0.f, 0.f, 1.f},
				{0.f, 0.f, 0.f, 1.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(3, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_T0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_V0, PS_REGISTER_T0, PS_REGISTER_V0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_V1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_V0, PS_REGISTER_V1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_V1, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_V0);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);
		break;
	case _widget_type_internal_occlusion_test:
		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, rasterizer_debug_options.lens_flare_occlusion_debug ? D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE : 0);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, rasterizer_debug_options.lens_flare_occlusion_debug);
		SetRenderStateSmart(D3DRS_ZBIAS, rasterizer_debug_options.zbias);

		rasterizer_set_vertex_shader_permutation(_vertex_shader_lens_flare, _rasterizer_vertex_type_dynamic_unlit, _vertex_shader_permutation_default);

		{
			short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
			short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
			real vsh_constants__screenproj[VSH_CONSTANTS__SCREENPROJ_COUNT][4] =
			{
				{2.f / viewport_width, 0.f, 0.f, -1.f - 1.f / viewport_width},
				{0.f, -2.f / viewport_height, 0.f, 1.f / viewport_height + 1.f},
				{0.f, 0.f, 1.f, 0.f},
				{0.f, 0.f, 0.f, 1.f},
				{0.f, 0.f, 0.f, 1.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ONE);
		rasterizer_set_pixel_shader(&pixel_shader);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 266, FALSE, "### ERROR unsupported widget type");
		break;
	}

	return;
}

boolean _rasterizer_widget_set_texture(
	short stage_index,
	long bitmap_group_index,
	short sequence_index)
{
	boolean result;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 285, global_d3d_device);

	if (bitmap_group_index!=NONE)
	{
		result = rasterizer_set_texture_non_blocking(stage_index, _bitmap_type_2d, _bitmap_usage_multiplicative, bitmap_group_index, sequence_index);
	}
	else
	{
		result = rasterizer_set_texture_direct_non_blocking(stage_index, global_rasterizer_data->glow.index, sequence_index);
	}

	if (!result)
	{
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	}

	return result;
}

void _rasterizer_widget_set_tint_factor(
	real tint_factor)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 318, global_d3d_device);

	IDirect3DDevice8_SetVertexData2f(global_d3d_device, D3DVSDE_TEXCOORD1, tint_factor, 0.f);

	return;
}

void _rasterizer_widget_set_zbuffer_enable(
	boolean zbuffer_enable)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 334, global_d3d_device);

	SetRenderStateSmart(D3DRS_ZENABLE, zbuffer_enable);

	return;
}

void _rasterizer_widget_draw_sprite2d(
	real_point2d const *point,
	real radius,
	real_vector2d const *scale,
	real_vector2d const *texture_size,
	real rotation,
	pixel32 color)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 356, point);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 357, global_d3d_device);

	if (radius>0.f)
	{
		real axis_x;
		real axis_y;
		real scale_x;
		real scale_y;
		short texture_width;
		short texture_height;

		if (rotation!=0.f)
		{
			real cos_theta = cosine(rotation);
			real sin_theta = sine(rotation);

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 369, fabs(cos_theta*cos_theta + sin_theta*sin_theta - 1.0f)<_real_epsilon);

			axis_x = cos_theta - sin_theta;
			axis_y = sin_theta + cos_theta;
		}
		else
		{
			axis_y = 1.f;
			axis_x = 1.f;
		}

		if (scale)
		{
			scale_x = scale->i;
			scale_y = scale->j;
		}
		else
		{
			scale_y = 1.f;
			scale_x = 1.f;
		}

		texture_width = texture_size ? (short)texture_size->i : 1;
		texture_height = texture_size ? (short)texture_size->j : 1;

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexDataColor(global_d3d_device, D3DVSDE_TEXCOORD0, color);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,0, 0);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, point->x - axis_x * scale_x, point->y - axis_y * scale_y);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,texture_width, 0);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, axis_y * scale_x + point->x, point->y - axis_x * scale_y);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,texture_width, texture_height);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, axis_x * scale_x + point->x, axis_y * scale_y + point->y);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,0, texture_height);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, point->x - axis_y * scale_x, axis_x * scale_y + point->y);
		IDirect3DDevice8_End(global_d3d_device);
	}

	return;
}

void _rasterizer_widget_draw_sprite3d(
	real_point3d const *point,
	real radius,
	real_vector2d const *scale,
	real rotation,
	pixel32 color)
{
	real_point3d center;
	real_vector2d axes;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 425, point);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 426, global_d3d_device);

	if (radius>0.f && rasterizer_widget_project_billboard(point, radius, &center, &axes))
	{
		real axis_x;
		real axis_y;
		real scale_x;
		real scale_y;

		if (rotation!=0.f)
		{
			real theta = rotation * (_pi / 180.f);
			real cos_theta = cosine(theta);
			real sin_theta = sine(theta);

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 437, fabs(cos_theta*cos_theta + sin_theta*sin_theta - 1.0f)<_real_epsilon);

			axis_x = cos_theta * axes.i - sin_theta * axes.j;
			axis_y = sin_theta * axes.i + cos_theta * axes.j;
		}
		else
		{
			axis_x = axes.i;
			axis_y = axes.j;
		}

		if (scale)
		{
			scale_x = scale->i;
			scale_y = scale->j;
		}
		else
		{
			scale_y = 1.f;
			scale_x = 1.f;
		}

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexDataColor(global_d3d_device, D3DVSDE_TEXCOORD0, color);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,0, 0);
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, center.x - axis_x * scale_x, center.y - axis_y * scale_y, center.z, 1.f);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,1, 0);
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, axis_y * scale_x + center.x, center.y - axis_x * scale_y, center.z, 1.f);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,1, 1);
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, axis_x * scale_x + center.x, axis_y * scale_y + center.y, center.z, 1.f);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR,0, 1);
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, center.x - axis_y * scale_x, axis_x * scale_y + center.y, center.z, 1.f);
		IDirect3DDevice8_End(global_d3d_device);
	}

	return;
}

void _rasterizer_widget_end(
	void)
{
	return;
}

long _rasterizer_widget_submit_occlusion_test(
	real_point3d const *point,
	real radius,
	long index)
{
	long occlusion_pixels;
	boolean success = TRUE;

	if (rasterizer_debug_options.lens_flare_occlusion_enabled)
	{
		real_point3d center;
		real_vector2d axes;

		if (rasterizer_widget_project_billboard(point, radius, &center, &axes))
		{
			rectangle2d bounds;

			axes.i = MAX(1.f, axes.i);
			axes.j = MAX(1.f, axes.j);
			bounds.x0 = (short)fast_ftol((real)floor(PIN(center.x - axes.i, -SHORT_MAX, SHORT_MAX)));
			bounds.y0 = (short)fast_ftol((real)floor(PIN(center.y - axes.j, -SHORT_MAX, SHORT_MAX)));
			bounds.x1 = (short)fast_ftol((real)floor(PIN(center.x + axes.i, -SHORT_MAX, SHORT_MAX)));
			bounds.y1 = (short)fast_ftol((real)floor(PIN(center.y + axes.j, -SHORT_MAX, SHORT_MAX)));
			occlusion_pixels = MAX(0, (bounds.x1 - bounds.x0) * (bounds.y1 - bounds.y0));

			if (occlusion_pixels>0)
			{
				IDirect3DDevice8_BeginVisibilityTest(global_d3d_device);
				IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
				IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x0, bounds.y0, center.z, 1.f);
				IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x1, bounds.y0, center.z, 1.f);
				IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x1, bounds.y1, center.z, 1.f);
				IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x0, bounds.y1, center.z, 1.f);
				IDirect3DDevice8_End(global_d3d_device);
				D3DCALL(success, IDirect3DDevice8_EndVisibilityTest(global_d3d_device, index));
			}
		}
		else
		{
			occlusion_pixels = 0;
		}
	}
	else
	{
		occlusion_pixels = 1;
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_widget_submit_occlusion_test failed");
	}

	return occlusion_pixels;
}

long _rasterizer_widget_get_occlusion_test_result(
	long index)
{
	unsigned long occlusion_test_result = NONE;

	if (rasterizer_debug_options.lens_flare_occlusion_enabled)
	{
		ULONGLONG timestamp;
		HRESULT hr;
		boolean success;

		hr = IDirect3DDevice8_GetVisibilityTestResult(global_d3d_device, index, &occlusion_test_result, &timestamp);

		if (hr==D3DERR_TESTINCOMPLETE)
		{
			rasterizer_spin_begin(_rasterizer_stall_occlusion_test);

			do
			{
				hr = IDirect3DDevice8_GetVisibilityTestResult(global_d3d_device, index, &occlusion_test_result, &timestamp);
			}
			while (hr==D3DERR_TESTINCOMPLETE);

			rasterizer_spin_end();
		}

		success = TRUE;
		D3DCALL(success, hr);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_widgets.c", 574, (occlusion_test_result&0x80000000)==0);

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_widget_get_occlusion_test_result failed");
		}
	}
	else
	{
		occlusion_test_result = 1;
	}

	return occlusion_test_result;
}
