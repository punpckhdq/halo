/*
HS_GLOBALS_EXTERNAL.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hs.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "network_messages.h"
#include "collisions.h"
#include "sound_manager.h"
#include "vehicles.h"
#include "ai_debug.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "render_debug.h"
#include "director.h"
#include "weapons.h"
#include "bipeds.h"
#include "texture_cache.h"
#include "structures.h"
#include "hud.h"
#include "network_client_manager.h"
#include "effects.h"
#include "object_lights.h"
#include "devices.h"
#include "projectiles.h"
#include "physics.h"
#include "ai_profile.h"
#include "sound_cache.h"
#include "point_physics.h"
#include "recorded_animations.h"
#include "decals.h"
#include "equipment.h"
#include "bink_playback.h"
#include "motion_sensor.h"
#include "material_effects.h"
#include "weather_particle_systems.h"
#include "collision_debug.h"
#include "structure_render.h"
#include "sound_dsound.h"
#include "network_connection.h"
#include "terminal.h"
#include "breakable_surfaces.h"
#include "collision_usage.h"
#include "objects.h"
#include "path.h"
#include "scenario.h"
#include "model_animation_definitions.h"

/* ---------- globals */

static struct hs_global_external screenshot_size_definition = {"screenshot_size", _hs_type_short_integer, &global_screenshot_size};
static struct hs_global_external screenshot_count_definition = {"screenshot_count", _hs_type_short_integer, &global_screenshot_count};
static struct hs_global_external player_spawn_count_definition = {"player_spawn_count", _hs_type_short_integer, &player_spawn_count};
static struct hs_global_external framerate_throttle_definition = {"framerate_throttle", _hs_type_boolean, &global_frame_rate_throttle};
static struct hs_global_external framerate_lock_definition = {"framerate_lock", _hs_type_boolean, &debug_force_frame_rate_update};
static struct hs_global_external debug_framerate_definition = {"debug_framerate", _hs_type_boolean, &debug_frame_rate};
static struct hs_global_external display_framerate_definition = {"display_framerate", _hs_type_boolean, &display_framerate};
static struct hs_global_external display_vblank_deltas_definition = {"display_vblank_deltas", _hs_type_boolean, &display_vblank_deltas};
static struct hs_global_external display_precache_progress_definition = {"display_precache_progress", _hs_type_boolean, &display_precache_progress};
static struct hs_global_external debug_game_save_definition = {"debug_game_save", _hs_type_boolean, &debug_game_save};
static struct hs_global_external terminal_render_definition = {"terminal_render", _hs_type_boolean, &terminal_render_enable}; /* fake name? */
static struct hs_global_external console_dump_to_file_definition = {"console_dump_to_file", _hs_type_boolean, &console_dump_to_file};
static struct hs_global_external rasterizer_near_clip_distance_definition = {"rasterizer_near_clip_distance", _hs_type_real, &rasterizer_globals.z_near};
static struct hs_global_external rasterizer_far_clip_distance_definition = {"rasterizer_far_clip_distance", _hs_type_real, &rasterizer_globals.z_far};
static struct hs_global_external rasterizer_first_person_weapon_near_clip_distance_definition = {"rasterizer_first_person_weapon_near_clip_distance", _hs_type_real, &rasterizer_globals.z_near_first_person};
static struct hs_global_external rasterizer_first_person_weapon_far_clip_distance_definition = {"rasterizer_first_person_weapon_far_clip_distance", _hs_type_real, &rasterizer_globals.z_far_first_person};
static struct hs_global_external rasterizer_pushbuffer_size_definition = {"rasterizer_pushbuffer_size", _hs_type_short_integer, &rasterizer_globals.pushbuffer_size}; /* fake name? */
static struct hs_global_external rasterizer_pushbuffer_kickoff_size_definition = {"rasterizer_pushbuffer_kickoff_size", _hs_type_short_integer, &rasterizer_globals.pushbuffer_kickoff_size}; /* fake name? */
static struct hs_global_external rasterizer_floating_point_zbuffer_definition = {"rasterizer_floating_point_zbuffer", _hs_type_boolean, &rasterizer_globals.use_floating_point_zbuffer};
static struct hs_global_external rasterizer_framerate_throttle_definition = {"rasterizer_framerate_throttle", _hs_type_boolean, &rasterizer_globals.use_rasterizer_frame_rate_throttle};
static struct hs_global_external rasterizer_framerate_stabilization_definition = {"rasterizer_framerate_stabilization", _hs_type_boolean, &rasterizer_globals.use_rasterizer_frame_rate_stabilization};
static struct hs_global_external rasterizer_refresh_rate_definition = {"rasterizer_refresh_rate", _hs_type_short_integer, &rasterizer_globals.refresh_rate};
static struct hs_global_external rasterizer_frame_bounds_left_definition = {"rasterizer_frame_bounds_left", _hs_type_short_integer, &rasterizer_globals.frame_bounds.x0};
static struct hs_global_external rasterizer_frame_bounds_right_definition = {"rasterizer_frame_bounds_right", _hs_type_short_integer, &rasterizer_globals.frame_bounds.x1};
static struct hs_global_external rasterizer_frame_bounds_top_definition = {"rasterizer_frame_bounds_top", _hs_type_short_integer, &rasterizer_globals.frame_bounds.y0};
static struct hs_global_external rasterizer_frame_bounds_bottom_definition = {"rasterizer_frame_bounds_bottom", _hs_type_short_integer, &rasterizer_globals.frame_bounds.y1};
static struct hs_global_external rasterizer_stats_definition = {"rasterizer_stats", _hs_type_short_integer, &rasterizer_debug_options.statistics_mode};
static struct hs_global_external rasterizer_mode_definition = {"rasterizer_mode", _hs_type_short_integer, &rasterizer_debug_options.drawing_mode};
static struct hs_global_external rasterizer_wireframe_definition = {"rasterizer_wireframe", _hs_type_boolean, &rasterizer_debug_options.wireframe_enabled};
static struct hs_global_external rasterizer_smart_definition = {"rasterizer_smart", _hs_type_boolean, &rasterizer_debug_options.smart_states_enabled};
static struct hs_global_external rasterizer_debug_model_vertices_definition = {"rasterizer_debug_model_vertices", _hs_type_boolean, &rasterizer_debug_options.debug_model_vertices_enabled};
static struct hs_global_external rasterizer_debug_model_lod_definition = {"rasterizer_debug_model_lod", _hs_type_short_integer, &rasterizer_debug_options.debug_model_lod};
static struct hs_global_external rasterizer_debug_transparents_definition = {"rasterizer_debug_transparents", _hs_type_boolean, &rasterizer_debug_options.debug_transparent_geometry_enabled};
static struct hs_global_external rasterizer_debug_meter_shader_definition = {"rasterizer_debug_meter_shader", _hs_type_boolean, &rasterizer_debug_options.debug_meter_shader_enabled};
static struct hs_global_external rasterizer_models_definition = {"rasterizer_models", _hs_type_boolean, &rasterizer_debug_options.draw_models};
static struct hs_global_external rasterizer_model_transparents_definition = {"rasterizer_model_transparents", _hs_type_boolean, &rasterizer_debug_options.draw_model_transparent_geometry};
static struct hs_global_external rasterizer_draw_first_person_weapon_first_definition = {"rasterizer_draw_first_person_weapon_first", _hs_type_boolean, &rasterizer_debug_options.draw_first_person_weapon_first};
static struct hs_global_external rasterizer_stencil_mask_definition = {"rasterizer_stencil_mask", _hs_type_boolean, &rasterizer_debug_options.stencil_mask_enabled};
static struct hs_global_external rasterizer_environment_definition = {"rasterizer_environment", _hs_type_boolean, &rasterizer_debug_options.draw_environment};
static struct hs_global_external rasterizer_environment_lightmaps_definition = {"rasterizer_environment_lightmaps", _hs_type_boolean, &rasterizer_debug_options.draw_environment_lightmaps};
static struct hs_global_external rasterizer_environment_shadows_definition = {"rasterizer_environment_shadows", _hs_type_boolean, &rasterizer_debug_options.draw_environment_shadows};
static struct hs_global_external rasterizer_environment_diffuse_lights_definition = {"rasterizer_environment_diffuse_lights", _hs_type_boolean, &rasterizer_debug_options.draw_environment_diffuse_lights};
static struct hs_global_external rasterizer_environment_diffuse_textures_definition = {"rasterizer_environment_diffuse_textures", _hs_type_boolean, &rasterizer_debug_options.draw_environment_textures};
static struct hs_global_external rasterizer_environment_decals_definition = {"rasterizer_environment_decals", _hs_type_boolean, &rasterizer_debug_options.draw_environment_decals};
static struct hs_global_external rasterizer_environment_specular_lights_definition = {"rasterizer_environment_specular_lights", _hs_type_boolean, &rasterizer_debug_options.draw_environment_specular_lights};
static struct hs_global_external rasterizer_environment_specular_lightmaps_definition = {"rasterizer_environment_specular_lightmaps", _hs_type_boolean, &rasterizer_debug_options.draw_environment_specular_lightmaps};
static struct hs_global_external rasterizer_environment_reflection_lightmap_mask_definition = {"rasterizer_environment_reflection_lightmap_mask", _hs_type_boolean, &rasterizer_debug_options.draw_environment_reflection_lightmap_masks};
static struct hs_global_external rasterizer_environment_reflection_mirrors_definition = {"rasterizer_environment_reflection_mirrors", _hs_type_boolean, &rasterizer_debug_options.draw_environment_reflection_mirrors};
static struct hs_global_external rasterizer_environment_reflections_definition = {"rasterizer_environment_reflections", _hs_type_boolean, &rasterizer_debug_options.draw_environment_reflections};
static struct hs_global_external rasterizer_environment_transparents_definition = {"rasterizer_environment_transparents", _hs_type_boolean, &rasterizer_debug_options.draw_environment_transparent_geometry};
static struct hs_global_external rasterizer_environment_fog_definition = {"rasterizer_environment_fog", _hs_type_boolean, &rasterizer_debug_options.draw_environment_fog};
static struct hs_global_external rasterizer_environment_fog_screen_definition = {"rasterizer_environment_fog_screen", _hs_type_boolean, &rasterizer_debug_options.draw_environment_fog_screen};
static struct hs_global_external rasterizer_water_definition = {"rasterizer_water", _hs_type_boolean, &rasterizer_debug_options.draw_water};
static struct hs_global_external rasterizer_lens_flares_definition = {"rasterizer_lens_flares", _hs_type_boolean, &rasterizer_debug_options.draw_lens_flares};
static struct hs_global_external rasterizer_dynamic_unlit_geometry_definition = {"rasterizer_dynamic_unlit_geometry", _hs_type_boolean, &rasterizer_debug_options.draw_dynamic_unlit_geometry};
static struct hs_global_external rasterizer_dynamic_lit_geometry_definition = {"rasterizer_dynamic_lit_geometry", _hs_type_boolean, &rasterizer_debug_options.draw_dynamic_lit_geometry};
static struct hs_global_external rasterizer_dynamic_screen_geometry_definition = {"rasterizer_dynamic_screen_geometry", _hs_type_boolean, &rasterizer_debug_options.draw_dynamic_screen_geometry};
static struct hs_global_external rasterizer_hud_motion_sensor_definition = {"rasterizer_hud_motion_sensor", _hs_type_boolean, &rasterizer_debug_options.draw_hud_motion_sensor};
static struct hs_global_external rasterizer_detail_objects_definition = {"rasterizer_detail_objects", _hs_type_boolean, &rasterizer_debug_options.draw_detail_objects};
static struct hs_global_external rasterizer_debug_geometry_definition = {"rasterizer_debug_geometry", _hs_type_boolean, &rasterizer_debug_options.draw_debug_geometry};
static struct hs_global_external rasterizer_debug_geometry_multipass_definition = {"rasterizer_debug_geometry_multipass", _hs_type_boolean, &rasterizer_debug_options.debug_geometry_multipass};
static struct hs_global_external rasterizer_fog_atmosphere_definition = {"rasterizer_fog_atmosphere", _hs_type_boolean, &rasterizer_debug_options.fog_atmospheric_enabled};
static struct hs_global_external rasterizer_fog_plane_definition = {"rasterizer_fog_plane", _hs_type_boolean, &rasterizer_debug_options.fog_planar_enabled};
static struct hs_global_external rasterizer_bump_mapping_definition = {"rasterizer_bump_mapping", _hs_type_boolean, &rasterizer_debug_options.bump_mapping_enabled};
static struct hs_global_external rasterizer_lightmap_ambient_definition = {"rasterizer_lightmap_ambient", _hs_type_real, &rasterizer_debug_options.lightmap_ambient};
static struct hs_global_external rasterizer_lightmap_mode_definition = {"rasterizer_lightmap_mode", _hs_type_short_integer, &rasterizer_globals.lightmap_mode};
static struct hs_global_external rasterizer_lightmaps_incident_radiosity_definition = {"rasterizer_lightmaps_incident_radiosity", _hs_type_boolean, &rasterizer_debug_options.lightmap_incident_radiosity_enabled};
static struct hs_global_external rasterizer_lightmaps_filtering_definition = {"rasterizer_lightmaps_filtering", _hs_type_boolean, &rasterizer_debug_options.lightmap_filtering_enabled};
static struct hs_global_external rasterizer_model_lighting_ambient_definition = {"rasterizer_model_lighting_ambient", _hs_type_real, &rasterizer_debug_options.model_lighting_ambient};
static struct hs_global_external rasterizer_environment_alpha_testing_definition = {"rasterizer_environment_alpha_testing", _hs_type_boolean, &rasterizer_debug_options.environment_alpha_testing_enabled};
static struct hs_global_external rasterizer_environment_specular_mask_definition = {"rasterizer_environment_specular_mask", _hs_type_boolean, &rasterizer_debug_options.environment_specular_mask_enabled};
static struct hs_global_external rasterizer_shadows_convolution_definition = {"rasterizer_shadows_convolution", _hs_type_boolean, &rasterizer_debug_options.shadow_convolution_enabled};
static struct hs_global_external rasterizer_shadows_debug_definition = {"rasterizer_shadows_debug", _hs_type_boolean, &rasterizer_debug_options.shadow_debug_enabled};
static struct hs_global_external rasterizer_water_mipmapping_definition = {"rasterizer_water_mipmapping", _hs_type_boolean, &rasterizer_debug_options.water_mipmapping_enabled};
static struct hs_global_external rasterizer_active_camouflage_definition = {"rasterizer_active_camouflage", _hs_type_boolean, &rasterizer_debug_options.active_camouflage_enabled};
static struct hs_global_external rasterizer_active_camouflage_multipass_definition = {"rasterizer_active_camouflage_multipass", _hs_type_boolean, &rasterizer_debug_options.active_camouflage_multipass_enabled};
static struct hs_global_external rasterizer_plasma_energy_definition = {"rasterizer_plasma_energy", _hs_type_boolean, &rasterizer_debug_options.plasma_energy_enabled};
static struct hs_global_external rasterizer_lens_flares_occlusion_definition = {"rasterizer_lens_flares_occlusion", _hs_type_boolean, &rasterizer_debug_options.lens_flare_occlusion_enabled};
static struct hs_global_external rasterizer_lens_flares_occlusion_debug_definition = {"rasterizer_lens_flares_occlusion_debug", _hs_type_boolean, &rasterizer_debug_options.lens_flare_occlusion_debug};
static struct hs_global_external rasterizer_ray_of_buddha_definition = {"rasterizer_ray_of_buddha", _hs_type_boolean, &rasterizer_debug_options.lens_flare_sun_glow_enabled};
static struct hs_global_external rasterizer_screen_flashes_definition = {"rasterizer_screen_flashes", _hs_type_boolean, &rasterizer_debug_options.screen_flash_enabled};
static struct hs_global_external rasterizer_screen_effects_definition = {"rasterizer_screen_effects", _hs_type_boolean, &rasterizer_debug_options.screen_effects_enabled};
static struct hs_global_external rasterizer_DXTC_noise_definition = {"rasterizer_DXTC_noise", _hs_type_boolean, &rasterizer_debug_options.DXTC_noise_enabled};
static struct hs_global_external rasterizer_soft_filter_definition = {"rasterizer_soft_filter", _hs_type_boolean, &rasterizer_debug_options.soft_filter_enabled};
static struct hs_global_external rasterizer_secondary_render_target_debug_definition = {"rasterizer_secondary_render_target_debug", _hs_type_boolean, &rasterizer_debug_options.secondary_render_target_debug_enabled};
static struct hs_global_external rasterizer_profile_log_definition = {"rasterizer_profile_log", _hs_type_boolean, &rasterizer_debug_options.profile_log_enabled};
static struct hs_global_external rasterizer_detail_objects_offset_multiplier_definition = {"rasterizer_detail_objects_offset_multiplier", _hs_type_real, &rasterizer_debug_options.detail_object_screen_facing_offset_multiplier};
static struct hs_global_external rasterizer_zbias_definition = {"rasterizer_zbias", _hs_type_long_integer, &rasterizer_debug_options.zbias};
static struct hs_global_external rasterizer_zoffset_definition = {"rasterizer_zoffset", _hs_type_real, &rasterizer_debug_options.zoffset};
static struct hs_global_external force_all_player_views_to_default_player_definition = {"force_all_player_views_to_default_player", _hs_type_boolean, &rasterizer_debug_options.force_all_player_views_to_default_player};
static struct hs_global_external rasterizer_safe_frame_bounds_definition = {"rasterizer_safe_frame_bounds", _hs_type_boolean, &rasterizer_debug_options.safe_frame_bounds_adjust_enabled};
static struct hs_global_external freeze_flying_camera_definition = {"freeze_flying_camera", _hs_type_short_integer, &rasterizer_debug_options.freeze_flying_camera};
static struct hs_global_external rasterizer_zsprites_definition = {"rasterizer_zsprites", _hs_type_boolean, &rasterizer_debug_options.zsprite_enabled};
static struct hs_global_external rasterizer_filthy_decal_fog_hack_definition = {"rasterizer_filthy_decal_fog_hack", _hs_type_boolean, &rasterizer_debug_options.filthy_decal_fog_hack_enabled};
static struct hs_global_external rasterizer_splitscreen_VB_optimization_definition = {"rasterizer_splitscreen_VB_optimization", _hs_type_boolean, &rasterizer_debug_options.splitscreen_VB_optimization_enabled};
static struct hs_global_external rasterizer_profile_print_locks_definition = {"rasterizer_profile_print_locks", _hs_type_boolean, &rasterizer_debug_options.profile_print_locks};
static struct hs_global_external rasterizer_profile_objectlock_time_definition = {"rasterizer_profile_objectlock_time", _hs_type_real, &rasterizer_debug_options.profile_objectlock_time};
static struct hs_global_external pad3_definition = {"pad3", _hs_type_short_integer, &rasterizer_debug_options.pad3};
static struct hs_global_external pad3_scale_definition = {"pad3_scale", _hs_type_real, &rasterizer_debug_options.pad3_scale};
static struct hs_global_external f0_definition = {"f0", _hs_type_real, &rasterizer_debug_options.f[0]};
static struct hs_global_external f1_definition = {"f1", _hs_type_real, &rasterizer_debug_options.f[1]};
static struct hs_global_external f2_definition = {"f2", _hs_type_real, &rasterizer_debug_options.f[2]};
static struct hs_global_external f3_definition = {"f3", _hs_type_real, &rasterizer_debug_options.f[3]};
static struct hs_global_external f4_definition = {"f4", _hs_type_real, &rasterizer_debug_options.f[4]};
static struct hs_global_external f5_definition = {"f5", _hs_type_real, &rasterizer_debug_options.f[5]};
static struct hs_global_external rasterizer_transparent_pixel_counter_definition = {"rasterizer_transparent_pixel_counter", _hs_type_boolean, &rasterizer_debug_options.transparent_pixel_counter_enabled};
static struct hs_global_external debug_no_frustum_clip_definition = {"debug_no_frustum_clip", _hs_type_boolean, &debug_no_frustum_clip};
static struct hs_global_external debug_frustum_definition = {"debug_frustum", _hs_type_boolean, &render_camera_debug_this_fucking_frustum};
static struct hs_global_external debug_bink_definition = {"debug_bink", _hs_type_boolean, &debug_bink};
static struct hs_global_external recover_saved_games_hack_definition = {"recover_saved_games_hack", _hs_type_boolean, &recover_saved_games_hack};
static struct hs_global_external radiosity_quality_definition = {"radiosity_quality", _hs_type_short_integer, NULL}; /* fake name? */
static struct hs_global_external radiosity_step_count_definition = {"radiosity_step_count", _hs_type_short_integer, NULL}; /* fake name? */
static struct hs_global_external radiosity_lines_definition = {"radiosity_lines", _hs_type_boolean, NULL}; /* fake name? */
static struct hs_global_external radiosity_normals_definition = {"radiosity_normals", _hs_type_boolean, NULL}; /* fake name? */
static struct hs_global_external structures_use_pvs_for_vs_definition = {"structures_use_pvs_for_vs", _hs_type_boolean, &structures_use_pvs_for_vs};
static struct hs_global_external debug_detail_objects_definition = {"debug_detail_objects", _hs_type_boolean, &debug_detail_objects};
static struct hs_global_external debug_texture_cache_definition = {"debug_texture_cache", _hs_type_boolean, &debug_texture_cache};
static struct hs_global_external temporary_hud_definition = {"temporary_hud", _hs_type_boolean, &temporary_hud};
static struct hs_global_external debug_object_garbage_collection_definition = {"debug_object_garbage_collection", _hs_type_boolean, &debug_object_garbage_collection};
static struct hs_global_external debug_render_freeze_definition = {"debug_render_freeze", _hs_type_boolean, &debug_render_freeze};
static struct hs_global_external debug_no_drawing_definition = {"debug_no_drawing", _hs_type_boolean, &debug_no_drawing};
static struct hs_global_external debug_input_target_definition = {"debug_input_target", _hs_type_short_integer, &debug_input_target};
static struct hs_global_external debug_leaf_index_definition = {"debug_leaf_index", _hs_type_long_integer, &debug_leaf_index};
static struct hs_global_external debug_leaf_portal_index_definition = {"debug_leaf_portal_index", _hs_type_long_integer, &debug_leaf_portal_index};
static struct hs_global_external debug_leaf_portals_definition = {"debug_leaf_portals", _hs_type_boolean, &debug_leaf_portals};
static struct hs_global_external debug_unit_all_animations_definition = {"debug_unit_all_animations", _hs_type_boolean, &debug_unit_all_animations};
static struct hs_global_external debug_unit_animations_definition = {"debug_unit_animations", _hs_type_boolean, &debug_unit_animations};
static struct hs_global_external debug_unit_illumination_definition = {"debug_unit_illumination", _hs_type_boolean, &debug_unit_illumination};
static struct hs_global_external debug_damage_taken_definition = {"debug_damage_taken", _hs_type_boolean, &debug_damage_taken};
static struct hs_global_external cheat_deathless_player_definition = {"cheat_deathless_player", _hs_type_boolean, &cheat.deathless_player};
static struct hs_global_external cheat_jetpack_definition = {"cheat_jetpack", _hs_type_boolean, &cheat.jetpack};
static struct hs_global_external cheat_infinite_ammo_definition = {"cheat_infinite_ammo", _hs_type_boolean, &cheat.infinite_ammo};
static struct hs_global_external cheat_bottomless_clip_definition = {"cheat_bottomless_clip", _hs_type_boolean, &cheat.bottomless_clip};
static struct hs_global_external cheat_bump_possession_definition = {"cheat_bump_possession", _hs_type_boolean, &cheat.bump_possession};
static struct hs_global_external cheat_super_jump_definition = {"cheat_super_jump", _hs_type_boolean, &cheat.super_jump};
static struct hs_global_external cheat_reflexive_damage_effects_definition = {"cheat_reflexive_damage_effects", _hs_type_boolean, &cheat.reflexive_damage_effects};
static struct hs_global_external cheat_medusa_definition = {"cheat_medusa", _hs_type_boolean, &cheat.medusa};
static struct hs_global_external cheat_omnipotent_definition = {"cheat_omnipotent", _hs_type_boolean, &cheat.omnipotent};
static struct hs_global_external cheat_controller_definition = {"cheat_controller", _hs_type_boolean, &cheat.controller_enabled};
static struct hs_global_external effects_corpse_nonviolent_definition = {"effects_corpse_nonviolent", _hs_type_boolean, &effects_corpse_nonviolent};
static struct hs_global_external debug_effects_nonviolent_definition = {"debug_effects_nonviolent", _hs_type_boolean, &debug_effects_nonviolent};
static struct hs_global_external debug_sound_cache_definition = {"debug_sound_cache", _hs_type_boolean, &debug_sound_cache};
static struct hs_global_external debug_sound_definition = {"debug_sound", _hs_type_boolean, &debug_sound};
static struct hs_global_external debug_looping_sound_definition = {"debug_looping_sound", _hs_type_boolean, &debug_looping_sound};
static struct hs_global_external debug_sound_channels_definition = {"debug_sound_channels", _hs_type_boolean, &debug_sound_channels};
static struct hs_global_external loud_dialog_hack_definition = {"loud_dialog_hack", _hs_type_boolean, &loud_dialog_hack};
static struct hs_global_external sound_gain_under_dialog_definition = {"sound_gain_under_dialog", _hs_type_real, &sound_gain_under_dialog};
static struct hs_global_external debug_sound_environment_definition = {"debug_sound_environment", _hs_type_boolean, &debug_sound_environment};
static struct hs_global_external object_light_ambient_base_definition = {"object_light_ambient_base", _hs_type_real, &object_light_ambient_base};
static struct hs_global_external object_light_ambient_scale_definition = {"object_light_ambient_scale", _hs_type_real, &object_light_ambient_scale};
static struct hs_global_external object_light_secondary_scale_definition = {"object_light_secondary_scale", _hs_type_real, &object_light_secondary_scale};
static struct hs_global_external object_light_interpolate_definition = {"object_light_interpolate", _hs_type_boolean, &object_light_interpolate};
static struct hs_global_external rider_ejection_definition = {"rider_ejection", _hs_type_boolean, &rider_ejection};
static struct hs_global_external stun_enable_definition = {"stun_enable", _hs_type_boolean, &stun_enable};
static struct hs_global_external collision_log_render_definition = {"collision_log_render", _hs_type_boolean, &collision_log_render_enable};
static struct hs_global_external collision_log_detailed_definition = {"collision_log_detailed", _hs_type_boolean, &collision_log_detailed};
static struct hs_global_external collision_log_extended_definition = {"collision_log_extended", _hs_type_boolean, &collision_log_extended};
static struct hs_global_external collision_log_totals_only_definition = {"collision_log_totals_only", _hs_type_boolean, &collision_log_totals_only};
static struct hs_global_external collision_log_time_definition = {"collision_log_time", _hs_type_boolean, &collision_log_time};
static struct hs_global_external profile_graph_definition = {"profile_graph", _hs_type_boolean, &profile_graph};
static struct hs_global_external profile_display_definition = {"profile_display", _hs_type_boolean, &profile_display};
static struct hs_global_external profile_timebase_ticks_definition = {"profile_timebase_ticks", _hs_type_boolean, &profile_timebase_ticks};
static struct hs_global_external profile_dump_frames_definition = {"profile_dump_frames", _hs_type_boolean, &profile_dump_frames};
static struct hs_global_external profile_dump_lost_frames_definition = {"profile_dump_lost_frames", _hs_type_boolean, &profile_dump_lost_frames};
static struct hs_global_external model_animation_compression_definition = {"model_animation_compression", _hs_type_boolean, &hs_model_animation_compression_enabled};
static struct hs_global_external model_animation_data_compressed_size_definition = {"model_animation_data_compressed_size", _hs_type_long_integer, &hs_model_animation_data_compressed_size};
static struct hs_global_external model_animation_data_uncompressed_size_definition = {"model_animation_data_uncompressed_size", _hs_type_long_integer, &hs_model_animation_data_uncompressed_size};
static struct hs_global_external model_animation_data_compression_savings_in_bytes_definition = {"model_animation_data_compression_savings_in_bytes", _hs_type_long_integer, &hs_model_animation_data_compression_savings_in_bytes};
static struct hs_global_external model_animation_data_compression_savings_in_bytes_at_import_definition = {"model_animation_data_compression_savings_in_bytes_at_import", _hs_type_long_integer, &hs_model_animation_data_compression_savings_in_bytes_at_import};
static struct hs_global_external model_animation_data_compression_savings_in_percent_definition = {"model_animation_data_compression_savings_in_percent", _hs_type_real, &hs_model_animation_data_compression_savings_in_percent};
static struct hs_global_external model_animation_bullshit0_definition = {"model_animation_bullshit0", _hs_type_long_integer, &hs_model_animation_bullshit[0]};
static struct hs_global_external model_animation_bullshit1_definition = {"model_animation_bullshit1", _hs_type_long_integer, &hs_model_animation_bullshit[4]};
static struct hs_global_external model_animation_bullshit2_definition = {"model_animation_bullshit2", _hs_type_long_integer, &hs_model_animation_bullshit[8]};
static struct hs_global_external model_animation_bullshit3_definition = {"model_animation_bullshit3", _hs_type_long_integer, &hs_model_animation_bullshit[12]};
static struct hs_global_external debug_portals_definition = {"debug_portals", _hs_type_boolean, &debug_portals};
static struct hs_global_external debug_sprites_definition = {"debug_sprites", _hs_type_boolean, &debug_sprites};
static struct hs_global_external debug_inactive_objects_definition = {"debug_inactive_objects", _hs_type_boolean, &debug_inactive_objects};
static struct hs_global_external render_contrails_definition = {"render_contrails", _hs_type_boolean, &render_contrails_enabled};
static struct hs_global_external render_particles_definition = {"render_particles", _hs_type_boolean, &render_particles_enabled};
static struct hs_global_external render_psystems_definition = {"render_psystems", _hs_type_boolean, &render_particle_systems_enabled};
static struct hs_global_external render_wsystems_definition = {"render_wsystems", _hs_type_boolean, &render_weather_particle_systems_enabled};
static struct hs_global_external debug_objects_definition = {"debug_objects", _hs_type_boolean, &debug_objects};
static struct hs_global_external debug_objects_position_velocity_definition = {"debug_objects_position_velocity", _hs_type_boolean, &debug_objects_position_velocity};
static struct hs_global_external debug_objects_root_node_definition = {"debug_objects_root_node", _hs_type_boolean, &debug_objects_root_node};
static struct hs_global_external debug_objects_bounding_spheres_definition = {"debug_objects_bounding_spheres", _hs_type_boolean, &debug_objects_bounding_spheres};
static struct hs_global_external debug_objects_collision_models_definition = {"debug_objects_collision_models", _hs_type_boolean, &debug_objects_collision_models};
static struct hs_global_external debug_objects_physics_definition = {"debug_objects_physics", _hs_type_boolean, &debug_objects_physics};
static struct hs_global_external debug_objects_names_definition = {"debug_objects_names", _hs_type_boolean, &debug_objects_names};
static struct hs_global_external debug_objects_pathfinding_spheres_definition = {"debug_objects_pathfinding_spheres", _hs_type_boolean, &debug_objects_pathfinding_spheres};
static struct hs_global_external debug_objects_unit_vectors_definition = {"debug_objects_unit_vectors", _hs_type_boolean, &debug_objects_unit_vectors};
static struct hs_global_external debug_objects_unit_seats_definition = {"debug_objects_unit_seats", _hs_type_boolean, &debug_objects_unit_seats};
static struct hs_global_external debug_objects_unit_mouth_apeture_definition = {"debug_objects_unit_mouth_apeture", _hs_type_boolean, &debug_objects_unit_mouth_apeture};
static struct hs_global_external debug_objects_biped_physics_pills_definition = {"debug_objects_biped_physics_pills", _hs_type_boolean, &debug_objects_biped_physics_pills};
static struct hs_global_external debug_objects_biped_autoaim_pills_definition = {"debug_objects_biped_autoaim_pills", _hs_type_boolean, &debug_objects_biped_autoaim_pills};
static struct hs_global_external debug_objects_vehicle_powered_mass_points_definition = {"debug_objects_vehicle_powered_mass_points", _hs_type_boolean, &debug_objects_vehicle_powered_mass_points};
static struct hs_global_external debug_objects_devices_definition = {"debug_objects_devices", _hs_type_boolean, &debug_objects_devices};
static struct hs_global_external render_model_nodes_definition = {"render_model_nodes", _hs_type_boolean, &render_model_nodes};
static struct hs_global_external render_model_vertex_counts_definition = {"render_model_vertex_counts", _hs_type_boolean, &render_model_vertex_counts};
static struct hs_global_external render_model_index_counts_definition = {"render_model_index_counts", _hs_type_boolean, &render_model_index_counts};
static struct hs_global_external render_model_markers_definition = {"render_model_markers", _hs_type_boolean, &render_model_markers};
static struct hs_global_external render_model_no_geometry_definition = {"render_model_no_geometry", _hs_type_boolean, &render_model_no_geometry};
static struct hs_global_external render_shadows_definition = {"render_shadows", _hs_type_boolean, &render_shadows};
static struct hs_global_external debug_damage_definition = {"debug_damage", _hs_type_boolean, &debug_damage};
static struct hs_global_external debug_scripting_definition = {"debug_scripting", _hs_type_boolean, &debug_scripting};
static struct hs_global_external debug_trigger_volumes_definition = {"debug_trigger_volumes", _hs_type_boolean, &debug_trigger_volumes};
static struct hs_global_external debug_point_physics_definition = {"debug_point_physics", _hs_type_boolean, &debug_point_physics};
static struct hs_global_external debug_physics_disable_penetration_freeze_definition = {"debug_physics_disable_penetration_freeze", _hs_type_boolean, &debug_physics_disable_penetration_freeze};
static struct hs_global_external debug_motion_sensor_draw_all_units_definition = {"debug_motion_sensor_draw_all_units", _hs_type_boolean, &debug_motion_sensor_draw_all_units};
static struct hs_global_external collision_debug_definition = {"collision_debug", _hs_type_boolean, &collision_debug};
static struct hs_global_external collision_debug_spray_definition = {"collision_debug_spray", _hs_type_boolean, &collision_debug_spray};
static struct hs_global_external collision_debug_features_definition = {"collision_debug_features", _hs_type_boolean, &collision_debug_features};
static struct hs_global_external collision_debug_repeat_definition = {"collision_debug_repeat", _hs_type_boolean, &collision_debug_repeat};
static struct hs_global_external collision_debug_flag_front_facing_surfaces_definition = {"collision_debug_flag_front_facing_surfaces", _hs_type_boolean, &collision_debug_flag_front_facing_surfaces};
static struct hs_global_external collision_debug_flag_back_facing_surfaces_definition = {"collision_debug_flag_back_facing_surfaces", _hs_type_boolean, &collision_debug_flag_back_facing_surfaces};
static struct hs_global_external collision_debug_flag_ignore_two_sided_surfaces_definition = {"collision_debug_flag_ignore_two_sided_surfaces", _hs_type_boolean, &collision_debug_flag_ignore_two_sided_surfaces};
static struct hs_global_external collision_debug_flag_ignore_invisible_surfaces_definition = {"collision_debug_flag_ignore_invisible_surfaces", _hs_type_boolean, &collision_debug_flag_ignore_invisible_surfaces};
static struct hs_global_external collision_debug_flag_ignore_breakable_surfaces_definition = {"collision_debug_flag_ignore_breakable_surfaces", _hs_type_boolean, &collision_debug_flag_ignore_breakable_surfaces};
static struct hs_global_external collision_debug_flag_structure_definition = {"collision_debug_flag_structure", _hs_type_boolean, &collision_debug_flag_structure};
static struct hs_global_external collision_debug_flag_media_definition = {"collision_debug_flag_media", _hs_type_boolean, &collision_debug_flag_media};
static struct hs_global_external collision_debug_flag_objects_definition = {"collision_debug_flag_objects", _hs_type_boolean, &collision_debug_flag_objects};
static struct hs_global_external collision_debug_flag_objects_bipeds_definition = {"collision_debug_flag_objects_bipeds", _hs_type_boolean, &collision_debug_flag_objects_bipeds};
static struct hs_global_external collision_debug_flag_objects_vehicles_definition = {"collision_debug_flag_objects_vehicles", _hs_type_boolean, &collision_debug_flag_objects_vehicles};
static struct hs_global_external collision_debug_flag_objects_weapons_definition = {"collision_debug_flag_objects_weapons", _hs_type_boolean, &collision_debug_flag_objects_weapons};
static struct hs_global_external collision_debug_flag_objects_equipment_definition = {"collision_debug_flag_objects_equipment", _hs_type_boolean, &collision_debug_flag_objects_equipment};
static struct hs_global_external collision_debug_flag_objects_projectiles_definition = {"collision_debug_flag_objects_projectiles", _hs_type_boolean, &collision_debug_flag_objects_projectiles};
static struct hs_global_external collision_debug_flag_objects_scenery_definition = {"collision_debug_flag_objects_scenery", _hs_type_boolean, &collision_debug_flag_objects_scenery};
static struct hs_global_external collision_debug_flag_objects_machines_definition = {"collision_debug_flag_objects_machines", _hs_type_boolean, &collision_debug_flag_objects_machines};
static struct hs_global_external collision_debug_flag_objects_controls_definition = {"collision_debug_flag_objects_controls", _hs_type_boolean, &collision_debug_flag_objects_controls};
static struct hs_global_external collision_debug_flag_objects_light_fixtures_definition = {"collision_debug_flag_objects_light_fixtures", _hs_type_boolean, &collision_debug_flag_objects_light_fixtures};
static struct hs_global_external collision_debug_flag_objects_placeholders_definition = {"collision_debug_flag_objects_placeholders", _hs_type_boolean, &collision_debug_flag_objects_placeholders};
static struct hs_global_external collision_debug_flag_try_to_keep_location_valid_definition = {"collision_debug_flag_try_to_keep_location_valid", _hs_type_boolean, &collision_debug_flag_try_to_keep_location_valid};
static struct hs_global_external collision_debug_flag_skip_passthrough_bipeds_definition = {"collision_debug_flag_skip_passthrough_bipeds", _hs_type_boolean, &collision_debug_flag_skip_passthrough_bipeds};
static struct hs_global_external collision_debug_flag_use_vehicle_physics_definition = {"collision_debug_flag_use_vehicle_physics", _hs_type_boolean, &collision_debug_flag_use_vehicle_physics};
static struct hs_global_external collision_debug_point_x_definition = {"collision_debug_point_x", _hs_type_real, &collision_debug_point.x};
static struct hs_global_external collision_debug_point_y_definition = {"collision_debug_point_y", _hs_type_real, &collision_debug_point.y};
static struct hs_global_external collision_debug_point_z_definition = {"collision_debug_point_z", _hs_type_real, &collision_debug_point.z};
static struct hs_global_external collision_debug_vector_i_definition = {"collision_debug_vector_i", _hs_type_real, &collision_debug_vector.i};
static struct hs_global_external collision_debug_vector_j_definition = {"collision_debug_vector_j", _hs_type_real, &collision_debug_vector.j};
static struct hs_global_external collision_debug_vector_k_definition = {"collision_debug_vector_k", _hs_type_real, &collision_debug_vector.k};
static struct hs_global_external collision_debug_length_definition = {"collision_debug_length", _hs_type_real, &collision_debug_length};
static struct hs_global_external collision_debug_width_definition = {"collision_debug_width", _hs_type_real, &collision_debug_width};
static struct hs_global_external collision_debug_height_definition = {"collision_debug_height", _hs_type_real, &collision_debug_height};
static struct hs_global_external collision_debug_phantom_bsp_definition = {"collision_debug_phantom_bsp", _hs_type_boolean, &collision_debug_phantom_bsp};
static struct hs_global_external debug_obstacle_path_definition = {"debug_obstacle_path", _hs_type_boolean, &debug_obstacle_path};
static struct hs_global_external debug_obstacle_path_on_failure_definition = {"debug_obstacle_path_on_failure", _hs_type_boolean, &debug_obstacle_path_on_failure};
static struct hs_global_external debug_obstacle_path_start_point_x_definition = {"debug_obstacle_path_start_point_x", _hs_type_real, &debug_obstacle_path_start_point.x};
static struct hs_global_external debug_obstacle_path_start_point_y_definition = {"debug_obstacle_path_start_point_y", _hs_type_real, &debug_obstacle_path_start_point.y};
static struct hs_global_external debug_obstacle_path_start_surface_index_definition = {"debug_obstacle_path_start_surface_index", _hs_type_long_integer, &debug_obstacle_path_start_surface_index};
static struct hs_global_external debug_obstacle_path_goal_point_x_definition = {"debug_obstacle_path_goal_point_x", _hs_type_real, &debug_obstacle_path_goal_point.x};
static struct hs_global_external debug_obstacle_path_goal_point_y_definition = {"debug_obstacle_path_goal_point_y", _hs_type_real, &debug_obstacle_path_goal_point.y};
static struct hs_global_external debug_obstacle_path_goal_surface_index_definition = {"debug_obstacle_path_goal_surface_index", _hs_type_long_integer, &debug_obstacle_path_goal_surface_index};
static struct hs_global_external debug_camera_definition = {"debug_camera", _hs_type_boolean, &debug_camera};
static struct hs_global_external debug_player_definition = {"debug_player", _hs_type_boolean, &debug_player};
static struct hs_global_external debug_structure_definition = {"debug_structure", _hs_type_boolean, &debug_structure};
static struct hs_global_external debug_bsp_definition = {"debug_bsp", _hs_type_boolean, &debug_bsp};
static struct hs_global_external debug_input_definition = {"debug_input", _hs_type_boolean, &debug_input};
static struct hs_global_external debug_permanent_decals_definition = {"debug_permanent_decals", _hs_type_boolean, &debug_permanent_decals};
static struct hs_global_external debug_fog_planes_definition = {"debug_fog_planes", _hs_type_boolean, &debug_fog_planes};
static struct hs_global_external breakable_surfaces_definition = {"breakable_surfaces", _hs_type_boolean, &breakable_surface_effect_enabled};
static struct hs_global_external decals_definition = {"decals", _hs_type_boolean, &decals_enabled};
static struct hs_global_external debug_decals_definition = {"debug_decals", _hs_type_boolean, &debug_decals};
static struct hs_global_external debug_object_lights_definition = {"debug_object_lights", _hs_type_boolean, &debug_object_lights};
static struct hs_global_external debug_lights_definition = {"debug_lights", _hs_type_boolean, &debug_lights};
static struct hs_global_external debug_biped_physics_definition = {"debug_biped_physics", _hs_type_boolean, &debug_biped_physics};
static struct hs_global_external debug_biped_skip_update_definition = {"debug_biped_skip_update", _hs_type_boolean, &debug_biped_skip_update};
static struct hs_global_external debug_biped_skip_collision_definition = {"debug_biped_skip_collision", _hs_type_boolean, &debug_biped_skip_collision};
static struct hs_global_external debug_biped_limp_body_disable_definition = {"debug_biped_limp_body_disable", _hs_type_boolean, &debug_biped_limp_body_disable};
static struct hs_global_external debug_collision_skip_objects_definition = {"debug_collision_skip_objects", _hs_type_boolean, &debug_collision_skip_objects};
static struct hs_global_external debug_collision_skip_vectors_definition = {"debug_collision_skip_vectors", _hs_type_boolean, &debug_collision_skip_vectors};
static struct hs_global_external debug_material_effects_definition = {"debug_material_effects", _hs_type_boolean, &debug_material_effects};
static struct hs_global_external weather_definition = {"weather", _hs_type_boolean, &weather};
static struct hs_global_external ai_profile_disable_definition = {"ai_profile_disable", _hs_type_boolean, &ai_profile.disable_ai};
static struct hs_global_external ai_profile_random_definition = {"ai_profile_random", _hs_type_boolean, &ai_profile.move_randomly};
static struct hs_global_external ai_show_definition = {"ai_show", _hs_type_boolean, &ai_profile.show};
static struct hs_global_external ai_show_stats_definition = {"ai_show_stats", _hs_type_boolean, &ai_profile.show_stats};
static struct hs_global_external ai_show_actors_definition = {"ai_show_actors", _hs_type_boolean, &ai_profile.show_actors};
static struct hs_global_external ai_show_swarms_definition = {"ai_show_swarms", _hs_type_boolean, &ai_profile.show_swarms};
static struct hs_global_external ai_show_paths_definition = {"ai_show_paths", _hs_type_boolean, &ai_profile.show_paths};
static struct hs_global_external ai_show_line_of_sight_definition = {"ai_show_line_of_sight", _hs_type_boolean, &ai_profile.show_line_of_sight};
static struct hs_global_external ai_show_prop_types_definition = {"ai_show_prop_types", _hs_type_boolean, &ai_profile.show_prop_types};
static struct hs_global_external ai_show_sound_distance_definition = {"ai_show_sound_distance", _hs_type_boolean, &ai_profile.show_sound_distance};
static struct hs_global_external ai_render_definition = {"ai_render", _hs_type_boolean, &ai_debug.render};
static struct hs_global_external ai_render_all_actors_definition = {"ai_render_all_actors", _hs_type_boolean, &ai_debug.render_all_actors};
static struct hs_global_external ai_render_inactive_actors_definition = {"ai_render_inactive_actors", _hs_type_boolean, &ai_debug.render_inactive_actors};
static struct hs_global_external ai_render_lineoffire_crouching_definition = {"ai_render_lineoffire_crouching", _hs_type_boolean, &ai_debug.render_lineoffire_crouching};
static struct hs_global_external ai_render_lineoffire_definition = {"ai_render_lineoffire", _hs_type_boolean, &ai_debug.render_lineoffire};
static struct hs_global_external ai_render_lineofsight_definition = {"ai_render_lineofsight", _hs_type_boolean, &ai_debug.render_lineofsight};
static struct hs_global_external ai_render_ballistic_lineoffire_definition = {"ai_render_ballistic_lineoffire", _hs_type_boolean, &ai_debug.render_ballistic_lineoffire};
static struct hs_global_external ai_render_encounter_activeregion_definition = {"ai_render_encounter_activeregion", _hs_type_boolean, &ai_debug.render_encounter_activeregion};
static struct hs_global_external ai_render_vision_cones_definition = {"ai_render_vision_cones", _hs_type_boolean, &ai_debug.render_vision_cones};
static struct hs_global_external ai_render_current_state_definition = {"ai_render_current_state", _hs_type_boolean, &ai_debug.render_current_state};
static struct hs_global_external ai_render_detailed_state_definition = {"ai_render_detailed_state", _hs_type_boolean, &ai_debug.render_detailed_state};
static struct hs_global_external ai_render_props_definition = {"ai_render_props", _hs_type_boolean, &ai_debug.render_props};
static struct hs_global_external ai_render_props_web_definition = {"ai_render_props_web", _hs_type_boolean, &ai_debug.render_props_web};
static struct hs_global_external ai_render_props_no_friends_definition = {"ai_render_props_no_friends", _hs_type_boolean, &ai_debug.render_props_no_friends};
static struct hs_global_external ai_render_props_unreachable_definition = {"ai_render_props_unreachable", _hs_type_boolean, &ai_debug.render_props_unreachable};
static struct hs_global_external ai_render_props_unopposable_definition = {"ai_render_props_unopposable", _hs_type_boolean, &ai_debug.render_props_unopposable};
static struct hs_global_external ai_render_props_target_weight_definition = {"ai_render_props_target_weight", _hs_type_boolean, &ai_debug.render_props_target_weight};
static struct hs_global_external ai_render_idle_look_definition = {"ai_render_idle_look", _hs_type_boolean, &ai_debug.render_idle_look};
static struct hs_global_external ai_render_support_surfaces_definition = {"ai_render_support_surfaces", _hs_type_boolean, &ai_debug.render_support_surfaces};
static struct hs_global_external ai_render_recent_damage_definition = {"ai_render_recent_damage", _hs_type_boolean, &ai_debug.render_recent_damage};
static struct hs_global_external ai_render_threats_definition = {"ai_render_threats", _hs_type_boolean, &ai_debug.render_threats};
static struct hs_global_external ai_render_emotions_definition = {"ai_render_emotions", _hs_type_boolean, &ai_debug.render_emotions};
static struct hs_global_external ai_render_audibility_definition = {"ai_render_audibility", _hs_type_boolean, &ai_debug.render_audibility};
static struct hs_global_external ai_render_aiming_vectors_definition = {"ai_render_aiming_vectors", _hs_type_boolean, &ai_debug.render_aiming_vectors};
static struct hs_global_external ai_render_secondary_looking_definition = {"ai_render_secondary_looking", _hs_type_boolean, &ai_debug.render_secondary_looking};
static struct hs_global_external ai_render_targets_definition = {"ai_render_targets", _hs_type_boolean, &ai_debug.render_targets};
static struct hs_global_external ai_render_targets_last_visible_definition = {"ai_render_targets_last_visible", _hs_type_boolean, &ai_debug.render_targets_last_visible};
static struct hs_global_external ai_render_states_definition = {"ai_render_states", _hs_type_boolean, &ai_debug.render_states};
static struct hs_global_external ai_render_vitality_definition = {"ai_render_vitality", _hs_type_boolean, &ai_debug.render_vitality};
static struct hs_global_external ai_render_active_cover_seeking_definition = {"ai_render_active_cover_seeking", _hs_type_boolean, &ai_debug.render_active_cover_seeking};
static struct hs_global_external ai_render_evaluations_definition = {"ai_render_evaluations", _hs_type_boolean, &ai_debug.render_evaluations};
static struct hs_global_external ai_render_pursuit_definition = {"ai_render_pursuit", _hs_type_boolean, &ai_debug.render_pursuit};
static struct hs_global_external ai_render_shooting_definition = {"ai_render_shooting", _hs_type_boolean, &ai_debug.render_shooting};
static struct hs_global_external ai_render_trigger_definition = {"ai_render_trigger", _hs_type_boolean, &ai_debug.render_trigger};
static struct hs_global_external ai_render_projectile_aiming_definition = {"ai_render_projectile_aiming", _hs_type_boolean, &ai_debug.render_projectile_aiming};
static struct hs_global_external ai_render_aiming_validity_definition = {"ai_render_aiming_validity", _hs_type_boolean, &ai_debug.render_aiming_validity};
static struct hs_global_external ai_render_speech_definition = {"ai_render_speech", _hs_type_boolean, &ai_debug.render_speech};
static struct hs_global_external ai_render_teams_definition = {"ai_render_teams", _hs_type_boolean, &ai_debug.render_teams};
static struct hs_global_external ai_render_player_ratings_definition = {"ai_render_player_ratings", _hs_type_boolean, &ai_debug.render_player_ratings};
static struct hs_global_external ai_render_spatial_effects_definition = {"ai_render_spatial_effects", _hs_type_boolean, &ai_debug.render_spatial_effects};
static struct hs_global_external ai_render_firing_positions_definition = {"ai_render_firing_positions", _hs_type_boolean, &ai_debug.render_firing_positions};
static struct hs_global_external ai_render_gun_positions_definition = {"ai_render_gun_positions", _hs_type_boolean, &ai_debug.render_gun_positions};
static struct hs_global_external ai_render_burst_geometry_definition = {"ai_render_burst_geometry", _hs_type_boolean, &ai_debug.render_burst_geometry};
static struct hs_global_external ai_render_vehicle_avoidance_definition = {"ai_render_vehicle_avoidance", _hs_type_boolean, &ai_debug.render_vehicle_avoidance};
static struct hs_global_external ai_render_vehicles_enterable_definition = {"ai_render_vehicles_enterable", _hs_type_boolean, &ai_debug.render_vehicles_enterable};
static struct hs_global_external ai_render_melee_check_definition = {"ai_render_melee_check", _hs_type_boolean, &ai_debug.render_melee_check};
static struct hs_global_external ai_render_dialogue_variants_definition = {"ai_render_dialogue_variants", _hs_type_boolean, &ai_debug.render_dialogue_variants};
static struct hs_global_external ai_render_grenade_decisions_definition = {"ai_render_grenade_decisions", _hs_type_boolean, &ai_debug.render_grenade_decisions};
static struct hs_global_external ai_render_danger_zones_definition = {"ai_render_danger_zones", _hs_type_boolean, &ai_debug.render_danger_zones};
static struct hs_global_external ai_render_charge_decisions_definition = {"ai_render_charge_decisions", _hs_type_boolean, &ai_debug.render_charge_decisions};
static struct hs_global_external ai_render_control_definition = {"ai_render_control", _hs_type_boolean, &ai_debug.render_control};
static struct hs_global_external ai_render_activation_definition = {"ai_render_activation", _hs_type_boolean, &ai_debug.render_activation};
static struct hs_global_external ai_render_paths_definition = {"ai_render_paths", _hs_type_boolean, &ai_debug.render_paths};
static struct hs_global_external ai_render_paths_selected_only_definition = {"ai_render_paths_selected_only", _hs_type_boolean, &ai_debug.render_paths_selected_only};
static struct hs_global_external ai_render_paths_destination_definition = {"ai_render_paths_destination", _hs_type_boolean, &ai_debug.render_paths_destination};
static struct hs_global_external ai_render_paths_raw_definition = {"ai_render_paths_raw", _hs_type_boolean, &ai_debug.render_paths_raw};
static struct hs_global_external ai_render_paths_current_definition = {"ai_render_paths_current", _hs_type_boolean, &ai_debug.render_paths_current};
static struct hs_global_external ai_render_paths_failed_definition = {"ai_render_paths_failed", _hs_type_boolean, &ai_debug.render_paths_failed};
static struct hs_global_external ai_render_paths_smoothed_definition = {"ai_render_paths_smoothed", _hs_type_boolean, &ai_debug.render_paths_smoothed};
static struct hs_global_external ai_render_paths_avoided_definition = {"ai_render_paths_avoided", _hs_type_boolean, &ai_debug.render_paths_avoided};
static struct hs_global_external ai_render_paths_avoidance_segment_definition = {"ai_render_paths_avoidance_segment", _hs_type_short_integer, &ai_debug.render_paths_avoidance_segment};
static struct hs_global_external ai_render_paths_avoidance_obstacles_definition = {"ai_render_paths_avoidance_obstacles", _hs_type_boolean, &ai_debug.render_paths_avoidance_obstacles};
static struct hs_global_external ai_render_paths_avoidance_search_definition = {"ai_render_paths_avoidance_search", _hs_type_boolean, &ai_debug.render_paths_avoidance_search};
static struct hs_global_external ai_render_paths_nodes_definition = {"ai_render_paths_nodes", _hs_type_boolean, &ai_debug.render_paths_nodes};
static struct hs_global_external ai_render_paths_nodes_all_definition = {"ai_render_paths_nodes_all", _hs_type_boolean, &ai_debug.render_paths_nodes_all};
static struct hs_global_external ai_render_paths_nodes_polygons_definition = {"ai_render_paths_nodes_polygons", _hs_type_boolean, &ai_debug.render_paths_nodes_polygons};
static struct hs_global_external ai_render_paths_nodes_costs_definition = {"ai_render_paths_nodes_costs", _hs_type_boolean, &ai_debug.render_paths_nodes_costs};
static struct hs_global_external ai_render_paths_nodes_closest_definition = {"ai_render_paths_nodes_closest", _hs_type_boolean, &ai_debug.render_paths_nodes_closest};
static struct hs_global_external ai_render_player_aiming_blocked_definition = {"ai_render_player_aiming_blocked", _hs_type_boolean, &ai_debug.render_player_aiming_blocked};
static struct hs_global_external ai_render_vector_avoidance_definition = {"ai_render_vector_avoidance", _hs_type_boolean, &ai_debug.render_vector_avoidance};
static struct hs_global_external ai_render_vector_avoidance_rays_definition = {"ai_render_vector_avoidance_rays", _hs_type_boolean, &ai_debug.render_vector_avoidance_rays};
static struct hs_global_external ai_render_vector_avoidance_sense_t_definition = {"ai_render_vector_avoidance_sense_t", _hs_type_boolean, &ai_debug.render_vector_avoidance_sense_t};
static struct hs_global_external ai_render_vector_avoidance_avoid_t_definition = {"ai_render_vector_avoidance_avoid_t", _hs_type_boolean, &ai_debug.render_vector_avoidance_avoid_t};
static struct hs_global_external ai_render_vector_avoidance_clear_time_definition = {"ai_render_vector_avoidance_clear_time", _hs_type_boolean, &ai_debug.render_vector_avoidance_clear_time};
static struct hs_global_external ai_render_vector_avoidance_weights_definition = {"ai_render_vector_avoidance_weights", _hs_type_boolean, &ai_debug.render_vector_avoidance_weights};
static struct hs_global_external ai_render_vector_avoidance_objects_definition = {"ai_render_vector_avoidance_objects", _hs_type_boolean, &ai_debug.render_vector_avoidance_objects};
static struct hs_global_external ai_render_vector_avoidance_intermediate_definition = {"ai_render_vector_avoidance_intermediate", _hs_type_boolean, &ai_debug.render_vector_avoidance_intermediate};
static struct hs_global_external ai_render_postcombat_definition = {"ai_render_postcombat", _hs_type_boolean, &ai_debug.render_postcombat};
static struct hs_global_external ai_print_pursuit_checks_definition = {"ai_print_pursuit_checks", _hs_type_boolean, &ai_debug.print_pursuit_checks};
static struct hs_global_external ai_print_rules_definition = {"ai_print_rules", _hs_type_boolean, &ai_debug.print_rules};
static struct hs_global_external ai_print_rule_values_definition = {"ai_print_rule_values", _hs_type_boolean, &ai_debug.print_rule_values};
static struct hs_global_external ai_print_major_upgrade_definition = {"ai_print_major_upgrade", _hs_type_boolean, &ai_debug.print_major_upgrade};
static struct hs_global_external ai_print_respawn_definition = {"ai_print_respawn", _hs_type_boolean, &ai_debug.print_respawn};
static struct hs_global_external ai_print_evaluation_statistics_definition = {"ai_print_evaluation_statistics", _hs_type_boolean, &ai_debug.print_evaluation_statistics};
static struct hs_global_external ai_print_communication_definition = {"ai_print_communication", _hs_type_boolean, &ai_debug.print_communication};
static struct hs_global_external ai_print_communication_player_definition = {"ai_print_communication_player", _hs_type_boolean, &ai_debug.print_communication_player};
static struct hs_global_external ai_print_vocalizations_definition = {"ai_print_vocalizations", _hs_type_boolean, &ai_debug.print_vocalizations};
static struct hs_global_external ai_print_placement_definition = {"ai_print_placement", _hs_type_boolean, &ai_debug.print_placement};
static struct hs_global_external ai_print_speech_definition = {"ai_print_speech", _hs_type_boolean, &ai_debug.print_speech};
static struct hs_global_external ai_print_speech_timers_definition = {"ai_print_speech_timers", _hs_type_boolean, &ai_debug.print_speech_timers};
static struct hs_global_external ai_print_allegiance_definition = {"ai_print_allegiance", _hs_type_boolean, &ai_debug.print_allegiance};
static struct hs_global_external ai_print_lost_speech_definition = {"ai_print_lost_speech", _hs_type_boolean, &ai_debug.print_lost_speech};
static struct hs_global_external ai_print_migration_definition = {"ai_print_migration", _hs_type_boolean, &ai_debug.print_migration};
static struct hs_global_external ai_print_automatic_migration_definition = {"ai_print_automatic_migration", _hs_type_boolean, &ai_debug.print_automatic_migration};
static struct hs_global_external ai_print_scripting_definition = {"ai_print_scripting", _hs_type_boolean, &ai_debug.print_scripting};
static struct hs_global_external ai_print_surprise_definition = {"ai_print_surprise", _hs_type_boolean, &ai_debug.print_surprise};
static struct hs_global_external ai_print_command_lists_definition = {"ai_print_command_lists", _hs_type_boolean, &ai_debug.print_command_lists};
static struct hs_global_external ai_print_damage_modifiers_definition = {"ai_print_damage_modifiers", _hs_type_boolean, &ai_debug.print_damage_modifiers};
static struct hs_global_external ai_print_secondary_looking_definition = {"ai_print_secondary_looking", _hs_type_boolean, &ai_debug.print_secondary_looking};
static struct hs_global_external ai_print_oversteer_definition = {"ai_print_oversteer", _hs_type_boolean, &ai_debug.print_oversteer};
static struct hs_global_external ai_print_conversations_definition = {"ai_print_conversations", _hs_type_boolean, &ai_debug.print_conversations};
static struct hs_global_external ai_print_killing_sprees_definition = {"ai_print_killing_sprees", _hs_type_boolean, &ai_debug.print_killing_sprees};
static struct hs_global_external ai_print_acknowledgement_definition = {"ai_print_acknowledgement", _hs_type_boolean, &ai_debug.print_acknowledgement};
static struct hs_global_external ai_print_unfinished_paths_definition = {"ai_print_unfinished_paths", _hs_type_boolean, &ai_debug.print_unfinished_paths};
static struct hs_global_external ai_print_bsp_transition_definition = {"ai_print_bsp_transition", _hs_type_boolean, &ai_debug.print_bsp_transition};
static struct hs_global_external ai_print_uncovering_definition = {"ai_print_uncovering", _hs_type_boolean, &ai_debug.print_uncovering};
static struct hs_global_external ai_debug_fast_los_definition = {"ai_debug_fast_los", _hs_type_boolean, &ai_debug.fast_los};
static struct hs_global_external ai_debug_oversteer_disable_definition = {"ai_debug_oversteer_disable", _hs_type_boolean, &ai_debug.oversteer_disable};
static struct hs_global_external ai_debug_evaluate_all_positions_definition = {"ai_debug_evaluate_all_positions", _hs_type_boolean, &ai_debug.evaluate_all_positions};
static struct hs_global_external ai_debug_path_definition = {"ai_debug_path", _hs_type_boolean, &ai_debug.path_enable};
static struct hs_global_external ai_debug_path_start_freeze_definition = {"ai_debug_path_start_freeze", _hs_type_boolean, &ai_debug.path_start_freeze};
static struct hs_global_external ai_debug_path_end_freeze_definition = {"ai_debug_path_end_freeze", _hs_type_boolean, &ai_debug.path_end_freeze};
static struct hs_global_external ai_debug_path_flood_definition = {"ai_debug_path_flood", _hs_type_boolean, &ai_debug.path_flood};
static struct hs_global_external ai_debug_path_maximum_radius_definition = {"ai_debug_path_maximum_radius", _hs_type_real, &ai_debug.path_maximum_radius};
static struct hs_global_external ai_debug_path_attractor_definition = {"ai_debug_path_attractor", _hs_type_boolean, &ai_debug.path_attractor};
static struct hs_global_external ai_debug_path_attractor_radius_definition = {"ai_debug_path_attractor_radius", _hs_type_real, &ai_debug.path_attractor_radius};
static struct hs_global_external ai_debug_path_attractor_weight_definition = {"ai_debug_path_attractor_weight", _hs_type_real, &ai_debug.path_attractor_weight};
static struct hs_global_external ai_debug_path_accept_radius_definition = {"ai_debug_path_accept_radius", _hs_type_real, &ai_debug.path_accept_radius};
static struct hs_global_external ai_debug_ballistic_lineoffire_freeze_definition = {"ai_debug_ballistic_lineoffire_freeze", _hs_type_boolean, &ai_debug.ballistic_lineoffire_freeze};
static struct hs_global_external ai_debug_communication_random_disabled_definition = {"ai_debug_communication_random_disabled", _hs_type_boolean, &ai_debug.communication_random_disabled};
static struct hs_global_external ai_debug_communication_timeout_disabled_definition = {"ai_debug_communication_timeout_disabled", _hs_type_boolean, &ai_debug.communication_timeout_disabled};
static struct hs_global_external ai_debug_communication_unit_repeat_disabled_definition = {"ai_debug_communication_unit_repeat_disabled", _hs_type_boolean, &ai_debug.communication_unit_repeat_disabled};
static struct hs_global_external ai_debug_communication_focus_enable_definition = {"ai_debug_communication_focus_enable", _hs_type_boolean, &ai_debug.communication_focus_enable};
static struct hs_global_external ai_debug_blind_definition = {"ai_debug_blind", _hs_type_boolean, &ai_debug.blind};
static struct hs_global_external ai_debug_deaf_definition = {"ai_debug_deaf", _hs_type_boolean, &ai_debug.deaf};
static struct hs_global_external ai_debug_invisible_player_definition = {"ai_debug_invisible_player", _hs_type_boolean, &ai_debug.invisible_player};
static struct hs_global_external ai_debug_ignore_player_definition = {"ai_debug_ignore_player", _hs_type_boolean, &ai_debug.ignore_player};
static struct hs_global_external ai_debug_flee_always_definition = {"ai_debug_flee_always", _hs_type_boolean, &ai_debug.flee_always};
static struct hs_global_external ai_debug_force_all_active_definition = {"ai_debug_force_all_active", _hs_type_boolean, &ai_debug.force_all_active};
static struct hs_global_external ai_debug_disable_wounded_sounds_definition = {"ai_debug_disable_wounded_sounds", _hs_type_boolean, &ai_debug.disable_wounded_sounds};
static struct hs_global_external ai_debug_force_vocalizations_definition = {"ai_debug_force_vocalizations", _hs_type_boolean, &ai_debug.force_vocalizations};
static struct hs_global_external ai_debug_force_crouch_definition = {"ai_debug_force_crouch", _hs_type_boolean, &ai_debug.force_crouch};
static struct hs_global_external ai_debug_path_disable_smoothing_definition = {"ai_debug_path_disable_smoothing", _hs_type_boolean, &ai_debug.path_disable_smoothing};
static struct hs_global_external ai_debug_path_disable_obstacle_avoidance_definition = {"ai_debug_path_disable_obstacle_avoidance", _hs_type_boolean, &ai_debug.path_disable_obstacle_avoidance};
static struct hs_global_external ai_fix_defending_guard_firing_positions_definition = {"ai_fix_defending_guard_firing_positions", _hs_type_boolean, &ai_debug.fix_defending_guard_firing_positions};
static struct hs_global_external ai_fix_actor_variants_definition = {"ai_fix_actor_variants", _hs_type_boolean, &ai_debug.fix_actor_variants};
static struct hs_global_external controls_enable_crouch_definition = {"controls_enable_crouch", _hs_type_boolean, &controls_enable_crouch};
static struct hs_global_external controls_swapped_definition = {"controls_swapped", _hs_type_boolean, &controls_swapped};
static struct hs_global_external controls_enable_doubled_spin_definition = {"controls_enable_doubled_spin", _hs_type_boolean, &controls_enable_doubled_spin};
static struct hs_global_external controls_swap_doubled_spin_state_definition = {"controls_swap_doubled_spin_state", _hs_type_boolean, &controls_swap_doubled_spin_state};
static struct hs_global_external player0_look_yaw_rate_definition = {"player0_look_yaw_rate", _hs_type_real, &player_look_yaw_rate[0]};
static struct hs_global_external player1_look_yaw_rate_definition = {"player1_look_yaw_rate", _hs_type_real, &player_look_yaw_rate[1]};
static struct hs_global_external player2_look_yaw_rate_definition = {"player2_look_yaw_rate", _hs_type_real, &player_look_yaw_rate[2]};
static struct hs_global_external player3_look_yaw_rate_definition = {"player3_look_yaw_rate", _hs_type_real, &player_look_yaw_rate[3]};
static struct hs_global_external player0_look_pitch_rate_definition = {"player0_look_pitch_rate", _hs_type_real, &player_look_pitch_rate[0]};
static struct hs_global_external player1_look_pitch_rate_definition = {"player1_look_pitch_rate", _hs_type_real, &player_look_pitch_rate[1]};
static struct hs_global_external player2_look_pitch_rate_definition = {"player2_look_pitch_rate", _hs_type_real, &player_look_pitch_rate[2]};
static struct hs_global_external player3_look_pitch_rate_definition = {"player3_look_pitch_rate", _hs_type_real, &player_look_pitch_rate[3]};
static struct hs_global_external player_autoaim_definition = {"player_autoaim", _hs_type_boolean, &player_autoaim_flag};
static struct hs_global_external player_magnetism_definition = {"player_magnetism", _hs_type_boolean, &player_magnetism_flag};
static struct hs_global_external debug_player_teleport_definition = {"debug_player_teleport", _hs_type_boolean, &debug_render_player_teleport};
static struct hs_global_external texture_cache_graph_definition = {"texture_cache_graph", _hs_type_boolean, &texture_cache_debug_options.display_cache_graph};
static struct hs_global_external texture_cache_list_definition = {"texture_cache_list", _hs_type_boolean, &texture_cache_debug_options.display_cache_list};
static struct hs_global_external director_camera_switch_fast_definition = {"director_camera_switch_fast", _hs_type_boolean, &director_camera_switch_fast};
static struct hs_global_external debug_recording_definition = {"debug_recording", _hs_type_boolean, &debug_recording};
static struct hs_global_external debug_recording_newlines_definition = {"debug_recording_newlines", _hs_type_short_integer, &debug_recording_newlines};
static struct hs_global_external debug_player_color_definition = {"debug_player_color", _hs_type_short_integer, &debug_player_color};
static struct hs_global_external find_all_fucked_up_shit_definition = {"find_all_fucked_up_shit", _hs_type_boolean, &find_all_fucked_up_shit};
static struct hs_global_external allow_out_of_sync_definition = {"allow_out_of_sync", _hs_type_boolean, &allow_out_of_sync};
static struct hs_global_external global_connection_dont_timeout_definition = {"global_connection_dont_timeout", _hs_type_boolean, &global_connection_dont_timeout};
static struct hs_global_external run_game_scripts_definition = {"run_game_scripts", _hs_type_boolean, NULL};

struct hs_global_external *hs_external_globals[] =
{
	&rasterizer_near_clip_distance_definition,
	&rasterizer_far_clip_distance_definition,
	&rasterizer_first_person_weapon_near_clip_distance_definition,
	&rasterizer_first_person_weapon_far_clip_distance_definition,
	&rasterizer_pushbuffer_size_definition,
	&rasterizer_pushbuffer_kickoff_size_definition,
	&rasterizer_floating_point_zbuffer_definition,
	&rasterizer_framerate_throttle_definition,
	&rasterizer_framerate_stabilization_definition,
	&rasterizer_refresh_rate_definition,
	&rasterizer_frame_bounds_left_definition,
	&rasterizer_frame_bounds_right_definition,
	&rasterizer_frame_bounds_top_definition,
	&rasterizer_frame_bounds_bottom_definition,
	&rasterizer_stats_definition,
	&rasterizer_mode_definition,
	&rasterizer_wireframe_definition,
	&rasterizer_smart_definition,
	&rasterizer_debug_model_vertices_definition,
	&rasterizer_debug_model_lod_definition,
	&rasterizer_debug_transparents_definition,
	&rasterizer_debug_meter_shader_definition,
	&rasterizer_models_definition,
	&rasterizer_model_transparents_definition,
	&rasterizer_draw_first_person_weapon_first_definition,
	&rasterizer_stencil_mask_definition,
	&rasterizer_environment_definition,
	&rasterizer_environment_lightmaps_definition,
	&rasterizer_environment_shadows_definition,
	&rasterizer_environment_diffuse_lights_definition,
	&rasterizer_environment_diffuse_textures_definition,
	&rasterizer_environment_decals_definition,
	&rasterizer_environment_specular_lights_definition,
	&rasterizer_environment_specular_lightmaps_definition,
	&rasterizer_environment_reflection_lightmap_mask_definition,
	&rasterizer_environment_reflection_mirrors_definition,
	&rasterizer_environment_reflections_definition,
	&rasterizer_environment_transparents_definition,
	&rasterizer_environment_fog_definition,
	&rasterizer_environment_fog_screen_definition,
	&rasterizer_water_definition,
	&rasterizer_lens_flares_definition,
	&rasterizer_dynamic_unlit_geometry_definition,
	&rasterizer_dynamic_lit_geometry_definition,
	&rasterizer_dynamic_screen_geometry_definition,
	&rasterizer_hud_motion_sensor_definition,
	&rasterizer_detail_objects_definition,
	&rasterizer_debug_geometry_definition,
	&rasterizer_debug_geometry_multipass_definition,
	&rasterizer_fog_atmosphere_definition,
	&rasterizer_fog_plane_definition,
	&rasterizer_bump_mapping_definition,
	&rasterizer_lightmap_ambient_definition,
	&rasterizer_lightmap_mode_definition,
	&rasterizer_lightmaps_incident_radiosity_definition,
	&rasterizer_lightmaps_filtering_definition,
	&rasterizer_model_lighting_ambient_definition,
	&rasterizer_environment_alpha_testing_definition,
	&rasterizer_environment_specular_mask_definition,
	&rasterizer_shadows_convolution_definition,
	&rasterizer_shadows_debug_definition,
	&rasterizer_water_mipmapping_definition,
	&rasterizer_active_camouflage_definition,
	&rasterizer_active_camouflage_multipass_definition,
	&rasterizer_plasma_energy_definition,
	&rasterizer_lens_flares_occlusion_definition,
	&rasterizer_lens_flares_occlusion_debug_definition,
	&rasterizer_ray_of_buddha_definition,
	&rasterizer_screen_flashes_definition,
	&rasterizer_screen_effects_definition,
	&rasterizer_DXTC_noise_definition,
	&rasterizer_soft_filter_definition,
	&rasterizer_secondary_render_target_debug_definition,
	&rasterizer_profile_log_definition,
	&rasterizer_detail_objects_offset_multiplier_definition,
	&rasterizer_zbias_definition,
	&rasterizer_zoffset_definition,
	&force_all_player_views_to_default_player_definition,
	&rasterizer_safe_frame_bounds_definition,
	&freeze_flying_camera_definition,
	&rasterizer_zsprites_definition,
	&rasterizer_filthy_decal_fog_hack_definition,
	&rasterizer_splitscreen_VB_optimization_definition,
	&rasterizer_profile_print_locks_definition,
	&rasterizer_profile_objectlock_time_definition,
	&pad3_definition,
	&pad3_scale_definition,
	&f0_definition,
	&f1_definition,
	&f2_definition,
	&f3_definition,
	&f4_definition,
	&f5_definition,
	&rasterizer_transparent_pixel_counter_definition,
	&debug_no_frustum_clip_definition,
	&debug_frustum_definition,
	&debug_bink_definition,
	&screenshot_size_definition,
	&screenshot_count_definition,
	&terminal_render_definition,
	&console_dump_to_file_definition,
	&player_spawn_count_definition,
	&debug_object_garbage_collection_definition,
	&debug_render_freeze_definition,
	&debug_no_drawing_definition,
	&debug_input_target_definition,
	&temporary_hud_definition,
	&debug_leaf_index_definition,
	&debug_leaf_portal_index_definition,
	&debug_leaf_portals_definition,
	&debug_unit_all_animations_definition,
	&debug_unit_animations_definition,
	&debug_unit_illumination_definition,
	&debug_damage_taken_definition,
	&cheat_deathless_player_definition,
	&cheat_jetpack_definition,
	&cheat_infinite_ammo_definition,
	&cheat_bottomless_clip_definition,
	&cheat_bump_possession_definition,
	&cheat_super_jump_definition,
	&cheat_reflexive_damage_effects_definition,
	&cheat_medusa_definition,
	&cheat_omnipotent_definition,
	&cheat_controller_definition,
	&effects_corpse_nonviolent_definition,
	&debug_effects_nonviolent_definition,
	&debug_sound_cache_definition,
	&debug_sound_definition,
	&debug_looping_sound_definition,
	&debug_sound_channels_definition,
	&loud_dialog_hack_definition,
	&sound_gain_under_dialog_definition,
	&debug_sound_environment_definition,
	&object_light_ambient_base_definition,
	&object_light_ambient_scale_definition,
	&object_light_secondary_scale_definition,
	&object_light_interpolate_definition,
	&model_animation_compression_definition,
	&model_animation_data_compressed_size_definition,
	&model_animation_data_uncompressed_size_definition,
	&model_animation_data_compression_savings_in_bytes_definition,
	&model_animation_data_compression_savings_in_bytes_at_import_definition,
	&model_animation_data_compression_savings_in_percent_definition,
	&model_animation_bullshit0_definition,
	&model_animation_bullshit1_definition,
	&model_animation_bullshit2_definition,
	&model_animation_bullshit3_definition,
	&rider_ejection_definition,
	&stun_enable_definition,
	&debug_sprites_definition,
	&debug_portals_definition,
	&debug_inactive_objects_definition,
	&render_contrails_definition,
	&render_particles_definition,
	&render_psystems_definition,
	&render_wsystems_definition,
	&debug_objects_definition,
	&debug_objects_position_velocity_definition,
	&debug_objects_root_node_definition,
	&debug_objects_bounding_spheres_definition,
	&debug_objects_collision_models_definition,
	&debug_objects_physics_definition,
	&debug_objects_names_definition,
	&debug_objects_pathfinding_spheres_definition,
	&debug_objects_unit_vectors_definition,
	&debug_objects_unit_seats_definition,
	&debug_objects_unit_mouth_apeture_definition,
	&debug_objects_biped_physics_pills_definition,
	&debug_objects_biped_autoaim_pills_definition,
	&debug_objects_vehicle_powered_mass_points_definition,
	&debug_objects_devices_definition,
	&render_model_nodes_definition,
	&render_model_vertex_counts_definition,
	&render_model_index_counts_definition,
	&render_model_markers_definition,
	&render_model_no_geometry_definition,
	&render_shadows_definition,
	&debug_damage_definition,
	&debug_scripting_definition,
	&debug_trigger_volumes_definition,
	&debug_point_physics_definition,
	&debug_physics_disable_penetration_freeze_definition,
	&debug_motion_sensor_draw_all_units_definition,
	&collision_debug_definition,
	&collision_debug_spray_definition,
	&collision_debug_features_definition,
	&collision_debug_repeat_definition,
	&collision_debug_flag_front_facing_surfaces_definition,
	&collision_debug_flag_back_facing_surfaces_definition,
	&collision_debug_flag_ignore_two_sided_surfaces_definition,
	&collision_debug_flag_ignore_invisible_surfaces_definition,
	&collision_debug_flag_ignore_breakable_surfaces_definition,
	&collision_debug_flag_structure_definition,
	&collision_debug_flag_media_definition,
	&collision_debug_flag_objects_definition,
	&collision_debug_flag_objects_bipeds_definition,
	&collision_debug_flag_objects_vehicles_definition,
	&collision_debug_flag_objects_weapons_definition,
	&collision_debug_flag_objects_equipment_definition,
	&collision_debug_flag_objects_projectiles_definition,
	&collision_debug_flag_objects_scenery_definition,
	&collision_debug_flag_objects_machines_definition,
	&collision_debug_flag_objects_controls_definition,
	&collision_debug_flag_objects_light_fixtures_definition,
	&collision_debug_flag_objects_placeholders_definition,
	&collision_debug_flag_try_to_keep_location_valid_definition,
	&collision_debug_flag_skip_passthrough_bipeds_definition,
	&collision_debug_flag_use_vehicle_physics_definition,
	&collision_debug_point_x_definition,
	&collision_debug_point_y_definition,
	&collision_debug_point_z_definition,
	&collision_debug_vector_i_definition,
	&collision_debug_vector_j_definition,
	&collision_debug_vector_k_definition,
	&collision_debug_length_definition,
	&collision_debug_width_definition,
	&collision_debug_height_definition,
	&collision_debug_phantom_bsp_definition,
	&collision_log_render_definition,
	&collision_log_detailed_definition,
	&collision_log_extended_definition,
	&collision_log_totals_only_definition,
	&collision_log_time_definition,
	&debug_obstacle_path_definition,
	&debug_obstacle_path_on_failure_definition,
	&debug_obstacle_path_start_point_x_definition,
	&debug_obstacle_path_start_point_y_definition,
	&debug_obstacle_path_start_surface_index_definition,
	&debug_obstacle_path_goal_point_x_definition,
	&debug_obstacle_path_goal_point_y_definition,
	&debug_obstacle_path_goal_surface_index_definition,
	&debug_camera_definition,
	&debug_player_definition,
	&debug_structure_definition,
	&debug_bsp_definition,
	&debug_input_definition,
	&debug_permanent_decals_definition,
	&debug_fog_planes_definition,
	&decals_definition,
	&debug_decals_definition,
	&debug_object_lights_definition,
	&debug_lights_definition,
	&debug_biped_physics_definition,
	&debug_biped_skip_update_definition,
	&debug_biped_skip_collision_definition,
	&debug_biped_limp_body_disable_definition,
	&debug_collision_skip_objects_definition,
	&debug_collision_skip_vectors_definition,
	&debug_material_effects_definition,
	&weather_definition,
	&breakable_surfaces_definition,
	&decals_definition,
	&profile_graph_definition,
	&profile_display_definition,
	&profile_timebase_ticks_definition,
	&profile_dump_frames_definition,
	&profile_dump_lost_frames_definition,
	&recover_saved_games_hack_definition,
	&radiosity_quality_definition,
	&radiosity_step_count_definition,
	&radiosity_lines_definition,
	&radiosity_normals_definition,
	&structures_use_pvs_for_vs_definition,
	&debug_texture_cache_definition,
	&debug_detail_objects_definition,
	&ai_render_definition,
	&ai_render_all_actors_definition,
	&ai_render_inactive_actors_definition,
	&ai_render_lineoffire_crouching_definition,
	&ai_render_lineoffire_definition,
	&ai_render_lineofsight_definition,
	&ai_render_ballistic_lineoffire_definition,
	&ai_render_encounter_activeregion_definition,
	&ai_render_vision_cones_definition,
	&ai_render_current_state_definition,
	&ai_render_detailed_state_definition,
	&ai_render_props_definition,
	&ai_render_props_web_definition,
	&ai_render_props_no_friends_definition,
	&ai_render_props_target_weight_definition,
	&ai_render_props_unreachable_definition,
	&ai_render_props_unopposable_definition,
	&ai_render_idle_look_definition,
	&ai_render_support_surfaces_definition,
	&ai_render_recent_damage_definition,
	&ai_render_threats_definition,
	&ai_render_emotions_definition,
	&ai_render_audibility_definition,
	&ai_render_aiming_vectors_definition,
	&ai_render_secondary_looking_definition,
	&ai_render_targets_definition,
	&ai_render_targets_last_visible_definition,
	&ai_render_states_definition,
	&ai_render_vitality_definition,
	&ai_render_active_cover_seeking_definition,
	&ai_render_evaluations_definition,
	&ai_render_pursuit_definition,
	&ai_render_shooting_definition,
	&ai_render_trigger_definition,
	&ai_render_projectile_aiming_definition,
	&ai_render_aiming_validity_definition,
	&ai_render_speech_definition,
	&ai_render_teams_definition,
	&ai_render_player_ratings_definition,
	&ai_render_spatial_effects_definition,
	&ai_render_firing_positions_definition,
	&ai_render_gun_positions_definition,
	&ai_render_burst_geometry_definition,
	&ai_render_vehicle_avoidance_definition,
	&ai_render_vehicles_enterable_definition,
	&ai_render_melee_check_definition,
	&ai_render_dialogue_variants_definition,
	&ai_render_grenade_decisions_definition,
	&ai_render_danger_zones_definition,
	&ai_render_charge_decisions_definition,
	&ai_render_control_definition,
	&ai_render_activation_definition,
	&ai_render_paths_definition,
	&ai_render_paths_selected_only_definition,
	&ai_render_paths_destination_definition,
	&ai_render_paths_current_definition,
	&ai_render_paths_failed_definition,
	&ai_render_paths_raw_definition,
	&ai_render_paths_smoothed_definition,
	&ai_render_paths_avoided_definition,
	&ai_render_paths_avoidance_segment_definition,
	&ai_render_paths_avoidance_obstacles_definition,
	&ai_render_paths_avoidance_search_definition,
	&ai_render_paths_nodes_definition,
	&ai_render_paths_nodes_all_definition,
	&ai_render_paths_nodes_polygons_definition,
	&ai_render_paths_nodes_costs_definition,
	&ai_render_paths_nodes_closest_definition,
	&ai_render_player_aiming_blocked_definition,
	&ai_render_vector_avoidance_definition,
	&ai_render_vector_avoidance_rays_definition,
	&ai_render_vector_avoidance_sense_t_definition,
	&ai_render_vector_avoidance_avoid_t_definition,
	&ai_render_vector_avoidance_clear_time_definition,
	&ai_render_vector_avoidance_weights_definition,
	&ai_render_vector_avoidance_objects_definition,
	&ai_render_vector_avoidance_intermediate_definition,
	&ai_render_postcombat_definition,
	&ai_print_pursuit_checks_definition,
	&ai_print_rules_definition,
	&ai_print_rule_values_definition,
	&ai_print_major_upgrade_definition,
	&ai_print_respawn_definition,
	&ai_print_evaluation_statistics_definition,
	&ai_print_communication_definition,
	&ai_print_communication_player_definition,
	&ai_print_vocalizations_definition,
	&ai_print_placement_definition,
	&ai_print_speech_definition,
	&ai_print_speech_timers_definition,
	&ai_print_allegiance_definition,
	&ai_print_lost_speech_definition,
	&ai_print_migration_definition,
	&ai_print_automatic_migration_definition,
	&ai_print_scripting_definition,
	&ai_print_surprise_definition,
	&ai_print_command_lists_definition,
	&ai_print_damage_modifiers_definition,
	&ai_print_secondary_looking_definition,
	&ai_print_oversteer_definition,
	&ai_print_conversations_definition,
	&ai_print_killing_sprees_definition,
	&ai_print_acknowledgement_definition,
	&ai_print_unfinished_paths_definition,
	&ai_print_bsp_transition_definition,
	&ai_print_uncovering_definition,
	&ai_profile_disable_definition,
	&ai_profile_random_definition,
	&ai_show_definition,
	&ai_show_stats_definition,
	&ai_show_actors_definition,
	&ai_show_swarms_definition,
	&ai_show_paths_definition,
	&ai_show_line_of_sight_definition,
	&ai_show_prop_types_definition,
	&ai_show_sound_distance_definition,
	&ai_debug_fast_los_definition,
	&ai_debug_oversteer_disable_definition,
	&ai_debug_evaluate_all_positions_definition,
	&ai_debug_path_definition,
	&ai_debug_path_start_freeze_definition,
	&ai_debug_path_end_freeze_definition,
	&ai_debug_path_flood_definition,
	&ai_debug_path_maximum_radius_definition,
	&ai_debug_path_attractor_definition,
	&ai_debug_path_attractor_radius_definition,
	&ai_debug_path_attractor_weight_definition,
	&ai_debug_path_accept_radius_definition,
	&ai_debug_ballistic_lineoffire_freeze_definition,
	&ai_debug_communication_random_disabled_definition,
	&ai_debug_communication_timeout_disabled_definition,
	&ai_debug_communication_unit_repeat_disabled_definition,
	&ai_debug_communication_focus_enable_definition,
	&ai_debug_blind_definition,
	&ai_debug_deaf_definition,
	&ai_debug_invisible_player_definition,
	&ai_debug_ignore_player_definition,
	&ai_debug_flee_always_definition,
	&ai_debug_force_all_active_definition,
	&ai_debug_disable_wounded_sounds_definition,
	&ai_debug_force_vocalizations_definition,
	&ai_debug_force_crouch_definition,
	&ai_debug_path_disable_smoothing_definition,
	&ai_debug_path_disable_obstacle_avoidance_definition,
	&ai_fix_defending_guard_firing_positions_definition,
	&ai_fix_actor_variants_definition,
	&controls_enable_crouch_definition,
	&controls_swapped_definition,
	&controls_enable_doubled_spin_definition,
	&controls_swap_doubled_spin_state_definition,
	&player0_look_yaw_rate_definition,
	&player1_look_yaw_rate_definition,
	&player2_look_yaw_rate_definition,
	&player3_look_yaw_rate_definition,
	&player0_look_pitch_rate_definition,
	&player1_look_pitch_rate_definition,
	&player2_look_pitch_rate_definition,
	&player3_look_pitch_rate_definition,
	&player_autoaim_definition,
	&player_magnetism_definition,
	&debug_player_teleport_definition,
	&texture_cache_graph_definition,
	&texture_cache_list_definition,
	&director_camera_switch_fast_definition,
	&debug_recording_definition,
	&debug_recording_newlines_definition,
	&debug_player_color_definition,
	&debug_framerate_definition,
	&display_framerate_definition,
	&display_vblank_deltas_definition,
	&framerate_throttle_definition,
	&framerate_lock_definition,
	&display_precache_progress_definition,
	&debug_game_save_definition,
	&allow_out_of_sync_definition,
	&global_connection_dont_timeout_definition,
	&find_all_fucked_up_shit_definition,
	&run_game_scripts_definition
};

short const hs_external_global_count = NUMBEROF(hs_external_globals);
