/*
RASTERIZER_COMMON.H

header included in hcex build.
*/

#ifndef __RASTERIZER_COMMON_H
#define __RASTERIZER_COMMON_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct fog_screen
{
	word flags;
	short layer_count;
	real near_distance;
	real far_distance;
	real near_density;
	real far_density;
	real start_distance_from_fog_plane;
	long unused1[1];
	pixel32 color;
	real  rotation_multiplier;
	real  strafing_multiplier;
	real  zoom_multiplier;
	long unused2[2];
	real  map_scale;
	tag_reference map;
	real  animation_period;
	real  animation_unused[1];
	real  wind_velocity_lower_bound;
	real  wind_velocity_upper_bound;
	real  wind_period_lower_bound;
	real  wind_period_upper_bound;
	real wind_acceleration_weight;
	real wind_perpendicular_weight;
	long wind_unused[2];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_COMMON_H
