/*
OBJECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "objects.h"
#include "data.h"

/* ---------- constants */

extern struct data_array* object_header_data;

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

struct object* object_get_and_verify_type(long object_index, unsigned long type_mask) {
	struct object* ptr = ((struct object_header*)datum_get(object_header_data, object_index))->object;
	match_vassert("c:\\halo\\SOURCE\\objects\\objects.c", 1690, (1 << ptr->type_id) & type_mask, csprintf(
		temporary,
		"got an object type we didn't expect (expected one of 0x%08x but got #%d).", type_mask, ptr->type_id));
	return ptr;
}

/* ---------- private code */
