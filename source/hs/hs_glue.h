/*
HS_GLUE.H

header included in hcex build.
*/

#ifndef __HS_GLUE_H
#define __HS_GLUE_H
#pragma once

/* ---------- macros */

#define HS_C_TYPE(type) HS_C_TYPE_##type /* fake name */

#define HS_C_TYPE__hs_type_boolean boolean /* fake name */
#define HS_C_TYPE__hs_type_real real /* fake name */
#define HS_C_TYPE__hs_type_short_integer short /* fake name */
#define HS_C_TYPE__hs_type_long_integer long /* fake name */
#define HS_C_TYPE__hs_type_string char * /* fake name */
#define HS_C_TYPE__hs_type_script short /* fake name */
#define HS_C_TYPE__hs_type_trigger_volume short /* fake name */
#define HS_C_TYPE__hs_type_cutscene_flag short /* fake name */
#define HS_C_TYPE__hs_type_cutscene_camera_point short /* fake name */
#define HS_C_TYPE__hs_type_cutscene_title short /* fake name */
#define HS_C_TYPE__hs_type_cutscene_recording short /* fake name */
#define HS_C_TYPE__hs_type_device_group short /* fake name */
#define HS_C_TYPE__hs_type_ai long /* fake name */
#define HS_C_TYPE__hs_type_ai_command_list short /* fake name */
#define HS_C_TYPE__hs_type_starting_profile short /* fake name */
#define HS_C_TYPE__hs_type_conversation short /* fake name */
#define HS_C_TYPE__hs_type_navpoint short /* fake name */
#define HS_C_TYPE__hs_type_hud_message short /* fake name */
#define HS_C_TYPE__hs_type_object_list long /* fake name */
#define HS_C_TYPE__hs_type_sound long /* fake name */
#define HS_C_TYPE__hs_type_effect long /* fake name */
#define HS_C_TYPE__hs_type_damage long /* fake name */
#define HS_C_TYPE__hs_type_looping_sound long /* fake name */
#define HS_C_TYPE__hs_type_animation_graph long /* fake name */
#define HS_C_TYPE__hs_type_actor_variant long /* fake name */
#define HS_C_TYPE__hs_type_damage_effect long /* fake name */
#define HS_C_TYPE__hs_type_object_definition long /* fake name */
#define HS_C_TYPE__hs_type_enum_game_difficulty short /* fake name */
#define HS_C_TYPE__hs_type_enum_team short /* fake name */
#define HS_C_TYPE__hs_type_enum_ai_default_state short /* fake name */
#define HS_C_TYPE__hs_type_enum_actor_type short /* fake name */
#define HS_C_TYPE__hs_type_enum_hud_corner short /* fake name */
#define HS_C_TYPE__hs_type_object long /* fake name */
#define HS_C_TYPE__hs_type_unit long /* fake name */
#define HS_C_TYPE__hs_type_vehicle long /* fake name */
#define HS_C_TYPE__hs_type_weapon long /* fake name */
#define HS_C_TYPE__hs_type_device long /* fake name */
#define HS_C_TYPE__hs_type_scenery long /* fake name */
#define HS_C_TYPE__hs_type_object_name short /* fake name */
#define HS_C_TYPE__hs_type_unit_name short /* fake name */
#define HS_C_TYPE__hs_type_vehicle_name short /* fake name */
#define HS_C_TYPE__hs_type_weapon_name short /* fake name */
#define HS_C_TYPE__hs_type_device_name short /* fake name */
#define HS_C_TYPE__hs_type_scenery_name short /* fake name */

#define HS_ARGUMENT(type, index) (*(HS_C_TYPE(type) *)&arguments[(index)]) /* fake name */

#define HS_EVALUATE_VOID(return_type, function, call) /* fake name */	\
static void function##_evaluate(										\
	short function_index,												\
	long thread_index,													\
	boolean initialize)													\
{																		\
	long *arguments = hs_macro_function_evaluate(function_index, thread_index, initialize);	\
																		\
	if (arguments)														\
	{																	\
		call;															\
		hs_return(thread_index, 0);										\
	}																	\
																		\
	return;																\
}

#define HS_EVALUATE_VALUE(return_type, function, call) /* fake name */	\
static void function##_evaluate(										\
	short function_index,												\
	long thread_index,													\
	boolean initialize)													\
{																		\
	long result = 0;													\
	long *arguments = hs_macro_function_evaluate(function_index, thread_index, initialize);	\
																		\
	if (arguments)														\
	{																	\
		*(HS_C_TYPE(return_type) *)&result = (call);					\
		hs_return(thread_index, result);								\
	}																	\
																		\
	return;																\
}

#define HS_EVALUATE_VOID_0(return_type, function, call) /* fake name */	\
static void function##_evaluate(										\
	short function_index,												\
	long thread_index,													\
	boolean initialize)													\
{																		\
	call;																\
	hs_return(thread_index, 0);											\
																		\
	return;																\
}

#define HS_EVALUATE_VALUE_0(return_type, function, call) /* fake name */\
static void function##_evaluate(										\
	short function_index,												\
	long thread_index,													\
	boolean initialize)													\
{																		\
	long result = 0;													\
																		\
	*(HS_C_TYPE(return_type) *)&result = (call);						\
	hs_return(thread_index, result);									\
																		\
	return;																\
}

#define HS_EVALUATE__hs_type_void HS_EVALUATE_VOID /* fake name */
#define HS_EVALUATE__hs_type_boolean HS_EVALUATE_VALUE /* fake name */
#define HS_EVALUATE__hs_type_real HS_EVALUATE_VALUE /* fake name */
#define HS_EVALUATE__hs_type_short_integer HS_EVALUATE_VALUE /* fake name */
#define HS_EVALUATE__hs_type_long_integer HS_EVALUATE_VALUE /* fake name */
#define HS_EVALUATE__hs_type_object_list HS_EVALUATE_VALUE /* fake name */
#define HS_EVALUATE__hs_type_enum_game_difficulty HS_EVALUATE_VALUE /* fake name */
#define HS_EVALUATE__hs_type_object HS_EVALUATE_VALUE /* fake name */
#define HS_EVALUATE__hs_type_unit HS_EVALUATE_VALUE /* fake name */

#define HS_EVALUATE_0__hs_type_void HS_EVALUATE_VOID_0 /* fake name */
#define HS_EVALUATE_0__hs_type_boolean HS_EVALUATE_VALUE_0 /* fake name */
#define HS_EVALUATE_0__hs_type_real HS_EVALUATE_VALUE_0 /* fake name */
#define HS_EVALUATE_0__hs_type_short_integer HS_EVALUATE_VALUE_0 /* fake name */
#define HS_EVALUATE_0__hs_type_long_integer HS_EVALUATE_VALUE_0 /* fake name */
#define HS_EVALUATE_0__hs_type_object_list HS_EVALUATE_VALUE_0 /* fake name */
#define HS_EVALUATE_0__hs_type_enum_game_difficulty HS_EVALUATE_VALUE_0 /* fake name */
#define HS_EVALUATE_0__hs_type_object HS_EVALUATE_VALUE_0 /* fake name */
#define HS_EVALUATE_0__hs_type_unit HS_EVALUATE_VALUE_0 /* fake name */

/* fake name */
#ifdef HS_COMPILER_ENABLED
#define HS_FUNCTION_0(return_type, name, function, documentation)					\
HS_EVALUATE_0_##return_type(return_type, function, function())						\
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	#name,																			\
	hs_macro_function_parse,														\
	function##_evaluate,															\
	documentation,																	\
	NULL,																			\
	0																				\
}

#define HS_FUNCTION_1(return_type, name, function, documentation, type0)			\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0)))	\
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	#name,																			\
	hs_macro_function_parse,														\
	function##_evaluate,															\
	documentation,																	\
	NULL,																			\
	1,																				\
	{type0}																			\
}

#define HS_FUNCTION_2(return_type, name, function, documentation, type0, type1)		\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	#name,																			\
	hs_macro_function_parse,														\
	function##_evaluate,															\
	documentation,																	\
	NULL,																			\
	2,																				\
	{type0, type1}																	\
}

#define HS_FUNCTION_3(return_type, name, function, documentation, type0, type1, type2)	\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	#name,																			\
	hs_macro_function_parse,														\
	function##_evaluate,															\
	documentation,																	\
	NULL,																			\
	3,																				\
	{type0, type1, type2}															\
}

#define HS_FUNCTION_4(return_type, name, function, documentation, type0, type1, type2, type3)	\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2), HS_ARGUMENT(type3, 3))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	#name,																			\
	hs_macro_function_parse,														\
	function##_evaluate,															\
	documentation,																	\
	NULL,																			\
	4,																				\
	{type0, type1, type2, type3}													\
}


#define HS_FUNCTION_5(return_type, name, function, documentation, type0, type1, type2, type3, type4) \
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2), HS_ARGUMENT(type3, 3), HS_ARGUMENT(type4, 4))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	#name,																			\
	hs_macro_function_parse,														\
	function##_evaluate,															\
	documentation,																	\
	NULL,																			\
	5,																				\
	{type0, type1, type2, type3, type4}												\
}


#define HS_FUNCTION_6(return_type, name, function, documentation, type0, type1, type2, type3, type4, type5) \
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2), HS_ARGUMENT(type3, 3), HS_ARGUMENT(type4, 4), HS_ARGUMENT(type5, 5))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	#name,																			\
	hs_macro_function_parse,														\
	function##_evaluate,															\
	documentation,																	\
	NULL,																			\
	6,																				\
	{type0, type1, type2, type3, type4, type5}										\
}

#else

#define HS_FUNCTION_0(return_type, name, function, documentation)					\
HS_EVALUATE_0_##return_type(return_type, function, function())						\
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	function##_evaluate,															\
	0																				\
}

#define HS_FUNCTION_1(return_type, name, function, documentation, type0)			\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0)))	\
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	function##_evaluate,															\
	1,																				\
	{type0}																			\
}

#define HS_FUNCTION_2(return_type, name, function, documentation, type0, type1)		\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	function##_evaluate,															\
	2,																				\
	{type0, type1}																	\
}

#define HS_FUNCTION_3(return_type, name, function, documentation, type0, type1, type2)	\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	function##_evaluate,															\
	3,																				\
	{type0, type1, type2}															\
}

#define HS_FUNCTION_4(return_type, name, function, documentation, type0, type1, type2, type3)	\
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2), HS_ARGUMENT(type3, 3))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	function##_evaluate,															\
	4,																				\
	{type0, type1, type2, type3}													\
}


#define HS_FUNCTION_5(return_type, name, function, documentation, type0, type1, type2, type3, type4) \
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2), HS_ARGUMENT(type3, 3), HS_ARGUMENT(type4, 4))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	function##_evaluate,															\
	5,																				\
	{type0, type1, type2, type3, type4}												\
}


#define HS_FUNCTION_6(return_type, name, function, documentation, type0, type1, type2, type3, type4, type5) \
HS_EVALUATE_##return_type(return_type, function, function(HS_ARGUMENT(type0, 0), HS_ARGUMENT(type1, 1), HS_ARGUMENT(type2, 2), HS_ARGUMENT(type3, 3), HS_ARGUMENT(type4, 4), HS_ARGUMENT(type5, 5))) \
static struct hs_function_definition const name##_definition =						\
{																					\
	return_type,																	\
	function##_evaluate,															\
	6,																				\
	{type0, type1, type2, type3, type4, type5}										\
}

#endif



#endif // __HS_GLUE_H
