/*
PHYSICS_VARIABLES.C

*/

/* ---------- headers */
#include "cseries.h"
#include "physics_variables.h"
#include "real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static real physics_variable_position_get_seek_direction(real position, struct physics_variable_position const *definition, boolean cyclical_position, real desired_position);

/* ---------- globals */

/* ---------- public code */

void physics_variable_position_update(
	real *position, 
	struct physics_variable_position const *definition, 
	boolean cyclical_position, 
	real speed)
{
	speed += *position;
	*position = speed;

	if (speed < definition->minimum_position)
	{
		if (cyclical_position)
		{
			*position = (definition->maximum_position - definition->minimum_position) + speed;
		}
		else
		{
			*position = definition->minimum_position;
		}
	}
	else if (speed > definition->maximum_position)
	{
		if (cyclical_position)
		{
			*position = speed - (definition->maximum_position - definition->minimum_position);
		}
		else
		{
			*position = definition->maximum_position;
		}
	}

	return;
}

void physics_variable_speed_update(
	real *speed, 
	struct physics_variable_speed const *definition,
	real magnitude)
{
	real absolute_magnitude = fabs(magnitude);
	real acceleration = absolute_magnitude*definition->acceleration;
	real deceleration = absolute_magnitude*definition->deceleration;

	if (magnitude > 0.0f)
	{
		real maximum_speed;

		if (*speed <= -deceleration)
		{
			*speed += deceleration;
		}
		else if (*speed >= 0.0f)
		{
			*speed += acceleration;
		}
		else
		{
			*speed = (*speed/deceleration + 1.0f)*acceleration;
		}

		maximum_speed = absolute_magnitude*definition->maximum_forward_speed;
		*speed = (maximum_speed < *speed) ? maximum_speed : *speed;
	}
	else if (magnitude < 0.0f)
	{
		real minimum_speed;

		if (*speed >= deceleration)
		{
			*speed -= deceleration;
		}
		else if (*speed <= 0.0f)
		{
			*speed -= acceleration;
		}
		else
		{
			*speed = (*speed/deceleration - 1.0f)*acceleration;
		}

		minimum_speed = -(absolute_magnitude*definition->maximum_reverse_speed);
		*speed = (minimum_speed > *speed) ? minimum_speed : *speed;
	}

	return;
}

boolean physics_variable_speed_update_seek(
	real *speed, 
	struct physics_variable_speed const *definition, 
	real desired_speed, 
	real magnitude)
{
	boolean result = FALSE;

	if (*speed > desired_speed)
	{
		physics_variable_speed_update(speed, definition, -magnitude);

		if (*speed <= desired_speed)
		{
			*speed = desired_speed;
			result = TRUE;
		}
	}
	else if (*speed < desired_speed)
	{
		physics_variable_speed_update(speed, definition, magnitude);

		if (*speed >= desired_speed)
		{
			*speed = desired_speed;
			result = TRUE;
		}
	}
	else
	{
		result = TRUE;
	}

	return result;
}

void physics_variable_update(
	real *position,
	real *speed, 
	struct physics_variable const *definition,
	boolean cyclical_position, 
	real magnitude)
{
	physics_variable_speed_update(speed, &definition->speed, magnitude);
	physics_variable_position_update(position, &definition->position, cyclical_position, *speed);

	return;
}

boolean physics_variable_position_update_seek(
	real *position,
	struct physics_variable_position const *definition, 
	boolean cyclical_position, 
	real desired_position, 
	real speed)
{
	boolean result = TRUE;
	real direction = physics_variable_position_get_seek_direction(*position, definition, cyclical_position, desired_position);

	if (direction != 0.0f)
	{
		physics_variable_position_update(position, definition, cyclical_position, direction*speed);

		if (physics_variable_position_get_seek_direction(*position, definition, cyclical_position, desired_position) == direction)
		{
			result = FALSE;
		}
	}

	if (result)
	{
		*position = desired_position;
	}

	return result;
}

boolean physics_variable_update_seek(
	real *position,
	real *speed, 
	struct physics_variable const *definition, 
	boolean cyclical_position, 
	real desired_position,
	real magnitude)
{
	boolean result = TRUE;
	struct physics_variable_position const *position_definition = &definition->position;
	real direction = physics_variable_position_get_seek_direction(*position, position_definition, cyclical_position, desired_position);

	if (direction != 0.0f)
	{
		physics_variable_speed_update(speed, &definition->speed, direction*magnitude);
		physics_variable_position_update(position, position_definition, cyclical_position, *speed);

		if (physics_variable_position_get_seek_direction(*position, position_definition, cyclical_position, desired_position) == direction)
		{
			result = FALSE;
		}
	}

	if (result)
	{
		*position = desired_position;
		*speed = 0.0f;
	}

	return result;
}

/* ---------- private code */

static real physics_variable_position_get_seek_direction(
	real position, 
	struct physics_variable_position const *definition, 
	boolean cyclical_position, 
	real desired_position)
{
	real direction = desired_position - position;

	if (direction != 0.0f)
	{
		if (cyclical_position && fabs(direction) > (definition->maximum_position - definition->minimum_position)*0.5f)
		{
			direction = -direction;
		}

		return (direction > 0.0f) ? 1.0f : -1.0f;
	}

	return direction;
}
