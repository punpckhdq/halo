/*
SOUND_DSOUND.H

file has inline function assertions.
*/

#ifndef __SOUND_DSOUND_H
#define __SOUND_DSOUND_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/SOUND_DSOUND_XBOX.C */

LPDIRECTSOUND dsound_get(void);

/* ---------- globals */

extern boolean debug_sound_channels;
extern HRESULT interrupt_result;

/* ---------- public code */

static __inline long dsound_volume_from_gain(
	real gain,
	long maximum_volume)
{
	long volume;

	match_assert("c:\\halo\\source\\sound\\sound_dsound.h", 35, gain>=0.f && gain<=1.f);

	if (gain==0.f)
	{
		volume = DSBVOLUME_MIN;
	}
	else
	{
		volume = (long)(log10(gain)*2000.0 + maximum_volume);
		volume = PIN(volume, DSBVOLUME_MIN, maximum_volume);
	}

	return volume;
}

__inline long dsound_frequency_from_pitch(
	long samples_per_second,
	real pitch)
{
	match_assert("c:\\halo\\source\\sound\\sound_dsound.h", 54, samples_per_second==22050 || samples_per_second==44100);

	return (long)PIN(samples_per_second*pitch, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX);
}

__inline long dsound_angle_from_angle(
	real angle)
{
	return (long)(angle * (180.f / _pi));
}

__inline long dsound_occlusion_from_occlusion(
	real occlusion)
{
	return dsound_volume_from_gain(1.f - occlusion, 0);
}

__inline long dsound_obstruction_from_obstruction(
	real obstruction)
{
	return dsound_volume_from_gain(1.f - obstruction, 0);
}

#endif // __SOUND_DSOUND_H
