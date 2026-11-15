/*
SHADER_TRANSPARENT_GENERIC_PREPROCESSOR.C
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

/* ---------- globals */

static long const color_input_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS] = /* fake name */
{
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	PS_REGISTER_T0,
	PS_REGISTER_T1,
	PS_REGISTER_T2,
	PS_REGISTER_T3,
	PS_REGISTER_V0,
	PS_REGISTER_V1,
	PS_REGISTER_R0,
	PS_REGISTER_R1,
	PS_REGISTER_C0,
	PS_REGISTER_C1,
	PS_REGISTER_T0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T2 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T3 | PS_CHANNEL_ALPHA,
	PS_REGISTER_V0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_V1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_R0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_R1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_C0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_C1 | PS_CHANNEL_ALPHA
};

static long const alpha_input_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS] = /* fake name */
{
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	PS_REGISTER_T0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T2 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T3 | PS_CHANNEL_ALPHA,
	PS_REGISTER_V0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_V1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_R0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_R1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_C0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_C1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T0 | PS_CHANNEL_BLUE,
	PS_REGISTER_T1 | PS_CHANNEL_BLUE,
	PS_REGISTER_T2 | PS_CHANNEL_BLUE,
	PS_REGISTER_T3 | PS_CHANNEL_BLUE,
	PS_REGISTER_V0 | PS_CHANNEL_BLUE,
	PS_REGISTER_V1 | PS_CHANNEL_BLUE,
	PS_REGISTER_R0 | PS_CHANNEL_BLUE,
	PS_REGISTER_R1 | PS_CHANNEL_BLUE,
	PS_REGISTER_C0 | PS_CHANNEL_BLUE,
	PS_REGISTER_C1 | PS_CHANNEL_BLUE
};

static long const color_input_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS] = /* fake name */
{
	PS_INPUTMAPPING_UNSIGNED_IDENTITY,
	PS_INPUTMAPPING_UNSIGNED_INVERT,
	PS_INPUTMAPPING_EXPAND_NORMAL,
	PS_INPUTMAPPING_EXPAND_NEGATE,
	PS_INPUTMAPPING_HALFBIAS_NORMAL,
	PS_INPUTMAPPING_HALFBIAS_NEGATE,
	PS_INPUTMAPPING_SIGNED_IDENTITY,
	PS_INPUTMAPPING_SIGNED_NEGATE
};

static long const alpha_input_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS] = /* fake name */
{
	PS_INPUTMAPPING_UNSIGNED_IDENTITY,
	PS_INPUTMAPPING_UNSIGNED_INVERT,
	PS_INPUTMAPPING_EXPAND_NORMAL,
	PS_INPUTMAPPING_EXPAND_NEGATE,
	PS_INPUTMAPPING_HALFBIAS_NORMAL,
	PS_INPUTMAPPING_HALFBIAS_NEGATE,
	PS_INPUTMAPPING_SIGNED_IDENTITY,
	PS_INPUTMAPPING_SIGNED_NEGATE
};

static long const constant_input_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_CONSTANT_INPUTS][NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS] = /* fake name */
{
	{PS_REGISTER_ZERO, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE_HALF, PS_REGISTER_ONE_HALF, PS_REGISTER_ZERO, PS_REGISTER_ZERO},
	{PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE, PS_REGISTER_ONE_HALF, PS_REGISTER_NEGATIVE_ONE_HALF, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE},
	{PS_REGISTER_ONE_HALF, PS_REGISTER_ONE_HALF, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ONE_HALF, PS_REGISTER_NEGATIVE_ONE_HALF},
	{PS_REGISTER_ZERO, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE_HALF, PS_REGISTER_ONE_HALF, PS_REGISTER_NEGATIVE_ONE, PS_REGISTER_ONE},
	{PS_REGISTER_ZERO, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE, PS_REGISTER_ONE, PS_REGISTER_NEGATIVE_ONE_HALF, PS_REGISTER_ONE_HALF, PS_REGISTER_NEGATIVE_ONE_HALF, PS_REGISTER_ONE_HALF}
};

static long const color_output_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS] = /* fake name */
{
	PS_REGISTER_DISCARD,
	PS_REGISTER_R0,
	PS_REGISTER_R1,
	PS_REGISTER_V0,
	PS_REGISTER_V1,
	PS_REGISTER_T0,
	PS_REGISTER_T1,
	PS_REGISTER_T2,
	PS_REGISTER_T3
};

static long const alpha_output_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS] = /* fake name */
{
	PS_REGISTER_DISCARD,
	PS_REGISTER_R0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_R1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_V0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_V1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T0 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T1 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T2 | PS_CHANNEL_ALPHA,
	PS_REGISTER_T3 | PS_CHANNEL_ALPHA
};

static long const color_output_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS] = /* fake name */
{
	PS_COMBINEROUTPUT_IDENTITY,
	PS_COMBINEROUTPUT_SHIFTRIGHT_1,
	PS_COMBINEROUTPUT_SHIFTLEFT_1,
	PS_COMBINEROUTPUT_SHIFTLEFT_2,
	PS_COMBINEROUTPUT_BIAS,
	PS_COMBINEROUTPUT_SHIFTLEFT_1_BIAS
};

static long const alpha_output_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS] = /* fake name */
{
	PS_COMBINEROUTPUT_IDENTITY,
	PS_COMBINEROUTPUT_SHIFTRIGHT_1,
	PS_COMBINEROUTPUT_SHIFTLEFT_1,
	PS_COMBINEROUTPUT_SHIFTLEFT_2,
	PS_COMBINEROUTPUT_BIAS,
	PS_COMBINEROUTPUT_SHIFTLEFT_1_BIAS
};

/* ---------- private code */

static long shader_stage_color_input(
	short register_index,
	short mapping_index)
{
	long result = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 212, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 213, mapping_index>=0 && mapping_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS);

	if (color_input_registers[register_index]==NONE)
	{
		result = constant_input_registers[register_index][mapping_index];
	}
	else
	{
		result = color_input_registers[register_index] | color_input_mappings[mapping_index];
	}

	return result;
}

static long shader_stage_color_output(
	short register_index)
{
	long result = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 233, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS);

	result = color_output_registers[register_index];

	return result;
}

static long shader_stage_color_output_flags(
	struct shader_transparent_generic_stage const *stage)
{
	long flags = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 246, stage);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 247, stage->color_output_mapping>=0 && stage->color_output_mapping<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 248, stage->color_output_AB_function>=0 && stage->color_output_AB_function<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTIONS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 249, stage->color_output_CD_function>=0 && stage->color_output_CD_function<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTIONS);

	flags = color_output_mappings[stage->color_output_mapping];

	if (stage->color_output_AB_function==_shader_transparent_generic_stage_output_function_dot_product)
	{
		flags |= PS_COMBINEROUTPUT_AB_DOT_PRODUCT;
	}

	if (stage->color_output_CD_function==_shader_transparent_generic_stage_output_function_dot_product)
	{
		flags |= PS_COMBINEROUTPUT_CD_DOT_PRODUCT;
	}

	if (TEST_FLAG(stage->flags, _shader_transparent_generic_stage_color_mux_bit))
	{
		flags |= PS_COMBINEROUTPUT_AB_CD_MUX;
	}

	return flags;
}

static long shader_stage_alpha_input(
	short register_index,
	short mapping_index)
{
	long result = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 266, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 267, mapping_index>=0 && mapping_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS);

	if (alpha_input_registers[register_index]==NONE)
	{
		result = constant_input_registers[register_index][mapping_index];
	}
	else
	{
		result = alpha_input_registers[register_index] | alpha_input_mappings[mapping_index];
	}

	return result;
}

static long shader_stage_alpha_output(
	short register_index)
{
	long result = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 287, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS);

	result = alpha_output_registers[register_index];

	return result;
}

static long shader_stage_alpha_output_flags(
	struct shader_transparent_generic_stage const *stage)
{
	long flags = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 300, stage);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 301, stage->alpha_output_mapping>=0 && stage->alpha_output_mapping<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS);

	flags = alpha_output_mappings[stage->alpha_output_mapping];

	if (TEST_FLAG(stage->flags, _shader_transparent_generic_stage_alpha_mux_bit))
	{
		flags |= PS_COMBINEROUTPUT_AB_CD_MUX;
	}

	return flags;
}

static boolean shader_map_verify(
	short map_index,
	struct shader_transparent_generic_map const *map)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 316, map);

	if (map->map.index==NONE)
	{
		error(_error_silent, "### ERROR transparent shader map #%d has no associated bitmap", map_index);
		success = FALSE;
	}

	if (map->mipmap_bias!=0.f)
	{
		error(_error_silent, "### ERROR unsupported: transparent shader map #%d has non-zero mipmap bias", map_index);
		success = FALSE;
	}

	return success;
}

static boolean shader_stage_verify(
	short stage_index,
	struct shader_transparent_generic_stage const *stage)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 340, stage);

	if ((stage->color_output_AB && stage->color_output_CD && stage->color_output_AB==stage->color_output_CD) ||
		(stage->color_output_AB && stage->color_output_AB_CD_mux_sum && stage->color_output_AB==stage->color_output_AB_CD_mux_sum) ||
		(stage->color_output_CD && stage->color_output_AB_CD_mux_sum && stage->color_output_CD==stage->color_output_AB_CD_mux_sum) ||
		(stage->alpha_output_AB && stage->alpha_output_CD && stage->alpha_output_AB==stage->alpha_output_CD) ||
		(stage->alpha_output_AB && stage->alpha_output_AB_CD_mux_sum && stage->alpha_output_AB==stage->alpha_output_AB_CD_mux_sum) ||
		(stage->alpha_output_CD && stage->alpha_output_AB_CD_mux_sum && stage->alpha_output_CD==stage->alpha_output_AB_CD_mux_sum))
	{
		error(_error_silent, "### ERROR transparent shader output conflict in stage #%d", stage_index);
		success = FALSE;
	}

	if ((stage->color_output_AB_function || stage->color_output_CD_function) && stage->color_output_AB_CD_mux_sum)
	{
		error(_error_silent, "### ERROR transparent shader evaluates dot product and AB+CD sum in stage #%d", stage_index);
		success = FALSE;
	}

	if (stage->alpha_output_AB==_shader_transparent_generic_stage_output_vertex_0_fog || stage->alpha_output_AB_CD_mux_sum==_shader_transparent_generic_stage_output_vertex_0_fog)
	{
		error(_error_silent, "### ERROR transparent shader writes to fog density register in stage #%d", stage_index);
		success = FALSE;
	}

	if (TEST_FLAG(stage->flags, _shader_transparent_generic_stage_color_mux_bit) && (stage->color_output_AB_function || stage->color_output_CD_function))
	{
		error(_error_silent, "### ERROR transparent shader evaluates dot product and mux[AB,CD] in stage #%d", stage_index);
		success = FALSE;
	}

	return success;
}

/* ---------- public code */

boolean shader_transparent_generic_create(
	struct shader const *shader,
	D3DPIXELSHADERDEF *pixel_shader)
{
	short stage_index;
	boolean success = TRUE;
	struct shader_transparent_generic const *shader_transparent_generic = NULL;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 388, shader);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c", 389, pixel_shader);

	shader_transparent_generic = shader_transparent_generic_get(shader);
	memset(pixel_shader, 0, sizeof(*pixel_shader));

	if (shader_transparent_generic->generic.maps.count>0 || shader_transparent_generic->generic.stages.count>0)
	{
		short map_index;

		pixel_shader->PSTextureModes = PS_TEXTUREMODES(
			shader_transparent_generic->generic.maps.count>0 ? (shader_transparent_generic->generic.type!=_shader_transparent_generic_type_2d_map ? PS_TEXTUREMODES_CUBEMAP : PS_TEXTUREMODES_PROJECT2D) : PS_TEXTUREMODES_NONE,
			shader_transparent_generic->generic.maps.count>1 ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE,
			shader_transparent_generic->generic.maps.count>2 ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE,
			shader_transparent_generic->generic.maps.count>3 ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE);

		for (map_index = 0; map_index<shader_transparent_generic->generic.maps.count; map_index++)
		{
			struct shader_transparent_generic_map const *map = TAG_BLOCK_GET_ELEMENT(&shader_transparent_generic->generic.maps, map_index, struct shader_transparent_generic_map);

			if (success)
			{
				success = shader_map_verify(map_index, map);
			}
		}
	}
	else
	{
		error(_error_silent, "### ERROR generic shader has no maps or stages");
		success = FALSE;
	}

	pixel_shader->PSCombinerCount = PS_COMBINERCOUNT(MAX(1, shader_transparent_generic->generic.stages.count) + 1, PS_COMBINERCOUNT_MUX_MSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);

	if (shader_transparent_generic->generic.stages.count>0)
	{
		for (stage_index = 0; stage_index<shader_transparent_generic->generic.stages.count; stage_index++)
		{
			long color_input_A;
			long color_input_B;
			long color_input_C;
			long color_input_D;
			long color_output_AB;
			long color_output_CD;
			long color_output_AB_CD_mux_sum;
			long color_output_flags;
			long alpha_input_A;
			long alpha_input_B;
			long alpha_input_C;
			long alpha_input_D;
			long alpha_output_AB;
			long alpha_output_CD;
			long alpha_output_AB_CD_mux_sum;
			long alpha_output_flags;
			struct shader_transparent_generic_stage const *stage = TAG_BLOCK_GET_ELEMENT(&shader_transparent_generic->generic.stages, stage_index, struct shader_transparent_generic_stage);

			if (success)
			{
				success = shader_stage_verify(stage_index, stage);
			}

			pixel_shader->PSConstant1[stage_index] = real_argb_color_to_pixel32(&stage->constant_color1);

			color_input_A = shader_stage_color_input(stage->color_input_A, stage->color_input_A_mapping);
			color_input_B = shader_stage_color_input(stage->color_input_B, stage->color_input_B_mapping);
			color_input_C = shader_stage_color_input(stage->color_input_C, stage->color_input_C_mapping);
			color_input_D = shader_stage_color_input(stage->color_input_D, stage->color_input_D_mapping);
			color_output_AB = shader_stage_color_output(stage->color_output_AB);
			color_output_CD = shader_stage_color_output(stage->color_output_CD);
			color_output_AB_CD_mux_sum = shader_stage_color_output(stage->color_output_AB_CD_mux_sum);
			color_output_flags = shader_stage_color_output_flags(stage);
			pixel_shader->PSRGBInputs[stage_index] = PS_COMBINERINPUTS(color_input_A, color_input_B, color_input_C, color_input_D);
			pixel_shader->PSRGBOutputs[stage_index] = PS_COMBINEROUTPUTS(color_output_AB & MASK(4), color_output_CD & MASK(4), color_output_AB_CD_mux_sum & MASK(4), color_output_flags);

			alpha_input_A = shader_stage_alpha_input(stage->alpha_input_A, stage->alpha_input_A_mapping);
			alpha_input_B = shader_stage_alpha_input(stage->alpha_input_B, stage->alpha_input_B_mapping);
			alpha_input_C = shader_stage_alpha_input(stage->alpha_input_C, stage->alpha_input_C_mapping);
			alpha_input_D = shader_stage_alpha_input(stage->alpha_input_D, stage->alpha_input_D_mapping);
			alpha_output_AB = shader_stage_alpha_output(stage->alpha_output_AB);
			alpha_output_CD = shader_stage_alpha_output(stage->alpha_output_CD);
			alpha_output_AB_CD_mux_sum = shader_stage_alpha_output(stage->alpha_output_AB_CD_mux_sum);
			alpha_output_flags = shader_stage_alpha_output_flags(stage);
			pixel_shader->PSAlphaInputs[stage_index] = PS_COMBINERINPUTS(alpha_input_A, alpha_input_B, alpha_input_C, alpha_input_D);
			pixel_shader->PSAlphaOutputs[stage_index] = PS_COMBINEROUTPUTS(alpha_output_AB & MASK(4), alpha_output_CD & MASK(4), alpha_output_AB_CD_mux_sum & MASK(4), alpha_output_flags);
		}
	}
	else
	{
		pixel_shader->PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader->PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader->PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader->PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
	}

	pixel_shader->PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
	pixel_shader->PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);

	return success;
}
