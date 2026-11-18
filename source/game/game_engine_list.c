/*
GAME_ENGINE_LIST.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game_engine_list.h"
#include "game_engine.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct game_engine *game_engines[NUMBER_OF_GAME_ENGINES]=
{
	NULL,
	&ctf_engine,
	&slayer_engine,
	&oddball_engine,
	&king_engine,
	&race_engine,
	&stub_engine,
	NULL
};

/* ---------- public code */

/* ---------- private code */
