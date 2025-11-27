/*
RASTERIZER_XBOX_MODELS.C

symbols in this file:
0015A150 01b0:
	_code_0015a150 (0000)
0015A300 0050:
	_code_0015a300 (0000)
0015A350 03f0:
	_code_0015a350 (0000)
0015A740 0220:
	_code_0015a740 (0000)
0015A960 0060:
	_code_0015a960 (0000)
0015A9C0 0010:
	_code_0015a9c0 (0000)
0015A9D0 0040:
	__rasterizer_models_begin (0000)
0015AA10 0080:
	__rasterizer_model_end (0000)
0015AA90 0030:
	__rasterizer_models_end (0000)
0015AAC0 0040:
	_rasterizer_model_ambient_reflection_tint (0000)
0015AB00 0c20:
	_rasterizer_model_draw_environment_shader (0000)
0015B720 01c0:
	__rasterizer_model_begin (0000)
0015B8E0 0510:
	__rasterizer_model_transparent_geometry_submit (0000)
0015BDF0 1430:
	__rasterizer_model_draw (0000)
00291818 0042:
	??_C@_0EC@KBJMJLJB@detail_mask?$DO?$DN0?5?$CG?$CG?5detail_mask?$DMNU@ (0000)
00291860 004e:
	??_C@_0EO@GAMPAHAK@detail_function?$DO?$DN0?5?$CG?$CG?5detail_fun@ (0000)
002918B0 0038:
	??_C@_0DI@FGKEFDJL@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
002918E8 0011:
	??_C@_0BB@BMPHCOIP@local_parameters?$AA@ (0000)
002918FC 002f:
	??_C@_0CP@IHIFHMCH@cc0_error?4blue?5?$DO?$DN0?40f?5?$CG?$CG?5cc0_err@ (0000)
0029192C 002f:
	??_C@_0CP@KGDHFBB@cc0_error?4green?$DO?$DN0?40f?5?$CG?$CG?5cc0_err@ (0000)
0029195C 002d:
	??_C@_0CN@LKMEOKHH@cc0_error?4red?5?$DO?$DN0?40f?5?$CG?$CG?5cc0_erro@ (0000)
0029198C 0023:
	??_C@_0CD@OECGPNGC@cc1?4blue?5?$DO?$DN0?40f?5?$CG?$CG?5cc1?4blue?5?$DM?$DN1?4@ (0000)
002919B0 0023:
	??_C@_0CD@DFKMLKHD@cc1?4green?$DO?$DN0?40f?5?$CG?$CG?5cc1?4green?$DM?$DN1?4@ (0000)
002919D4 0021:
	??_C@_0CB@CPJFGMCD@cc1?4red?5?$DO?$DN0?40f?5?$CG?$CG?5cc1?4red?5?$DM?$DN1?40f@ (0000)
002919F8 0023:
	??_C@_0CD@EPNLIMOH@cc0?4blue?5?$DO?$DN0?40f?5?$CG?$CG?5cc0?4blue?5?$DM?$DN1?4@ (0000)
00291A1C 0023:
	??_C@_0CD@JOFBMLPG@cc0?4green?$DO?$DN0?40f?5?$CG?$CG?5cc0?4green?$DM?$DN1?4@ (0000)
00291A40 0021:
	??_C@_0CB@PMOEFNHH@cc0?4red?5?$DO?$DN0?40f?5?$CG?$CG?5cc0?4red?5?$DM?$DN1?40f@ (0000)
00291A68 0074:
	??_C@_0HE@IBEOFMMD@global_window_parameters?4fog?4atm@ (0000)
00291ADC 0039:
	??_C@_0DJ@MAOJHDPF@group?9?$DOactive_camouflage_transpa@ (0000)
00291B18 0030:
	??_C@_0DA@GOHJDLCN@local_parameters?9?$DOeffect?4source_@ (0000)
00291B48 0032:
	??_C@_0DC@CEHNCDIC@shader_type_is_valid_for_model?$CIs@ (0000)
00291B7C 003b:
	??_C@_0DL@NALNBEEB@?$CD?$CD?$CD?5ERROR?5model?5effect?5type?5?$CD?$CFd?5@ (0000)
00291BB8 0045:
	??_C@_0EF@FGOGCHIG@diffuse_change_color?4blue?5?$DO?$DN0?40f@ (0000)
00291C00 0045:
	??_C@_0EF@PPFPLJEH@diffuse_change_color?4green?$DO?$DN0?40f@ (0000)
00291C48 0043:
	??_C@_0ED@PEIKOPIA@diffuse_change_color?4red?5?$DO?$DN0?40f?5@ (0000)
00291C8C 003b:
	??_C@_0DL@BIOGNNCB@external_color?9?$DOblue?5?$DO?$DN0?40f?5?$CG?$CG?5e@ (0000)
00291CC8 003b:
	??_C@_0DL@HMDMJINO@external_color?9?$DOgreen?$DO?$DN0?40f?5?$CG?$CG?5e@ (0000)
00291D04 0039:
	??_C@_0DJ@CNNDHOMN@external_color?9?$DOred?5?$DO?$DN0?40f?5?$CG?$CG?5ex@ (0000)
00291D40 004b:
	??_C@_0EL@GNHHGHCF@self_illumination_color?4blue?5?$DO?$DN0@ (0000)
00291D90 004b:
	??_C@_0EL@HBGILNPJ@self_illumination_color?4green?$DO?$DN0@ (0000)
00291DE0 0049:
	??_C@_0EJ@FOGGPAPE@self_illumination_color?4red?5?$DO?$DN0?4@ (0000)
00291E2C 003d:
	??_C@_0DN@HPMCIMLL@shader_model?9?$DOmodel?4self_illumin@ (0000)
00291E6C 0038:
	??_C@_0DI@PCENFHCM@local_model_effect_type?$DN?$DN_render@ (0000)
00291EA4 0026:
	??_C@_0CG@IGIAPBFN@shader?9?$DObase?4type?$DN?$DN_shader_type_@ (0000)
0030CEFB 0001:
	_data_0030cefb (0000)
00465D68 00bf:
	_bss_00465d68 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
