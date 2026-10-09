/*
FIRST_PERSON_WEAPONS.H

header included in hcex build.
*/

#ifndef __FIRST_PERSON_WEAPONS_H
#define __FIRST_PERSON_WEAPONS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/FIRST_PERSON_WEAPONS.C */

void first_person_weapon_message_from_weapon(long weapon_index, short message_type);
void first_person_weapon_center_flashlight(long unit_index, union real_point3d *position, union real_vector3d *forward, union real_vector3d *up);
boolean first_person_weapon_adjust_light(long weapon_index, char const *marker_name, union real_point3d *position, union real_vector3d *forward, union real_vector3d *up);
short first_person_weapon_get_marker_by_name_render(long weapon_index, char const *name, struct object_marker *markers, short maximum_marker_count);

/* ---------- globals */

/* ---------- public code */

#endif // __FIRST_PERSON_WEAPONS_H
