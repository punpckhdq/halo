/*
PLAYER_PROFILE.C

symbols in this file:
001B07E0 0060:
	_player_profiles_dispose (0000)
001B0840 0020:
	_player_profiles_enumerate_available_to_local_player_index (0000)
001B0860 0030:
	_player_profile_delete (0000)
001B0890 0130:
	_player_profile_get_from_path (0000)
001B09C0 0010:
	_player_profile_number_of_available_primary_colors (0000)
001B09D0 0090:
	_player_profile_get_rgb_color (0000)
001B0A60 0310:
	_player_profile_get_highest_completed_solo_level (0000)
001B0D70 0010:
	_player_profile_get_enclosing_directory_path (0000)
001B0D80 00b0:
	_code_001b0d80 (0000)
001B0E30 0280:
	_code_001b0e30 (0000)
001B10B0 0160:
	_code_001b10b0 (0000)
001B1210 01d0:
	_player_profile_new (0000)
001B13E0 0060:
	_player_profile_get (0000)
001B1440 0050:
	_player_profile_get_argb_color (0000)
001B1490 0020:
	_player_profile_get_random_good_color (0000)
001B14B0 0020:
	_player_profile_get_random_color (0000)
001B14D0 0120:
	_code_001b14d0 (0000)
001B15F0 00a0:
	_code_001b15f0 (0000)
001B1690 0020:
	_player_profiles_initialize (0000)
001B16B0 0040:
	_player_profile_save (0000)
001B16F0 00c0:
	_player_profile_save_last_level_played (0000)
001B17B0 00e0:
	_player_profile_save_level_completed (0000)
002A8430 003c:
	??_C@_0DM@NLGMBPHE@waiting?5for?5asynchronous?5player?5@ (0000)
002A846C 0037:
	??_C@_0DH@HAKHJMPK@player_profile_delete?$CI?$CJ?5failed?5?$CI@ (0000)
002A84A4 0023:
	??_C@_0CD@MKCOKJEP@failed?5to?5open?5player?5profile?5fi@ (0000)
002A84C8 001e:
	??_C@_0BO@NCKKNDPE@failed?5to?5read?5player?5profile?$AA@ (0000)
002A84E8 0027:
	??_C@_0CH@JNHLCLHF@checksum?5failed?5on?5player?5profil@ (0000)
002A8510 0015:
	??_C@_0BF@CBGBCFBJ@full_path?5?$CG?$CG?5profile?$AA@ (0000)
002A8528 002c:
	??_C@_0CM@FCBBDNNL@c?3?2halo?2SOURCE?2saved?5games?2playe@ (0000)
002A8554 001f:
	??_C@_0BP@MEGJCLGO@profile?5?$CG?$CG?5level?5?$CG?$CG?5difficulty?$AA@ (0000)
002A8574 0030:
	??_C@_0DA@HDHACNBF@unknown?5default?5profile?5configur@ (0000)
002A85A4 003e:
	??_C@_0DO@LAANFIGE@?$CIprofile?5?$CB?$DN?5NULL?$CJ?5?$CG?$CG?5?$CIi?$DO?$DN0?$CJ?5?$CG?$CG?5?$CI@ (0000)
002A85E8 0050:
	??_C@_0FA@LJKGPDPC@failed?5to?5get?5saved?5game?5files?5m@ (0000)
002A8638 0028:
	??_C@_0CI@FKNBKFEC@failed?5to?5read?5player?5profile?5fr@ (0000)
002A8660 004e:
	??_C@_0EO@GPPPPIDI@checksum?5failed?5on?5player?5profil@ (0000)
002A86B0 0038:
	??_C@_0DI@OJDDKHHL@waiting?5for?5asynchronous?5player?5@ (0000)
002A86E8 0019:
	??_C@_0BJ@PCHLPGBN@end?5player?5profile?5write?$AA@ (0000)
002A8704 002e:
	??_C@_0CO@BDMJPAMG@metadata?5name?5may?5not?5match?5game@ (0000)
002A8734 0027:
	??_C@_0CH@JHKEOGPG@failed?5to?5write?5player?5profile?5t@ (0000)
002A875C 001b:
	??_C@_0BL@IELAHCC@begin?5player?5profile?5write?$AA@ (0000)
002A8778 002c:
	??_C@_0CM@MOJFCEID@failed?5to?5open?5newly?5created?5pla@ (0000)
002A87A4 0032:
	??_C@_0DC@FAJPEHGL@failed?5to?5initialize?5newly?5creat@ (0000)
002A87D8 003e:
	??_C@_0DO@JBICLLBJ@?$CD?$CD?$CD?5DEBUG?5unlocking?5all?5solo?5lev@ (0000)
002A8818 0041:
	??_C@_0EB@MNDMAHGB@failed?5to?5create?1update?5default?5@ (0000)
002A885C 0031:
	??_C@_0DB@HMAPCDAL@failed?5to?5close?5default?5player?5p@ (0000)
002A8890 0032:
	??_C@_0DC@MFJJMMHK@z?3?2saved?2player_profiles?2default@ (0000)
002A88C4 0035:
	??_C@_0DF@LHIOELOG@?$CIlevel?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIlevel?$DMNUMBER_OF_S@ (0000)
002A8900 0062:
	??_C@_0GC@HGEBHOGG@failed?5to?5save?5player?8s?5current?5@ (0000)
002A8968 007d:
	??_C@_0HN@FLNBFKEC@?$CIlevel?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIlevel?$DMNUMBER_OF_S@ (0000)
00316880 0048:
	_data_00316880 (0000)
004D27E8 006c:
	_player_profile_globals (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
