/*
PERIODIC_FUNCTIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"
#include "tag_groups.h"
#include "periodic_functions.h"

/* ---------- constants */

enum
{
	FUNCTION_TABLE_SIZE = 1024,
	PERIODIC_FUNCTION_PERIODS_PER_TABLE = 28,
	WRAPPING_FUNCTION_FLAGS = FLAG(_periodic_function_slide)|FLAG(_periodic_function_slide_with_random_period),
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void periodic_function_build_variable_period_x_table(real *x_table);
static void transition_function_build_table(short function_type, byte *integer_table);
static void periodic_function_build_table(short function_type, byte *integer_table);

/* ---------- globals */

static char *global_periodic_functions_enum_strings[NUMBER_OF_PERIODIC_FUNCTIONS] =
{
	"one",
	"zero",
	"cosine",
	"cosine (variable period)",
	"diagonal wave",
	"diagonal wave (variable period)",
	"slide",
	"slide (variable period)",
	"noise",
	"jitter",
	"wander",
	"spark"
};

struct enum_definition global_periodic_functions_enum =
{
	NUMBER_OF_PERIODIC_FUNCTIONS,
	global_periodic_functions_enum_strings
};

static char *global_transition_functions_enum_strings[NUMBER_OF_TRANSITION_FUNCTIONS] =
{
	"linear",
	"early",
	"very early",
	"late",
	"very late",
	"cosine"
};

struct enum_definition global_transition_functions_enum =
{
	NUMBER_OF_TRANSITION_FUNCTIONS,
	global_transition_functions_enum_strings
};

static byte *periodic_function_tables[NUMBER_OF_PERIODIC_FUNCTIONS];
static byte *transition_function_tables[NUMBER_OF_TRANSITION_FUNCTIONS];
static boolean function_tables_initialized;

/* ---------- public code */

void periodic_functions_initialize(
	void)
{
	short periodic_function_type;
	short transition_function_type;

	match_assert("c:\\halo\\SOURCE\\math\\periodic_functions.c", 67, !function_tables_initialized);
	function_tables_initialized = TRUE;

	*get_global_random_seed_address() = 0x20f3f660;

	for (periodic_function_type = 0; periodic_function_type<NUMBER_OF_PERIODIC_FUNCTIONS; periodic_function_type++)
	{
		periodic_function_tables[periodic_function_type] = match_malloc("c:\\halo\\SOURCE\\math\\periodic_functions.c", 78, FUNCTION_TABLE_SIZE);
		if (periodic_function_tables[periodic_function_type])
		{
			periodic_function_build_table(periodic_function_type, periodic_function_tables[periodic_function_type]);
		}
		else
		{
			function_tables_initialized = FALSE;
		}
	}

	for (transition_function_type = 0; transition_function_type<NUMBER_OF_TRANSITION_FUNCTIONS; transition_function_type++)
	{
		transition_function_tables[transition_function_type] = match_malloc("c:\\halo\\SOURCE\\math\\periodic_functions.c", 96, FUNCTION_TABLE_SIZE);
		if (transition_function_tables[transition_function_type])
		{
			transition_function_build_table(transition_function_type, transition_function_tables[transition_function_type]);
		}
		else
		{
			function_tables_initialized = FALSE;
		}
	}

	return;
}

void periodic_functions_dispose(
	void)
{
	if (function_tables_initialized)
	{
		short periodic_function_type;
		short transition_function_type;

		for (periodic_function_type = 0; periodic_function_type<NUMBER_OF_PERIODIC_FUNCTIONS; periodic_function_type++)
		{
			match_free("c:\\halo\\SOURCE\\math\\periodic_functions.c", 122, periodic_function_tables[periodic_function_type]);
		}

		for (transition_function_type = 0; transition_function_type<NUMBER_OF_TRANSITION_FUNCTIONS; transition_function_type++)
		{
			match_free("c:\\halo\\SOURCE\\math\\periodic_functions.c", 132, transition_function_tables[transition_function_type]);
		}

		function_tables_initialized = FALSE;
	}

	return;
}

real periodic_function_evaluate(
	short function_type,
	real time)
{
	real result;

	if (function_type==_periodic_function_one)
	{
		result = 1.f;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\math\\periodic_functions.c", 157, function_type>=0 && function_type<NUMBER_OF_PERIODIC_FUNCTIONS);

		if (function_tables_initialized)
		{
			long integer_time;
			real fractional_time;

			{
				real const function_period = 25.6f;

				time*= function_period;
			}

			fractional_time = (real)fmod(time, 1.0);
			integer_time = fast_ftol(time-fractional_time)&(FUNCTION_TABLE_SIZE-1);

			{
				real const one_over_255 = 1.f/255.f;
				byte const *table = periodic_function_tables[function_type];
				real result0;
				real result1;

				result0 = table[integer_time]*one_over_255;
				result1 = table[(integer_time+1)&(FUNCTION_TABLE_SIZE-1)]*one_over_255;

				if (TEST_FLAG(WRAPPING_FUNCTION_FLAGS, function_type))
				{
					if (result0>0.75f && result1<0.25f)
					{
						result1+= 1.f;
					}

					result = (1.f-fractional_time)*result0 + result1*fractional_time;
					if (result>1.f)
					{
						result-= 1.f;
					}
				}
				else
				{
					result = (1.f-fractional_time)*result0 + result1*fractional_time;
				}
			}
		}
		else
		{
			result = 0.f;
		}
	}

	return result;
}

real transition_function_evaluate(
	short function_type,
	real value)
{
	real result;

	if (value<0.f)
	{
		value = 0.f;
	}
	else if (value>1.f)
	{
		value = 1.f;
	}

	if (function_type==_transition_function_linear)
	{
		result = value;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\math\\periodic_functions.c", 216, function_type>=0 && function_type<NUMBER_OF_TRANSITION_FUNCTIONS);

		if (function_tables_initialized)
		{
			real const one_over_255 = 1.f/255.f;
			byte const *table = transition_function_tables[function_type];
			real index = value*(FUNCTION_TABLE_SIZE-1);
			real fractional_index = (real)fmod(index, 1.0);
			short integer_index = (short)fast_ftol(index-0.5f);

			if (integer_index==FUNCTION_TABLE_SIZE-1)
			{
				result = table[integer_index]*one_over_255;
			}
			else
			{
				real result0 = table[integer_index]*one_over_255;
				real result1 = table[integer_index+1]*one_over_255;

				result = result0*(1.f-fractional_index) + result1*fractional_index;
			}
		}
		else
		{
			result = 0.f;
		}
	}

	return result;
}

/* ---------- private code */

static void periodic_function_build_variable_period_x_table(
	real *x_table)
{
	real x = 0.f;
	short index;

	for (index = 0; index<FUNCTION_TABLE_SIZE; index++)
	{
		x_table[index] = x;
		x+= real_random()*0.25f+0.25f
			+ (cosine(2.f*_pi*4.1f*index/FUNCTION_TABLE_SIZE)+1.f)*real_random()
			+ (cosine(2.f*_pi*5.1f*index/FUNCTION_TABLE_SIZE)+1.f)*real_random()
			+ (cosine(2.f*_pi*7.3f*index/FUNCTION_TABLE_SIZE)+1.f)*real_random();
	}

	for (index = 0; index<FUNCTION_TABLE_SIZE; index++)
	{
		x_table[index]/= x;
	}

	return;
}

static void transition_function_build_table(
	short function_type,
	byte *integer_table)
{
	short index;

	for (index = 0; index<FUNCTION_TABLE_SIZE; index++)
	{
		real x = (real)index/(FUNCTION_TABLE_SIZE-1);
		real v;
		long integer_value;

		switch (function_type)
		{
		case _transition_function_linear:
			v = x;
			break;
		case _transition_function_early:
			v = power(x, 0.5f);
			break;
		case _transition_function_very_early:
			v = power(x, 0.25f);
			break;
		case _transition_function_late:
			v = power(x, 2.f);
			break;
		case _transition_function_very_late:
			v = power(x, 4.f);
			break;
		case _transition_function_cosine:
			v = (sine(x*_pi - _pi/2.f)+1.f)*0.5f;
			break;
		default:
			match_halt("c:\\halo\\SOURCE\\math\\periodic_functions.c", 411);
		}

		integer_value = (long)(v*255.f);
		integer_table[index] = (byte)PIN(integer_value, 0, 255);
	}

	return;
}

static void periodic_function_build_table(
	short function_type,
	byte *integer_table)
{
	real x_table[FUNCTION_TABLE_SIZE];
	real real_table[FUNCTION_TABLE_SIZE];
	real value0 = REAL_MAX;
	real value1 = -REAL_MAX;
	short index;

	periodic_function_build_variable_period_x_table(x_table);

	for (index = 0; index<FUNCTION_TABLE_SIZE; index++)
	{
		real x = (real)index/((real)FUNCTION_TABLE_SIZE/PERIODIC_FUNCTION_PERIODS_PER_TABLE);
		real variable_x = x_table[index]*PERIODIC_FUNCTION_PERIODS_PER_TABLE;
		real value;

		switch (function_type)
		{
		case _periodic_function_one:
			value = 1.f;
			break;
		case _periodic_function_zero:
			value = 0.f;
			break;
		case _periodic_function_cosine:
			value = cosine(x*2.f*_pi);
			break;
		case _periodic_function_cosine_with_random_period:
			value = cosine(variable_x*2.f*_pi);
			break;
		case _periodic_function_slide:
			value = (real)fmod(x, 1.0);
			break;
		case _periodic_function_slide_with_random_period:
			value = (real)fmod(variable_x, 1.0);
			break;
		case _periodic_function_diagonal_wave:
			value = (real)fmod(x, 1.0);
			value = value<0.5f ? 2.f*value : 1.f - 2.f*(value-0.5f);
			break;
		case _periodic_function_diagonal_wave_with_random_period:
			value = (real)fmod(variable_x, 1.0);
			value = value<0.5f ? 2.f*value : 1.f - 2.f*(value-0.5f);
			break;
		case _periodic_function_noise:
			value = real_random();
			break;
		case _periodic_function_jitter:
		case _periodic_function_wander:
			value = (sine(x*(_pi/2.f))*cosine(x*(2.f*_pi*7.f)) + cosine(x*(2.f*_pi*4.f))*cosine(x*(2.f*_pi/7.f)))*0.5f + cosine(x*2.f*_pi)*sine(x*_pi);
			break;
		case _periodic_function_spark:
			value = (real)fmod(variable_x, 1.0);
			value*= value;
			break;
		default:
			match_halt("c:\\halo\\SOURCE\\math\\periodic_functions.c", 499);
		}

		if (value>value1)
		{
			value1 = value;
		}

		if (value<value0)
		{
			value0 = value;
		}

		real_table[index] = value;
	}

	{
		real range = TEST_FLAG(WRAPPING_FUNCTION_FLAGS, function_type) ? 0.f : value1-value0;

		for (index = 0; index<FUNCTION_TABLE_SIZE; index++)
		{
			real value = range!=0.f ? (real_table[index]-value0)/range : real_table[index];
			long integer_value = (long)(value*255.f);

			integer_table[index] = (byte)PIN(integer_value, 0, 255);
		}
	}

	return;
}
