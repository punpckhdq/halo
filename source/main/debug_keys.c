/*
DEBUG_KEYS.C

symbols in this file:
000EF410 0080:
	_debug_keys_initialize (0000)
000EF490 0030:
	_debug_keys_dispose (0000)
000EF4C0 0120:
	_debug_keys_update (0000)
000EF5E0 0020:
	_code_000ef5e0 (0000)
000EF600 0020:
	_code_000ef600 (0000)
000EF620 0020:
	_code_000ef620 (0000)
000EF640 0020:
	_code_000ef640 (0000)
000EF660 0020:
	_code_000ef660 (0000)
000EF680 0020:
	_code_000ef680 (0000)
000EF6A0 0020:
	_code_000ef6a0 (0000)
000EF6C0 0020:
	_code_000ef6c0 (0000)
000EF6E0 0010:
	_code_000ef6e0 (0000)
002794B8 000e:
	??_C@_0O@JNACAKKL@Profile?5Reset?$AA@ (0000)
002794C8 000f:
	??_C@_0P@FCOHHKMH@Play?5animation?$AA@ (0000)
002794D8 0011:
	??_C@_0BB@GOEIKNDP@Erase?5All?5Actors?$AA@ (0000)
002794EC 0011:
	??_C@_0BB@HFPDDBOA@Show?5Actor?5Spray?$AA@ (0000)
00279500 0012:
	??_C@_0BC@IFCACBAC@Select?5Prev?5Actor?$AA@ (0000)
00279514 0012:
	??_C@_0BC@KDHIPGAF@Select?5Next?5Actor?$AA@ (0000)
00279528 0016:
	??_C@_0BG@CMDLFNAD@Select?5Next?5Encounter?$AA@ (0000)
00279540 0016:
	??_C@_0BG@JJEFBEKO@Select?5Prev?5Encounter?$AA@ (0000)
00279558 0012:
	??_C@_0BC@KLKBLONB@Select?5This?5Actor?$AA@ (0000)
0027956C 0016:
	??_C@_0BG@IECAMAON@global_debug_key_down?$AA@ (0000)
00279584 0021:
	??_C@_0CB@MBIIKDHK@c?3?2halo?2SOURCE?2main?2debug_keys?4c@ (0000)
00307750 00c8:
	_global_debug_key_list (0000)
00455748 0004:
	_global_debug_key_down (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "debug_keys.h"

#include "ai/ai_debug.h"
#include "ai/ai_profile.h"
#include "cseries/profile.h"
#include "math/integer_math.h"
#include "input/input.h"

/* ---------- constants */



/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void debug_key_select_actor(boolean key_is_down);
static void debug_key_select_prev_encounter(boolean key_is_down);
static void debug_key_select_next_encounter(boolean key_is_down);
static void debug_key_select_next_actor(boolean key_is_down);
static void debug_key_select_prev_actor(boolean key_is_down);
static void debug_key_show_actor_spray(boolean key_is_down);
static void debug_key_erase_all_actors(boolean key_is_down);
static void debug_key_play_animation(boolean key_is_down);
static void debug_key_profile_reset(boolean key_is_down);

/* ---------- globals */

// Bitvector that stores all the debug keys that are down
static long *global_debug_key_down;

struct debug_key global_debug_key_list[10] =
{
	{ "Select This Actor", _key_f1, _debug_key_no_modifier, &debug_key_select_actor, FALSE, NULL },
	{ "Select Prev Encounter", _key_f2, _debug_key_no_modifier, &debug_key_select_prev_encounter, FALSE, NULL },
	{ "Select Next Encounter", _key_f3, _debug_key_no_modifier, &debug_key_select_next_encounter, FALSE, NULL },
	{ "Select Next Actor", _key_f4, _debug_key_no_modifier, &debug_key_select_next_actor, FALSE, NULL },
	{ "Select Prev Actor", _key_f4, _debug_key_shift, &debug_key_select_prev_actor, FALSE, NULL },
	{ "Show Actor Spray", _key_f5, _debug_key_no_modifier, &debug_key_show_actor_spray, FALSE, NULL },
	{ "Erase All Actors", _key_f6, _debug_key_no_modifier, &debug_key_erase_all_actors, FALSE, NULL },
	{ "Play animation", _key_k, _debug_key_no_modifier, &debug_key_play_animation, FALSE, NULL },
	{ "Profile Reset", _key_f9, _debug_key_no_modifier, &debug_key_profile_reset, FALSE, NULL },
	{ NULL, NONE, _debug_key_no_modifier, NULL, FALSE, NULL }
};

/* ---------- public code */

void debug_keys_initialize(
	void)
{
	long key_count;

	for (key_count= 0; global_debug_key_list[key_count].name; key_count++)
	{
		if (global_debug_key_list[key_count].variable)
		{
			*global_debug_key_list[key_count].variable= FALSE;
		}
	}

	global_debug_key_down= (long *)match_malloc("c:\\halo\\SOURCE\\main\\debug_keys.c", 88, BIT_VECTOR_SIZE_IN_BYTES(key_count));
	match_assert("c:\\halo\\SOURCE\\main\\debug_keys.c", 89, global_debug_key_down);
	memset(global_debug_key_down, 0, BIT_VECTOR_SIZE_IN_BYTES(key_count));

	return;
}

void debug_keys_dispose(
	void)
{
	if (global_debug_key_down)
	{
		debug_free(global_debug_key_down, "c:\\halo\\SOURCE\\main\\debug_keys.c", 100);
		global_debug_key_down= NULL;
	}

	return;
}

void debug_keys_update(
	void)
{
	boolean modifier_down[NUMBER_OF_DEBUG_KEY_MODIFIERS];
	long key_index;

	const boolean shift_down= input_key_is_down(_key_shift);
	const boolean control_down= input_key_is_down(_key_control);
	modifier_down[_debug_key_no_modifier]= !shift_down && !control_down;
	modifier_down[_debug_key_shift]= shift_down && !control_down;
	modifier_down[_debug_key_ctrl]= !shift_down && control_down;
	modifier_down[_debug_key_shift_ctrl]= shift_down && control_down;

	
	for (key_index= 0; global_debug_key_list[key_index].name; key_index++)
	{
		boolean down= 
			input_key_is_down(global_debug_key_list[key_index].key_code) && 
			modifier_down[global_debug_key_list[key_index].modifier];

		if (!global_debug_key_list[key_index].toggle_variable &&
			global_debug_key_list[key_index].variable != NULL)
		{
			*global_debug_key_list[key_index].variable= down;
		}

		if (BIT_VECTOR_TEST_FLAG(global_debug_key_down, key_index))
		{
			if (!down)
			{
				BIT_VECTOR_SET_FLAG(global_debug_key_down, key_index, FALSE);
				if (global_debug_key_list[key_index].toggle_variable &&
					global_debug_key_list[key_index].variable)
				{
					*global_debug_key_list[key_index].variable= *global_debug_key_list[key_index].variable == FALSE;
				}

				if (global_debug_key_list[key_index].function)
				{
					global_debug_key_list[key_index].function(FALSE);
				}
			}
		}
		else if (down)
		{
			BIT_VECTOR_SET_FLAG(global_debug_key_down, key_index, TRUE);
			if (global_debug_key_list[key_index].function)
			{
				global_debug_key_list[key_index].function(TRUE);
			}
		}
	}

	return;
}

/* ---------- private code */

static void debug_key_select_actor(
	boolean key_is_down)
{
	if (key_is_down)
	{
		ai_debug.select_actor= TRUE;
	}

	return;
}

static void debug_key_select_next_encounter(
	boolean key_is_down)
{
	if (key_is_down)
	{
		ai_debug_change_selected_encounter(TRUE);
	}

	return;
}

static void debug_key_select_prev_encounter(
	boolean key_is_down)
{
	if (key_is_down)
	{
		ai_debug_change_selected_encounter(FALSE);
	}

	return;
}

static void debug_key_select_next_actor(
	boolean key_is_down)
{
	if (key_is_down)
	{
		ai_debug_change_selected_actor(TRUE);
	}

	return;
}

static void debug_key_select_prev_actor(
	boolean key_is_down)
{
	if (key_is_down)
	{
		ai_debug_change_selected_actor(FALSE);
	}

	return;
}

static void debug_key_show_actor_spray(
	boolean key_is_down)
{
	if (key_is_down)
	{
		ai_profile_change_render_spray();
	}

	return;
}

static void debug_key_profile_reset(
	boolean key_is_down)
{
	if (key_is_down)
	{
		profile_initialize();
	}

	return;
}

static void debug_key_erase_all_actors(
	boolean key_is_down)
{
	if (key_is_down)
	{
		ai_scripting_erase_all();
	}

	return;
}

static void debug_key_play_animation(
	boolean key_is_down)
{
	// Dead code
	return;
}
