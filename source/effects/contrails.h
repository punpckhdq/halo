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

long contrail_new(long definition_index, long object_index, short attachment_index);
void contrail_owner_collision(long contrail_index, unsigned char object_dying, real dt);

/* ---------- globals */

/* ---------- public code */

#endif // __CONTRAILS_H
