/*
DEBUG_KEYS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "debug_keys.h"
#include "ai_debug.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "profile.h"
#include "integer_math.h"
#include "input.h"

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
