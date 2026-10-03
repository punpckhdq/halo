/*
CONTRAILS.H

header included in hcex build.
*/

#ifndef __CONTRAILS_H
#define __CONTRAILS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/CONTRAILS.C */

void contrails_disconnect_from_structure_bsp(void);
void contrails_reconnect_to_structure_bsp(void);
long contrail_new(long definition_index, long object_index, short attachment_index);
void contrail_owner_collision(long contrail_index, unsigned char object_dying, real dt);

/* ---------- globals */

/* ---------- public code */

#endif // __CONTRAILS_H
