/*
RECORDED_ANIMATION_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __RECORDED_ANIMATION_DEFINITIONS_H
#define __RECORDED_ANIMATION_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	RECORDED_ANIMATION_VERSION = 4,
	MAXIMUM_RECORDED_ANIMATIONS_PER_MAP = 1024,
	MAXIMUM_RECORDED_ANIMATION_DATA_SIZE = 2*1024*1024,
	RECORDED_ANIMATION_UNIT_CONTROL_DATA_VERSION = 4
};

/* ---------- macros */

/* ---------- structures */

struct recorded_animation_definition
{
	char name[TAG_STRING_LENGTH+1];
	byte version;
	byte flags;
	byte unit_control_data_version;
	byte pad0;
	word ticks;
	word pad1;
	long pad2[1];
	struct tag_data animation_data;
};

/* ---------- prototypes/RECORDED_ANIMATION_DEFINITIONS.C */

short scenario_get_animation_by_name(struct scenario *scenario, char const *animation_name);

/* ---------- prototypes/RECORDED_ANIMATION_INITIALIZE.C */

void recorded_animation_byteswap_unit_control(char **playback_stream, byte unit_version);
void recorded_animation_initialize_unit_control(struct unit_control_data *control, char const **playback_stream, byte unit_version);
void recorded_animation_write_unit_control(struct unit_control_data *control, char **playback_stream, byte unit_version);

/* ---------- globals */

extern struct tag_data_definition recorded_animation_event_stream_data;
extern struct tag_block_definition recorded_animation_block;

/* ---------- public code */

#endif // __RECORDED_ANIMATION_DEFINITIONS_H
