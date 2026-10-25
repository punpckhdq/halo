/*
RASTERIZER_XBOX_DETAIL_OBJECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "main.h"
#include "detail_object_definitions.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "structure_bsp_definitions.h"

/* ---------- globals */

static IDirect3DVertexBuffer8 *local_d3d_vertex_buffer;

/* ---------- private code */

static void detail_object_build_vertices(
	struct detail_object_collection_definition const *collection_definition,
	struct detail_object const *detail_objects,
	struct detail_object_vertex *detail_object_vertices,
	long detail_object_count)
{
	long detail_object_index;

	for (detail_object_index = 0; detail_object_index < detail_object_count; detail_object_index++)
	{
		struct detail_object_type_definition const *type_definition;
		struct detail_object_vertex vertex;
		short type_index;
		short frame_index;
		word color;
		struct detail_object const *detail_object = &detail_objects[detail_object_index];

		vertex.position[0] = detail_object->position[0];
		vertex.position[1] = detail_object->position[1];
		vertex.position[2] = detail_object->position[2];

		color = detail_object->color;
		vertex.color[0] = PIXEL16_565_TO_PIXEL32_RED(color);
		vertex.color[1] = PIXEL16_565_TO_PIXEL32_GREEN(color);
		vertex.color[2] = PIXEL16_565_TO_PIXEL32_BLUE(color);

		type_index = (detail_object->data >> 4) % collection_definition->type_definitions.count;
		type_definition = TAG_BLOCK_GET_ELEMENT(&collection_definition->type_definitions, type_index, struct detail_object_type_definition);
		frame_index = (detail_object->data & MASK(4)) % type_definition->frame_count + type_definition->first_frame_index;

		vertex.data = (frame_index << 8) | (type_index << 4);

		*detail_object_vertices++ = vertex;
		vertex.data++;
		*detail_object_vertices++ = vertex;
		vertex.data++;
		*detail_object_vertices++ = vertex;
		vertex.data++;
		*detail_object_vertices++ = vertex;
	}

	return;
}

/* ---------- public code */

boolean rasterizer_detail_objects_initialize(
	void)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 98, global_d3d_device);

	D3DCALL(success, IDirect3DDevice8_CreateVertexBuffer(global_d3d_device,
		RASTERIZER_MAXIMUM_DETAIL_OBJECTS_PER_FRAME*NUMBER_OF_VERTICES_PER_QUADRILATERAL*sizeof(struct detail_object_vertex),
		RASTERIZER_DYNAMIC_BUFFER_USAGE,
		0,
		RASTERIZER_DYNAMIC_BUFFER_POOL,
		&local_d3d_vertex_buffer));

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_detail_objects_initialize failed");
	}

	return success;
}

void rasterizer_detail_objects_dispose(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 119, local_d3d_vertex_buffer);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 120, global_d3d_device);

	if (local_d3d_vertex_buffer)
	{
		IDirect3DVertexBuffer8_Release(local_d3d_vertex_buffer);
		local_d3d_vertex_buffer = NULL;
	}

	return;
}

void _rasterizer_detail_objects_begin(
	void)
{
	boolean success = TRUE;

	rasterizer_profile_begin(_rasterizer_profile_detail_objects);

	if (rasterizer_debug_options.draw_detail_objects && main_get_window_count() <= 1)
	{
		real vsh_constants__detailobj[VSH_CONSTANTS__DETAILOBJ_COUNT * 4];

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 136, local_d3d_vertex_buffer);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 137, global_d3d_device);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_NONE);
		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, D3DZB_TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

		vsh_constants__detailobj[0] = 1.f / 256.f;
		vsh_constants__detailobj[1] = 16.f;
		vsh_constants__detailobj[2] = 541.f;
		vsh_constants__detailobj[3] = 659.f;
		vsh_constants__detailobj[4] = (real)DETAIL_OBJECT_CELL_SIZE;
		vsh_constants__detailobj[5] = (real)DETAIL_OBJECT_CELL_SIZE;
		vsh_constants__detailobj[6] = (real)DETAIL_OBJECT_CELL_SIZE;
		vsh_constants__detailobj[7] = rasterizer_debug_options.detail_object_screen_facing_offset_multiplier;
		vsh_constants__detailobj[8] = 1.f;
		vsh_constants__detailobj[9] = 1.f;
		vsh_constants__detailobj[10] = 0.5f;
		vsh_constants__detailobj[11] = 0.f;
		vsh_constants__detailobj[12] = 0.f;
		vsh_constants__detailobj[13] = 1.f;
		vsh_constants__detailobj[14] = -0.5f;
		vsh_constants__detailobj[15] = 0.f;
		vsh_constants__detailobj[16] = 0.f;
		vsh_constants__detailobj[17] = 0.f;
		vsh_constants__detailobj[18] = -0.5f;
		vsh_constants__detailobj[19] = 1.f;
		vsh_constants__detailobj[20] = 1.f;
		vsh_constants__detailobj[21] = 0.f;
		vsh_constants__detailobj[22] = 0.5f;
		vsh_constants__detailobj[23] = 1.f;

		D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
			VSH_CONSTANTS__DETAILOBJ_OFFSET,
			vsh_constants__detailobj,
			VSH_CONSTANTS__DETAILOBJ_COUNT));

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);
		rasterizer_set_pixel_shader(&pixel_shader);

		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device,
			0,
			local_d3d_vertex_buffer,
			sizeof(struct detail_object_vertex)));
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_detail_objects_begin failed");
	}

	return;
}

void _rasterizer_detail_objects_rebuild_vertices(
	struct detail_object_view_data const *detail_object_view_data)
{
	boolean success = TRUE;

	if (rasterizer_debug_options.draw_detail_objects && main_get_window_count() <= 1)
	{
		struct scenario *scenario = global_scenario_get();
		struct detail_object_vertex *vertices = NULL;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 215, detail_object_view_data);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 216, local_d3d_vertex_buffer);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 217, global_d3d_device);

		rasterizer_globals.current_lock_operation = _rasterizer_lock_detail_objects;
		D3DCALL(success, IDirect3DVertexBuffer8_Lock(local_d3d_vertex_buffer,
			0,
			RASTERIZER_MAXIMUM_DETAIL_OBJECTS_PER_FRAME*NUMBER_OF_VERTICES_PER_QUADRILATERAL*sizeof(struct detail_object_vertex),
			(BYTE **)&vertices,
			0));
		rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

		if (success && vertices)
		{
			struct structure_detail_object_data *detail_object_data;
			struct detail_object *detail_objects;
			long vertex_index;
			long detail_object_count;
			boolean warned;
			short layer_index;

			detail_object_data = global_structure_bsp_get()->detail_object_data.count ? TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->detail_object_data, 0, struct structure_detail_object_data) : NULL;
			detail_objects = TAG_BLOCK_GET_ELEMENT(&detail_object_data->detail_objects, 0, struct detail_object);
			vertex_index = 0;
			detail_object_count = 0;
			warned = FALSE;

			for (layer_index = 0; layer_index < detail_object_view_data->layer_count; layer_index++)
			{
				short cell_index;
				struct detail_object_layer_data *layer = &detail_object_view_data->layers[layer_index];
				struct scenario_detail_object_collection_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(&scenario->detail_object_collection_palette, layer->collection_definition_index, struct scenario_detail_object_collection_palette_entry);
				struct detail_object_collection_definition *collection_definition = detail_object_collection_definition_get(palette_entry->reference.index);

				for (cell_index = 0; cell_index < layer->cell_count; cell_index++)
				{
					struct detail_object_cell_data *cell = &layer->cells[cell_index];
					long cell_detail_object_count = MIN(cell->detail_object_count, RASTERIZER_MAXIMUM_DETAIL_OBJECTS_PER_FRAME - detail_object_count);

					detail_object_build_vertices(
						collection_definition,
						&detail_objects[cell->first_detail_object_index],
						&vertices[vertex_index],
						cell_detail_object_count);

					cell->internal__first_vertex_index = vertex_index;
					vertex_index += NUMBER_OF_VERTICES_PER_QUADRILATERAL * cell->detail_object_count;

					if (cell->detail_object_count > cell_detail_object_count)
					{
						cell->detail_object_count = cell_detail_object_count;

						if (!warned)
						{
							warned = TRUE;
							error(
								_error_silent,
								"### ERROR too many detail object submitted for frame (max=#%d)",
								RASTERIZER_MAXIMUM_DETAIL_OBJECTS_PER_FRAME);
						}
					}

					detail_object_count += cell_detail_object_count;
				}
			}
		}

		D3DCALL(success, IDirect3DVertexBuffer8_Unlock(local_d3d_vertex_buffer));
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_detail_objects_rebuild_vertices failed");
	}

	return;
}

void _rasterizer_detail_objects_draw(
	struct detail_object_view_data const *detail_object_view_data)
{
	boolean success = TRUE;

	if (rasterizer_debug_options.draw_detail_objects && main_get_window_count() <= 1)
	{
		short layer_index;
		struct scenario *scenario = global_scenario_get();

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 306, detail_object_view_data);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 307, global_d3d_device);

		for (layer_index = 0; layer_index < detail_object_view_data->layer_count; layer_index++)
		{
			real_vector4d vsh_constants__detailobj_typedata[MAXIMUM_DETAIL_OBJECT_TYPES_PER_COLLECTION];
			real_vector4d vsh_constants__detailobj_framedata[MAXIMUM_DETAIL_OBJECT_SPRITES_PER_COLLECTION];
			short frame_count;
			short type_index;
			short sequence_index;
			short cell_index;
			struct detail_object_layer_data *layer = &detail_object_view_data->layers[layer_index];
			struct scenario_detail_object_collection_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(&scenario->detail_object_collection_palette, layer->collection_definition_index, struct scenario_detail_object_collection_palette_entry);
			struct detail_object_collection_definition *collection_definition = detail_object_collection_definition_get(palette_entry->reference.index);
			struct bitmap_group *bitmap_group = bitmap_group_get(collection_definition->map.index);

			rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, collection_definition->map.index, 0);

			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			if (collection_definition->collection_type != NONE)
			{
				rasterizer_set_vertex_shader_permutation(33, _rasterizer_vertex_type_detail_object, collection_definition->collection_type);
			}

			frame_count = 0;

			for (type_index = 0; type_index < collection_definition->type_definitions.count; type_index++)
			{
				real_vector4d *typedata;
				struct detail_object_type_definition *type_definition = TAG_BLOCK_GET_ELEMENT(&collection_definition->type_definitions, type_index, struct detail_object_type_definition);
				struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, type_definition->sequence_index, struct bitmap_group_sequence);
				struct bitmap_data *bitmap = TAG_BLOCK_GET_ELEMENT(&bitmap_group->bitmaps, sequence->first_bitmap_index, struct bitmap_data);
				real fade_scale = type_definition->far_fade_distance - type_definition->near_fade_distance;
				real size = type_definition->size_min;

				if (fade_scale > 0.f)
				{
					fade_scale = 1.f / fade_scale;
				}

				typedata = &vsh_constants__detailobj_typedata[type_index];
				typedata->i = fade_scale * type_definition->far_fade_distance;
				typedata->j = -fade_scale;
				typedata->k = (real)bitmap->width * size;
				typedata->l = (real)bitmap->height * size;
			}

			for (sequence_index = 0; sequence_index < bitmap_group->sequences.count; sequence_index++)
			{
				short sprite_index;
				struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, sequence_index, struct bitmap_group_sequence);

				for (sprite_index = 0; sprite_index < sequence->sprites.count; sprite_index++)
				{
					real_vector4d *framedata;
					struct bitmap_group_sprite *sprite = TAG_BLOCK_GET_ELEMENT(&sequence->sprites, sprite_index, struct bitmap_group_sprite);

					TAG_BLOCK_GET_ELEMENT(&bitmap_group->bitmaps, sprite->bitmap_index, struct bitmap_data);

					framedata = &vsh_constants__detailobj_framedata[frame_count++];
					framedata->i = sprite->bounds.x0;
					framedata->j = sprite->bounds.y0;
					framedata->k = sprite->bounds.x1 - sprite->bounds.x0;
					framedata->l = sprite->bounds.y1 - sprite->bounds.y0;
				}
			}

			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__DETAILOBJ_TYPEDATA_OFFSET,
				vsh_constants__detailobj_typedata,
				collection_definition->type_definitions.count));
			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__DETAILOBJ_FRAMEDATA_OFFSET,
				vsh_constants__detailobj_framedata,
				frame_count));

			for (cell_index = 0; cell_index < layer->cell_count; cell_index++)
			{
				struct detail_object_cell_data *cell = &layer->cells[cell_index];

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_detail_objects.c", 394, cell->z_reference_vector);

				D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device,
					2,
					(real)(cell->cell_x*DETAIL_OBJECT_CELL_SIZE),
					(real)(cell->cell_y*DETAIL_OBJECT_CELL_SIZE),
					(cell->cell_z*(real)DETAIL_OBJECT_CELL_SIZE),
					1.0f));
				D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device,
					3,
					cell->z_reference_vector->i,
					cell->z_reference_vector->j,
					cell->z_reference_vector->k,
					cell->z_reference_vector->l));
				D3DCALL(success, IDirect3DDevice8_DrawVertices(global_d3d_device,
					D3DPT_QUADLIST,
					cell->internal__first_vertex_index,
					cell->detail_object_count*NUMBER_OF_VERTICES_PER_QUADRILATERAL));
			}
		}

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_detail_objects_draw failed");
		}
	}

	return;
}

void _rasterizer_detail_objects_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_detail_objects);

	return;
}
