/*
ACTOR_LOOKING.C

symbols in this file:
00015E80 0070:
	_actor_look_secondary_stop (0000)
00015EF0 0130:
	_actor_look_compute_prop_interest (0000)
00016020 0050:
	_code_00016020 (0000)
00016070 0370:
	_actor_look_secondary (0000)
000163E0 0080:
	_code_000163e0 (0000)
00016460 00f0:
	_code_00016460 (0000)
00016550 00b0:
	_actor_looking_test_validity (0000)
00016600 0260:
	_code_00016600 (0000)
00016860 0160:
	_code_00016860 (0000)
000169C0 0260:
	_code_000169c0 (0000)
00016C20 0050:
	_valid_real_normal2d (0000)
00016C70 0660:
	_code_00016c70 (0000)
000172D0 0210:
	_code_000172d0 (0000)
000174E0 0170:
	_code_000174e0 (0000)
00017650 1270:
	_actor_look_update (0000)
000188C0 0080:
	_actor_look_affect_movement (0000)
00244C74 0070:
	_global_secondary_look_priorities (0000)
	_global_secondary_look_times (0038)
00244CE4 000e:
	??_C@_0O@FNONJPCB@?$CFs?3?5look?9stop?$AA@ (0000)
00244CF4 0004:
	__real@3fe66666 (0000)
00244CF8 0012:
	??_C@_0BC@CBOPKOHG@?$CFs?3?5look?5?$CFs?5?$CFs?5?$CFd?$AA@ (0000)
00244D0C 0009:
	??_C@_08IILCFMOF@scripted?$AA@ (0000)
00244D18 0007:
	??_C@_06NCEIAKB@danger?$AA@ (0000)
00244D20 0007:
	??_C@_06GJOIPFFF@damage?$AA@ (0000)
00244D28 000c:
	??_C@_0M@KFECPMLC@combat?9prop?$AA@ (0000)
00244D34 000f:
	??_C@_0P@HANCNCFH@comm?9direction?$AA@ (0000)
00244D44 000a:
	??_C@_09OLJCJFBL@comm?9prop?$AA@ (0000)
00244D50 000e:
	??_C@_0O@BOCADBDG@shooting?9prop?$AA@ (0000)
00244D60 0009:
	??_C@_08NDPBJKMG@new?9prop?$AA@ (0000)
00244D6C 0007:
	??_C@_06LDFNIMBB@impact?$AA@ (0000)
00244D74 000c:
	??_C@_0M@JOHOFPJB@moving?9prop?$AA@ (0000)
00244D80 000c:
	??_C@_0M@PINLHHEI@environment?$AA@ (0000)
00244D8C 0010:
	??_C@_0BA@MGHLNDAM@override?9facing?$AA@ (0000)
00244D9C 0009:
	??_C@_08PNBPAFID@override?$AA@ (0000)
00244DA8 000d:
	??_C@_0N@OPBBNFGJ@stop?9and?9aim?$AA@ (0000)
00244DB8 000d:
	??_C@_0N@FFMCKHMJ@turn?9and?9aim?$AA@ (0000)
00244DC8 0004:
	??_C@_03MKHEFFMA@aim?$AA@ (0000)
00244DCC 0009:
	??_C@_08IECGFEAB@idle?9aim?$AA@ (0000)
00244DD8 000a:
	??_C@_09CPPFOLKN@idle?9look?$AA@ (0000)
00244DE4 0008:
	??_C@_07DLHCIBDH@default?$AA@ (0000)
00244DEC 0037:
	??_C@_0DH@BKMNBGNP@?$CItype?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CItype?5?$DM?5NUMBER_OF@ (0000)
00244E24 0022:
	??_C@_0CC@OBFKKCBI@c?3?2halo?2SOURCE?2ai?2actor_looking?4@ (0000)
00244E48 0004:
	__real@3ada740e (0000)
00244E50 0049:
	??_C@_0EJ@PDHFHBNF@?$CFs?3?5logic?5error?5in?5actor?5looking@ (0000)
00244E9C 0008:
	??_C@_07FJPMPBHO@looking?$AA@ (0000)
00244EA4 0007:
	??_C@_06CPNACJHN@aiming?$AA@ (0000)
00244EAC 0007:
	??_C@_06LJBECLPP@facing?$AA@ (0000)
00244EB4 000e:
	??_C@_0O@LDGGHEKC@result_vector?$AA@ (0000)
00244EC4 0023:
	??_C@_0CD@JLCDLMPD@denormalized?5?$CFf?3?5?$DMerror?$DO?5?$CItype?5?$CF@ (0000)
00244EE8 0021:
	??_C@_0CB@ILCNAOOH@denormalized?5?$CFf?3?5object?5?$CI0x?$CF08X?$CJ@ (0000)
00244F0C 001f:
	??_C@_0BP@JMCBPAFA@denormalized?5?$CFf?3?5danger?5?$CInone?$CJ?$AA@ (0000)
00244F2C 0023:
	??_C@_0CD@BJDFLJJM@denormalized?5?$CFf?3?5danger?5?$CI?$CFf?5?$CFf?5?$CF@ (0000)
00244F50 0023:
	??_C@_0CD@MPCIMEDM@denormalized?5?$CFf?3?5vector?5?$CI?$CFf?5?$CFf?5?$CF@ (0000)
00244F74 0022:
	??_C@_0CC@KKBEICMF@denormalized?5?$CFf?3?5point?5?$CI?$CFf?5?$CFf?5?$CFf@ (0000)
00244F98 003a:
	??_C@_0DK@KHHBJFIN@denormalized?5?$CFf?3?5prop?50x?$CF08X?$CFs?5?$CI@ (0000)
00244FD4 000b:
	??_C@_0L@CGKLOAKK@?5?$CIinvalid?$CJ?$AA@ (0000)
00244FE0 001f:
	??_C@_0BP@IAJLBFML@denormalized?5?$CFf?3?5target?5?$CInone?$CJ?$AA@ (0000)
00245000 0036:
	??_C@_0DG@LJDBLOA@denormalized?5?$CFf?3?5target?5?$CIprop?50x@ (0000)
00245038 0042:
	??_C@_0EC@ONEINDFA@denormalized?5?$CFf?3?5target?5type?5?$CFd?5@ (0000)
0024507C 003c:
	??_C@_0DM@KFLCBPJA@denormalized?5?$CFf?3?5target?5type?5?$CFd?5@ (0000)
002450B8 0032:
	??_C@_0DC@MBKOGPLI@denormalized?5?$CFf?3?5?$CFsmoving?5?$CIp?$CFf?5?$CF@ (0000)
002450EC 0005:
	??_C@_04LKOGMCLF@not?5?$AA@ (0000)
002450F4 000a:
	??_C@_09HPKLLIJF@direction?$AA@ (0000)
00245100 000e:
	??_C@_0O@DNCJFFDC@specification?$AA@ (0000)
00245110 001e:
	??_C@_0BO@ICLEICLJ@?$CGactor?9?$DOoutput?4looking_vector?$AA@ (0000)
00245130 001d:
	??_C@_0BN@MLJELDAF@?$CGactor?9?$DOoutput?4aiming_vector?$AA@ (0000)
00245150 001d:
	??_C@_0BN@BNGLBJLG@?$CGactor?9?$DOoutput?4facing_vector?$AA@ (0000)
00245170 0004:
	__real@3f666666 (0000)
00245174 0027:
	??_C@_0CH@MHPDGGDP@?$CFs?3?5assert_valid_real_normal2d?$CI?$CF@ (0000)
0024519C 002f:
	??_C@_0CP@OAIIPIIE@?$CIreal_vector2d?5?$CK?$CJ?5?$CGactor?9?$DOoutput@ (0000)
002451CC 0013:
	??_C@_0BD@CADKODCD@?$CGidle_minor_vector?$AA@ (0000)
002451E0 0024:
	??_C@_0CE@LCIPMIDM@actor?9?$DOcontrol?4idle_minor_timer?5@ (0000)
00245204 0013:
	??_C@_0BD@JGOPOGFP@?$CGidle_major_vector?$AA@ (0000)
00245218 004b:
	??_C@_0EL@IKADNPKK@?$CBactor?9?$DOcontrol?4idle_major_activ@ (0000)
00245264 0016:
	??_C@_0BG@GOAPBADG@?$CFs?3?5look?5timer?5expire?$AA@ (0000)
0024527C 0012:
	??_C@_0BC@OJIPDKMD@?$CGsecondary_vector?$AA@ (0000)
00245290 0010:
	??_C@_0BA@GFAAOIFJ@?$CGprimary_vector?$AA@ (0000)
002452A0 0027:
	??_C@_0CH@JLLNHPPI@?$CGactor?9?$DOcontrol?4desired_looking_@ (0000)
002452C8 0026:
	??_C@_0CG@KOFPIDJD@?$CGactor?9?$DOcontrol?4desired_aiming_v@ (0000)
002452F0 0026:
	??_C@_0CG@HIKACJCA@?$CGactor?9?$DOcontrol?4desired_facing_v@ (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
