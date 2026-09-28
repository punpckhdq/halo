/*
HUD_UNIT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "meter_definitions.h"
#include "ui_widget.h"
#include "hud.h"
#include "hud_messaging.h"
#include "weapon_hud_interface_definition.h"
#include "motion_sensor.h"
#include "interface_panels.h"
#include "inventory_displays.h"
#include "unit_hud_interface_definition.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "input.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "network_messages.h"
#include "weapon_interface_definitions.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "render_debug.h"
#include "director.h"
#include "weapons.h"
#include "texture_cache.h"
#include "font_group.h"
#include "game_engine_list.h"
#include "cinematics.h"
#include "player_effects.h"

/* ---------- constants */

enum
{
	_hud_panel_health_dont_show_bit = 0,
	_hud_panel_health_blink_bit,
	_hud_panel_shield_dont_show_bit,
	_hud_panel_shield_blink_bit,
	_hud_panel_motion_sensor_dont_show_bit,
	_hud_panel_motion_sensor_blink_bit,
	NUMBER_OF_SCRIPTED_UNIT_PANELS,
};

/* ---------- structures */

struct unit_hud_state
{
	real last_shield_vitality;
	real last_body_vitality;
	real fade_time;
	long last_shield_hit_time;
	long last_shield_flash_time;
	long last_health_flash_time;
	long last_motion_sensor_flash_time;
	long last_unit_index;
	unsigned short auxilary_active_type_flags;
	short auxilary_flash_time[NUMBER_OF_UNIT_AUXILARY_METERS];
	unsigned short sound_flags;
	long last_sound_handles[MAXIMUM_NUMBER_OF_HUD_SOUNDS];
};

struct unit_hud_globals_definition
{
	struct unit_hud_state hud_states[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	long script_flags;
};

/* ---------- prototypes */

long verify_tag_reference(struct tag_reference const *reference);

void *_texture_cache_bitmap_get_hardware_format(struct bitmap_data const *bitmap, boolean block, boolean load);

static void unit_hud_outline_mapper_tick(void);
static void unit_hud_shield_meter_mapper_tick(void);
static void initialize_hud_state(struct unit_hud_state *hud_state);
static struct unit_hud_state *get_hud_state(short local_player_index);
static void hud_update_unit_local_player(short local_player_index);

/* ---------- globals */

static struct unit_hud_globals_definition *unit_hud_globals = NULL;

/* ---------- public code */

void unit_hud_shield_meter_mapper_init(
	void)
{
	return;
}

void hud_initialize_unit_interface(
	void)
{
	unit_hud_globals = game_state_malloc("hud unit interface", NULL, sizeof(*unit_hud_globals));
	match_assert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 272, unit_hud_globals);

	return;
}

void hud_initialize_unit_interface_for_new_map(
	void)
{
	short local_player_index;

	match_assert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 283, unit_hud_globals);
	memset(unit_hud_globals, 0, sizeof(*unit_hud_globals));

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct unit_hud_state *hud_state = get_hud_state(local_player_index);

		initialize_hud_state(hud_state);
		hud_state->sound_flags = 0;
		memset(hud_state->last_sound_handles, NONE, sizeof(hud_state->last_sound_handles));
	}

	return;
}

void hud_dispose_unit_interface_from_old_map(
	void)
{
	return;
}

void hud_dispose_unit_interface(
	void)
{
	return;
}

void scripted_hud_show_health(
	boolean show)
{
	SET_FLAG(unit_hud_globals->script_flags, _hud_panel_health_dont_show_bit, !show);

	return;
}

void scripted_hud_blink_health(
	boolean blink)
{
	SET_FLAG(unit_hud_globals->script_flags, _hud_panel_health_blink_bit, blink);

	return;
}

void scripted_hud_show_shield(
	boolean show)
{
	SET_FLAG(unit_hud_globals->script_flags, _hud_panel_shield_dont_show_bit, !show);

	return;
}

void scripted_hud_blink_shield(
	boolean blink)
{
	SET_FLAG(unit_hud_globals->script_flags, _hud_panel_shield_blink_bit, blink);

	return;
}

void scripted_hud_show_motion_sensor(
	boolean show)
{
	SET_FLAG(unit_hud_globals->script_flags, _hud_panel_motion_sensor_dont_show_bit, !show);

	return;
}

void scripted_hud_blink_motion_sensor(
	boolean blink)
{
	SET_FLAG(unit_hud_globals->script_flags, _hud_panel_motion_sensor_blink_bit, blink);

	return;
}

void hud_play_unit_sounds(
	struct player_datum *player,
	boolean show_hud)
{
	struct unit_hud_state *hud_state = get_hud_state(player->local_player_index);
	long unit_index = player->unit_index;
	struct unit_datum *unit;

	if (unit_index == NONE)
		unit_index = hud_state->last_unit_index;
	unit = unit_try_and_get(unit_index);
	if (unit)
	{
		struct unit_definition const *definition = unit_definition_get(unit->definition_index);
		long hud_index = unit_definition_get_active_hud_index(definition, local_player_count() > 1);

		if (hud_index != NONE)
		{
			struct unit_hud_interface_definition *hud_definition = tag_get(UNIT_HUD_INTERFACE_TAG, hud_index);
			long flags = 0;

			if (TEST_FLAG(unit->object.flags, _object_on_media_bit) || !(unit->object.body_vitality > 0.f))
			{
				hud_state->last_unit_index = NONE;
			}
			else if (show_hud && !cinematic_in_progress())
			{
				if (hud_state->last_shield_vitality != -1.f &&
					game_engine_has_shield(local_player_get_player_index(player->local_player_index)) &&
					!TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_shield_dont_show_bit))
				{
					SET_FLAG(flags, _unit_hud_shield_recharging, TEST_FLAG(unit->object.damage_flags, _object_shield_charging_bit));
					SET_FLAG(flags, _unit_hud_shield_damage, hud_state->last_shield_vitality > unit->object.shield_vitality);
					SET_FLAG(flags, _unit_hud_shield_low, unit->object.shield_vitality < .25f && unit->object.shield_vitality > 0.f);
					SET_FLAG(flags, _unit_hud_shield_empty, unit->object.shield_vitality == 0.f);
				}
				if (!TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_health_dont_show_bit))
				{
					SET_FLAG(flags, _unit_hud_health_low, unit->object.body_vitality < .25f);
					SET_FLAG(flags, _unit_hud_health_empty, TEST_FLAG(unit->object.damage_flags, _object_dead_bit));
					SET_FLAG(flags, _unit_hud_minor_damage, hud_state->last_body_vitality > unit->object.body_vitality && hud_state->last_body_vitality - unit->object.body_vitality < .1875f);
					SET_FLAG(flags, _unit_hud_major_damage, hud_state->last_body_vitality - unit->object.body_vitality >= .1875f);
				}
			}
			hud_play_sound(player->local_player_index, flags, &hud_definition->warning_sounds, hud_state->last_sound_handles, &hud_state->sound_flags);
		}
	}

	return;
}

void hud_fix_unit_data(
	short old_local_player_index,
	short new_local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 427, old_local_player_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 428, new_local_player_index!=NONE);

	*get_hud_state(new_local_player_index) = *get_hud_state(old_local_player_index);

	return;
}

void hud_render_damage_indicators(
	short local_player_index)
{
	if (local_player_index != NONE)
	{
		long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;

		if (unit_try_and_get(unit_index))
		{
			struct hud_damage_indicators_definition const *definition = &hud_globals->damage_indicators;
			real scale = hud_globals_get_scale(local_player_count() > 1);
			byte damage_indicators[4];
			real_point2d position;
			short index;

			player_effect_get_damage_indicators(local_player_index, damage_indicators);
			for (index = 0; index < 4; index++)
			{
				if (damage_indicators[index] > 0 && damage_indicators[index] < 30)
				{
					real theta;

					switch (index)
					{
					case 0:
						position.y = render.camera.window_bounds.y0 + definition->top_offset;
						position.x = (render.camera.viewport_bounds.x0 + render.camera.viewport_bounds.x1) / 2;
						theta = _pi;
						break;
					case 1:
						position.x = render.camera.window_bounds.x0 + definition->left_offset;
						position.y = (render.camera.viewport_bounds.y0 + render.camera.viewport_bounds.y1) / 2;
						theta = _pi / 2.f;
						break;
					case 2:
						position.y = render.camera.window_bounds.y1 - definition->bottom_offset;
						position.x = (render.camera.viewport_bounds.x0 + render.camera.viewport_bounds.x1) / 2;
						theta = 0.f;
						break;
					case 3:
						position.x = render.camera.window_bounds.x1 - definition->right_offset;
						position.y = (render.camera.viewport_bounds.y0 + render.camera.viewport_bounds.y1) / 2;
						theta = 3.f * _pi / 2.f;
						break;
					default:
						match_unreachable("c:\\halo\\SOURCE\\interface\\hud_unit.c", 1024);
						break;
					}

					position.x -= render.camera.viewport_bounds.x0;
					position.y -= render.camera.viewport_bounds.y0;
					{
						long bitmap_index = definition->indicator_bitmap.index;
						short sequence_index = local_player_count() > 1 ? definition->multiplayer_sequence_index : definition->sequence_index;
						struct bitmap_data const *bitmap = NULL;
						real_rectangle2d const *clip = NULL;
						point2d corner;

						hud_retrieve_bitmap_and_bounding_rect(bitmap_index, sequence_index, 0, &bitmap, &clip);
						if (bitmap && _texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
						{
							corner.x = (short)position.x;
							corner.y = (short)position.y;
							hud_draw_bitmap_direct(bitmap, _hud_center, &corner, clip, scale, theta, definition->color, FALSE);
						}
					}
				}
			}
		}
		else
		{
			player_effect_clear_damage_indicators(local_player_index);
		}
	}

	return;
}

void hud_tick_shield(
	long player_index,
	real amount)
{
	short local_player_index = player_get(player_index)->local_player_index;

	if (local_player_index != NONE)
		get_hud_state(local_player_index)->last_shield_vitality -= amount;

	return;
}

void hud_update_unit(
	void)
{
	short local_player_index;

	for (local_player_index = local_player_get_next(NONE); local_player_index != NONE; local_player_index = local_player_get_next(local_player_index))
		hud_update_unit_local_player(local_player_index);

	return;
}

void hud_render_unit_interface(
	struct player_datum *player)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));
	match_assert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 521, player->local_player_index==render.local_player_index);

	unit_hud_shield_meter_mapper_tick();
	unit_hud_outline_mapper_tick();

	if (player->local_player_index == render.local_player_index && player->unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(player->unit_index);
		struct unit_definition const *definition = unit_definition_get(unit->definition_index);
		short local_player_index = player->local_player_index;
		long player_index = local_player_get_player_index(local_player_index);
		struct unit_hud_state *hud_state = get_hud_state(local_player_index);
		long unit_indices[18] = { player->unit_index };
		long unit_hud_indices[18] = { unit_definition_get_active_hud_index(definition, local_player_count() > 1) };
		long unit_count = 1;
		unsigned long auxilary_flags;
		unsigned long aux_activated_when_disabled_flags;
		real auxilary_values[NUMBER_OF_UNIT_AUXILARY_METERS];

		if (hud_state->last_unit_index == NONE)
			initialize_hud_state(get_hud_state(player->local_player_index));

		hud_state->last_unit_index = player->unit_index;

		if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
		{
			long parent_index = unit->object.parent_object_index;
			struct unit_datum *parent = unit_get(parent_index);
			struct unit_definition const *parent_definition = unit_definition_get(parent->definition_index);
			struct unit_seat const *seat = TAG_BLOCK_GET_ELEMENT(&parent_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);
			long hud_index;

			get_hud_state(local_player_index);
			hud_index = unit_definition_get_active_hud_index(parent_definition, local_player_count() > 1);
			if (TEST_FLAG(seat->flags, _unit_seat_is_driver_bit))
			{
				long child_index;

				if (hud_index != NONE)
				{
					unit_indices[unit_count] = parent_index;
					unit_hud_indices[unit_count++] = hud_index;
				}
				for (child_index = parent->object.first_child_object_index; child_index != NONE && unit_count < NUMBEROF(unit_indices); )
				{
					struct object_datum *child_object = object_get(child_index);
					struct unit_datum *child = unit_try_and_get(child_index);

					if (child && child->object.parent_object_index == parent_index && child->unit.parent_seat_index != NONE)
					{
						unit_indices[unit_count] = child_index;
						unit_hud_indices[unit_count++] = unit_definition_get_seat_active_hud_index(parent_definition, child->unit.parent_seat_index, local_player_count() > 1);
					}
					child_index = child_object->object.next_object_index;
				}
			}
		}

		auxilary_flags = 0;
		SET_FLAG(auxilary_flags, _auxilary_meter_integrated_light, unit->unit.integrated_light_power == 1.f);
		aux_activated_when_disabled_flags = 0;
		SET_FLAG(aux_activated_when_disabled_flags, _auxilary_meter_integrated_light,
			!TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit) && unit->unit.integrated_light_battery < .2f &&
			TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit));
		auxilary_values[_auxilary_meter_integrated_light] = unit->unit.integrated_light_battery;

		while (unit_count)
		{
			struct unit_datum *hud_unit;

			unit_count--;
			hud_unit = unit_try_and_get(unit_indices[unit_count]);
			if (hud_unit && unit_hud_indices[unit_count] != NONE)
			{
				struct unit_hud_interface_definition *hud_definition = tag_get(UNIT_HUD_INTERFACE_TAG, unit_hud_indices[unit_count]);

				if (hud_definition->background.interface_bitmap.index != NONE)
				{
					short draw_flags = 0;

					SET_FLAG(draw_flags, _hud_draw_disabled_bit, TEST_FLAG(hud_unit->object.damage_flags, _object_dead_bit));
					SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);
					hud_draw_static_element(local_player_index, &hud_definition->absolute_placement, &hud_definition->background, draw_flags, NONE);
				}

				if (game_engine_has_shield(player_index) && !TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_shield_dont_show_bit))
				{
					short draw_flags = 0;

					SET_FLAG(draw_flags, _hud_draw_flashing_bit, hud_unit->object.shield_vitality < .25f || TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_shield_blink_bit));
					SET_FLAG(draw_flags, _hud_draw_disabled_bit, TEST_FLAG(hud_unit->object.damage_flags, _object_dead_bit));
					SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);
					if (unit_count == 0)
					{
						if (TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
						{
							if (hud_state->last_shield_flash_time == NONE)
								hud_state->last_shield_flash_time = game_time_get();
						}
						else
							hud_state->last_shield_flash_time = NONE;
					}

					if (hud_definition->shield_meter.meter.meter_bitmap.index != NONE)
					{
						real last_shield_vitality;
						short value_scale;
						struct meter_hud_element_definition overcharge_meter;
						static long overcharge_count = 4;

						game_engine_running();
						last_shield_vitality = unit_count == 0 ? hud_state->last_shield_vitality : hud_unit->object.shield_vitality;
						value_scale = hud_definition->shield_meter.meter.value_scale ? hud_definition->shield_meter.meter.value_scale : 255;
						overcharge_meter = hud_definition->shield_meter.meter;
						{
							unsigned long color[5] = { 0, 0xFF0000, 0x00FF00, 0xFFFF00, 0x7F00FF };
							unsigned long const *current_color = color;
							long overcharge_index;

							for (overcharge_index = 0; overcharge_index <= overcharge_count; current_color++, overcharge_index++)
							{
								boolean first = overcharge_index == 0;
								real vitality = PIN(hud_unit->object.shield_vitality - overcharge_index, 0.f, 1.f);
								real last_vitality = PIN(last_shield_vitality - overcharge_index, 0.f, 1.f);
								boolean fading;

								if (last_vitality > vitality)
									fading = TRUE;
								else
								{
									fading = FALSE;
									last_vitality = vitality;
								}
								if (vitality <= 0.f && last_vitality <= 0.f)
									break;
								overcharge_meter.min_color = *current_color;
								overcharge_meter.max_color = *current_color;
								hud_draw_meter(local_player_index, &hud_definition->absolute_placement,
									first ? &hud_definition->shield_meter.meter : &overcharge_meter,
									(byte)PIN(fast_ftol(vitality * value_scale), 0, 255),
									(byte)PIN(fast_ftol(last_vitality * value_scale), 0, 255),
									draw_flags, fading ? hud_state->fade_time : -1.f, vitality);
							}
						}
					}
					if (hud_definition->shield_meter.background.interface_bitmap.index != NONE)
						hud_draw_static_element(local_player_index, &hud_definition->absolute_placement, &hud_definition->shield_meter.background, draw_flags, hud_state->last_shield_flash_time);
				}

				if (!TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_health_dont_show_bit))
				{
					short draw_flags = 0;

					SET_FLAG(draw_flags, _hud_draw_flashing_bit, TEST_FLAG(hud_unit->object.damage_flags, _object_shield_depleted_bit) || TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_health_blink_bit));
					SET_FLAG(draw_flags, _hud_draw_disabled_bit, TEST_FLAG(hud_unit->object.damage_flags, _object_dead_bit));
					SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);
					if (unit_count == 0)
					{
						if (TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
						{
							if (hud_state->last_health_flash_time == NONE)
								hud_state->last_health_flash_time = game_time_get();
						}
						else
							hud_state->last_health_flash_time = NONE;
					}
					if (hud_definition->health_meter.meter.meter_bitmap.index != NONE)
					{
						short value_scale = hud_definition->health_meter.meter.value_scale ? hud_definition->health_meter.meter.value_scale : 8;
						struct meter_hud_element_definition health_meter = hud_definition->health_meter.meter;

						if (hud_unit->object.body_vitality >= hud_definition->health_meter.health_extras.max_cutoff)
							health_meter.min_color = health_meter.max_color;
						else if (hud_unit->object.body_vitality <= hud_definition->health_meter.health_extras.min_cutoff)
							health_meter.max_color = health_meter.min_color;
						else
							health_meter.min_color = health_meter.max_color = hud_definition->health_meter.health_extras.mid_color;
						hud_draw_meter(local_player_index, &hud_definition->absolute_placement, &health_meter,
							(byte)PIN(fast_ftol_C(hud_unit->object.body_vitality * value_scale), 0, 255),
							(byte)PIN(fast_ftol_C(hud_unit->object.body_vitality * value_scale), 0, 255),
							draw_flags, -1.f, hud_unit->object.body_vitality);
					}
					if (hud_definition->health_meter.background.interface_bitmap.index != NONE)
						hud_draw_static_element(local_player_index, &hud_definition->absolute_placement, &hud_definition->health_meter.background, draw_flags, hud_state->last_health_flash_time);
					hud_state->last_body_vitality = hud_unit->object.body_vitality;
				}

				if (unit_count == 0 && !TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_motion_sensor_dont_show_bit) && game_engine_hud_draw_motion_sensor(player_index))
				{
					struct hud_absolute_placement_definition absolute_placement;
					point2d point;
					short draw_flags = 0;

					absolute_placement.corner = _hud_bottom_left;
					SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);
					SET_FLAG(draw_flags, _hud_draw_flashing_bit, TEST_FLAG(unit_hud_globals->script_flags, _hud_panel_motion_sensor_blink_bit));
					if (TEST_FLAG(draw_flags, _hud_draw_flashing_bit))
					{
						if (hud_state->last_motion_sensor_flash_time == NONE)
							hud_state->last_motion_sensor_flash_time = game_time_get();
					}
					else
						hud_state->last_motion_sensor_flash_time = NONE;
					if (hud_definition->motion_sensor.background.interface_bitmap.index != NONE)
						hud_draw_static_element(local_player_index, &absolute_placement, &hud_definition->motion_sensor.background, draw_flags, NONE);
					if (hud_definition->motion_sensor.foreground.interface_bitmap.index != NONE)
						hud_draw_static_element(local_player_index, &absolute_placement, &hud_definition->motion_sensor.foreground, draw_flags, NONE);
					hud_calculate_point(local_player_index, &absolute_placement, &hud_definition->blip_placement, NULL, local_player_count() > 1, 0.f, &point);
					motion_sensor_draw_screen(local_player_index, local_player_count() > 1, &point);
				}
				{
					struct auxilary_panel_definition *panel = &hud_definition->auxilary_panel;
					unsigned short overlay_flags = 0;
					short index;
					short draw_flags = 0;

					SET_FLAG(overlay_flags, _auxilary_overlay_team, game_engine_has_teams());
					SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);
					for (index = 0; index < panel->auxilary_overlays.count; index++)
					{
						struct auxilary_overlay_definition *overlay = TAG_BLOCK_GET_ELEMENT(&panel->auxilary_overlays, index, struct auxilary_overlay_definition);

						if (TEST_FLAG(overlay_flags, overlay->type))
						{
							if (TEST_FLAG(overlay->flags, _auxilary_overlay_use_team_color_bit))
								overlay->static_element.colors.color = real_rgb_color_to_pixel32(&hud_unit->object.base_change_colors[0]) | 0xFF000000;
							hud_draw_static_element(local_player_index, &panel->absolute_placement, &overlay->static_element, draw_flags, NONE);
						}
					}
				}
				{
					short index;

					for (index = 0; index < hud_definition->auxilary_meters.count; index++)
					{
						struct auxilary_meter_definition *meter = TAG_BLOCK_GET_ELEMENT(&hud_definition->auxilary_meters, index, struct auxilary_meter_definition);

						if (TEST_FLAG(hud_state->auxilary_active_type_flags, meter->type) && !TEST_FLAG(auxilary_flags, meter->type))
							hud_state->auxilary_flash_time[meter->type] = NONE;
						if (TEST_FLAG(auxilary_flags, meter->type))
						{
							long background_index = verify_tag_reference(&meter->panel.background.interface_bitmap);
							long meter_index = verify_tag_reference(&meter->panel.meter.meter_bitmap);

							short draw_flags = 0;

							SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);
							SET_FLAG(draw_flags, _hud_draw_flashing_bit, auxilary_values[meter->type] <= meter->panel.aux_extras.min_cutoff);
							hud_state->auxilary_flash_time[meter->type] += game_time_get_elapsed();
							hud_state->auxilary_flash_time[meter->type] %= 2 * get_flash_duration(&meter->panel.background.colors);
							if (background_index != NONE)
								hud_draw_static_element(local_player_index, &hud_definition->absolute_placement, &meter->panel.background, draw_flags, game_time_get() - hud_state->auxilary_flash_time[meter->type]);
							if (meter_index != NONE)
							{
								real value_scale = meter->panel.meter.value_scale;

								hud_draw_meter(local_player_index, &hud_definition->absolute_placement, &meter->panel.meter,
									(byte)PIN(fast_ftol(auxilary_values[meter->type] * value_scale), 0, 255),
									(byte)PIN(fast_ftol(auxilary_values[meter->type] * value_scale), 0, 255),
									draw_flags, -1.f, auxilary_values[meter->type]);
							}
						}
						else if (TEST_FLAG(aux_activated_when_disabled_flags, meter->type) ||
							(hud_state->auxilary_flash_time[meter->type] != NONE && hud_state->auxilary_flash_time[meter->type] < get_flash_duration(&meter->panel.background.colors)))
						{
							long background_index = verify_tag_reference(&meter->panel.background.interface_bitmap);

							short draw_flags = 0;

							SET_FLAG(draw_flags, _hud_draw_in_multiplayer_bit, local_player_count() > 1);
							hud_state->auxilary_flash_time[meter->type] += game_time_get_elapsed();
							if (background_index != NONE)
								hud_draw_static_element(local_player_index, &hud_definition->absolute_placement, &meter->panel.background, draw_flags | FLAG(_hud_draw_flashing_bit), game_time_get() - hud_state->auxilary_flash_time[meter->type]);
						}
						else
							hud_state->auxilary_flash_time[meter->type] = NONE;
					}
				}
				hud_state->auxilary_active_type_flags = (unsigned short)auxilary_flags;
			}
		}
	}
	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_vassert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 969, return_eip == get_return_eip(), "corrupt return address!");
		match_vassert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 969, corrupt_index == NONE, csprintf(temporary, "corrupt stack at %d!", corrupt_index));
	}

	return;
}

/* ---------- private code */

static void unit_hud_outline_mapper_tick(
	void)
{
	return;
}

static void unit_hud_shield_meter_mapper_tick(
	void)
{
	return;
}

static void initialize_hud_state(
	struct unit_hud_state *hud_state)
{
	memset(hud_state->auxilary_flash_time, NONE, sizeof(hud_state->auxilary_flash_time));
	hud_state->last_shield_vitality = hud_state->last_body_vitality = -1.f;
	hud_state->last_health_flash_time = NONE;
	hud_state->last_motion_sensor_flash_time = NONE;
	hud_state->fade_time = -1.f;
	hud_state->last_unit_index = NONE;

	return;
}

static struct unit_hud_state *get_hud_state(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 262, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	match_assert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 263, unit_hud_globals);

	return &unit_hud_globals->hud_states[local_player_index];
}

static void hud_update_unit_local_player(
	short local_player_index)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	long unit_index;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));
	unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		struct unit_hud_state *hud_state = get_hud_state(local_player_index);

		if (hud_state->last_body_vitality == -1.f)
			hud_state->last_body_vitality = unit->object.body_vitality;
		if (hud_state->last_shield_vitality == -1.f)
			hud_state->last_shield_vitality = unit->object.shield_vitality;

		if (hud_state->last_shield_vitality > unit->object.shield_vitality)
		{
			if (hud_state->fade_time < 0.f || hud_state->fade_time > 1.f)
				hud_state->last_shield_hit_time = game_time_get();
			if (game_time_get() - hud_state->last_shield_hit_time < 15)
			{
				hud_state->fade_time = 0.f;
			}
			else
			{
				hud_state->last_shield_vitality = unit->object.shield_vitality;
				hud_state->fade_time += (real)(game_time_get() - hud_state->last_shield_hit_time) / 30.f;
				hud_state->last_shield_hit_time = game_time_get();
			}
		}
		else if (hud_state->last_shield_vitality < unit->object.shield_vitality)
		{
			hud_state->last_shield_vitality = unit->object.shield_vitality;
			hud_state->fade_time = -1.f;
			hud_state->last_shield_hit_time = game_time_get();
		}
		else
		{
			hud_state->last_shield_vitality = unit->object.shield_vitality;
			if (hud_state->fade_time > 0.f)
				hud_state->fade_time += (real)(game_time_get() - hud_state->last_shield_hit_time) / 30.f;
			hud_state->last_shield_hit_time = game_time_get();
		}
	}
	if (cinematic_in_progress())
	{
		long player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE)
			hud_play_unit_sounds(player_get(player_index), hud_scripted_globals->show_hud);
	}
	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_vassert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 513, return_eip == get_return_eip(), "corrupt return address!");
		match_vassert("c:\\halo\\SOURCE\\interface\\hud_unit.c", 513, corrupt_index == NONE, csprintf(temporary, "corrupt stack at %d!", corrupt_index));
	}

	return;
}
