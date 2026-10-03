/*
RECORDED_ANIMATION_INITIALIZE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "recorded_animation_definitions.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct unit_control_data_entry
{
	struct byte_swap_definition *bs_def;
	long size;
	long offset;
};

/* ---------- prototypes */

/* ---------- globals */

static byte_swap_code real_vector2d_bs_codes[] =
{
	_4byte,
	_4byte
};

static struct byte_swap_definition real_vector2d_bs_definition =
{
	"real_vector2d",
	sizeof(real_vector2d),
	real_vector2d_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code real_vector3d_bs_codes[] =
{
	_4byte,
	_4byte,
	_4byte
};

static struct byte_swap_definition real_vector3d_bs_definition =
{
	"real_vector3d",
	sizeof(real_vector3d),
	real_vector3d_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static struct unit_control_data_entry unit_control_v1_map[] =
{
	{ &byte_bs_definition, sizeof(char), offsetof(struct unit_control_data, animation_state) },
	{ &byte_bs_definition, sizeof(char), offsetof(struct unit_control_data, aiming_speed) },
	{ &word_bs_definition, sizeof(word), offsetof(struct unit_control_data, control_flags) },
	{ &word_bs_definition, sizeof(short), offsetof(struct unit_control_data, weapon_index) },
	{ &word_bs_definition, sizeof(short), NONE },
	{ &real_vector2d_bs_definition, sizeof(real_vector2d), offsetof(struct unit_control_data, throttle) },
	{ &real_vector3d_bs_definition, sizeof(real_vector3d), offsetof(struct unit_control_data, facing_vector) },
	{ &real_vector3d_bs_definition, sizeof(real_vector3d), offsetof(struct unit_control_data, aiming_vector) },
	{ &real_vector3d_bs_definition, sizeof(real_vector3d), offsetof(struct unit_control_data, looking_vector) },
	{ NULL, NONE, NONE }
};

static struct unit_control_data_entry unit_control_v2_map[] =
{
	{ &long_bs_definition, sizeof(real), offsetof(struct unit_control_data, primary_trigger) },
	{ NULL, NONE, NONE }
};

static struct unit_control_data_entry unit_control_v3_map[] =
{
	{ &word_bs_definition, sizeof(short), offsetof(struct unit_control_data, grenade_index) },
	{ NULL, NONE, NONE }
};

static struct unit_control_data_entry unit_control_v4_map[] =
{
	{ &word_bs_definition, sizeof(short), offsetof(struct unit_control_data, zoom_level) },
	{ NULL, NONE, NONE }
};

static struct unit_control_data_entry *unit_control_data_map[RECORDED_ANIMATION_UNIT_CONTROL_DATA_VERSION] =
{
	unit_control_v1_map,
	unit_control_v2_map,
	unit_control_v3_map,
	unit_control_v4_map
};

/* ---------- public code */

void recorded_animation_byteswap_unit_control(
	char **playback_stream,
	byte unit_version)
{
	short version_index;

	for (version_index = 0; version_index < MAX(unit_version, 1); version_index++)
	{
		struct unit_control_data_entry *entry;

		for (entry = unit_control_data_map[version_index]; entry->size != NONE; entry++)
		{
			byte_swap_data(entry->bs_def, *playback_stream, 1);
			*playback_stream += entry->size;
		}
	}

	return;
}

void recorded_animation_initialize_unit_control(
	struct unit_control_data *control,
	char const **playback_stream,
	byte unit_version)
{
	short version_index;

	memset(control, 0, sizeof(struct unit_control_data));
	control->zoom_level = NONE;

	for (version_index = 0; version_index < MAX(unit_version, 1); version_index++)
	{
		struct unit_control_data_entry *entry;

		for (entry = unit_control_data_map[version_index]; entry->size != NONE; entry++)
		{
			if (entry->offset != NONE)
			{
				memcpy((byte *)control + entry->offset, *playback_stream, entry->size);
			}
			*playback_stream += entry->size;
		}
	}

	return;
}

void recorded_animation_write_unit_control(
	struct unit_control_data *control,
	char **playback_stream,
	byte unit_version)
{
	short version_index;

	for (version_index = 0; version_index < MAX(unit_version, 1); version_index++)
	{
		struct unit_control_data_entry *entry;

		for (entry = unit_control_data_map[version_index]; entry->size != NONE; entry++)
		{
			memcpy(*playback_stream, (byte *)control + entry->offset, entry->size);
			*playback_stream += entry->size;
		}
	}

	return;
}

/* ---------- private code */
