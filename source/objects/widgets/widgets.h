/*
WIDGETS.H

header included in hcex build.
*/

#ifndef __WIDGETS_H
#define __WIDGETS_H
#pragma once

/* ---------- constants */

enum
{
	_widget_type_flag = 0,
	_widget_type_antenna,
	_widget_type_glow,
	_widget_type_light_volume,
	_widget_type_lightning,
	NUMBER_OF_WIDGET_TYPES,

	_widget_type_internal_sprite = NUMBER_OF_WIDGET_TYPES,
	_widget_type_internal_occlusion_test,
	_widget_type_internal____,
	NUMBER_OF_INTERNAL_WIDGET_TYPES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/WIDGETS.C */

void widgets_initialize(void);
void widgets_initialize_for_new_map(void);
void widgets_dispose_from_old_map(void);
void widgets_dispose(void);
void widgets_new(long object_index);
void widgets_delete(long object_index);

/* ---------- globals */

/* ---------- public code */

#endif // __WIDGETS_H
