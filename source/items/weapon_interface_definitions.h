/*
WEAPON_INTERFACE_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __WEAPON_INTERFACE_DEFINITIONS_H
#define __WEAPON_INTERFACE_DEFINITIONS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct weapon_interface_definition
{
	struct tag_reference first_person_model;
	struct tag_reference first_person_animations;
	long unused[1];
	struct tag_reference hud_interface;
};

struct weapon_magazine_interface_definition
{
	long unused[2];
	rectangle2d unused_rectangles[2];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __WEAPON_INTERFACE_DEFINITIONS_H
