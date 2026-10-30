/*
UNIT_DIALOGUE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "units.h"
#include "dialogue_definitions.h"
#include "network_game_globals.h"
#include "console.h"
#include "actors.h"
#include "sound_manager.h"
#include "ai_debug.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "damage_effect_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void unit_dialogue_setup(long unit_index);
static long unit_find_dialogue_variant(struct unit_definition *unit_definition, short variant_number);
static void unit_lose_speech(long unit_index, short play_type, struct unit_speech_item *speech_item);

/* ---------- globals */

short const global_speech_override_priorities[NUMBER_OF_UNIT_SPEECH_PRIORITIES] =
{
	_unit_speech_none,
	_unit_speech_none,
	_unit_speech_idle,
	_unit_speech_idle,
	_unit_speech_pain,
	_unit_speech_pain,
	_unit_speech_shout,
	_unit_speech_shout,
	_unit_speech_involuntary,
	_unit_speech_involuntary,
	_unit_speech_death,
};

real const global_speech_queue_times[NUMBER_OF_UNIT_SPEECH_PRIORITIES] =
{
	0.f,
	0.f,
	0.f,
	1.5f,
	3.f,
	4.f,
	REAL_MAX,
	3.f,
	3.f,
	3.f,
	3.f,
};

char const *global_speech_priority_names[NUMBER_OF_UNIT_SPEECH_PRIORITIES] =
{
	"none",
	"idle",
	"pain",
	"talk",
	"communicate",
	"shout",
	"script",
	"involuntary",
	"exclaim",
	"scream",
	"death",
};

/* ---------- public code */

short unit_test_speech(
	long unit_index,
	short priority,
	boolean allow_recursive_lookup,
	boolean allow_queue,
	long *unit_last_speech_time,
	short *vocalization_type_reference,
	long *sound_definition_index_reference)
{
	struct unit_datum *unit = unit_get(unit_index);
	long current_time = game_time_get();
	short play_type = _unit_play_speech_none;
	short vocalization_type;
	long sound_definition_index;

	match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 128, vocalization_type_reference);
	match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 129, sound_definition_index_reference);
	match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 130, (priority >= 0) && (priority < NUMBER_OF_UNIT_SPEECH_PRIORITIES));

	vocalization_type = *vocalization_type_reference;
	sound_definition_index = *sound_definition_index_reference;

	if (sound_definition_index == NONE && unit->unit.dialogue_index != NONE && vocalization_type != NONE)
	{
		struct dialogue_definition *dialogue = dialogue_definition_get(unit->unit.dialogue_index);

		while (vocalization_type != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 144, (vocalization_type >= 0) && (vocalization_type < NUMBER_OF_VOCALIZATION_TYPES));
			sound_definition_index = dialogue->vocalizations[vocalization_type].index;
			if (!allow_recursive_lookup || sound_definition_index != NONE || AI_BEHAVIOR(force_vocalizations))
			{
				break;
			}

			vocalization_type = dialogue_vocalization_lookup[vocalization_type];
		}
	}

	if ((!TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || priority == _unit_speech_death) &&
		(AI_BEHAVIOR(force_vocalizations) || sound_definition_index != NONE))
	{
		short current_priority = unit->unit.speech.current.priority;

		if (current_priority == _unit_speech_none)
		{
			play_type = _unit_play_speech_immediate;
		}
		else
		{
			short blocking_priority = MAX(current_priority, unit->unit.speech.queued.priority);

			switch (priority)
			{
			case _unit_speech_pain:
			case _unit_speech_involuntary:
			case _unit_speech_death:
				if (unit->unit.speech.played && !unit->unit.speech.sound_timer && priority > blocking_priority)
				{
					current_priority = _unit_speech_none;
					blocking_priority = unit->unit.speech.queued.priority;
				}
				break;
			}

			if (global_speech_override_priorities[priority] >= blocking_priority)
			{
				play_type = _unit_play_speech_immediate_dequeue;
			}
			else if (priority >= _unit_speech_involuntary && global_speech_override_priorities[priority] >= current_priority)
			{
				play_type = _unit_play_speech_immediate;
			}
			else if (allow_queue)
			{
				real queue_time = global_speech_queue_times[priority];
				boolean queue;

				if (queue_time == 0.f)
				{
					queue = FALSE;
				}
				else if (queue_time == REAL_MAX)
				{
					queue = TRUE;
				}
				else
				{
					queue = unit->unit.speech.post_delay_timer + unit->unit.speech.sound_timer < (short)(queue_time * TICKS_PER_SECOND);
				}

				if (queue)
				{
					if (priority <= blocking_priority)
					{
						if (priority <= unit->unit.speech.queued.priority)
						{
							queue = FALSE;
						}
						else
						{
							switch (current_priority)
							{
							case _unit_speech_pain:
							case _unit_speech_involuntary:
								queue = TRUE;
								break;
							}
							if (priority == _unit_speech_scripted)
							{
								queue = TRUE;
							}
						}
					}
					else
					{
						queue = TRUE;
					}
				}

				if (queue)
				{
					play_type = _unit_play_speech_queue;
				}
			}
		}
	}

	*vocalization_type_reference = vocalization_type;
	*sound_definition_index_reference = sound_definition_index;
	if (unit_last_speech_time)
	{
		*unit_last_speech_time = unit->unit.speech.last_speech_finished_time;
	}

	return play_type;
}

void unit_speak(
	long unit_index,
	short play_type,
	struct unit_speech_item *speech_item)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 302, speech_item);

	if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || speech_item->priority == _unit_speech_death)
	{
		if (play_type >= _unit_play_speech_immediate)
		{
			if (unit->unit.speech.current.priority > _unit_speech_none && !unit->unit.speech.played)
			{
				unit_lose_speech(unit_index, _unit_play_speech_immediate, &unit->unit.speech.current);
			}

			unit->unit.speech.current = *speech_item;
			if (play_type == _unit_play_speech_immediate_dequeue && unit->unit.speech.queued.priority > _unit_speech_none)
			{
				if (speech_item != &unit->unit.speech.queued)
				{
					unit_lose_speech(unit_index, _unit_play_speech_queue, &unit->unit.speech.queued);
				}
				unit->unit.speech.queued.priority = _unit_speech_none;
			}

			unit->unit.speech.played = FALSE;
			unit->unit.speech.notified_ai = FALSE;
			unit->unit.speech.finished = FALSE;
			unit->unit.speech.impulse_sound_index = NONE;
			unit->unit.speech.pre_delay_timer = unit->unit.speech.current.delay_time;
			unit->unit.speech.post_delay_timer = unit->unit.speech.current.pause_time;
			unit->unit.speech.ai_delay_timer = unit->unit.speech.current.ai_notification_delay;

			if (unit->unit.speech.current.sound_definition_index != NONE)
			{
				unit->unit.speech.sound_timer = (TICKS_PER_SECOND * sound_definition_get(unit->unit.speech.current.sound_definition_index)->runtime_maximum_play_time) / 1000;
			}
			else
			{
				match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 360, AI_BEHAVIOR(force_vocalizations));
				unit->unit.speech.sound_timer = 45;
			}
		}
		else if (play_type == _unit_play_speech_queue)
		{
			match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 366, unit->unit.speech.current.priority > _unit_speech_none);
			unit->unit.speech.queued = *speech_item;
		}
	}

	return;
}

void unit_notify_impulse_sound(
	long unit_index,
	long sound_definition_index,
	long impulse_sound_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short vocalization_type = NONE;
	long lookup_definition_index = sound_definition_index;
	struct unit_speech_item speech;
	short play_type;

	play_type = unit_test_speech(unit_index, _unit_speech_scripted, FALSE, FALSE, NULL, &vocalization_type, &lookup_definition_index);
	play_type = MAX(play_type, _unit_play_speech_immediate);

	memset(&speech, 0, sizeof(speech));
	speech.priority = _unit_speech_scripted;
	speech.vocalization_type = NONE;
	speech.sound_definition_index = sound_definition_index;
	speech.pause_time = 24;
	ai_communication_packet_new(&speech.ai);
	unit_speak(unit_index, play_type, &speech);

	match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 406, unit->unit.speech.current.sound_definition_index == sound_definition_index);
	unit->unit.speech.impulse_sound_index = impulse_sound_index;
	unit->unit.speech.played = TRUE;
	unit->unit.speech.pre_delay_timer = 0;
	ai_communication_started(unit_index, _unit_speech_scripted, NONE, &unit->unit.speech.current.ai);

	return;
}

boolean unit_is_speaking(
	long unit_index)
{
	return unit_get(unit_index)->unit.speech.current.priority > _unit_speech_none;
}

boolean unit_make_damage_sound(
	long unit_index,
	struct damage_data *damage_data,
	boolean died,
	boolean died_instantly,
	real body_damage,
	real shield_damage)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (unit->unit.dialogue_index != NONE)
	{
		short vocalization_type = NONE;
		short damage_category = _damage_category_none;
		boolean damaged = unit->object.recent_body_damage > 0.f;
		boolean major_damage = unit->object.recent_body_damage >= 0.6f;
		boolean involuntary = FALSE;
		short effect_type = NONE;
		short effect_volume = _ai_sound_volume_silent;

		if (damage_data && damage_data->definition_index != NONE)
		{
			struct damage_effect_definition *damage_effect = damage_effect_definition_get(damage_data->definition_index);
			damage_category = damage_effect->damage.category;
		}

		if (died)
		{
			long actor_index = (unit->unit.swarm_actor_index != NONE) ? unit->unit.swarm_actor_index : unit->unit.actor_index;
			boolean in_combat = FALSE;
			boolean flying = FALSE;

			if (damage_data->definition_index != NONE)
			{
				struct damage_effect_definition *damage_effect = damage_effect_definition_get(damage_data->definition_index);
				flying = damage_effect->damage.instantaneous_acceleration >= 2.f;
			}

			if (actor_index != NONE)
			{
				struct actor_datum *actor = actor_get(actor_index);
				in_combat = actor->state.combat_status >= _actor_combat_status_definite;
			}
			else
			{
				if (unit->object.recent_body_damage > body_damage + 0.2f)
				{
					in_combat = TRUE;
				}
			}

			if (damage_category == _damage_category_falling)
			{
				vocalization_type = _vocalization_death_falling;
			}
			else if (damage_category == _damage_category_flame)
			{
				vocalization_type = _vocalization_death_agonizing;
			}
			else if (flying)
			{
				vocalization_type = _vocalization_death_flying;
			}
			else if (!in_combat)
			{
				vocalization_type = _vocalization_death_quiet;
			}
			else if (died_instantly)
			{
				vocalization_type = _vocalization_death_instant;
			}
			else
			{
				vocalization_type = _vocalization_death_violent;
			}

			involuntary = TRUE;
			if (vocalization_type != _vocalization_death_quiet)
			{
				effect_type = _ai_unit_effect_death_scream;
				effect_volume = (vocalization_type == _vocalization_death_instant) ? _ai_sound_volume_quiet : _ai_sound_volume_medium;
			}
		}
		else if (!unit->unit.speech.damage_major_timer)
		{
			if (damage_category == _damage_category_falling)
			{
				involuntary = TRUE;
				vocalization_type = _vocalization_pain_falling;
			}
			else if (major_damage)
			{
				involuntary = TRUE;
				vocalization_type = _vocalization_pain_body_major;
			}
			else if (!unit->unit.speech.damage_minor_timer && unit->unit.speech.damage_minor_sounds < 3)
			{
				if (unit->unit.speech.current.priority == _unit_speech_none || real_random() < 0.4f)
				{
					vocalization_type = damaged ? _vocalization_pain_body : _vocalization_pain_shield;
				}
			}
		}

		if (vocalization_type != NONE)
		{
			long sound_definition_index = NONE;
			short priority = died ? _unit_speech_death : (involuntary ? _unit_speech_involuntary : _unit_speech_pain);
			short play_type = unit_test_speech(unit_index, priority, TRUE, FALSE, NULL, &vocalization_type, &sound_definition_index);

			if (ai_debug.disable_wounded_sounds)
			{
				play_type = _unit_play_speech_none;
			}

			if (play_type > _unit_play_speech_none)
			{
				struct unit_speech_item speech;

				memset(&speech, 0, sizeof(speech));
				speech.priority = priority;
				speech.vocalization_type = vocalization_type;
				speech.sound_definition_index = sound_definition_index;
				speech.pause_time = 7;
				ai_communication_packet_new(&speech.ai);
				unit_speak(unit_index, play_type, &speech);
				result = TRUE;
			}
		}

		if (result)
		{
			if (major_damage)
			{
				unit->unit.speech.damage_major_timer = 60;
			}
			else
			{
				unit->unit.speech.damage_minor_timer = 30;
				unit->unit.speech.damage_minor_decay_timer = 22;
				unit->unit.speech.damage_minor_sounds++;
			}
		}

		if (effect_type != NONE)
		{
			ai_handle_unit_effect(unit_index, effect_type, effect_volume);
		}
	}

	return result;
}

boolean unit_scream(
	long unit_index,
	short scream_type)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;
	short vocalization_type;

	match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 599, (scream_type >= 0) && (scream_type < NUMBER_OF_UNIT_SCREAM_TYPES));

	switch (scream_type)
	{
	case _unit_scream_falling:
		vocalization_type = _vocalization_scream_fear;
		break;
	case _unit_scream_grenade_attached_to_us:
		if (real_random() < 0.5f)
		{
			vocalization_type = _vocalization_hurt_enemy_grenade;
			break;
		}
	case _unit_scream_burning_to_death:
		vocalization_type = _vocalization_scream_pain;
		break;
	case _unit_scream_destroyed_limb:
		vocalization_type = _vocalization_maimed_limb;
		break;
	case _unit_scream_destroyed_head:
		vocalization_type = _vocalization_maimed_head;
		break;
	case _unit_scream_resurrection:
		vocalization_type = _vocalization_resurrect;
		break;
	default:
		match_unreachable("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 635);
	}

	if (unit->unit.dialogue_index != NONE)
	{
		struct dialogue_definition *dialogue = dialogue_definition_get(unit->unit.dialogue_index);
		long sound_definition_index = dialogue->vocalizations[vocalization_type].index;

		if (sound_definition_index != NONE)
		{
			short play_type = unit_test_speech(unit_index, _unit_speech_scream, TRUE, FALSE, NULL, &vocalization_type, &sound_definition_index);

			if (play_type > _unit_play_speech_none)
			{
				struct unit_speech_item speech;

				memset(&speech, 0, sizeof(speech));
				speech.priority = _unit_speech_scream;
				speech.vocalization_type = vocalization_type;
				speech.sound_definition_index = sound_definition_index;
				speech.pause_time = 7;
				ai_communication_packet_new(&speech.ai);
				unit_speak(unit_index, play_type, &speech);
				return TRUE;
			}
		}
	}

	return result;
}

void unit_dialogue_update(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (TEST_FLAG(unit->unit.flags, _unit_must_set_up_dialogue_bit))
	{
		unit_dialogue_setup(unit_index);
		SET_FLAG(unit->unit.flags, _unit_must_set_up_dialogue_bit, FALSE);
	}

	if (unit->unit.speech.damage_minor_decay_timer > 0 && !--unit->unit.speech.damage_minor_decay_timer && unit->unit.speech.damage_minor_sounds > 0)
	{
		unit->unit.speech.damage_minor_sounds--;
		unit->unit.speech.damage_minor_decay_timer = 22;
	}

	if (unit->unit.speech.damage_minor_timer > 0)
	{
		unit->unit.speech.damage_minor_timer--;
	}

	// DECOMP NOTE: This is authentic code, but looks like a bug and they probably meant to do 'major' here instead of minor 2x
	if (unit->unit.speech.damage_minor_timer > 0)
	{
		unit->unit.speech.damage_minor_timer--;
	}

	if (unit->unit.speech.current.priority > _unit_speech_none)
	{
		if (unit->unit.speech.pre_delay_timer > 0)
		{
			unit->unit.speech.pre_delay_timer--;
		}
		else
		{
			if (!unit->unit.speech.played)
			{
				struct object_marker head_marker;
				short node_index;
				real_point3d node_position;
				real_vector3d node_forward;

				if (object_get_marker_by_name(unit_index, "head", &head_marker, 1))
				{
					node_index = head_marker.node_index;
					node_position = head_marker.node_matrix.position;
					node_forward = head_marker.node_matrix.forward;
				}
				else
				{
					node_index = 0;
					node_position = *global_origin3d;
					node_forward = *global_forward3d;
				}

				if (unit->unit.speech.current.sound_definition_index != NONE)
				{
					unit->unit.speech.impulse_sound_index = object_impulse_sound_new(unit_index, unit->unit.speech.current.sound_definition_index, node_index, &node_position, &node_forward, 1.f);
				}

				ai_communication_started(unit_index, unit->unit.speech.current.priority, unit->unit.speech.current.vocalization_type, &unit->unit.speech.current.ai);
				unit->unit.speech.played = TRUE;
			}

			if (unit->unit.speech.ai_delay_timer > 0)
			{
				unit->unit.speech.ai_delay_timer--;
			}

			if (unit->unit.speech.sound_timer > 0)
			{
				match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 757, unit->unit.speech.current.priority > _unit_speech_none);
				if (!--unit->unit.speech.sound_timer)
				{
					unit->unit.speech.impulse_sound_index = NONE;
				}
			}
			else
			{
				if (!unit->unit.speech.finished)
				{
					ai_communication_finished(unit_index, unit->unit.speech.current.priority, unit->unit.speech.current.vocalization_type, FALSE, NONE, &unit->unit.speech.current.ai);
					unit->unit.speech.finished = TRUE;
				}

				if (unit->unit.speech.post_delay_timer > 0)
				{
					unit->unit.speech.post_delay_timer--;
				}

				if (!unit->unit.speech.post_delay_timer)
				{
					unit->unit.speech.ai_delay_timer = 0;
				}
			}
		}
	}

	if (!unit->unit.speech.ai_delay_timer && !unit->unit.speech.notified_ai)
	{
		ai_communication_notify(unit_index, unit->unit.speech.current.priority, unit->unit.speech.current.vocalization_type, &unit->unit.speech.current.ai);
		unit->unit.speech.notified_ai = TRUE;
	}

	if (unit->unit.speech.current.priority > _unit_speech_none && !unit->unit.speech.sound_timer && !unit->unit.speech.post_delay_timer)
	{
		unit->unit.speech.current.priority = _unit_speech_none;
	}

	if (unit->unit.speech.current.priority == _unit_speech_none && unit->unit.speech.queued.priority > _unit_speech_none)
	{
		unit_speak(unit_index, _unit_play_speech_immediate_dequeue, &unit->unit.speech.queued);
	}

	return;
}

void unit_dialogue_determine_variant(
	long unit_index)
{
	static long sequential_counter = 0;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *definition = unit_definition_get(unit->definition_index);

	if (!unit->object.variant_number)
	{
		short available_variants[MAXIMUM_DIALOGUE_VARIANTS_PER_UNIT];
		short variant_count = 0;
		short variant_index;

		for (variant_index = 0; variant_index < definition->unit.dialogue_variants.count; variant_index++)
		{
			struct dialogue_variant_definition *variant = TAG_BLOCK_GET_ELEMENT(&definition->unit.dialogue_variants, variant_index, struct dialogue_variant_definition);

			if (variant->variant_number < 100)
			{
				if (variant_count >= NUMBEROF(available_variants))
				{
					error(_error_silent, "unit_dialogue_determine_variant overflowed variant array");
					break;
				}

				available_variants[variant_count++] = variant->variant_number;
			}
		}

		if (variant_count > 0)
		{
			unit->object.variant_number = available_variants[sequential_counter % variant_count];
			sequential_counter++;
		}
	}

	return;
}

char const *unit_get_speech_priority_name(
	short priority)
{
	char const *name = "<error>";

	if (priority >= 0 && priority < NUMBER_OF_UNIT_SPEECH_PRIORITIES)
	{
		name = global_speech_priority_names[priority];
	}

	return name;
}

short unit_get_speech_priority_by_name(
	char const *name)
{
	short priority = _unit_speech_none;
	short priority_index;

	for (priority_index = 0; priority_index < NUMBER_OF_UNIT_SPEECH_PRIORITIES; priority_index++)
	{
		if (!strcmp(global_speech_priority_names[priority_index], name))
		{
			priority = priority_index;
			break;
		}
	}

	return priority;
}

char *unit_describe_speech(
	long unit_index,
	boolean verbose,
	short buffer_length,
	char *buffer)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.speech.current.priority == _unit_speech_none)
	{
		_snprintf(buffer, buffer_length, "<none>");
	}
	else
	{
		char const *sound_name = "<none>";

		if (unit->unit.speech.current.sound_definition_index != NONE)
		{
			sound_name = tag_get_name(unit->unit.speech.current.sound_definition_index);
		}

		if (verbose)
		{
			short count = 2;
			char const *separator = sound_name;

			while (count > 0 && separator)
			{
				separator = strchr(separator, '\\');
				if (separator)
				{
					sound_name = ++separator;
				}
			}
		}
		else
		{
			char const *separator = strrchr(sound_name, '\\');

			if (separator)
			{
				sound_name = separator + 1;
			}
		}

		if (unit->unit.speech.current.vocalization_type == NONE)
		{
			_snprintf(buffer, buffer_length, "%s", sound_name);
		}
		else if (verbose)
		{
			_snprintf(buffer, buffer_length, "%s %s", dialogue_get_vocalization_name(unit->unit.speech.current.vocalization_type, FALSE), sound_name);
		}
		else
		{
			_snprintf(buffer, buffer_length, "%s", dialogue_get_vocalization_name(unit->unit.speech.current.vocalization_type, FALSE));
		}
	}

	return buffer;
}

static void unit_dialogue_setup(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *definition = unit_definition_get(unit->definition_index);
	long dialogue_index = NONE;

	if (unit->object.variant_number > 0)
	{
		dialogue_index = unit_find_dialogue_variant(definition, unit->object.variant_number);
	}

	if (dialogue_index == NONE)
	{
		dialogue_index = unit_find_dialogue_variant(definition, 0);
	}

	if (dialogue_index == NONE)
	{
		dialogue_index = unit_find_dialogue_variant(definition, NONE);
	}

	unit->unit.dialogue_index = dialogue_index;

	return;
}

static long unit_find_dialogue_variant(
	struct unit_definition *unit_definition,
	short variant_number)
{
	long dialogue_index = NONE;
	short available_indices[MAXIMUM_DIALOGUE_VARIANTS_PER_UNIT];
	short available_count = 0;
	short variant_index;

	for (variant_index = 0; variant_index < unit_definition->unit.dialogue_variants.count; variant_index++)
	{
		struct dialogue_variant_definition *variant = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.dialogue_variants, variant_index, struct dialogue_variant_definition);

		if (variant_number == NONE || variant->variant_number == variant_number)
		{
			available_indices[available_count++] = variant_index;
		}
	}

	if (available_count > 0)
	{
		short chosen_index;

		if (available_count == 1)
		{
			chosen_index = available_indices[0];
		}
		else
		{
			chosen_index = available_indices[random_range(0, available_count)];
		}

		dialogue_index = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.dialogue_variants, chosen_index, struct dialogue_variant_definition)->dialogue_variant.index;
		match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 1032, dialogue_definition_get(dialogue_index));
	}

	return dialogue_index;
}

static void unit_lose_speech(
	long unit_index,
	short play_type,
	struct unit_speech_item *speech_item)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\unit_dialogue.c", 1045, speech_item);

	if (ai_debug.print_lost_speech)
	{
		struct unit_definition *definition = unit_definition_get(unit->definition_index);
		char const *speech_name;

		if (speech_item->vocalization_type != NONE)
		{
			speech_name = dialogue_get_vocalization_name(speech_item->vocalization_type, FALSE);
		}
		else if (speech_item->sound_definition_index != NONE)
		{
			char const *separator;

			speech_name = tag_get_name(speech_item->sound_definition_index);
			separator = strrchr(speech_name, '\\');
			if (separator)
			{
				speech_name = separator + 1;
			}
		}
		else
		{
			speech_name = "<unknown>";
		}

		console_printf(FALSE, "%s: lost %s speech %s", definition->object.model.name, (play_type == _unit_play_speech_immediate) ? "waiting" : "queued", speech_name);
	}

	return;
}

/* ---------- private code */
