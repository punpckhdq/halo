/*
HUD_DRAW.C
*/

/* ---------- headers */

#include "cseries.h"
#include "meter_definitions.h"
#include "hud.h"
#include "hud_messaging.h"
#include "weapon_hud_interface_definition.h"
#include "motion_sensor.h"
#include "interface_panels.h"
#include "inventory_displays.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "bitmaps_inlines.h"
#include "network_messages.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
#include "director.h"
#include "weapons.h"
#include "texture_cache.h"
#include "font_group.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static real_rectangle2d const *get_sprite_clip_rect(long bitmap_index, short sequence_index, short frame);
static void hud_draw_bitmap_with_meter(struct rasterizer_meter_parameters *meter_parameters, struct bitmap_data const *bitmap, struct hud_absolute_placement_definition const *absolute_placement, struct hud_placement_definition const *placement, real_rectangle2d const *clip, real scale, real theta, unsigned long color32, boolean in_multiplayer, boolean is_interface_bitmap, boolean is_crosshair_bitmap);
static void hud_calculate_bitmap_bounds(struct bitmap_data const *bitmap, short placement_type, real_rectangle2d const *clip, real_rectangle2d *bounds, boolean is_interface_bitmap);
static void hud_draw_bitmap_internal(struct rasterizer_meter_parameters *meter_parameters, struct bitmap_data const *bitmap, point2d const *point, real_rectangle2d const *clip, real_rectangle2d const *bounds, real_vector2d const *xy_scale, real theta, unsigned long color);
static boolean hud_draw_multitexture_overlay_get_current_weapon_definition(struct player_datum const *player, struct weapon_interface_state *weapon_state);
static void hud_draw_multitexture_overlay(struct multitexture_overlay_hud_element_definition const *overlay, short local_player_index, point2d const *point, real_rectangle2d const *clip, real_rectangle2d const *bounds, real_vector2d const *xy_scale, real theta, unsigned long color);

/* ---------- globals */

/* ---------- public code */

static real_rectangle2d const *get_sprite_clip_rect(
	long bitmap_index,
	short sequence_index,
	short frame)
{
	real_rectangle2d const *result = NULL;
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	if (bitmap_index != NONE && sequence_index != NONE && frame != NONE)
	{
		struct bitmap_group *group = bitmap_group_get(bitmap_index);

		if (sequence_index < group->sequences.count)
		{
			struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index, struct bitmap_group_sequence);

			if (sequence->sprites.count)
			{
				result = &TAG_BLOCK_GET_ELEMENT(&sequence->sprites, frame % sequence->sprites.count, struct bitmap_group_sprite)->bounds;
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 100);
	}

	return result;
}

real hud_globals_get_scale(
	boolean in_multiplayer)
{
	return 1.f;
}

void hud_calculate_point(
	short local_player_index,
	struct hud_absolute_placement_definition const *absolute_placement,
	struct hud_placement_definition const *placement,
	struct bitmap_data const *bitmap_data,
	boolean in_multiplayer,
	real override_scale,
	point2d *result)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	real scale;
	real_point2d temp_result;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	if (!in_multiplayer || override_scale == 0.f)
	{
		scale = hud_globals_get_scale(in_multiplayer && !TEST_FLAG(placement->multiplayer_scaling_flags, _hud_dont_scale_offset_bit));
	}
	else
	{
		scale = override_scale;
	}

	match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 125, render.local_player_index==local_player_index);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 126, absolute_placement);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 127, placement);

	if (absolute_placement->corner < _hud_center)
	{
		temp_result.x = (real)(placement->offset.x * (TEST_FLAG(absolute_placement->corner, 0) ? -1 : 1)) * scale + render.camera.window_bounds.v[((absolute_placement->corner & FLAG(0)) << 1) | 1] - render.camera.viewport_bounds.x0;
		temp_result.y = (real)(placement->offset.y * (TEST_FLAG(absolute_placement->corner, 1) ? -1 : 1)) * scale + render.camera.window_bounds.v[absolute_placement->corner & FLAG(1)] - render.camera.viewport_bounds.y0;
	}
	else
	{
		point2d origin;

		origin.x = (render.camera.window_bounds.x1 + render.camera.window_bounds.x0) / 2;
		origin.y = (render.camera.window_bounds.y1 + render.camera.window_bounds.y0) / 2;
		temp_result.x = (real)(origin.x - render.camera.viewport_bounds.x0) + placement->offset.x * scale;
		temp_result.y = (real)(origin.y - render.camera.viewport_bounds.y0) + placement->offset.y * scale;
	}

	if (bitmap_data)
	{
		switch (absolute_placement->corner)
		{
		case _hud_top_left:
			temp_result.x += bitmap_data->registration_point.x * scale;
			temp_result.y += bitmap_data->registration_point.y * scale;
			break;
		case _hud_top_right:
			temp_result.x += (bitmap_data->registration_point.x - bitmap_data->width) * scale;
			temp_result.y += bitmap_data->registration_point.y * scale;
			break;
		case _hud_bottom_left:
			temp_result.x += bitmap_data->registration_point.x * scale;
			temp_result.y += (bitmap_data->registration_point.y - bitmap_data->height) * scale;
			break;
		case _hud_bottom_right:
			temp_result.x += (bitmap_data->registration_point.x - bitmap_data->width) * scale;
			temp_result.y += (bitmap_data->registration_point.y - bitmap_data->height) * scale;
			break;
		case _hud_center:
			temp_result.x += (bitmap_data->registration_point.x + bitmap_data->width / 2) * scale;
			temp_result.y += (bitmap_data->registration_point.y + bitmap_data->width / 2) * scale;
			break;
		default:
			match_unreachable("c:\\halo\\SOURCE\\interface\\hud_draw.c", 174);
			break;
		}
	}

	result->x = (short)fast_ftol(temp_result.x);
	result->y = (short)fast_ftol(temp_result.y);

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 181);
	}

	return;
}

void hud_retrieve_bitmap_and_bounding_rect(
	long bitmap_group_index,
	short sequence_index,
	short frame_index,
	struct bitmap_data const **bitmap,
	real_rectangle2d const **clip)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));
	match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 193, bitmap);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 194, clip);

	if (bitmap_group_index != NONE)
	{
		struct bitmap_group *group = bitmap_group_get(bitmap_group_index);

		if (sequence_index < group->sequences.count)
		{
			struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index, struct bitmap_group_sequence);

			frame_index &= MASK(15);
			match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 205, frame_index >= 0);

			if (sequence->sprites.count)
			{
				struct bitmap_group_sprite *sprite = TAG_BLOCK_GET_ELEMENT(&sequence->sprites, frame_index % sequence->sprites.count, struct bitmap_group_sprite);

				*bitmap = TAG_BLOCK_GET_ELEMENT(&group->bitmaps, sprite->bitmap_index, struct bitmap_data);
			}
			else
			{
				*bitmap = bitmap_group_get_bitmap_from_sequence(bitmap_group_index, sequence_index, frame_index);
			}
		}
	}

	if (*bitmap)
	{
		*clip = get_sprite_clip_rect(bitmap_group_index, sequence_index, frame_index);
	}
	else
	{
		*clip = NULL;
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 228);
	}

	return;
}

long get_flash_duration(
	struct hud_color_definition const *hud_color)
{
	return fast_ftol(hud_color->flash_period * TICKS_PER_SECOND);
}

unsigned long get_flash_color(
	struct hud_color_definition const *hud_color_def,
	long reference_value)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	real_argb_color out_color;
	real_argb_color hud_color;
	real_argb_color hud_flash_color;
	real flash_time;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	flash_time = (real)fmod((game_time_get() - reference_value) * SECONDS_PER_TICK, hud_color_def->flash_period);
	pixel32_to_real_argb_color(hud_color_def->color, &hud_color);
	pixel32_to_real_argb_color(hud_color_def->flash_color, &hud_flash_color);

	if (flash_time < hud_color_def->number_of_flashes * (hud_color_def->flash_delay + hud_color_def->flash_length))
	{
		real time = (real)fmod(flash_time, hud_color_def->flash_delay + hud_color_def->flash_length);

		if (!reference_value)
		{
			if (TEST_FLAG(hud_color_def->flash_flags, _hud_flash_reverse_colors_bit))
			{
				out_color = hud_color;
			}
			else
			{
				out_color = hud_flash_color;
			}
		}
		else
		{
			if (time < hud_color_def->flash_length)
			{
				real fraction = (real)(1.0 - (cosine(time / hud_color_def->flash_length * 6.283f) + 1.0) / 2.0);

				if (fraction < 0.f)
				{
					fraction = 0.f;
				}

				if (fraction > 1.f)
				{
					fraction = 1.f;
				}

				fraction = square_root(fraction);

				if (TEST_FLAG(hud_color_def->flash_flags, _hud_flash_reverse_colors_bit))
				{
					vectors_interpolate(
						(real_vector3d const *)&hud_flash_color,
						(real_vector3d const *)&hud_color,
						fraction,
						(real_vector3d *)&out_color);
					scalars_interpolate(hud_flash_color.blue, hud_color.blue, fraction, &out_color.blue);
				}
				else
				{
					vectors_interpolate(
						(real_vector3d const *)&hud_color,
						(real_vector3d const *)&hud_flash_color,
						fraction,
						(real_vector3d *)&out_color);
					scalars_interpolate(hud_color.blue, hud_flash_color.blue, fraction, &out_color.blue);
				}
			}
			else if (TEST_FLAG(hud_color_def->flash_flags, _hud_flash_reverse_colors_bit))
			{
				out_color = hud_flash_color;
			}
			else
			{
				out_color = hud_color;
			}
		}
	}
	else
	{
		out_color = hud_color;
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 311);
	}

	return real_argb_color_to_pixel32(&out_color);
}

void hud_draw_meter(
	short local_player_index,
	struct hud_absolute_placement_definition const *placement,
	struct meter_hud_element_definition const *meter,
	byte min_value,
	byte max_value,
	short draw_flags,
	real reference_time,
	real reference_value)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct bitmap_group *bitmap_group;
	struct bitmap_data *bitmap;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	bitmap_group = bitmap_group_get(verify_tag_reference(&meter->meter_bitmap));
	bitmap = bitmap_group_get_bitmap_from_sequence(meter->meter_bitmap.index, meter->sequence_index, 0);

	if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
	{
		struct rasterizer_meter_parameters meter_parameters;
		real_rectangle2d const *clip = get_sprite_clip_rect(verify_tag_reference(&meter->meter_bitmap), meter->sequence_index, 0);
		real scale = hud_globals_get_scale(TEST_FLAG(draw_flags, _hud_draw_in_multiplayer_bit) && !TEST_FLAG(meter->placement.multiplayer_scaling_flags, _hud_dont_scale_offset_bit));
		boolean is_interface_bitmap = bitmap_group->type == _bitmap_group_type_interface_bitmaps;
		short min_alpha = MAX(meter->minimum_value, PIN(fast_ftol_C((real)(meter->alpha_multiplier * min_value + meter->alpha_bias)), 0, UNSIGNED_CHAR_MAX));
		short max_alpha = MAX(meter->minimum_value, PIN(fast_ftol_C((real)(meter->alpha_multiplier * max_value + meter->alpha_bias)), 0, UNSIGNED_CHAR_MAX));

		if (TEST_FLAG(draw_flags, _hud_draw_disabled_bit))
		{
			meter_parameters.gradient_min_color = 0;
			meter_parameters.gradient_max_color = 0;
			meter_parameters.flash_color = 0;
		}
		else if (!TEST_FLAG(meter->meter_flags, _hud_meter_switch_color_on_state_change))
		{
			real fade = reference_time < 0.f ? 0.f : PIN(1.f - reference_time, 0.f, 1.f);
			real_rgb_color flash_color;

			pixel32_to_real_rgb_color(meter->flash_color, &flash_color);
			flash_color.red *= fade;
			flash_color.green *= fade;
			flash_color.blue *= fade;
			meter_parameters.gradient_min_color = (min_alpha << 24) | (meter->min_color & 0xFFFFFF);
			meter_parameters.gradient_max_color = meter->max_color & 0xFFFFFF;
			meter_parameters.flash_color = (max_alpha << 24) | (real_rgb_color_to_pixel32(&flash_color) & 0xFFFFFF);
		}
		else if (TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
		{
			if (TEST_FLAG(meter->meter_flags, _hud_meter_interpolates_between_min_max_bit))
			{
				real_rgb_color min_color;
				real_rgb_color max_color;
				real_rgb_color color;
				real interpolation_value;

				pixel32_to_real_rgb_color(meter->min_color, &min_color);
				pixel32_to_real_rgb_color(meter->max_color, &max_color);

				if (TEST_FLAG(meter->meter_flags, _hud_meter_invert_interpolation_value_bit))
				{
					interpolation_value = 1.f - reference_value;
				}
				else
				{
					interpolation_value = reference_value;
				}

				rgb_colors_interpolate(&color, 0, &min_color, &max_color, interpolation_value);
				meter_parameters.gradient_min_color = (min_alpha << 24) | real_rgb_color_to_pixel32(&color);
				meter_parameters.gradient_max_color = real_rgb_color_to_pixel32(&color);
				meter_parameters.flash_color = min_alpha << 24;
			}
			else
			{
				meter_parameters.gradient_min_color = (min_alpha << 24) | (meter->max_color & 0xFFFFFF);
				meter_parameters.gradient_max_color = meter->max_color & 0xFFFFFF;
				meter_parameters.flash_color = min_alpha << 24;
			}
		}
		else
		{
			meter_parameters.gradient_min_color = (min_alpha << 24) | (meter->min_color & 0xFFFFFF);
			meter_parameters.gradient_max_color = meter->min_color & 0xFFFFFF;
			meter_parameters.flash_color = min_alpha << 24;
		}

		meter_parameters.background_color = (meter->empty_color & 0xFFFFFF) | ((UNSIGNED_CHAR_MAX - (meter->empty_color >> 24)) << 24);
		meter_parameters.tint_color = real_alpha_intensity_to_pixel32(meter->fade, 1.f - meter->opacity);
		meter_parameters.gradient = 1.f;
		meter_parameters.flash_color_is_negative = FALSE;
		meter_parameters.tint_mode_2 = TRUE;
		hud_draw_bitmap_with_meter(
			&meter_parameters,
			bitmap,
			placement,
			&meter->placement,
			clip,
			scale,
			0.f,
			0xFFFFFFFF,
			TEST_FLAG(draw_flags, _hud_draw_in_multiplayer_bit),
			is_interface_bitmap,
			FALSE);
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 428);
	}

	return;
}

void hud_draw_numbers(
	short local_player_index,
	struct hud_absolute_placement_definition const *placement,
	struct number_hud_element_definition const *numbers,
	short value,
	short decimal_value,
	short draw_flags,
	long flash_reference_time,
	real override_scale)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	long hud_number_index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	hud_number_index = interface_get_tag_index(_interface_hud_digits);

	if (hud_number_index != NONE)
	{
		struct hud_number_definition *hud_number = hud_number_definition_get(hud_number_index);
		struct bitmap_group *bitmap_group = bitmap_group_get(hud_number->number_bitmap.index);
		struct bitmap_data *source_bitmap = bitmap_group_get_bitmap_from_sequence(hud_number->number_bitmap.index, 0, 0);
		boolean kilometers = value > 999;
		point2d corner;
		point2d cursor;
		short i;

		if (_texture_cache_bitmap_get_hardware_format(source_bitmap, FALSE, TRUE))
		{
			boolean negative = value < 0;
			real digit_count = (real)(numbers->digits + (numbers->fractional_digits && decimal_value != NONE ? MIN(numbers->fractional_digits, MAXIMUM_NUMBER_OF_HUD_DECIMAL_DIGITS) + 1 : 0));
			real decimal_point_width = (real)(numbers->fractional_digits ? hud_number->decimal_point_width : 0);
			real scale;

			if (override_scale > 0.f)
			{
				scale = override_scale;
			}
			else
			{
				scale = hud_globals_get_scale(TEST_FLAG(draw_flags, _hud_draw_in_multiplayer_bit) && !TEST_FLAG(numbers->placement.multiplayer_scaling_flags, _hud_dont_scale_offset_bit));
			}

			if (TEST_FLAG(numbers->number_flags, _hud_number_show_trailing_m))
			{
				digit_count += 1.f;

				if (kilometers)
				{
					decimal_value = value * 10;
					value /= 1000;
				}
			}

			value = abs(value);
			hud_calculate_point(
				local_player_index,
				placement,
				&numbers->placement,
				NULL,
				TEST_FLAG(draw_flags, _hud_draw_in_multiplayer_bit),
				0.f,
				&corner);

			switch (placement->corner)
			{
			case _hud_top_left:
			case _hud_bottom_left:
				cursor.x = (short)(corner.x + ((digit_count - 2.f) * hud_number->screen_width + decimal_point_width) * scale);
				break;
			case _hud_center:
				cursor.x = (short)(corner.x + ((digit_count - 1.f) * hud_number->screen_width + decimal_point_width) * scale * 0.5f);
				break;
			default:
				match_unreachable("c:\\halo\\SOURCE\\interface\\hud_draw.c", 493);
			case _hud_top_right:
			case _hud_bottom_right:
				cursor.x = corner.x;
				break;
			}

			if (source_bitmap)
			{
				unsigned long color;

				if (TEST_FLAG(draw_flags, _hud_draw_disabled_bit))
				{
					color = numbers->colors.disabled_color;
				}
				else if (TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
				{
					color = get_flash_color(&numbers->colors, flash_reference_time);
				}
				else
				{
					color = numbers->colors.color;
				}

				if (TEST_FLAG(numbers->number_flags, _hud_number_show_trailing_m))
				{
					point2d point;
					struct bitmap_data const *number_bitmap;
					real_rectangle2d const *clip;

					point.x = cursor.x;
					point.y = corner.y;
					number_bitmap = NULL;
					clip = NULL;
					hud_retrieve_bitmap_and_bounding_rect(
						hud_number->number_bitmap.index,
						0,
						kilometers ? _hud_number_kilometers_index : _hud_number_meters_index,
						&number_bitmap,
						&clip);
					match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 515, source_bitmap==number_bitmap);
					hud_draw_bitmap_direct(
						number_bitmap,
						placement->corner,
						&point,
						clip,
						scale,
						0.f,
						color,
						bitmap_group->type == _bitmap_group_type_interface_bitmaps);
					cursor.x = (short)(cursor.x - hud_number->screen_width * scale);
				}

				if (numbers->fractional_digits && decimal_value >= 0)
				{
					short fractional_digits = MIN(numbers->fractional_digits, MAXIMUM_NUMBER_OF_HUD_DECIMAL_DIGITS);

					for (i = fractional_digits; i < MAXIMUM_NUMBER_OF_HUD_DECIMAL_DIGITS; i++)
					{
						decimal_value /= 10;
					}

					for (i = 0; i < fractional_digits; i++)
					{
						short digit = decimal_value % 10;
						point2d point;
						struct bitmap_data const *number_bitmap;
						real_rectangle2d const *clip;

						point.x = cursor.x;
					point.y = corner.y;
						number_bitmap = NULL;
						clip = NULL;
						hud_retrieve_bitmap_and_bounding_rect(
							hud_number->number_bitmap.index,
							0,
							digit,
							&number_bitmap,
							&clip);
						match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 539, source_bitmap==number_bitmap);
						hud_draw_bitmap_direct(
							number_bitmap,
							placement->corner,
							&point,
							clip,
							scale,
							0.f,
							color,
							bitmap_group->type == _bitmap_group_type_interface_bitmaps);
						cursor.x = (short)(cursor.x - hud_number->screen_width * scale);
						decimal_value /= 10;
					}

					cursor.x = (short)(cursor.x + hud_number->screen_width * scale);
					cursor.x = (short)(cursor.x - hud_number->decimal_point_width * scale);
					{
						short digit = _hud_number_decimal_index;
						point2d point;
						struct bitmap_data const *number_bitmap;
						real_rectangle2d const *clip;

						point.x = cursor.x;
					point.y = corner.y;
						number_bitmap = NULL;
						clip = NULL;
						hud_retrieve_bitmap_and_bounding_rect(
							hud_number->number_bitmap.index,
							0,
							digit,
							&number_bitmap,
							&clip);
						match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 556, source_bitmap==number_bitmap);
						hud_draw_bitmap_direct(
							number_bitmap,
							placement->corner,
							&point,
							clip,
							scale,
							0.f,
							color,
							bitmap_group->type == _bitmap_group_type_interface_bitmaps);
						cursor.x = (short)(cursor.x - hud_number->screen_width * scale);
					}
				}

				for (i = 0; i < numbers->digits; i++)
				{
					short digit = value % 10;
					point2d point;
					struct bitmap_data const *number_bitmap;
					real_rectangle2d const *clip;

					if (!value && !TEST_FLAG(numbers->number_flags, _hud_number_show_all_leading_zeros_bit))
					{
						break;
					}

					point.x = cursor.x;
					point.y = corner.y;
					number_bitmap = NULL;
					clip = NULL;
					hud_retrieve_bitmap_and_bounding_rect(
						hud_number->number_bitmap.index,
						0,
						digit,
						&number_bitmap,
						&clip);
					match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 575, source_bitmap==number_bitmap);
					hud_draw_bitmap_direct(
						number_bitmap,
						placement->corner,
						&point,
						clip,
						scale,
						0.f,
						color,
						bitmap_group->type == _bitmap_group_type_interface_bitmaps);
					cursor.x = (short)(cursor.x - hud_number->screen_width * scale);
					value /= 10;
				}

				if (negative)
				{
					point2d point;
					struct bitmap_data const *number_bitmap;
					real_rectangle2d const *clip;

					point.x = cursor.x;
					point.y = corner.y;
					number_bitmap = NULL;
					clip = NULL;
					hud_retrieve_bitmap_and_bounding_rect(
						hud_number->number_bitmap.index,
						0,
						_hud_number_negative_sign_index,
						&number_bitmap,
						&clip);
					match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 595, source_bitmap==number_bitmap);
					hud_draw_bitmap_direct(
						number_bitmap,
						placement->corner,
						&point,
						clip,
						scale,
						0.f,
						color,
						bitmap_group->type == _bitmap_group_type_interface_bitmaps);
				}
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 602);
	}

	return;
}

void hud_draw_static_element(
	short local_player_index,
	struct hud_absolute_placement_definition const *placement,
	struct static_hud_element_definition const *static_element,
	short draw_flags,
	long flash_reference_time)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct bitmap_group *bitmap_group;
	struct bitmap_data *bitmap;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	bitmap_group = bitmap_group_get(verify_tag_reference(&static_element->interface_bitmap));
	bitmap = bitmap_group_get_bitmap_from_sequence(static_element->interface_bitmap.index, static_element->sequence_index, 0);

	if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
	{
		real_rectangle2d const *clip = get_sprite_clip_rect(verify_tag_reference(&static_element->interface_bitmap), static_element->sequence_index, 0);
		unsigned long color;
		real scale;
		boolean is_interface_bitmap;
		short overlay_index;

		if (TEST_FLAG(draw_flags, _hud_draw_disabled_bit))
		{
			color = static_element->colors.disabled_color;
		}
		else if (TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
		{
			color = get_flash_color(&static_element->colors, flash_reference_time);
		}
		else
		{
			color = static_element->colors.color;
		}

		scale = hud_globals_get_scale(TEST_FLAG(draw_flags, _hud_draw_in_multiplayer_bit) && !TEST_FLAG(static_element->placement.multiplayer_scaling_flags, _hud_dont_scale_offset_bit));
		is_interface_bitmap = bitmap_group->type == _bitmap_group_type_interface_bitmaps;
		hud_draw_bitmap_with_meter(
			NULL,
			bitmap,
			placement,
			&static_element->placement,
			clip,
			scale,
			0.f,
			color,
			TEST_FLAG(draw_flags, _hud_draw_in_multiplayer_bit),
			is_interface_bitmap,
			FALSE);

		for (overlay_index = 0; overlay_index < static_element->multitexture_overlays.count; overlay_index++)
		{
			struct multitexture_overlay_hud_element_definition *overlay = TAG_BLOCK_GET_ELEMENT(&static_element->multitexture_overlays, overlay_index, struct multitexture_overlay_hud_element_definition);
			real_rectangle2d default_clip;
			real_vector2d xy_scale;
			real_rectangle2d local_box;
			point2d point;

			default_clip.x0 = 0.f;
			default_clip.x1 = 1.f;
			default_clip.y0 = 0.f;
			default_clip.y1 = 1.f;

			if (is_interface_bitmap)
			{
				default_clip.x1 *= bitmap->width;
				default_clip.y1 *= bitmap->height;
			}

			if (!clip)
			{
				clip = &default_clip;
			}

			scale_vector2d(&static_element->placement.scale, scale, &xy_scale);
			hud_calculate_point(
				render.local_player_index,
				placement,
				&static_element->placement,
				NULL,
				TEST_FLAG(draw_flags, _hud_draw_in_multiplayer_bit) && !TEST_FLAG(static_element->placement.multiplayer_scaling_flags, _hud_dont_scale_offset_bit),
				0.f,
				&point);
			hud_calculate_bitmap_bounds(bitmap, placement->corner, clip, &local_box, is_interface_bitmap);
			hud_draw_multitexture_overlay(overlay, local_player_index, &point, clip, &local_box, &xy_scale, 0.f, color);
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 685);
	}

	return;
}

void hud_draw_weapon_overlays(
	short local_player_index,
	struct hud_absolute_placement_definition const *placement,
	struct weapon_hud_overlay_definition const *overlays,
	long type_flags,
	long reference_time,
	short draw_flags,
	boolean in_multiplayer)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	long item_index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	for (item_index = 0; item_index < overlays->items.count; item_index++)
	{
		struct weapon_hud_overlay_item *item = TAG_BLOCK_GET_ELEMENT(&overlays->items, item_index, struct weapon_hud_overlay_item);

		if (!TEST_FLAG(item->flags, _hud_overlay_runtime_invalid_bit) && (item->type & type_flags))
		{
			real scale = hud_globals_get_scale(in_multiplayer && !TEST_FLAG(item->placement.multiplayer_scaling_flags, _hud_dont_scale_offset_bit));
			struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(overlays->bitmap.index)->sequences, item->sequence_index, struct bitmap_group_sequence);
			short frame_index;
			unsigned long color;
			struct bitmap_data const *source_bitmap;
			real_rectangle2d const *clip;

			if (TEST_FLAG(item->flags, _hud_overlay_flashes_bit) && TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
			{
				color = get_flash_color(&item->colors, reference_time);
			}
			else
			{
				color = item->colors.color;
			}

			if (TEST_FLAG(item->flags, _hud_overlay_flashes_bit) && TEST_FLAG(draw_flags, _hud_draw_flashing_bit) && item->frame_rate > 0)
			{
				frame_index = (short)((game_time_get() - reference_time) / item->frame_rate / TICKS_PER_SECOND % sequence->sprites.count);
			}
			else
			{
				frame_index = 0;
			}

			source_bitmap = NULL;
			clip = NULL;
			hud_retrieve_bitmap_and_bounding_rect(
				overlays->bitmap.index,
				item->sequence_index,
				frame_index,
				&source_bitmap,
				&clip);

			if (source_bitmap)
			{
				if (_texture_cache_bitmap_get_hardware_format(source_bitmap, FALSE, TRUE))
				{
					hud_draw_bitmap_with_meter(
						NULL,
						source_bitmap,
						placement,
						&item->placement,
						clip,
						scale,
						0.f,
						color,
						in_multiplayer,
						FALSE,
						FALSE);
				}
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 748);
	}

	return;
}

void hud_draw_bitmap(
	struct bitmap_data const *bitmap,
	struct hud_absolute_placement_definition const *absolute_placement,
	struct hud_placement_definition const *placement,
	real_rectangle2d const *clip,
	real scale,
	real theta,
	unsigned long color,
	boolean in_multiplayer,
	boolean is_interface_bitmap,
	boolean is_crosshair_bitmap)
{
	hud_draw_bitmap_with_meter(
		NULL,
		bitmap,
		absolute_placement,
		placement,
		clip,
		scale,
		theta,
		color,
		in_multiplayer,
		is_interface_bitmap,
		is_crosshair_bitmap);

	return;
}

static void hud_draw_bitmap_with_meter(
	struct rasterizer_meter_parameters *meter_parameters,
	struct bitmap_data const *bitmap,
	struct hud_absolute_placement_definition const *absolute_placement,
	struct hud_placement_definition const *placement,
	real_rectangle2d const *clip,
	real scale,
	real theta,
	unsigned long color32,
	boolean in_multiplayer,
	boolean is_interface_bitmap,
	boolean is_crosshair_bitmap)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	real_rectangle2d default_clip;
	real_rectangle2d local_box;
	real_vector2d xy_scale;
	point2d point;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	default_clip.x0 = 0.f;
	default_clip.x1 = 1.f;
	default_clip.y0 = 0.f;
	default_clip.y1 = 1.f;

	if (is_interface_bitmap)
	{
		default_clip.x1 *= bitmap->width;
		default_clip.y1 *= bitmap->height;
	}

	if (!clip)
	{
		clip = &default_clip;
	}

	scale_vector2d(&placement->scale, scale, &xy_scale);
	hud_calculate_point(
		render.local_player_index,
		absolute_placement,
		placement,
		NULL,
		in_multiplayer && !TEST_FLAG(placement->multiplayer_scaling_flags, _hud_dont_scale_offset_bit),
		0.f,
		&point);
	hud_calculate_bitmap_bounds(bitmap, absolute_placement->corner, clip, &local_box, is_interface_bitmap);
	hud_draw_bitmap_internal(meter_parameters, bitmap, &point, clip, &local_box, &xy_scale, theta, color32);

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 814);
	}

	return;
}

void hud_draw_bitmap_direct(
	struct bitmap_data const *bitmap,
	short placement,
	point2d const *point,
	real_rectangle2d const *clip,
	real scale,
	real theta,
	unsigned long color,
	boolean is_interface_bitmap)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	real_rectangle2d default_clip;
	real_rectangle2d local_box;
	real_vector2d xy_scale;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	default_clip.x0 = 0.f;
	default_clip.x1 = 1.f;
	default_clip.y0 = 0.f;
	default_clip.y1 = 1.f;

	if (is_interface_bitmap)
	{
		default_clip.x1 *= bitmap->width;
		default_clip.y1 *= bitmap->height;
	}

	if (!clip)
	{
		clip = &default_clip;
	}

	xy_scale.i = scale;
	xy_scale.j = scale;
	hud_calculate_bitmap_bounds(bitmap, placement, clip, &local_box, is_interface_bitmap);
	hud_draw_bitmap_internal(NULL, bitmap, point, clip, &local_box, &xy_scale, theta, color);

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 856);
	}

	return;
}

/* ---------- private code */

static void hud_calculate_bitmap_bounds(
	struct bitmap_data const *bitmap,
	short placement_type,
	real_rectangle2d const *clip,
	real_rectangle2d *bounds,
	boolean is_interface_bitmap)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	real width;
	real height;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	width = (clip->x1 - clip->x0) * (is_interface_bitmap ? 1 : bitmap->width);
	height = (clip->y1 - clip->y0) * (is_interface_bitmap ? 1 : bitmap->height);

	switch (placement_type)
	{
	case _hud_top_left:
		bounds->x0 = 0.f;
		bounds->x1 = width;
		bounds->y0 = 0.f;
		bounds->y1 = height;
		break;
	case _hud_top_right:
		bounds->x0 = -width;
		bounds->x1 = 0.f;
		bounds->y0 = 0.f;
		bounds->y1 = height;
		break;
	case _hud_bottom_left:
		bounds->x0 = 0.f;
		bounds->x1 = width;
		bounds->y0 = -height;
		bounds->y1 = 0.f;
		break;
	case _hud_bottom_right:
		bounds->x0 = -width;
		bounds->x1 = 0.f;
		bounds->y0 = -height;
		bounds->y1 = 0.f;
		break;
	case _hud_center:
		bounds->x0 = -width / 2.f;
		bounds->x1 = width / 2.f;
		bounds->y0 = -height / 2.f;
		bounds->y1 = height / 2.f;
		break;
	default:
		match_unreachable("c:\\halo\\SOURCE\\interface\\hud_draw.c", 904);
		break;
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 907);
	}

	return;
}

static void hud_draw_bitmap_internal(
	struct rasterizer_meter_parameters *meter_parameters,
	struct bitmap_data const *bitmap,
	point2d const *point,
	real_rectangle2d const *clip,
	real_rectangle2d const *bounds,
	real_vector2d const *xy_scale,
	real theta,
	unsigned long color)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct dynamic_screen_vertex vertices[4];
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	real sine_theta;
	real cosine_theta;
	short vertex_index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	sine_theta = sine(theta);
	cosine_theta = cosine(theta);

	for (vertex_index = 0; vertex_index < 4; vertex_index++)
	{
		real u = ((vertex_index + 1) & 2) ? clip->x1 : clip->x0;
		real v = vertex_index > 1 ? clip->y1 : clip->y0;
		real x = ((vertex_index + 1) & 2) ? bounds->x1 : bounds->x0;
		real y = vertex_index > 1 ? bounds->y1 : bounds->y0;

		vertices[vertex_index].position.x = (real)(fast_ftol((x * cosine_theta - y * sine_theta) * xy_scale->i) + point->x);
		vertices[vertex_index].position.y = (real)(fast_ftol((x * sine_theta + y * cosine_theta) * xy_scale->j) + point->y);
		vertices[vertex_index].texcoord.x = u;
		vertices[vertex_index].texcoord.y = v;
		vertices[vertex_index].color = color;
	}

	memset(&parameters, 0, sizeof(parameters));
	parameters.map_texture_scale[0].j = 1.f;
	parameters.map_texture_scale[0].i = 1.f;
	parameters.map_scale[0].j = 1.f;
	parameters.map_scale[0].i = 1.f;
	parameters.meter_parameters = meter_parameters;
	parameters.point_sampled = meter_parameters && local_player_count() == 1;
	parameters.framebuffer_blend_function = _shader_framebuffer_blend_function_alpha_multiply_add;
	parameters.map[0] = (struct bitmap_data *)bitmap;
	rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 985);
	}

	return;
}

static boolean hud_draw_multitexture_overlay_get_current_weapon_definition(
	struct player_datum const *player,
	struct weapon_interface_state *weapon_state)
{
	boolean result = FALSE;
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	long weapon_index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	weapon_index = unit_inventory_get_weapon(player->unit_index, unit_get(player->unit_index)->unit.current_weapon_index);

	if (weapon_index == NONE)
	{
		struct unit_datum *unit = unit_get(player->unit_index);

		if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
		{
			struct unit_datum *parent = unit_get(unit->object.parent_object_index);
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition_get(parent->definition_index)->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

			if (TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit))
			{
				weapon_index = unit_inventory_get_weapon(unit->object.parent_object_index, unit_get(unit->object.parent_object_index)->unit.current_weapon_index);
			}
		}
	}

	match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 1021, player->local_player_index==render.local_player_index);

	if (weapon_index != NONE)
	{
		weapon_definition_get(weapon_get(weapon_index)->definition_index);
		weapon_build_weapon_interface_state(weapon_index, weapon_state);
		result = TRUE;
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 1032);
	}

	return result;
}

static void hud_draw_multitexture_overlay(
	struct multitexture_overlay_hud_element_definition const *overlay,
	short local_player_index,
	point2d const *point,
	real_rectangle2d const *clip,
	real_rectangle2d const *bounds,
	real_vector2d const *xy_scale,
	real theta,
	unsigned long color)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct dynamic_screen_vertex vertices[4];
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	real_point2d texture_offset[3];
	real_rgb_color texture_tint[3];
	real texture_fade[3];
	real_vector2d geometry_offset;
	struct weapon_interface_state weapon_state;
	real sine_theta;
	real cosine_theta;
	struct player_datum *player;
	short vertex_index;
	short map_index;
	short function_index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	sine_theta = sine(theta);
	cosine_theta = cosine(theta);
	texture_offset[0].x = overlay->map_offset[0].i;
	texture_offset[0].y = overlay->map_offset[0].j;
	texture_offset[1].x = overlay->map_offset[1].i;
	texture_offset[1].y = overlay->map_offset[1].j;
	texture_offset[2].x = overlay->map_offset[2].i;
	texture_offset[2].y = overlay->map_offset[2].j;
	texture_tint[0].red = 0.f;
	texture_tint[0].green = 0.f;
	texture_tint[0].blue = 0.f;
	texture_tint[1].red = 0.f;
	texture_tint[1].green = 0.f;
	texture_tint[1].blue = 0.f;
	texture_tint[2].red = 0.f;
	texture_tint[2].green = 0.f;
	texture_tint[2].blue = 0.f;
	texture_fade[0] = 1.f;
	texture_fade[1] = 1.f;
	texture_fade[2] = 1.f;
	geometry_offset.i = 0.f;
	geometry_offset.j = 0.f;

	player = player_get(local_player_get_player_index(local_player_index));
	hud_draw_multitexture_overlay_get_current_weapon_definition(player, &weapon_state);

	for (vertex_index = 0; vertex_index < 4; vertex_index++)
	{
		real u = ((vertex_index + 1) & 2) ? clip->x1 : clip->x0;
		real v = vertex_index > 1 ? clip->y1 : clip->y0;
		real x = ((vertex_index + 1) & 2) ? bounds->x1 : bounds->x0;
		real y = vertex_index > 1 ? bounds->y1 : bounds->y0;

		vertices[vertex_index].position.x = (real)(fast_ftol((x * cosine_theta - y * sine_theta) * xy_scale->i) + point->x);
		vertices[vertex_index].position.y = (real)(fast_ftol((x * sine_theta + y * cosine_theta) * xy_scale->j) + point->y);
		vertices[vertex_index].texcoord.x = u;
		vertices[vertex_index].texcoord.y = v;
		vertices[vertex_index].color = color;
	}

	memset(&parameters, 0, sizeof(parameters));
	parameters.map_texture_scale[0].j = 1.f;
	parameters.map_texture_scale[0].i = 1.f;
	parameters.map_scale[0].j = 1.f;
	parameters.map_scale[0].i = 1.f;
	parameters.meter_parameters = NULL;
	parameters.point_sampled = local_player_count() == 1;
	parameters.map[0] = bitmap_group_get_bitmap_from_sequence(overlay->map[0].index, 0, 0);
	parameters.map[1] = bitmap_group_get_bitmap_from_sequence(overlay->map[1].index, 0, 0);
	parameters.map[2] = bitmap_group_get_bitmap_from_sequence(overlay->map[2].index, 0, 0);

	for (map_index = 0; map_index < NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS; map_index++)
	{
		if (parameters.map[map_index])
		{
			real_vector2d texture_scale;
			boolean non_power_of_two;

			texture_scale.i = overlay->map_scale[map_index].i == 0.f ? 1.f : 1.f / overlay->map_scale[map_index].i;
			texture_scale.j = overlay->map_scale[map_index].j == 0.f ? 1.f : 1.f / overlay->map_scale[map_index].j;
			non_power_of_two = ((parameters.map[map_index]->width - 1) & parameters.map[map_index]->width) || ((parameters.map[map_index]->height - 1) & parameters.map[map_index]->height);

			if (non_power_of_two)
			{
				parameters.map_texture_scale[map_index].i = 1.f / parameters.map[map_index]->width;
				parameters.map_texture_scale[map_index].j = 1.f / parameters.map[map_index]->height;
			}
			else
			{
				parameters.map_texture_scale[map_index].i = 1.f;
				parameters.map_texture_scale[map_index].j = 1.f;
			}

			parameters.map_offset[map_index] = &texture_offset[map_index];
			parameters.map_scale[map_index].i = texture_scale.i;
			parameters.map_scale[map_index].j = texture_scale.j;
			parameters.map_wrapped[map_index] = (boolean)overlay->map_clamp[map_index];
		}

		if (map_index < 2)
		{
			short *out_modes[2];

			out_modes[0] = &parameters.map0_to_1_blend_function;
			out_modes[1] = &parameters.map1_to_2_blend_function;

			switch (overlay->map_blending_function[map_index])
			{
			case _hud_multitexture_overlay_hud_element_blend_function_add:
				*out_modes[map_index] = 0;
				break;
			case _hud_multitexture_overlay_hud_element_blend_function_subtract:
				*out_modes[map_index] = 2;
				break;
			case _hud_multitexture_overlay_hud_element_blend_function_multiply:
				*out_modes[map_index] = 1;
				break;
			case _hud_multitexture_overlay_hud_element_blend_function_multiply2x:
				*out_modes[map_index] = 3;
				break;
			case _hud_multitexture_overlay_hud_element_blend_function_dot:
				*out_modes[map_index] = 4;
				break;
			}
		}
		else
		{
			parameters.framebuffer_blend_function = overlay->framebuffer_blend_function;
		}
	}

	for (function_index = 0; function_index < overlay->functions.count; function_index++)
	{
		static real theta;
		struct multitexture_overlay_hud_element_effector_definition *effector;
		real source_value;
		real dest_value;
		real_rgb_color dest_color;

		theta += 0.05f;
		effector = TAG_BLOCK_GET_ELEMENT(&overlay->functions, function_index, struct multitexture_overlay_hud_element_effector_definition);

		switch (effector->source)
		{
		case _hud_multitexture_overlay_effector_source_player_pitch:
		{
			long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;
			real_vector3d direction;
			real_euler_angles2d angles;

			unit_get_aiming_vector(unit_index, &direction);
			euler_angles2d_from_vector3d(&angles, &direction);
			source_value = angles.pitch;
			break;
		}
		case _hud_multitexture_overlay_effector_source_player_pitch_tangent:
			source_value = 0.f;
			break;
		case _hud_multitexture_overlay_effector_source_player_yaw:
			source_value = 0.f;
			break;
		case _hud_multitexture_overlay_effector_source_weapon_ammo_loaded:
			source_value = (real)weapon_state.magazines[0].rounds_loaded;
			break;
		case _hud_multitexture_overlay_effector_source_weapon_ammo_total:
			source_value = (real)weapon_state.magazines[0].rounds_remaining;
			break;
		case _hud_multitexture_overlay_effector_source_weapon_heat:
			source_value = weapon_state.heat;
			break;
		case _hud_multitexture_overlay_effector_source_explicit:
			source_value = effector->in_bounds[0];
			break;
		case _hud_multitexture_overlay_effector_source_zoom_level:
			source_value = (real)player_control_get_zoom_level(local_player_index);
			break;
		}

		if (effector->in_bounds[1] == effector->in_bounds[0] || effector->out_bounds[1] == effector->out_bounds[0])
		{
			dest_value = effector->out_bounds[0];
			dest_color = effector->tint_color_lower_bounds;
		}
		else
		{
			real fraction = PIN((source_value - effector->in_bounds[0]) / (effector->in_bounds[1] - effector->in_bounds[0]), 0.f, 1.f);

			scalars_interpolate(effector->out_bounds[0], effector->out_bounds[1], fraction, &dest_value);
			rgb_colors_interpolate(
				&dest_color,
				0,
				&effector->tint_color_lower_bounds,
				&effector->tint_color_upper_bounds,
				fraction);
		}

		switch (effector->destination)
		{
		case _hud_multitexture_overlay_effector_destination_geometry_offset:
			geometry_offset.i = effector->destination_type == _hud_multitexture_overlay_effector_type_horizontal_offset ? dest_value : 0.f;
			geometry_offset.j = effector->destination_type == _hud_multitexture_overlay_effector_type_vertical_offset ? dest_value : 0.f;
			parameters.offset = &geometry_offset;
			break;
		case _hud_multitexture_overlay_effector_destination_primary_map:
			if (effector->destination_type == _hud_multitexture_overlay_effector_type_tint)
			{
				texture_tint[0] = dest_color;
				parameters.map_tint[0] = &texture_tint[0];
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_horizontal_offset)
			{
				parameters.map_offset[0]->x += dest_value;
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_vertical_offset)
			{
				parameters.map_offset[0]->y += dest_value;
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_alpha)
			{
				texture_fade[0] = dest_value;
				parameters.map_fade[0] = &texture_fade[0];
				match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 1287, dest_value>=0.0f && dest_value<=1.0f);
			}
			break;
		case _hud_multitexture_overlay_effector_destination_secondary_map:
			if (effector->destination_type == _hud_multitexture_overlay_effector_type_tint)
			{
				texture_tint[1] = dest_color;
				parameters.map_tint[1] = &texture_tint[1];
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_horizontal_offset)
			{
				parameters.map_offset[1]->x += dest_value;
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_vertical_offset)
			{
				parameters.map_offset[1]->y += dest_value;
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_alpha)
			{
				texture_fade[1] = dest_value;
				parameters.map_fade[1] = &texture_fade[1];
				match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 1310, dest_value>=0.0f && dest_value<=1.0f);
			}
			break;
		case _hud_multitexture_overlay_effector_destination_tertiary_map:
			if (effector->destination_type == _hud_multitexture_overlay_effector_type_tint)
			{
				texture_tint[2] = dest_color;
				parameters.map_tint[2] = &texture_tint[2];
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_horizontal_offset)
			{
				parameters.map_offset[2]->x += dest_value;
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_vertical_offset)
			{
				parameters.map_offset[2]->y += dest_value;
			}
			else if (effector->destination_type == _hud_multitexture_overlay_effector_type_alpha)
			{
				texture_fade[2] = dest_value;
				parameters.map_fade[2] = &texture_fade[2];
				match_assert("c:\\halo\\SOURCE\\interface\\hud_draw.c", 1333, dest_value>=0.0f && dest_value<=1.0f);
			}
			break;
		}
	}

	rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\hud_draw.c", 1372);
	}

	return;
}
