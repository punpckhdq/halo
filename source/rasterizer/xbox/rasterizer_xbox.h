/*
RASTERIZER_XBOX.H
*/

#ifndef __RASTERIZER_XBOX_H
#define __RASTERIZER_XBOX_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RASTERIZER_XBOX.C */

void rasterizer_preinitialize__fill_you_up_with_the_devils_cock(void);


/* ---------- prototypes/RASTERIZER_XBOX_PROFILE.C */

void rasterizer_profile_begin(short profile);

void rasterizer_profile_end(short profile);

/* ---------- globals */

extern D3DDevice global_d3d_device;

/* ---------- public code */

#endif // __RASTERIZER_XBOX_H
