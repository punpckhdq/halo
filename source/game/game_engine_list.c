/*
GAME_ENGINE_LIST.C

symbols in this file:
002DE510 0020:
	_game_engines (0000)
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
