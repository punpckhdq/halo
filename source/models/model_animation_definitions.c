/*
MODEL_ANIMATION_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "model_animation_definitions.h"
#include "vehicles.h"
#include "sound_definitions.h"

/* ---------- globals */

static struct animation_list_entry weapon_type_animation_list_entries[] =
{
	{ "reload-1", _animation_replacement },
	{ "reload-2", _animation_replacement },
	{ "chamber-1", _animation_overlay },
	{ "chamber-2", _animation_overlay },
	{ "fire-1", _animation_overlay },
	{ "fire-2", _animation_overlay },
	{ "charged-1", _animation_overlay },
	{ "charged-2", _animation_overlay },
	{ "melee", _animation_overlay },
	{ "overheat", _animation_overlay }
};

struct animation_list weapon_type_animation_list = { NUMBEROF(weapon_type_animation_list_entries), weapon_type_animation_list_entries };

static struct animation_list_entry weapon_class_animation_list_entries[] =
{
	{ "idle", _animation_base },
	{ "gesture", _animation_base },
	{ "turn-left", _animation_base },
	{ "turn-right", _animation_base },
	{ "dive-front", _animation_base },
	{ "dive-back", _animation_base },
	{ "dive-left", _animation_base },
	{ "dive-right", _animation_base },
	{ "move-front", _animation_base },
	{ "move-back", _animation_base },
	{ "move-left", _animation_base },
	{ "move-right", _animation_base },
	{ "slide-front", _animation_base },
	{ "slide-back", _animation_base },
	{ "slide-left", _animation_base },
	{ "slide-right", _animation_base },
	{ "airborne", _animation_base },
	{ "land-soft", _animation_base },
	{ "land-hard", _animation_base },
	{ "unused", _animation_overlay },
	{ "throw-grenade", _animation_base },
	{ "disarm", _animation_replacement },
	{ "drop", _animation_replacement },
	{ "ready", _animation_replacement },
	{ "put-away", _animation_replacement },
	{ "aim-still", _animation_overlay },
	{ "aim-move", _animation_overlay },
	{ "surprise-front", _animation_base },
	{ "surprise-back", _animation_base },
	{ "berserk", _animation_base },
	{ "evade-left", _animation_base },
	{ "evade-right", _animation_base },
	{ "signal-move", _animation_base },
	{ "signal-attack", _animation_base },
	{ "warn", _animation_base },
	{ "stunned-front", _animation_base },
	{ "stunned-back", _animation_base },
	{ "stunned-left", _animation_base },
	{ "stunned-right", _animation_base },
	{ "melee", _animation_base },
	{ "celebrate", _animation_base },
	{ "panic", _animation_base },
	{ "melee-airborne", _animation_base },
	{ "flaming", _animation_base },
	{ "resurrect-front", _animation_base },
	{ "resurrect-back", _animation_base },
	{ "melee-continuous", _animation_base },
	{ "feeding", _animation_base },
	{ "leap-start", _animation_base },
	{ "leap-airborne", _animation_base },
	{ "leap-melee", _animation_base },
	{ "zapping", _animation_base },
	{ "unused", _animation_base },
	{ "unused", _animation_base },
	{ "unused", _animation_base }
};

struct animation_list weapon_class_animation_list = { NUMBEROF(weapon_class_animation_list_entries), weapon_class_animation_list_entries };

char const *damage_type_strings[] =
{
	"s-ping",
	"h-ping",
	"s-kill",
	"h-kill"
};

char const *damage_direction_strings[] =
{
	"front",
	"left",
	"right",
	"back"
};

char const *damage_part_strings[] =
{
	"gut",
	"chest",
	"head",
	"l-arm",
	"l-hand",
	"l-leg",
	"l-foot",
	"r-arm",
	"r-hand",
	"r-leg",
	"r-foot"
};

static struct animation_list_entry unit_seat_animation_list_entries[] =
{
	{ "airborne-dead", _animation_base },
	{ "landing-dead", _animation_base },
	{ "acc-front-back", _animation_overlay },
	{ "acc-left-right", _animation_overlay },
	{ "acc-up-down", _animation_overlay },
	{ "push", _animation_overlay },
	{ "twist", _animation_overlay },
	{ "enter", _animation_base },
	{ "exit", _animation_base },
	{ "look", _animation_overlay },
	{ "talk", _animation_overlay },
	{ "emotions", NONE },
	{ "unused", NONE },
	{ "user0", NONE },
	{ "user1", NONE },
	{ "user2", NONE },
	{ "user3", NONE },
	{ "user4", NONE },
	{ "user5", NONE },
	{ "user6", NONE },
	{ "user7", NONE },
	{ "user8", NONE },
	{ "user9", NONE },
	{ "flying-front", _animation_base },
	{ "flying-back", _animation_base },
	{ "flying-left", _animation_base },
	{ "flying-right", _animation_base },
	{ "opening", _animation_base },
	{ "closing", _animation_base },
	{ "hovering", _animation_base }
};

struct animation_list unit_seat_animation_list = { NUMBEROF(unit_seat_animation_list_entries), unit_seat_animation_list_entries };

static struct animation_list_entry first_person_weapon_animation_list_entries[] =
{
	{ "idle", _animation_base },
	{ "posing", _animation_base },
	{ "fire-1", _animation_base },
	{ "moving", _animation_overlay },
	{ "overlays", _animation_overlay },
	{ "light-off", _animation_base },
	{ "light-on", _animation_base },
	{ "reload-empty", _animation_base },
	{ "reload-full", _animation_base },
	{ "overheated", _animation_base },
	{ "ready", _animation_base },
	{ "put-away", _animation_base },
	{ "overcharged", _animation_base },
	{ "melee", _animation_base },
	{ "fire-2", _animation_base },
	{ "overcharged-jitter", _animation_overlay },
	{ "throw-grenade", _animation_base },
	{ "ammunition", _animation_overlay },
	{ "misfire-1", _animation_base },
	{ "misfire-2", _animation_base },
	{ "throw-overheated", _animation_base },
	{ "overheating", _animation_base },
	{ "overheating-again", _animation_base },
	{ "enter", _animation_base },
	{ "exit-empty", _animation_base },
	{ "exit-full", _animation_base },
	{ "o-h-exit", _animation_base },
	{ "o-h-s-enter", _animation_base }
};

struct animation_list first_person_weapon_animation_list = { NUMBEROF(first_person_weapon_animation_list_entries), first_person_weapon_animation_list_entries };

static struct animation_list_entry weapon_animation_list_entries[] =
{
	{ "idle", _animation_base },
	{ "ready", _animation_base },
	{ "put-away", _animation_base },
	{ "reload-1", _animation_base },
	{ "reload-2", _animation_base },
	{ "chamber-1", _animation_base },
	{ "chamber-2", _animation_base },
	{ "charged-1", _animation_base },
	{ "charged-2", _animation_base },
	{ "fire-1", _animation_base },
	{ "fire-2", _animation_base }
};

struct animation_list weapon_animation_list = { NUMBEROF(weapon_animation_list_entries), weapon_animation_list_entries };

static struct animation_list_entry vehicle_animation_list_entries[] =
{
	{ "steering", _animation_overlay },
	{ "roll", _animation_overlay },
	{ "throttle", _animation_overlay },
	{ "velocity", _animation_overlay },
	{ "braking", _animation_overlay },
	{ "ground-speed", _animation_overlay },
	{ "occupied", _animation_overlay },
	{ "unoccupied", _animation_overlay }
};

struct animation_list vehicle_animation_list = { NUMBEROF(vehicle_animation_list_entries), vehicle_animation_list_entries };

static struct animation_list_entry device_animation_list_entries[] =
{
	{ "position", _animation_overlay },
	{ "power", _animation_overlay }
};

struct animation_list device_animation_list = { NUMBEROF(device_animation_list_entries), device_animation_list_entries };

/* ---------- public code */

void *animation_get_frame_data(
	struct animation const *animation,
	short frame_index)
{
	boolean compressed = TEST_FLAG(animation->flags, _animation_compressed_bit) && hs_model_animation_compression_enabled;
	byte *data = tag_data_get_pointer(&animation->data, 0, 0);

	match_assert("c:\\halo\\SOURCE\\models\\model_animation_definitions.c", MATCH_LINE_NTSC(1146, 1147), frame_index>=0 && frame_index<animation->frame_count);

	if (compressed)
	{
		data += animation->compressed_data_offset;
	}
	else
	{
		data += animation->frame_size*frame_index;
	}

	return data;
}

void *animation_get_frame_info(
	struct animation const *animation,
	short frame_index,
	short size)
{
	match_assert("c:\\halo\\SOURCE\\models\\model_animation_definitions.c", MATCH_LINE_NTSC(1166, 1167), frame_index>=0 && frame_index<animation->frame_count);

	return tag_data_get_pointer(&animation->frame_info, size*frame_index, size);
}

char *animation_list_get_string(
	struct animation_list *list,
	short index)
{
	char *result = NULL;

	if (index<list->count)
	{
		result = list->animations[index].name;
	}

	if (!result)
	{
		result = "#<invalid>";
	}

	return result;
}
