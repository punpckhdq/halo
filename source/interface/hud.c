/*
HUD.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hud.h"
#include "meter_definitions.h"
#include "hud_messaging.h"
#include "weapon_hud_interface_definition.h"
#include "motion_sensor.h"
#include "hud_messaging_definitions.h"
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
#include "console.h"
#include "game_state.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "text_group.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
#include "director.h"
#include "weapons.h"
#include "game_sound.h"
#include "texture_cache.h"
#include "font_group.h"
#include "cinematics.h"
#include "devices.h"
#include "equipment_definitions.h"

/* ---------- constants */

enum
{
	NUMBER_OF_TEMPORARY_HUD_RETICLE_POINTS = 16 /* fake name */
};

/* ---------- prototypes */

static boolean weapon_state_is_depleted(struct weapon_interface_state const *state);
static short get_object_icon_text_index(long object_index);
static void hud_show_action_response(long player_index);
static void temporary_hud_draw_reticle(real angle, real_argb_color const *color);
static void hud_draw_friendly_indicator(long player_index);
static void hud_draw_players(void);
static void temporary_hud_draw(void);

/* ---------- globals */

struct hud_globals_definition *hud_globals = NULL;
struct hud_scripted_globals_definition *hud_scripted_globals = NULL;

/* ---------- public code */

static boolean weapon_state_is_depleted(
	struct weapon_interface_state const *state)
{
	return (state->magazines[0].rounds_loaded_maximum && !state->magazines[0].rounds_loaded && !state->magazines[0].rounds_remaining) ||
		state->age == 1.f;
}

void hud_initialize(
	void)
{
	hud_scripted_globals = game_state_malloc("hud scripted globals", NULL, sizeof(*hud_scripted_globals));
	match_assert("c:\\halo\\SOURCE\\interface\\hud.c", 87, hud_scripted_globals);
	hud_messaging_initialize();
	hud_initialize_unit_interface();
	hud_initialize_weapon_interface();
	hud_initialize_nav_points();
	motion_sensor_initialize();

	return;
}

void hud_dispose(
	void)
{
	motion_sensor_dispose();
	hud_dispose_nav_points();
	hud_dispose_weapon_interface();
	hud_dispose_unit_interface();
	hud_messaging_dispose();

	return;
}

void hud_initialize_for_new_map(
	void)
{
	memset(hud_scripted_globals, 0, sizeof(*hud_scripted_globals));
	hud_scripted_globals->show_hud = TRUE;
	match_assert("c:\\halo\\SOURCE\\interface\\hud.c", 113, interface_get_tag_index(_interface_hud_globals)!=NONE);
	hud_globals = hud_globals_definition_get(interface_get_tag_index(_interface_hud_globals));
	hud_messaging_initialize_for_new_map();
	hud_initialize_unit_interface_for_new_map();
	hud_initialize_weapon_interface_for_new_map();
	hud_initialize_nav_points_for_new_map();
	motion_sensor_initialize_for_new_map();

	return;
}

void hud_dispose_from_old_map(
	void)
{
	motion_sensor_dispose_from_old_map();
	hud_dispose_nav_points_from_old_map();
	hud_dispose_weapon_interface_from_old_map();
	hud_dispose_unit_interface_from_old_map();
	hud_messaging_dispose_from_old_map();

	return;
}

boolean scripted_show_hud(
	boolean show)
{
	hud_scripted_globals->show_hud = show;

	return hud_scripted_globals->show_hud;
}

boolean scripted_show_hud_help_text(
	boolean show)
{
	hud_scripted_globals->show_hud_help_text = show;

	return hud_scripted_globals->show_hud_help_text;
}

void hud_update(
	void)
{
	hud_update_weapon();
	hud_update_unit();
	hud_update_nav_points();
	hud_messaging_globals_update();

	if (game_engine_force_single_screen())
	{
		short local_player_index;

		for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
		{
			long player_index = local_player_get_player_index(local_player_index);

			if (player_index != NONE)
			{
				hud_play_unit_sounds(player_get(player_index), FALSE);
			}
		}
	}

	return;
}

void hud_draw_screen(
	void)
{
	long player_index = local_player_get_player_index(render.local_player_index);
	short perspective = director_get_perspective(render.local_player_index);

	rasterizer_hud_begin();

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);

		if ((!game_engine_running() || game_engine_display_team_indicators()) && !cinematic_in_progress())
		{
			hud_draw_players();
		}

		if (!game_time_get_paused() && render.local_player_index == local_player_get_next(NONE))
		{
			motion_sensor_tick();
		}

		if (hud_scripted_globals->show_hud)
		{
			if (perspective != _director_perspective_neutral && perspective != _director_perspective_scripted && player->unit_index != NONE)
			{
				hud_render_weapon_interface(player);
				hud_show_action_response(player_index);
				hud_play_unit_sounds(player, hud_scripted_globals->show_hud);
				hud_render_unit_interface(player);
				hud_render_nav_points(render.local_player_index);
				hud_render_damage_indicators(render.local_player_index);
			}
			else
			{
				hud_show_action_response(player_index);
				hud_play_unit_sounds(player, hud_scripted_globals->show_hud);
			}
		}
		else
		{
			hud_play_unit_sounds(player, FALSE);
		}

		hud_messaging_update(render.local_player_index);
	}

	rasterizer_hud_end();

	if (temporary_hud)
	{
		temporary_hud_draw();
	}

	return;
}

static short get_object_icon_text_index(
	long object_index)
{
	return object_index == NONE ? NONE : object_definition_get(object_get(object_index)->definition_index)->object.icon_text_index;
}

static void hud_show_action_response(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	short respawn_failure = players_get_respawn_failure();

	if (respawn_failure != _player_respawn_failure_none && player->unit_index == NONE)
	{
		switch (respawn_failure)
		{
		case _player_respawn_failure_combat:
			hud_set_state_message(render.local_player_index, _hud_message_respawn_failed_combat);
			return;
		case _player_respawn_failure_enemies:
			hud_set_state_message(render.local_player_index, _hud_message_respawn_failed_unsafe);
			return;
		case _player_respawn_failure_moving:
			hud_set_state_message(render.local_player_index, _hud_message_respawn_failed_moving);
			return;
		case _player_respawn_failure_vehicle:
			hud_set_state_message(render.local_player_index, _hud_message_respawn_failed_vehicle);
			return;
		default:
			match_unreachable("c:\\halo\\SOURCE\\interface\\hud.c", 424);
			break;
		}
	}
	else
	{
		short item_name_index = get_object_icon_text_index(player->action_object_index);

		switch (player->action_result)
		{
		case _player_action_result_pickup_powerup:
		case _player_action_result_pickup_weapon:
			hud_set_state_message(render.local_player_index, _hud_message_pickup);
			hud_set_state_message_text(render.local_player_index, 0, item_name_index, FALSE);
			break;
		case _player_action_result_swap_for_powerup:
			hud_set_state_message(render.local_player_index, _hud_message_swap_powerup);
			hud_set_state_message_text(
				render.local_player_index,
				0,
				get_object_icon_text_index(unit_get_current_equipment(player->unit_index)),
				FALSE);
			hud_set_state_message_text(render.local_player_index, 1, item_name_index, FALSE);
			break;
		case _player_action_result_exit_vehicle:
		{
			struct unit_datum *unit = unit_get(player->unit_index);

			hud_set_state_message(render.local_player_index, _hud_message_exit_vehicle);
			hud_set_state_message_text(
				render.local_player_index,
				0,
				get_object_icon_text_index(unit->object.parent_object_index),
				FALSE);
			break;
		}
		case _player_action_result_touch_device:
		{
			struct control_datum *control = control_get(player->action_object_index);

			if (control->control.hud_override_index != NONE)
			{
				hud_set_state_message(render.local_player_index, _hud_message_custom_device);
				hud_set_state_message_text(render.local_player_index, 0, control->control.hud_override_index, TRUE);
			}
			else
			{
				hud_set_state_message(render.local_player_index, _hud_message_touch_device);
				hud_set_state_message_text(render.local_player_index, 0, item_name_index, FALSE);
			}
			break;
		}
		case _player_action_result_swap_for_weapon:
		{
			struct weapon_datum *weapon = weapon_try_and_get(player->action_object_index);

			if (weapon)
			{
				struct icon_hud_element_definition const *icon = NULL;
				struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

				if (weapon_definition->weapon.interface_definition.hud_interface.index != NONE)
				{
					icon = &weapon_hud_interface_definition_get(weapon_definition->weapon.interface_definition.hud_interface.index)->messaging_icon;

					if (icon->sequence_index == NONE)
					{
						icon = NULL;
					}
				}

				hud_set_state_message(render.local_player_index, _hud_message_swap_weapon);

				if (icon)
				{
					hud_set_state_message_icon(render.local_player_index, 0, icon);
				}
				else
				{
					hud_set_state_message_text(render.local_player_index, 0, item_name_index, FALSE);
				}
			}
			break;
		}
		case _player_action_result_add_weapon_to_inventory:
		{
			struct weapon_datum *weapon = weapon_try_and_get(player->action_object_index);

			if (weapon)
			{
				struct icon_hud_element_definition const *icon = NULL;
				struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

				if (weapon_definition->weapon.interface_definition.hud_interface.index != NONE)
				{
					icon = &weapon_hud_interface_definition_get(weapon_definition->weapon.interface_definition.hud_interface.index)->messaging_icon;

					if (icon->sequence_index == NONE)
					{
						icon = NULL;
					}
				}

				hud_set_state_message(render.local_player_index, _hud_message_pickup);

				if (icon)
				{
					hud_set_state_message_icon(render.local_player_index, 0, icon);
				}
				else
				{
					hud_set_state_message_text(render.local_player_index, 0, item_name_index, FALSE);
				}
			}
			break;
		}
		case _player_action_result_enter_vehicle:
		case _player_action_result_evict_from_vehicle:
			hud_set_state_message(render.local_player_index, _hud_message_enter_vehicle);
			hud_set_state_message_text(
				render.local_player_index,
				0,
				TAG_BLOCK_GET_ELEMENT(&unit_definition_get(unit_get(player->action_object_index)->definition_index)->unit.seats, player->action_seat_index, struct unit_seat)->icon_text_index,
				FALSE);
			hud_set_state_message_text(render.local_player_index, 1, item_name_index, FALSE);
			break;
		case _player_action_result_flip_vehicle:
			hud_set_state_message(render.local_player_index, _hud_message_flip_vehicle);
			hud_set_state_message_text(
				render.local_player_index,
				0,
				get_object_icon_text_index(player->action_object_index),
				FALSE);
			break;
		default:
		{
			wchar_t buffer[1024];

			if (game_engine_get_state_message(player_index, buffer, NUMBEROF(buffer)))
			{
				hud_enable_custom_state_message(render.local_player_index, TRUE);
				hud_set_state_text(render.local_player_index, buffer);
			}
			else if (player->unit_index == NONE)
			{
				hud_enable_custom_state_message(render.local_player_index, FALSE);
			}
			else
			{
				long weapon_index = unit_inventory_get_weapon(player->unit_index, unit_get(player->unit_index)->unit.current_weapon_index);
				struct unit_datum *unit = unit_get(player->unit_index);
				boolean message_set = FALSE;
				boolean can_switch_weapons = TRUE;

				if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
				{
					struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
					struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition_get(parent_unit->definition_index)->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

					can_switch_weapons = !TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit) && !TEST_FLAG(seat->flags, _unit_seat_is_driver_bit);
				}

				if (weapon_index != NONE && can_switch_weapons)
				{
					struct weapon_interface_state weapon_state;
					long next_weapon_index = weapon_index;

					weapon_build_weapon_interface_state(weapon_index, &weapon_state);

					if (weapon_state_is_depleted(&weapon_state))
					{
						short weapon_slot_index = unit->unit.current_weapon_index;
						short weapon_count = unit_get_weapon_count(player->unit_index);

						do
						{
							weapon_slot_index = unit_inventory_next_weapon(player->unit_index, weapon_slot_index, 1);
							next_weapon_index = unit_inventory_get_weapon(player->unit_index, weapon_slot_index);
							weapon_build_weapon_interface_state(next_weapon_index, &weapon_state);
						}
						while (weapon_state_is_depleted(&weapon_state) && next_weapon_index != weapon_index && --weapon_count == 0);

						if (!weapon_state_is_depleted(&weapon_state) && next_weapon_index != weapon_index)
						{
							struct weapon_datum *next_weapon;

							hud_set_state_message(render.local_player_index, _hud_message_remind_to_switch_weapons);
							next_weapon = weapon_try_and_get(next_weapon_index);

							if (next_weapon)
							{
								struct icon_hud_element_definition const *icon = NULL;
								struct weapon_definition *weapon_definition = weapon_definition_get(next_weapon->definition_index);

								if (weapon_definition->weapon.interface_definition.hud_interface.index != NONE)
								{
									icon = &weapon_hud_interface_definition_get(weapon_definition->weapon.interface_definition.hud_interface.index)->messaging_icon;

									if (icon->sequence_index == NONE)
									{
										icon = NULL;
									}
								}

								if (icon)
								{
									hud_set_state_message_icon(render.local_player_index, 0, icon);
								}
								else
								{
									hud_set_state_message_text(render.local_player_index, 0, item_name_index, FALSE);
								}

								message_set = TRUE;
							}
						}
					}
				}

				if (!message_set)
				{
					hud_enable_custom_state_message(render.local_player_index, FALSE);
				}
			}
			break;
		}
		}
	}

	return;
}

wchar_t const *hud_get_item_string(
	long index)
{
	static wchar_t *default_string = L"";
	wchar_t const *result = default_string;
	long string_list_index = hud_globals->messaging.hud_item_messages.index;

	if (string_list_index != NONE)
	{
		struct unicode_string_list_group_header *string_list = unicode_string_list_definition_get(string_list_index);

		if (string_list && index >= 0 && index < string_list->string_references.count)
		{
			result = unicode_string_list_get_string(string_list_index, (short)index);
		}
	}

	return result;
}

void hud_picked_up_grenade(
	short local_player_index,
	long grenade_definition_index)
{
	if (local_player_index != NONE)
	{
		hud_add_item_message(local_player_index, grenade_definition_index, 1, NONE);
	}

	return;
}

void hud_picked_up_ammunition(
	short local_player_index,
	long weapon_definition_index,
	short count)
{
	if (local_player_index != NONE)
	{
		hud_add_item_message(local_player_index, weapon_definition_index, count, 1);
	}

	return;
}

void hud_picked_up_weapon(
	short local_player_index,
	long weapon_definition_index)
{
	if (local_player_index != NONE)
	{
		hud_add_item_message(local_player_index, weapon_definition_index, 0, 0);
	}

	return;
}

void hud_picked_up_powerup(
	short local_player_index,
	long powerup_definition_index)
{
	if (local_player_index != NONE)
	{
		hud_add_item_message(local_player_index, powerup_definition_index, 0, 0);
	}

	return;
}

static void temporary_hud_draw_reticle(
	real angle,
	real_argb_color const *color)
{
	real_point3d points[NUMBER_OF_TEMPORARY_HUD_RETICLE_POINTS];
	short point_index;
	real radius = tangent(angle * 0.5f) / 16.f;
	real delta = 2.f * _pi / NUMBER_OF_TEMPORARY_HUD_RETICLE_POINTS;
	real theta = 0.f;

	for (point_index = 0; point_index < NUMBER_OF_TEMPORARY_HUD_RETICLE_POINTS; point_index++)
	{
		set_real_point3d(&points[point_index], cosine(theta) * radius, sine(theta) * radius, -1.f/16.f);
		matrix4x3_transform_point(&render.frustum.view_to_world, &points[point_index], &points[point_index]);
		theta += delta;
	}

	for (point_index = 0; point_index < NUMBER_OF_TEMPORARY_HUD_RETICLE_POINTS; point_index++)
	{
		rasterizer_debug_line(&points[point_index], &points[(point_index + 1) % NUMBER_OF_TEMPORARY_HUD_RETICLE_POINTS], color);
	}

	return;
}

void hud_load(
	boolean begin)
{
	short string_index = begin ? hud_globals->loading_begin_index : hud_globals->loading_end_index;

	scripted_hud_messages_clear();

	if (string_index != NONE)
	{
		hud_print_message(local_player_get_next(NONE), hud_get_item_string(string_index));
	}

	return;
}

void hud_autosave(
	boolean begin)
{
	short string_index = begin ? hud_globals->checkpoint_begin_index : hud_globals->checkpoint_end_index;

	scripted_hud_messages_clear();

	if (begin && hud_globals->checkpoint_sound.index != NONE)
	{
		unspatialized_impulse_sound_new(hud_globals->checkpoint_sound.index, 1.f);
	}

	if (string_index != NONE)
	{
		short local_player_index;

		for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
		{
			if (local_player_get_player_index(local_player_index) != NONE)
			{
				hud_print_message(local_player_index, hud_get_item_string(string_index));
			}
		}
	}

	return;
}

static void hud_draw_friendly_indicator(
	long player_index)
{
	real_point3d center;
	real_point2d screen_pos;
	real_point3d view_point;
	point2d corner;
	real_rectangle2d clip;
	struct player_datum *player = player_get(player_index);

	unit_get_head_position(player->unit_index, &center);
	center.z += 0.3f;
	matrix4x3_transform_point(&render.frustum.world_to_view, &center, &view_point);

	if (render_camera_view_to_screen(&render.camera, &render.frustum, &view_point, &screen_pos))
	{
		struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(interface_get_tag_index(_interface_bitmap_multiplayer_hud), 0, 0);

		if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
		{
			real fade;

			corner.x = (short)fast_ftol(screen_pos.x) - render.camera.viewport_bounds.x0;
			corner.y = (short)fast_ftol(screen_pos.y) - render.camera.viewport_bounds.y0;
			fade = PIN(1.f - (-view_point.z - render.frustum.z_near) * 100.f / (render.frustum.z_far - render.frustum.z_near), 0.075f, 1.f);
			clip.x0 = 0.f;
			clip.x1 = 1.f;
			clip.y0 = (1.f - fade) * 0.5f;
			clip.y1 = 1.f;
			interface_draw_bitmap(bitmap, &corner, &clip, 1.f, 0.f, 1.f);
		}
	}

	return;
}

static void hud_draw_players(
	void)
{
	long player_index = local_player_get_player_index(render.local_player_index);
	long team_index = player_get(player_index)->team_index;

	if (player_index != NONE)
	{
		long player_indices[16];
		struct data_iterator target_player_iterator;
		struct player_datum *target_player;
		long index;
		long player_count = 0;

		data_iterator_new(&target_player_iterator, player_data);

		while ((target_player = (struct player_datum *)data_iterator_next(&target_player_iterator)) != NULL)
		{
			if (player_index != target_player_iterator.index && team_index == target_player->team_index && target_player->unit_index != NONE)
			{
				player_indices[player_count++] = target_player_iterator.index;
			}
		}

		for (index = 0; index < player_count; index++)
		{
			hud_draw_friendly_indicator(player_indices[index]);
		}
	}

	return;
}

static void temporary_hud_draw(
	void)
{
	long player_index = local_player_get_player_index(render.local_player_index);

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);
			struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
			long weapon_index = unit_inventory_get_weapon(player->unit_index, unit_get(player->unit_index)->unit.current_weapon_index);
			long equipment_index = unit_get_current_equipment(player->unit_index);

			if (equipment_index != NONE)
			{
				sprintf(temporary, "%s (press WHITE to use)|n", strrchr(tag_get_name(object_get(equipment_index)->definition_index), '\\') + 1);
			}
			else
			{
				sprintf(temporary, "|n");
			}

			if (unit->unit.current_grenade_index != NONE)
			{
				struct game_globals_grenade *grenade = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, unit->unit.current_grenade_index, struct game_globals_grenade);

				sprintf(
					temporary + strlen(temporary),
					"%d %s|n",
					unit->unit.grenade_counts[unit->unit.current_grenade_index],
					tag_name_strip_path(tag_get_name(grenade->projectile.index)));
			}

			if (weapon_index != NONE)
			{
				struct weapon_trigger *trigger;
				struct weapon_trigger_definition *trigger_definition;
				real error;
				real angle;
				struct weapon_datum *weapon = weapon_get(weapon_index);
				struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

				if (weapon_definition->weapon.magazines.count > 0)
				{
					struct weapon_magazine_definition *magazine_definition = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.magazines, 0, struct weapon_magazine_definition);

					sprintf(
						temporary + strlen(temporary),
						"%s|ntotal %d/%d|nloaded %d/%d|nheat %3.2f|nage %3.2f|n",
						strrchr(tag_get_name(weapon->definition_index), '\\') + 1,
						weapon->weapon.magazines[0].rounds_total,
						magazine_definition->rounds_total_maximum,
						weapon->weapon.magazines[0].rounds_loaded,
						magazine_definition->rounds_loaded_maximum,
						weapon->weapon.heat,
						weapon->weapon.age);
				}
				else
				{
					sprintf(temporary + strlen(temporary), "%s|n", strrchr(tag_get_name(weapon->definition_index), '\\') + 1);
				}

				sprintf(
					temporary + strlen(temporary),
					"%s%s",
					player->powerup_durations[_player_powerup_active_camouflage] > 0.f ? "ACTIVE-CAMOUFLAGE " : "",
					player->powerup_durations[_player_powerup_full_spectrum_vision] > 0.f ? "FULL-SPECTRUM VISION " : "");
				temporary_hud_draw_reticle(
					weapon_definition->weapon.aim_assist_parameters.autoaim_angle / weapon_get_zoom_magnification(weapon_index, player_control_get_zoom_level(render.local_player_index)),
					player_control_get_autoaim_level(render.local_player_index) < 1.f ? global_real_argb_blue : global_real_argb_red);
				trigger = &weapon->weapon.triggers[0];
				trigger_definition = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.triggers, 0, struct weapon_trigger_definition);
				error = TEST_FLAG(trigger_definition->flags, _weapon_trigger_analog_rate_of_fire_bit) ? weapon->weapon.primary_trigger : trigger->error;
				angle = (1.f - error) * trigger_definition->projectile_error_angle_lower_bound + error * trigger_definition->projectile_error_angle_upper_bound;
				temporary_hud_draw_reticle(angle, global_real_argb_yellow);
			}

			{
				rectangle2d bounds = render.camera.window_bounds;

				bounds.x0 += 100;
				draw_string_set_format(_text_style_plain, _text_justification_left, 0);
				draw_string_set_color(global_real_argb_white);
				rasterizer_draw_string(&bounds, NULL, NULL, 0, temporary);
			}
		}
	}

	return;
}
