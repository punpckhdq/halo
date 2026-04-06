/*
AIM_ASSIST.H

header included in hcex build.
*/

#ifndef __AIM_ASSIST_H
#define __AIM_ASSIST_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct aim_assist_parameters
{
	real autoaim_angle;
	real autoaim_distance;
	real magnetism_angle;
	real magnetism_distance;
	real deviation_angle;
	real unused;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __AIM_ASSIST_H
