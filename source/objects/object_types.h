/*
OBJECT_TYPES.H
*/

#ifndef __OBJECT_TYPES_H
#define __OBJECT_TYPES_H
#pragma once

/* ---------- constants */

enum
{
	_object_type_biped= 0,
	_object_type_vehicle,
	_object_type_weapon,
	_object_type_equipment,
	_object_type_garbage,
	_object_type_projectile,
	_object_type_scenery,
	_object_type_machine,
	_object_type_control,
	_object_type_light_fixture,
	_object_type_placeholder,
	_object_type_sound_scenery,
	NUMBER_OF_OBJECT_TYPES,

	_object_mask_all=                       UNSIGNED_LONG_MAX,
	_object_mask_unit=                      (FLAG(_object_type_biped)|FLAG(_object_type_vehicle)),
	_object_mask_biped=                     FLAG(_object_type_biped),
	_object_mask_vehicle=                   FLAG(_object_type_vehicle),
	_object_mask_item=                      (FLAG(_object_type_weapon)|FLAG(_object_type_equipment)|FLAG(_object_type_garbage)),
	_object_mask_weapon=                    FLAG(_object_type_weapon),
	_object_mask_equipment=                 FLAG(_object_type_equipment),
	_object_mask_garbage=                   FLAG(_object_type_garbage),
	_object_mask_projectile=                FLAG(_object_type_projectile),
	_object_mask_scenery=                   FLAG(_object_type_scenery),
	_object_mask_sound_scenery=             FLAG(_object_type_sound_scenery),
	_object_mask_device=                    (FLAG(_object_type_machine)|FLAG(_object_type_control)|FLAG(_object_type_light_fixture)),
	_object_mask_machine=                   FLAG(_object_type_machine),
	_object_mask_control=                   FLAG(_object_type_control),
	_object_mask_light_fixture=             FLAG(_object_type_light_fixture),
	_object_mask_placeholder=               FLAG(_object_type_placeholder),

	_object_mask_sightblocking=				(FLAG(_object_type_vehicle)|FLAG(_object_type_scenery)|FLAG(_object_type_machine)),
	_object_mask_cannot_interpolate=		(FLAG(_object_type_projectile)|FLAG(_object_type_scenery)|FLAG(_object_type_machine)|FLAG(_object_type_control)|
											FLAG(_object_type_light_fixture)|FLAG(_object_type_placeholder)|FLAG(_object_type_sound_scenery)),
	_object_mask_remove_on_bsp_switch=		(FLAG(_object_type_scenery)|FLAG(_object_type_light_fixture))
};

/* ---------- structures */

struct object_type_definition {
	char* name;
	unsigned long group_tag;
	short datum_size;
	byte padc_pad9c[0x90];
	struct object_type_definition* next;
};

#endif // __OBJECT_TYPES_H
