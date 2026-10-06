/*
MODEL_ANIMATIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "model_animation_definitions.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"

/* ---------- structures */

struct animation_compressed_data_header /* fake name */
{
	long rotation_keyframe_frame_indices_offset;
	long rotation_default_data_offset;
	long rotation_keyframe_data_offset;
	long translation_keyframe_info_offset;
	long translation_keyframe_frame_indices_offset;
	long translation_default_data_offset;
	long translation_keyframe_data_offset;
	long scale_keyframe_info_offset;
	long scale_keyframe_frame_indices_offset;
	long scale_default_data_offset;
	long scale_keyframe_data_offset;
	unsigned long rotation_keyframe_info[1];
};

/* ---------- prototypes */

static boolean animation_is_compressed(struct animation const *animation);

static short animation_keyframe_search(short const *keyframe_frame_indices, short keyframe_count, short target_frame_index);
static void animation_get_keyframe_rotation(struct animation const *animation, real real_frame_index, short adjusted_node_index, short node_index, real_quaternion *result);
static void animation_get_keyframe_translation(struct animation const *animation, real real_frame_index, short adjusted_node_index, short node_index, real_point3d *result);
static void animation_get_keyframe_scale(struct animation const *animation, real real_frame_index, short adjusted_node_index, short node_index, real *result);

/* ---------- globals */

boolean hs_model_animation_compression_enabled = TRUE;
long hs_model_animation_data_compressed_size = 0;
long hs_model_animation_data_uncompressed_size = 0;
long hs_model_animation_data_compression_savings_in_bytes = 0;
long hs_model_animation_data_compression_savings_in_bytes_at_import = 0;
long hs_model_animation_data_compression_savings_in_percent = 0;
char hs_model_animation_bullshit[16] = {0};

/* ---------- public code */

static boolean animation_is_compressed(
	struct animation const *animation)
{
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 38, animation);

	return TEST_FLAG(animation->flags, _animation_compressed_bit) && (hs_model_animation_compression_enabled || !animation->compressed_data_offset);
}

short build_damage_animation_index(
	short damage_type,
	short damage_direction,
	short damage_part)
{
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 55, damage_type>=0 && damage_type<NUMBER_OF_ANIMATION_DAMAGE_TYPES);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 56, damage_direction>=0 && damage_direction<NUMBER_OF_ANIMATION_DAMAGE_DIRECTIONS);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 57, damage_part>=0 && damage_part<NUMBER_OF_DAMAGE_PARTS);

	return (damage_type*NUMBER_OF_ANIMATION_DAMAGE_DIRECTIONS + damage_direction)*NUMBER_OF_DAMAGE_PARTS + damage_part;
}

void animation_get_x_offsets(
	struct animation *animation,
	real *key_frame_offset_reference,
	real *last_frame_offset_reference)
{
	short frame_index;
	real key_frame_offset = 0.f;
	real offset = 0.f;
	byte *frame_info = animation->frame_info.address;

	for (frame_index = 0; frame_index<animation->frame_count; frame_index++)
	{
		switch (animation->frame_info_type)
		{
		case _animation_frame_info_xy_translation:
			offset += ((struct animation_frame_info_xy_translation *)frame_info)->offset.i;
			frame_info += sizeof(struct animation_frame_info_xy_translation);
			break;
		case _animation_frame_info_xy_translation_yaw_rotation:
			offset += ((struct animation_frame_info_xy_translation_yaw_rotation *)frame_info)->offset.i;
			frame_info += sizeof(struct animation_frame_info_xy_translation_yaw_rotation);
			break;
		case _animation_frame_info_xyz_translation_yaw_rotation:
			offset += ((struct animation_frame_info_xyz_translation_yaw_rotation *)frame_info)->offset.i;
			frame_info += sizeof(struct animation_frame_info_xyz_translation_yaw_rotation);
			break;
		}

		if (frame_index==animation->private_key_frame_index)
		{
			key_frame_offset = offset;
		}
	}

	if (last_frame_offset_reference)
	{
		*last_frame_offset_reference = offset;
	}

	if (key_frame_offset_reference)
	{
		*key_frame_offset_reference = key_frame_offset;
	}

	return;
}

void animation_frame_get_xy_translation(
	struct animation *animation,
	short frame_index,
	real_vector2d *offset)
{
	if (animation->frame_info_type==_animation_frame_info_xy_translation)
	{
		struct animation_frame_info_xy_translation *frame_info = animation_get_frame_info(animation, frame_index, sizeof(struct animation_frame_info_xy_translation));

		*offset = frame_info->offset;
	}
	else
	{
		set_real_vector2d(offset, 0.f, 0.f);
	}

	return;
}

void animation_set_frame_size(
	struct animation *animation)
{
	short node_index;
	short frame_size = 0;

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 123, animation);

	for (node_index = 0; node_index<animation->node_count; node_index++)
	{
		if (BIT_VECTOR_TEST_FLAG(animation->nodes_with_rotation_flags, node_index))
		{
			frame_size += sizeof(struct compressed_quaternion_8byte);
		}

		if (BIT_VECTOR_TEST_FLAG(animation->nodes_with_translation_flags, node_index))
		{
			frame_size += sizeof(real_point3d);
		}

		if (BIT_VECTOR_TEST_FLAG(animation->nodes_with_scale_flags, node_index))
		{
			frame_size += sizeof(real);
		}
	}

	animation->frame_size = frame_size;

	return;
}

short animation_update_internal(
	enum animation_update_kind render_or_affects_game_state,
	long animation_graph_index,
	struct animation_state *state,
	long *triggered_sound_index)
{
	struct animation *animation;
	struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);
	short result = _animation_running;

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 147, state);

	animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, state->index, struct animation);

	if (triggered_sound_index)
	{
		if (animation->sound_index!=NONE && animation_sound_frame_index(animation)==state->frame_index)
		{
			*triggered_sound_index = TAG_BLOCK_GET_ELEMENT(&animation_graph->sound_references, animation->sound_index, struct animation_graph_sound_reference)->sound.index;
		}
		else
		{
			*triggered_sound_index = NONE;
		}
	}

	if (++state->frame_index>=animation->frame_count)
	{
		if (animation_loop_frame_index(animation)>0)
		{
			state->frame_index = MIN(animation_loop_frame_index(animation), animation->frame_count-1);
			result = _animation_looped;
		}
		else
		{
			state->index = animation_choose_random_permutation_internal(render_or_affects_game_state, animation_graph_index, animation->runtime_parent_animation_index);
			state->frame_index = 0;
			result = _animation_restarted;
		}
	}
	else if (state->frame_index+1==animation->frame_count && animation_loop_frame_index(animation)==0)
	{
		result = _animation_will_restart_on_next_frame;
	}
	else if (state->frame_index==animation_key_frame_index(animation) || state->frame_index==animation_second_key_frame_index(animation))
	{
		result = _animation_key_frame;
	}

	return result;
}

void animation_get_root_matrix(
	struct model const *model,
	struct animation const *animation,
	short frame_index,
	real_matrix4x3 *matrix)
{
	real_orientation frame_orientations[MAXIMUM_NODES_PER_MODEL];

	animation_get_node_orientations(model, animation, frame_index, frame_orientations);
	matrix4x3_from_point_and_quaternion(matrix, &frame_orientations[0].translation, &frame_orientations[0].rotation);

	return;
}

void animation_get_root_velocity(
	struct model const *model,
	struct animation const *animation,
	short frame_index,
	real_vector3d *velocity)
{
	real_orientation frame1_orientations[MAXIMUM_NODES_PER_MODEL];
	real_orientation frame0_orientations[MAXIMUM_NODES_PER_MODEL];

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 221, animation->frame_count>1);

	if (!frame_index)
	{
		frame_index++;
	}

	animation_get_node_orientations(model, animation, frame_index, frame1_orientations);
	animation_get_node_orientations(model, animation, frame_index-1, frame0_orientations);
	vector_from_points3d(&frame0_orientations[0].translation, &frame1_orientations[0].translation, velocity);

	return;
}

void animation_get_node_orientations(
	struct model const *model,
	struct animation const *animation,
	short frame_index,
	real_orientation *node_orientations)
{
	boolean valid = TRUE;

	if (animation->type!=_animation_base)
	{
		valid = FALSE;
	}
	else if (model &&
		((animation->node_list_checksum && animation->node_list_checksum!=model->node_list_checksum && model->node_list_checksum) ||
		model->nodes.count!=animation->node_count))
	{
		valid = FALSE;
	}

	if (valid)
	{
		unsigned long nodes_with_translation_flags;
		unsigned long nodes_with_rotation_flags;
		unsigned long nodes_with_scale_flags;
		short node_index;
		boolean compressed = animation_is_compressed(animation);
		byte *data = animation_get_frame_data(animation, frame_index);
		byte *default_data = animation_get_default_data(animation);
		short rotation_index = 0;
		short translation_index = 0;
		short scale_index = 0;

		for (node_index = 0; node_index<animation->node_count; node_index++)
		{
			real_orientation *orientation = &node_orientations[node_index];

			if (!(node_index&MASK(LONG_BITS_BITS)))
			{
				short flags_index = node_index>>LONG_BITS_BITS;

				nodes_with_translation_flags = animation->nodes_with_translation_flags[flags_index];
				nodes_with_rotation_flags = animation->nodes_with_rotation_flags[flags_index];
				nodes_with_scale_flags = animation->nodes_with_scale_flags[flags_index];
			}

			if (TEST_FLAG(nodes_with_rotation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_rotation(animation, (real)frame_index, rotation_index++, node_index, &orientation->rotation);
				}
				else
				{
					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)data, &orientation->rotation);
					data += sizeof(struct compressed_quaternion_8byte);
				}
			}
			else
			{
				if (compressed)
				{
					quaternion_decompress_6byte_renormalized((struct compressed_quaternion_6byte *)(data + ((struct animation_compressed_data_header *)data)->rotation_default_data_offset) + node_index, &orientation->rotation);
				}
				else
				{
					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)default_data, &orientation->rotation);
					default_data += sizeof(struct compressed_quaternion_8byte);
				}
			}
			nodes_with_rotation_flags >>= 1;

			if (TEST_FLAG(nodes_with_translation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_translation(animation, (real)frame_index, translation_index++, node_index, &orientation->translation);
				}
				else
				{
					orientation->translation = *(real_point3d *)data;
					data += sizeof(real_point3d);
				}
			}
			else
			{
				if (compressed)
				{
					orientation->translation = ((real_point3d *)(data + ((struct animation_compressed_data_header *)data)->translation_default_data_offset))[node_index];
				}
				else
				{
					orientation->translation = *(real_point3d *)default_data;
					default_data += sizeof(real_point3d);
				}
			}
			nodes_with_translation_flags >>= 1;

			if (TEST_FLAG(nodes_with_scale_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_scale(animation, (real)frame_index, scale_index++, node_index, &orientation->scale);
				}
				else
				{
					orientation->scale = *(real *)data;
					data += sizeof(real);
				}
			}
			else
			{
				if (compressed)
				{
					orientation->scale = 1.f;
				}
				else
				{
					orientation->scale = *(real *)default_data;
					default_data += sizeof(real);
				}
			}
			nodes_with_scale_flags >>= 1;
		}

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 321, compressed || (byte *)data-(byte *)animation_get_frame_data(animation, frame_index)==animation->frame_size);
		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 322, compressed || (byte *)default_data-(byte *)animation_get_default_data(animation)==animation->default_data.size);
	}
	else
	{
		model_get_node_orientations(model, node_orientations);
	}

	return;
}

void replacement_animation_apply(
	struct animation const *animation,
	short frame_index,
	real_orientation *node_orientations)
{
	if (animation->type==_animation_replacement && frame_index>=0 && frame_index<animation->frame_count)
	{
		unsigned long nodes_with_translation_flags;
		unsigned long nodes_with_rotation_flags;
		unsigned long nodes_with_scale_flags;
		short node_index;
		boolean compressed = animation_is_compressed(animation);
		byte *data = animation_get_frame_data(animation, frame_index);
		short rotation_index = 0;
		short translation_index = 0;
		short scale_index = 0;

		for (node_index = 0; node_index<animation->node_count; node_index++)
		{
			real_orientation *orientation = &node_orientations[node_index];

			if (!(node_index&MASK(LONG_BITS_BITS)))
			{
				short flags_index = node_index>>LONG_BITS_BITS;

				nodes_with_translation_flags = animation->nodes_with_translation_flags[flags_index];
				nodes_with_rotation_flags = animation->nodes_with_rotation_flags[flags_index];
				nodes_with_scale_flags = animation->nodes_with_scale_flags[flags_index];
			}

			if (TEST_FLAG(nodes_with_rotation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_rotation(animation, (real)frame_index, rotation_index++, node_index, &orientation->rotation);
				}
				else
				{
					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)data, &orientation->rotation);
					data += sizeof(struct compressed_quaternion_8byte);
				}
			}
			nodes_with_rotation_flags >>= 1;

			if (TEST_FLAG(nodes_with_translation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_translation(animation, (real)frame_index, translation_index++, node_index, &orientation->translation);
				}
				else
				{
					orientation->translation = *(real_point3d *)data;
					data += sizeof(real_point3d);
				}
			}
			nodes_with_translation_flags >>= 1;

			if (TEST_FLAG(nodes_with_scale_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_scale(animation, (real)frame_index, scale_index++, node_index, &orientation->scale);
				}
				else
				{
					orientation->scale = *(real *)data;
					data += sizeof(real);
				}
			}
			nodes_with_scale_flags >>= 1;
		}

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 391, compressed || ((byte *)data-(byte *)animation_get_frame_data(animation, frame_index)==animation->frame_size));
	}

	return;
}

void overlay_animation_apply(
	struct animation const *animation,
	short frame_index,
	real_orientation *node_orientations)
{
	if (animation->type==_animation_overlay && frame_index>=0 && frame_index<animation->frame_count)
	{
		unsigned long nodes_with_translation_flags;
		unsigned long nodes_with_rotation_flags;
		unsigned long nodes_with_scale_flags;
		short node_index;
		boolean compressed = animation_is_compressed(animation);
		byte *data = animation_get_frame_data(animation, frame_index);
		short rotation_index = 0;
		short translation_index = 0;
		short scale_index = 0;

		for (node_index = 0; node_index<animation->node_count; node_index++)
		{
			real_quaternion rotation;
			real_point3d translation;
			real scale;
			real_orientation *orientation = &node_orientations[node_index];

			if (!(node_index&MASK(LONG_BITS_BITS)))
			{
				short flags_index = node_index>>LONG_BITS_BITS;

				nodes_with_translation_flags = animation->nodes_with_translation_flags[flags_index];
				nodes_with_rotation_flags = animation->nodes_with_rotation_flags[flags_index];
				nodes_with_scale_flags = animation->nodes_with_scale_flags[flags_index];
			}

			if (TEST_FLAG(nodes_with_rotation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_rotation(animation, (real)frame_index, rotation_index++, node_index, &rotation);
				}
				else
				{
					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)data, &rotation);
					data += sizeof(struct compressed_quaternion_8byte);
				}
				quaternions_multiply(&rotation, &orientation->rotation, &orientation->rotation);
			}
			nodes_with_rotation_flags >>= 1;

			if (TEST_FLAG(nodes_with_translation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_translation(animation, (real)frame_index, translation_index++, node_index, &translation);
				}
				else
				{
					translation = *(real_point3d *)data;
					data += sizeof(translation);
				}
				orientation->translation.x += translation.x;
				orientation->translation.y += translation.y;
				orientation->translation.z += translation.z;
			}
			nodes_with_translation_flags >>= 1;

			if (TEST_FLAG(nodes_with_scale_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_scale(animation, (real)frame_index, scale_index++, node_index, &scale);
				}
				else
				{
					scale = *(real *)data;
					data += sizeof(scale);
				}
				orientation->scale *= scale;
			}
			nodes_with_scale_flags >>= 1;
		}

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 470, compressed || ((byte *)data-(byte *)animation_get_frame_data(animation, frame_index)==animation->frame_size));
	}

	return;
}

void overlay_animation_apply_scaled(
	struct animation const *animation,
	short frame_index,
	real animation_scale,
	real_orientation *node_orientations)
{
	real inverse_animation_scale = 1.f - animation_scale;

	if (animation->type==_animation_overlay && frame_index>=0 && frame_index<animation->frame_count)
	{
		unsigned long nodes_with_translation_flags;
		unsigned long nodes_with_rotation_flags;
		unsigned long nodes_with_scale_flags;
		short node_index;
		boolean compressed = animation_is_compressed(animation);
		byte *data = animation_get_frame_data(animation, frame_index);
		short rotation_index = 0;
		short translation_index = 0;
		short scale_index = 0;

		for (node_index = 0; node_index<animation->node_count; node_index++)
		{
			real_quaternion rotation;
			real_point3d translation;
			real scale;
			real_orientation *orientation = &node_orientations[node_index];

			if (!(node_index&MASK(LONG_BITS_BITS)))
			{
				short flags_index = node_index>>LONG_BITS_BITS;

				nodes_with_translation_flags = animation->nodes_with_translation_flags[flags_index];
				nodes_with_rotation_flags = animation->nodes_with_rotation_flags[flags_index];
				nodes_with_scale_flags = animation->nodes_with_scale_flags[flags_index];
			}

			if (TEST_FLAG(nodes_with_rotation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_rotation(animation, (real)frame_index, rotation_index++, node_index, &rotation);
				}
				else
				{
					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)data, &rotation);
					data += sizeof(struct compressed_quaternion_8byte);
				}
				quaternions_interpolate(global_identity_quaternion, &rotation, animation_scale, &rotation);
				quaternions_multiply(&rotation, &orientation->rotation, &orientation->rotation);
			}
			nodes_with_rotation_flags >>= 1;

			if (TEST_FLAG(nodes_with_translation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_translation(animation, (real)frame_index, translation_index++, node_index, &translation);
				}
				else
				{
					translation = *(real_point3d *)data;
					data += sizeof(translation);
				}
				orientation->translation.x += translation.x*animation_scale;
				orientation->translation.y += translation.y*animation_scale;
				orientation->translation.z += translation.z*animation_scale;
			}
			nodes_with_translation_flags >>= 1;

			if (TEST_FLAG(nodes_with_scale_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_scale(animation, (real)frame_index, scale_index++, node_index, &scale);
				}
				else
				{
					scale = *(real *)data;
					data += sizeof(scale);
				}
				orientation->scale *= scale*animation_scale + inverse_animation_scale;
			}
			nodes_with_scale_flags >>= 1;
		}

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 554, compressed || ((byte *)data-(byte *)animation_get_frame_data(animation, frame_index)==animation->frame_size));
	}

	return;
}

void overlay_animation_apply_continuous(
	struct animation const *animation,
	real real_frame_index,
	real_orientation *node_orientations)
{
	real fraction = (real)fmod(real_frame_index, 1.);
	short frame_index = (short)fast_ftol((real)floor(fabs(real_frame_index)));

	if (real_frame_index<0.f || real_frame_index>(real)animation->frame_count)
	{
		error(_error_silent, "### ERROR animation frame index out of bounds A(%f,%x) -- tell Bernie!!", real_frame_index, *(long *)&real_frame_index);
	}

	if (frame_index>=animation->frame_count)
	{
		frame_index = animation->frame_count-1;
		real_frame_index = (real)frame_index;
		fraction = 1.f;
	}

	if (animation->type==_animation_overlay)
	{
		unsigned long nodes_with_translation_flags;
		unsigned long nodes_with_rotation_flags;
		unsigned long nodes_with_scale_flags;
		short node_index;
		boolean compressed = animation_is_compressed(animation);
		short next_frame_index = frame_index==animation->frame_count-1 ? 0 : frame_index+1;
		byte *data = animation_get_frame_data(animation, frame_index);
		byte *next_data = animation_get_frame_data(animation, next_frame_index);
		short rotation_index = 0;
		short translation_index = 0;
		short scale_index = 0;

		for (node_index = 0; node_index<animation->node_count; node_index++)
		{
			real_quaternion rotation;
			real_point3d translation;
			real scale;
			real_orientation *orientation = &node_orientations[node_index];

			if (!(node_index&MASK(LONG_BITS_BITS)))
			{
				short flags_index = node_index>>LONG_BITS_BITS;

				nodes_with_translation_flags = animation->nodes_with_translation_flags[flags_index];
				nodes_with_rotation_flags = animation->nodes_with_rotation_flags[flags_index];
				nodes_with_scale_flags = animation->nodes_with_scale_flags[flags_index];
			}

			if (TEST_FLAG(nodes_with_rotation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_rotation(animation, (real)frame_index, rotation_index++, node_index, &rotation);
				}
				else
				{
					real_quaternion this_rotation;
					real_quaternion next_rotation;

					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)data, &this_rotation);
					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)next_data, &next_rotation);
					data += sizeof(struct compressed_quaternion_8byte);
					next_data += sizeof(struct compressed_quaternion_8byte);
					quaternions_interpolate_and_normalize(&this_rotation, &next_rotation, fraction, &rotation);
				}
				quaternions_multiply(&rotation, &orientation->rotation, &orientation->rotation);
			}
			nodes_with_rotation_flags >>= 1;

			if (TEST_FLAG(nodes_with_translation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_translation(animation, real_frame_index, translation_index++, node_index, &translation);
				}
				else
				{
					real_point3d *this_translation = (real_point3d *)data;
					real_point3d *next_translation = (real_point3d *)next_data;

					data += sizeof(real_point3d);
					next_data += sizeof(real_point3d);
					points_interpolate(this_translation, next_translation, fraction, &translation);
				}
				orientation->translation.x += translation.x;
				orientation->translation.y += translation.y;
				orientation->translation.z += translation.z;
			}
			nodes_with_translation_flags >>= 1;

			if (TEST_FLAG(nodes_with_scale_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_scale(animation, real_frame_index, scale_index++, node_index, &scale);
				}
				else
				{
					real this_scale;
					real next_scale;

					this_scale = *(real *)data;
					data += sizeof(real);
					next_scale = *(real *)next_data;
					next_data += sizeof(real);
					scalars_interpolate(this_scale, next_scale, fraction, &scale);
				}
				orientation->scale *= scale;
			}
			nodes_with_scale_flags >>= 1;
		}

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 693, compressed || ((byte *)data-(byte *)animation_get_frame_data(animation, frame_index)==animation->frame_size));
		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 694, compressed || ((byte *)next_data-(byte *)animation_get_frame_data(animation, next_frame_index)==animation->frame_size));
	}

	return;
}

void overlay_animation_apply_continuous_scaled(
	struct animation const *animation,
	real real_frame_index,
	real animation_scale,
	real_orientation *node_orientations)
{
	real inverse_animation_scale = 1.f - animation_scale;
	real fraction = (real)fmod(real_frame_index, 1.);
	short frame_index = (short)fast_ftol((real)floor(real_frame_index));

	if (real_frame_index<0.f || real_frame_index>(real)animation->frame_count)
	{
		error(_error_silent, "### ERROR animation frame index out of bounds B(%f,%x) -- tell Bernie!!", real_frame_index, *(long *)&real_frame_index);
	}

	if (frame_index>=animation->frame_count)
	{
		frame_index = animation->frame_count-1;
		real_frame_index = (real)frame_index;
		fraction = 1.f;
	}

	if (animation->type==_animation_overlay)
	{
		unsigned long nodes_with_translation_flags;
		unsigned long nodes_with_rotation_flags;
		unsigned long nodes_with_scale_flags;
		short node_index;
		boolean compressed = animation_is_compressed(animation);
		short next_frame_index = frame_index==animation->frame_count-1 ? 0 : frame_index+1;
		byte *data = animation_get_frame_data(animation, frame_index);
		byte *next_data = animation_get_frame_data(animation, next_frame_index);
		short rotation_index = 0;
		short translation_index = 0;
		short scale_index = 0;

		for (node_index = 0; node_index<animation->node_count; node_index++)
		{
			real_quaternion rotation;
			real_point3d translation;
			real scale;
			real_orientation *orientation = &node_orientations[node_index];

			if (!(node_index&MASK(LONG_BITS_BITS)))
			{
				short flags_index = node_index>>LONG_BITS_BITS;

				nodes_with_translation_flags = animation->nodes_with_translation_flags[flags_index];
				nodes_with_rotation_flags = animation->nodes_with_rotation_flags[flags_index];
				nodes_with_scale_flags = animation->nodes_with_scale_flags[flags_index];
			}

			if (TEST_FLAG(nodes_with_rotation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_rotation(animation, (real)frame_index, rotation_index++, node_index, &rotation);
				}
				else
				{
					real_quaternion this_rotation;
					real_quaternion next_rotation;

					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)data, &this_rotation);
					quaternion_decompress_8byte((struct compressed_quaternion_8byte *)next_data, &next_rotation);
					data += sizeof(struct compressed_quaternion_8byte);
					next_data += sizeof(struct compressed_quaternion_8byte);
					quaternions_interpolate_and_normalize(&this_rotation, &next_rotation, fraction, &rotation);
				}
				quaternions_interpolate_and_normalize(global_identity_quaternion, &rotation, animation_scale, &rotation);
				quaternions_multiply(&rotation, &orientation->rotation, &orientation->rotation);
			}
			nodes_with_rotation_flags >>= 1;

			if (TEST_FLAG(nodes_with_translation_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_translation(animation, real_frame_index, translation_index++, node_index, &translation);
				}
				else
				{
					real_point3d *this_translation = (real_point3d *)data;
					real_point3d *next_translation = (real_point3d *)next_data;

					data += sizeof(real_point3d);
					next_data += sizeof(real_point3d);
					points_interpolate(this_translation, next_translation, fraction, &translation);
				}
				orientation->translation.x += translation.x*animation_scale;
				orientation->translation.y += translation.y*animation_scale;
				orientation->translation.z += translation.z*animation_scale;
			}
			nodes_with_translation_flags >>= 1;

			if (TEST_FLAG(nodes_with_scale_flags, 0))
			{
				if (compressed)
				{
					animation_get_keyframe_scale(animation, real_frame_index, scale_index++, node_index, &scale);
				}
				else
				{
					real this_scale;
					real next_scale;

					this_scale = *(real *)data;
					data += sizeof(real);
					next_scale = *(real *)next_data;
					next_data += sizeof(real);
					scalars_interpolate(this_scale, next_scale, fraction, &scale);
				}
				orientation->scale *= scale*animation_scale + inverse_animation_scale;
			}
			nodes_with_scale_flags >>= 1;
		}

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 820, compressed || ((byte *)data-(byte *)animation_get_frame_data(animation, frame_index)==animation->frame_size));
		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 821, compressed || ((byte *)next_data-(byte *)animation_get_frame_data(animation, next_frame_index)==animation->frame_size));
	}

	return;
}

void aiming_screen_apply(
	struct animation const *animation,
	struct animation_aiming_screen_bounds const *aiming_screen,
	real direction,
	real elevation,
	real_orientation *node_orientations)
{
	short yaw_frame_count = aiming_screen->negative_yaw_frame_count + aiming_screen->positive_yaw_frame_count + 1;
	short pitch_frame_count = aiming_screen->negative_pitch_frame_count + aiming_screen->positive_pitch_frame_count + 1;

	if (animation->type==_animation_overlay && animation->frame_count>=yaw_frame_count*pitch_frame_count)
	{
		real pitch_delta;
		real e;
		short e0;
		real e_fraction;
		boolean compressed = animation_is_compressed(animation);
		real yaw_delta = direction<0.f ? aiming_screen->negative_yaw_delta : aiming_screen->positive_yaw_delta;
		real d = yaw_delta==0.f ? 0.f : direction/yaw_delta;
		short d0 = (short)d;
		real d_fraction = (real)fmod(d, 1.);

		if (d_fraction<0.f)
		{
			d_fraction += 1.f;
			d0--;
		}

		if (d0>=aiming_screen->positive_yaw_frame_count)
		{
			d0 = aiming_screen->positive_yaw_frame_count-1;
			d_fraction = 1.f;
		}

		if (d0<-aiming_screen->negative_yaw_frame_count)
		{
			d0 = -aiming_screen->negative_yaw_frame_count;
			d_fraction = 0.f;
		}
		d0 += aiming_screen->negative_yaw_frame_count;
		match_vassert("c:\\halo\\SOURCE\\models\\model_animations.c", 869, d_fraction>=0.f && d_fraction<=1.f, csprintf(temporary, "d0==%f direction(%f) yaw_delta(%f,%f)", d_fraction, direction, aiming_screen->negative_yaw_delta, aiming_screen->positive_yaw_delta));

		pitch_delta = elevation<0.f ? aiming_screen->negative_pitch_delta : aiming_screen->positive_pitch_delta;
		e = pitch_delta==0.f ? 0.f : elevation/pitch_delta;
		e0 = (short)e;
		e_fraction = (real)fmod(e, 1.);
		if (e_fraction<0.f)
		{
			e_fraction += 1.f;
			e0--;
		}

		if (e0>=aiming_screen->positive_pitch_frame_count)
		{
			e0 = aiming_screen->positive_pitch_frame_count-1;
			e_fraction = 1.f;
		}

		if (e0<-aiming_screen->negative_pitch_frame_count)
		{
			e0 = -aiming_screen->negative_pitch_frame_count;
			e_fraction = 0.f;
		}
		e0 += aiming_screen->negative_pitch_frame_count;

		if (e0>=0 && e0<pitch_frame_count && d0>=0 && d0<yaw_frame_count)
		{
			unsigned long nodes_with_translation_flags;
			unsigned long nodes_with_rotation_flags;
			short node_index;
			short d1 = d0+1==yaw_frame_count ? d0 : d0+1;
			short e1 = e0+1==pitch_frame_count ? e0 : e0+1;
			short d0_e0_frame_index = e0*yaw_frame_count + d0;
			short d1_e0_frame_index = e0*yaw_frame_count + d1;
			short d0_e1_frame_index = e1*yaw_frame_count + d0;
			short d1_e1_frame_index = e1*yaw_frame_count + d1;
			byte *d0_e0_data = animation_get_frame_data(animation, d0_e0_frame_index);
			byte *d1_e0_data = animation_get_frame_data(animation, d1_e0_frame_index);
			byte *d0_e1_data = animation_get_frame_data(animation, d0_e1_frame_index);
			byte *d1_e1_data = animation_get_frame_data(animation, d1_e1_frame_index);
			short rotation_index = 0;
			short translation_index = 0;

			for (node_index = 0; node_index<animation->node_count; node_index++)
			{
				real_orientation *orientation = &node_orientations[node_index];

				if (!(node_index&MASK(LONG_BITS_BITS)))
				{
					short flags_index = node_index>>LONG_BITS_BITS;

					nodes_with_translation_flags = animation->nodes_with_translation_flags[flags_index];
					nodes_with_rotation_flags = animation->nodes_with_rotation_flags[flags_index];
				}

				if (TEST_FLAG(nodes_with_rotation_flags, 0))
				{
					real_quaternion d0_e0_quaternion;
					real_quaternion d1_e0_quaternion;
					real_quaternion d0_e1_quaternion;
					real_quaternion d1_e1_quaternion;
					real_quaternion e0_quaternion;
					real_quaternion e1_quaternion;
					real_quaternion interpolated_quaternion;

					if (compressed)
					{
						animation_get_keyframe_rotation(animation, (real)d0_e0_frame_index, rotation_index, node_index, &d0_e0_quaternion);
						animation_get_keyframe_rotation(animation, (real)d1_e0_frame_index, rotation_index, node_index, &d1_e0_quaternion);
						animation_get_keyframe_rotation(animation, (real)d0_e1_frame_index, rotation_index, node_index, &d0_e1_quaternion);
						animation_get_keyframe_rotation(animation, (real)d1_e1_frame_index, rotation_index, node_index, &d1_e1_quaternion);
						rotation_index++;
					}
					else
					{
						quaternion_decompress_8byte((struct compressed_quaternion_8byte *)d0_e0_data, &d0_e0_quaternion);
						d0_e0_data += sizeof(struct compressed_quaternion_8byte);
						quaternion_decompress_8byte((struct compressed_quaternion_8byte *)d1_e0_data, &d1_e0_quaternion);
						d1_e0_data += sizeof(struct compressed_quaternion_8byte);
						quaternion_decompress_8byte((struct compressed_quaternion_8byte *)d0_e1_data, &d0_e1_quaternion);
						d0_e1_data += sizeof(struct compressed_quaternion_8byte);
						quaternion_decompress_8byte((struct compressed_quaternion_8byte *)d1_e1_data, &d1_e1_quaternion);
						d1_e1_data += sizeof(struct compressed_quaternion_8byte);
					}

					quaternions_interpolate_and_normalize(&d0_e0_quaternion, &d1_e0_quaternion, d_fraction, &e0_quaternion);
					quaternions_interpolate_and_normalize(&d0_e1_quaternion, &d1_e1_quaternion, d_fraction, &e1_quaternion);
					quaternions_interpolate_and_normalize(&e0_quaternion, &e1_quaternion, e_fraction, &interpolated_quaternion);
					quaternions_multiply(&interpolated_quaternion, &orientation->rotation, &orientation->rotation);
				}
				nodes_with_rotation_flags >>= 1;

				if (TEST_FLAG(nodes_with_translation_flags, 0))
				{
					real_point3d d0_e0_translation;
					real_point3d d1_e0_translation;
					real_point3d d0_e1_translation;
					real_point3d d1_e1_translation;
					real inverse_d_fraction = 1.f - d_fraction;
					real inverse_e_fraction = 1.f - e_fraction;

					if (compressed)
					{
						animation_get_keyframe_translation(animation, (real)d0_e0_frame_index, translation_index, node_index, &d0_e0_translation);
						animation_get_keyframe_translation(animation, (real)d1_e0_frame_index, translation_index, node_index, &d1_e0_translation);
						animation_get_keyframe_translation(animation, (real)d0_e1_frame_index, translation_index, node_index, &d0_e1_translation);
						animation_get_keyframe_translation(animation, (real)d1_e1_frame_index, translation_index, node_index, &d1_e1_translation);
						translation_index++;
					}
					else
					{
						d0_e0_translation = *(real_point3d *)d0_e0_data;
						d0_e0_data += sizeof(d0_e0_translation);
						d1_e0_translation = *(real_point3d *)d1_e0_data;
						d1_e0_data += sizeof(d1_e0_translation);
						d0_e1_translation = *(real_point3d *)d0_e1_data;
						d0_e1_data += sizeof(d0_e1_translation);
						d1_e1_translation = *(real_point3d *)d1_e1_data;
						d1_e1_data += sizeof(d1_e1_translation);
					}

					orientation->translation.x += (d0_e0_translation.x*inverse_d_fraction + d1_e0_translation.x*d_fraction)*inverse_e_fraction + (d0_e1_translation.x*inverse_d_fraction + d1_e1_translation.x*d_fraction)*e_fraction;
					orientation->translation.y += (d0_e0_translation.y*inverse_d_fraction + d1_e0_translation.y*d_fraction)*inverse_e_fraction + (d0_e1_translation.y*inverse_d_fraction + d1_e1_translation.y*d_fraction)*e_fraction;
					orientation->translation.z += (d0_e0_translation.z*inverse_d_fraction + d1_e0_translation.z*d_fraction)*inverse_e_fraction + (d0_e1_translation.z*inverse_d_fraction + d1_e1_translation.z*d_fraction)*e_fraction;
				}
				nodes_with_translation_flags >>= 1;
			}
		}
	}

	return;
}

short animation_choose_random_permutation_internal(
	enum animation_update_kind render_or_affects_game_state,
	long animation_graph_index,
	short animation_index)
{
	struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);
	real random = render_or_affects_game_state==animation_update_kind_affects_game_state ? real_random() : real_local_random();

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1008, (animation_update_kind_affects_game_state==render_or_affects_game_state) || (animation_update_kind_render_only==render_or_affects_game_state));

	while (animation_index!=NONE)
	{
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

		if (random<=animation->runtime_normalized_weight)
		{
			break;
		}

		animation_index = animation->next_animation_index;
	}

	return animation_index;
}

void quaternion_decompress_8byte(
	struct compressed_quaternion_8byte const *compressed,
	real_quaternion *decompressed)
{
	decompressed->v.i = compressed->i*(1.f/SHORT_MAX);
	decompressed->v.j = compressed->j*(1.f/SHORT_MAX);
	decompressed->v.k = compressed->k*(1.f/SHORT_MAX);
	decompressed->w = compressed->w*(1.f/SHORT_MAX);

	return;
}

void quaternion_decompress_6byte(
	struct compressed_quaternion_6byte const *compressed,
	real_quaternion *decompressed)
{
	word iiij = compressed->iiij;
	word jjkk = compressed->jjkk;
	word kwww = compressed->kwww;
	short i = (iiij&0xfff0) | (iiij>>12);
	short j = (iiij<<12) | ((jjkk&0xff00)>>4) | (iiij&0xf);
	short k = (jjkk<<8) | ((kwww&0xf000)>>8) | ((jjkk&0xf0)>>4);
	short w = (kwww<<4) | ((kwww&0xf00)>>8);

	decompressed->v.i = i*(1.f/SHORT_MAX);
	decompressed->v.j = j*(1.f/SHORT_MAX);
	decompressed->v.k = k*(1.f/SHORT_MAX);
	decompressed->w = w*(1.f/SHORT_MAX);

	return;
}

void quaternion_decompress_6byte_renormalized(
	struct compressed_quaternion_6byte const *compressed,
	real_quaternion *decompressed)
{
	quaternion_decompress_6byte(compressed, decompressed);
	quaternion_normalize(decompressed);

	return;
}

void quaternion_compress_8byte(
	real_quaternion const *decompressed,
	struct compressed_quaternion_8byte *compressed)
{
	compressed->i = (short)(decompressed->v.i*SHORT_MAX);
	compressed->j = (short)(decompressed->v.j*SHORT_MAX);
	compressed->k = (short)(decompressed->v.k*SHORT_MAX);
	compressed->w = (short)(decompressed->w*SHORT_MAX);

	return;
}

void quaternion_compress_6byte(
	real_quaternion const *decompressed,
	struct compressed_quaternion_6byte *compressed)
{
	word i = (word)(decompressed->v.i*SHORT_MAX);
	word j = (word)(decompressed->v.j*SHORT_MAX);
	word k = (word)(decompressed->v.k*SHORT_MAX);
	word w = (word)(decompressed->w*SHORT_MAX);

	compressed->iiij = (i&0xfff0) | (j>>12);
	compressed->jjkk = ((j&0xff0)<<4) | (k>>8);
	compressed->kwww = ((k&0xf0)<<8) | (w>>4);

	return;
}

void inverse_kinematics_adjust_matrices(
	real_matrix4x3 *desired_hand_matrix,
	real_matrix4x3 *shoulder_matrix,
	real_matrix4x3 *elbow_matrix,
	real_matrix4x3 *hand_matrix)
{
	real_vector3d upper_arm_vector;
	real_vector3d hand_direction;
	real_vector3d normal;
	real_vector3d perpendicular;
	real_point3d elbow_position;
	real maximum_distance;
	real along;
	real remaining;
	real offset;
	real upper_arm_length = distance3d(&shoulder_matrix->position, &elbow_matrix->position);
	real forearm_length = distance3d(&elbow_matrix->position, &hand_matrix->position);
	real shoulder_to_hand_distance = distance3d(&desired_hand_matrix->position, &shoulder_matrix->position);
	real maximum_distance_fraction = 0.98f;

	upper_arm_vector.i = elbow_matrix->position.x - shoulder_matrix->position.x;
	upper_arm_vector.j = elbow_matrix->position.y - shoulder_matrix->position.y;
	upper_arm_vector.k = elbow_matrix->position.z - shoulder_matrix->position.z;
	{
		real inverse_distance = 1.f/shoulder_to_hand_distance;

		hand_direction.i = (desired_hand_matrix->position.x - shoulder_matrix->position.x)*inverse_distance;
		hand_direction.j = (desired_hand_matrix->position.y - shoulder_matrix->position.y)*inverse_distance;
		hand_direction.k = (desired_hand_matrix->position.z - shoulder_matrix->position.z)*inverse_distance;
	}
	cross_product3d(&hand_direction, &upper_arm_vector, &normal);
	normalize3d(&normal);
	cross_product3d(&normal, &hand_direction, &perpendicular);

	maximum_distance = (upper_arm_length + forearm_length)*maximum_distance_fraction;
	if (maximum_distance<shoulder_to_hand_distance)
	{
		desired_hand_matrix->position.x = hand_direction.i*maximum_distance + shoulder_matrix->position.x;
		desired_hand_matrix->position.y = hand_direction.j*maximum_distance + shoulder_matrix->position.y;
		desired_hand_matrix->position.z = hand_direction.k*maximum_distance + shoulder_matrix->position.z;
		shoulder_to_hand_distance = maximum_distance;
	}

	along = (upper_arm_length*upper_arm_length + shoulder_to_hand_distance*shoulder_to_hand_distance - forearm_length*forearm_length)/(2.f*shoulder_to_hand_distance);
	remaining = shoulder_to_hand_distance - along;
	offset = square_root(upper_arm_length*upper_arm_length - along*along);

	{
		real_vector3d *forward = &shoulder_matrix->forward;
		real_vector3d *left = &shoulder_matrix->left;
		real_vector3d *up = &shoulder_matrix->up;

		forward->i = along*hand_direction.i + offset*perpendicular.i;
		forward->j = along*hand_direction.j + offset*perpendicular.j;
		forward->k = along*hand_direction.k + offset*perpendicular.k;
		normalize3d(forward);
		cross_product3d(forward, left, up);
		normalize3d(up);
		cross_product3d(up, forward, left);

		elbow_position.x = upper_arm_length*forward->i + shoulder_matrix->position.x;
		elbow_position.y = upper_arm_length*forward->j + shoulder_matrix->position.y;
		elbow_position.z = upper_arm_length*forward->k + shoulder_matrix->position.z;
	}

	{
		real_vector3d *forward = &elbow_matrix->forward;
		real_vector3d *left = &elbow_matrix->left;
		real_vector3d *up = &elbow_matrix->up;
		real_point3d *position = &elbow_matrix->position;

		forward->i = remaining*hand_direction.i - offset*perpendicular.i;
		forward->j = remaining*hand_direction.j - offset*perpendicular.j;
		forward->k = remaining*hand_direction.k - offset*perpendicular.k;
		normalize3d(forward);
		cross_product3d(forward, left, up);
		normalize3d(up);
		cross_product3d(up, forward, left);
		*position = elbow_position;
	}

	*hand_matrix = *desired_hand_matrix;

	return;
}

void animation_graph_node_matrices_from_orientations(
	long animation_graph_index,
	real_matrix4x3 *node_matrices,
	real_orientation const *node_orientations,
	real_point3d const *origin,
	real_vector3d const *forward,
	real_vector3d const *up)
{
	real_matrix4x3 root_matrix;
	short node_stack[MAXIMUM_NODES_PER_MODEL];
	real_matrix4x3 orientation_matrix;
	struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);

	matrix4x3_from_point_and_vectors(&root_matrix, origin, forward, up);
	if (animation_graph->nodes.count>0)
	{
		short read_index = 0;
		short write_index = 1;

		node_stack[0] = 0;

		while (read_index!=write_index)
		{
			short node_index = node_stack[read_index++];
			struct animation_graph_node *node = TAG_BLOCK_GET_ELEMENT(&animation_graph->nodes, node_index, struct animation_graph_node);
			real_matrix4x3 *parent_matrix = node_index==0 ? &root_matrix : &node_matrices[node->parent_node_index];

			matrix4x3_from_orientation(&orientation_matrix, &node_orientations[node_index]);
			matrix4x3_multiply(parent_matrix, &orientation_matrix, &node_matrices[node_index]);

			if (node->next_sibling_node_index!=NONE)
			{
				match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1250, write_index<MAXIMUM_NODES_PER_MODEL);
				node_stack[write_index++] = node->next_sibling_node_index;
			}

			if (node->first_child_node_index!=NONE)
			{
				match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1256, write_index<MAXIMUM_NODES_PER_MODEL);
				node_stack[write_index++] = node->first_child_node_index;
			}
		}
	}

	return;
}

void interpolate_node_orientations(
	short node_count,
	real_orientation *original_node_orientations,
	real_orientation *target_node_orientations,
	short frame_index,
	short frame_count)
{
	short node_index;
	real fraction = (real)(frame_index+1)/(real)frame_count;
	real inverse_fraction = 1.f - fraction;

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1277, frame_count>0);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1278, frame_index<frame_count);

	for (node_index = 0; node_index<node_count; node_index++)
	{
		real_orientation *target = &target_node_orientations[node_index];
		real_orientation *original = &original_node_orientations[node_index];

		target->scale = inverse_fraction*original->scale + fraction*target->scale;
		quaternions_interpolate_and_normalize(&original->rotation, &target->rotation, fraction, &target->rotation);
		target->translation.x = inverse_fraction*original->translation.x + fraction*target->translation.x;
		target->translation.y = inverse_fraction*original->translation.y + fraction*target->translation.y;
		target->translation.z = inverse_fraction*original->translation.z + fraction*target->translation.z;
	}

	return;
}

short animation_graph_get_animation_by_name(
	long animation_graph_index,
	char const *animation_name)
{
	short animation_index;
	struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);

	for (animation_index = 0; animation_index<animation_graph->animations.count; animation_index++)
	{
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

		if (!_stricmp(animation_name, animation->name))
		{
			return animation_index;
		}
	}

	return NONE;
}

/* ---------- private code */

static short animation_keyframe_search(
	short const *keyframe_frame_indices,
	short keyframe_count,
	short target_frame_index)
{
	short keyframe_index;
	short low = 0;
	short high = keyframe_count-1;
	short infinite_loop_killer = 0;

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1334, keyframe_count>1);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1335, keyframe_frame_indices);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1336, keyframe_frame_indices[0]>0);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1337, target_frame_index>=0 && target_frame_index<keyframe_frame_indices[keyframe_count-1]);

	while (TRUE)
	{
		keyframe_index = (low+high)>>1;
		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1343, keyframe_index>=0 && keyframe_index<keyframe_count);

		if (keyframe_index+1<keyframe_count && keyframe_frame_indices[keyframe_index+1]<=target_frame_index)
		{
			low = keyframe_index;
		}
		else if (keyframe_frame_indices[keyframe_index]>target_frame_index)
		{
			high = keyframe_index;
		}
		else
		{
			break;
		}

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1356, ++infinite_loop_killer<200);
	}

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1360, keyframe_index>=0 && keyframe_index<keyframe_count-1);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1361, target_frame_index>=keyframe_frame_indices[keyframe_index] && target_frame_index<keyframe_frame_indices[keyframe_index+1]);

	return keyframe_index;
}

static void animation_get_keyframe_rotation(
	struct animation const *animation,
	real real_frame_index,
	short adjusted_node_index,
	short node_index,
	real_quaternion *result)
{
	struct animation_compressed_data_header *header = tag_data_get_pointer(&animation->data, animation->compressed_data_offset, 0);
	struct compressed_quaternion_6byte *default_data = (struct compressed_quaternion_6byte *)((byte *)header + header->rotation_default_data_offset);
	unsigned long keyframe_info = header->rotation_keyframe_info[adjusted_node_index];
	short first_keyframe_index = (short)(keyframe_info>>12);
	short keyframe_count = (short)(keyframe_info&0xfff);

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1428, real_frame_index>=0.0f);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1430, real_frame_index<(real)animation->frame_count);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1432, keyframe_count>=0);

	if (keyframe_count==0)
	{
		quaternion_decompress_6byte_renormalized(&default_data[node_index], result);
	}
	else
	{
		short this_keyframe_frame_index;
		short next_keyframe_frame_index;
		struct compressed_quaternion_6byte *this_data;
		struct compressed_quaternion_6byte *next_data;
		struct compressed_quaternion_6byte *keyframe_data = (struct compressed_quaternion_6byte *)((byte *)header + header->rotation_keyframe_data_offset) + first_keyframe_index;
		word *keyframe_frame_indices = (word *)((byte *)header + header->rotation_keyframe_frame_indices_offset) + first_keyframe_index;
		short frame_index = (short)fast_ftol((real)floor(real_frame_index));

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1451, frame_index>=0 && frame_index<=keyframe_frame_indices[keyframe_count-1]);
		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1452, keyframe_frame_indices[keyframe_count-1]==animation->frame_count-1);

		if (frame_index<keyframe_frame_indices[0])
		{
			this_keyframe_frame_index = 0;
			this_data = default_data + node_index;
			next_keyframe_frame_index = keyframe_frame_indices[0];
			next_data = keyframe_data + 0;
		}
		else if (frame_index==keyframe_frame_indices[keyframe_count-1])
		{
			this_keyframe_frame_index = keyframe_frame_indices[keyframe_count-1];
			this_data = keyframe_data + keyframe_count-1;
			next_keyframe_frame_index = keyframe_frame_indices[keyframe_count-1]+1;
			next_data = default_data + node_index;
		}
		else
		{
			short keyframe_index = animation_keyframe_search(keyframe_frame_indices, keyframe_count, frame_index);

			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1472, keyframe_index>=0 && keyframe_index<keyframe_count-1);
			this_keyframe_frame_index = keyframe_frame_indices[keyframe_index];
			this_data = keyframe_data + keyframe_index;
			next_keyframe_frame_index = keyframe_frame_indices[keyframe_index+1];
			next_data = keyframe_data + keyframe_index+1;
		}

		if (real_frame_index==(real)this_keyframe_frame_index)
		{
			quaternion_decompress_6byte_renormalized(this_data, result);
		}
		else
		{
			real_quaternion this_rotation;
			real_quaternion next_rotation;
			real fraction = (real_frame_index-this_keyframe_frame_index)/(next_keyframe_frame_index-this_keyframe_frame_index);

			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1491, real_frame_index>=(real)this_keyframe_frame_index);
			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1492, real_frame_index< (real)next_keyframe_frame_index);
			quaternion_decompress_6byte(this_data, &this_rotation);
			quaternion_decompress_6byte(next_data, &next_rotation);
			quaternions_interpolate_and_normalize(&this_rotation, &next_rotation, fraction, result);
		}
	}

	return;
}

static void animation_get_keyframe_translation(
	struct animation const *animation,
	real real_frame_index,
	short adjusted_node_index,
	short node_index,
	real_point3d *result)
{
	struct animation_compressed_data_header *header = tag_data_get_pointer(&animation->data, animation->compressed_data_offset, 0);
	real_point3d *default_data = (real_point3d *)((byte *)header + header->translation_default_data_offset);
	unsigned long keyframe_info = *(unsigned long *)((byte *)header + header->translation_keyframe_info_offset + adjusted_node_index*sizeof(unsigned long));
	short first_keyframe_index = (short)(keyframe_info>>12);
	short keyframe_count = (short)(keyframe_info&0xfff);

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1522, real_frame_index>=0.0f);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1524, real_frame_index<(real)animation->frame_count);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1526, keyframe_count>=0);

	if (keyframe_count==0)
	{
		*result = default_data[node_index];
	}
	else
	{
		short this_keyframe_frame_index;
		short next_keyframe_frame_index;
		real_point3d *this_data;
		real_point3d *next_data;
		real_point3d *keyframe_data = (real_point3d *)((byte *)header + header->translation_keyframe_data_offset) + first_keyframe_index;
		word *keyframe_frame_indices = (word *)((byte *)header + header->translation_keyframe_frame_indices_offset) + first_keyframe_index;
		short frame_index = (short)fast_ftol((real)floor(real_frame_index));

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1545, frame_index>=0 && frame_index<=keyframe_frame_indices[keyframe_count-1]);
		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1546, keyframe_frame_indices[keyframe_count-1]==animation->frame_count-1);

		if (frame_index<keyframe_frame_indices[0])
		{
			this_keyframe_frame_index = 0;
			this_data = default_data + node_index;
			next_keyframe_frame_index = keyframe_frame_indices[0];
			next_data = keyframe_data + 0;
		}
		else if (frame_index==keyframe_frame_indices[keyframe_count-1])
		{
			this_keyframe_frame_index = keyframe_frame_indices[keyframe_count-1];
			this_data = keyframe_data + keyframe_count-1;
			next_keyframe_frame_index = keyframe_frame_indices[keyframe_count-1]+1;
			next_data = default_data + node_index;
		}
		else
		{
			short keyframe_index = animation_keyframe_search(keyframe_frame_indices, keyframe_count, frame_index);

			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1566, keyframe_index>=0 && keyframe_index<keyframe_count-1);
			this_keyframe_frame_index = keyframe_frame_indices[keyframe_index];
			this_data = keyframe_data + keyframe_index;
			next_keyframe_frame_index = keyframe_frame_indices[keyframe_index+1];
			next_data = keyframe_data + keyframe_index+1;
		}

		if (real_frame_index==(real)this_keyframe_frame_index)
		{
			*result = *this_data;
		}
		else
		{
			real fraction = (real_frame_index-this_keyframe_frame_index)/(next_keyframe_frame_index-this_keyframe_frame_index);
			
			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1583, real_frame_index>=(real)this_keyframe_frame_index);
			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1584, real_frame_index< (real)next_keyframe_frame_index);
			points_interpolate(this_data, next_data, fraction, result);
		}
	}

	return;
}

static void animation_get_keyframe_scale(
	struct animation const *animation,
	real real_frame_index,
	short adjusted_node_index,
	short node_index,
	real *result)
{
	struct animation_compressed_data_header *header = tag_data_get_pointer(&animation->data, animation->compressed_data_offset, 0);
	real *default_data = (real *)((byte *)header + header->scale_default_data_offset);
	unsigned long keyframe_info = *(unsigned long *)((byte *)header + header->scale_keyframe_info_offset + adjusted_node_index*sizeof(unsigned long));
	short first_keyframe_index = (short)(keyframe_info>>12);
	short keyframe_count = (short)(keyframe_info&0xfff);

	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1610, real_frame_index>=0.0f);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1612, real_frame_index<(real)animation->frame_count);
	match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1614, keyframe_count>=0);

	if (keyframe_count==0)
	{
		*result = default_data[adjusted_node_index];
	}
	else
	{
		short this_keyframe_frame_index;
		short next_keyframe_frame_index;
		real this_scale;
		real next_scale;
		real *keyframe_data = (real *)((byte *)header + header->scale_keyframe_data_offset) + first_keyframe_index;
		word *keyframe_frame_indices = (word *)((byte *)header + header->scale_keyframe_frame_indices_offset) + first_keyframe_index;
		short frame_index = (short)fast_ftol((real)floor(real_frame_index));

		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1634, frame_index>=0 && frame_index<=keyframe_frame_indices[keyframe_count-1]);
		match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1635, keyframe_frame_indices[keyframe_count-1]==animation->frame_count-1);

		if (frame_index<keyframe_frame_indices[0])
		{
			this_keyframe_frame_index = 0;
			this_scale = default_data[adjusted_node_index];
			next_keyframe_frame_index = keyframe_frame_indices[0];
			next_scale = keyframe_data[0];
		}
		else if (frame_index==keyframe_frame_indices[keyframe_count-1])
		{
			this_keyframe_frame_index = keyframe_frame_indices[keyframe_count-1];
			this_scale = keyframe_data[keyframe_count-1];
			next_keyframe_frame_index = keyframe_frame_indices[keyframe_count-1]+1;
			next_scale = default_data[adjusted_node_index];
		}
		else
		{
			short keyframe_index = animation_keyframe_search(keyframe_frame_indices, keyframe_count, frame_index);

			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1655, keyframe_index>=0 && keyframe_index<keyframe_count-1);
			this_keyframe_frame_index = keyframe_frame_indices[keyframe_index];
			this_scale = keyframe_data[keyframe_index];
			next_keyframe_frame_index = keyframe_frame_indices[keyframe_index+1];
			next_scale = keyframe_data[keyframe_index+1];
		}

		if (real_frame_index==(real)this_keyframe_frame_index)
		{
			*result = this_scale;
		}
		else
		{
			real fraction = (real_frame_index-this_keyframe_frame_index)/(next_keyframe_frame_index-this_keyframe_frame_index);
			
			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1672, real_frame_index>=(real)this_keyframe_frame_index);
			match_assert("c:\\halo\\SOURCE\\models\\model_animations.c", 1673, real_frame_index< (real)next_keyframe_frame_index);
			scalars_interpolate(this_scale, next_scale, fraction, result);
		}
	}

	return;
}
