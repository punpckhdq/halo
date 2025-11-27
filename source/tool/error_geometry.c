/*
ERROR_GEOMETRY.C

symbols in this file:
000F3550 0050:
	_code_000f3550 (0000)
000F35A0 0040:
	_error_geometry_initialize (0000)
000F35E0 0020:
	_error_geometry_dispose (0000)
000F3600 00a0:
	_error_geometry_set_name (0000)
000F36A0 01c0:
	_error_geometry_line (0000)
000F3860 0200:
	_error_geometry_triangle (0000)
000F3A60 01f0:
	_error_geometry_polygon (0000)
000F3C50 02e0:
	_error_geometry_polygon_list (0000)
000F3F30 0240:
	_error_geometry_polygon_mesh__textured_with_no_import_scale (0000)
000F4170 0080:
	_error_geometry_comment (0000)
000F41F0 0200:
	_error_geometry_rectangle3d (0000)
000F43F0 0170:
	_error_geometry_bounded_line (0000)
000F4560 0260:
	_error_geometry_bounded_triangle (0000)
000F47C0 01c0:
	_error_geometry_bounded_polygon (0000)
000F4980 03f0:
	_error_geometry_set_transform (0000)
000F4D70 00c0:
	_error_geometry_point (0000)
000F4E30 00d0:
	_error_geometry_bounded_point (0000)
00279D38 0013:
	??_C@_0BD@LDNMMFHP@?$CDVRML?5V1?40?5ascii?6?6?$AA@ (0000)
00279D4C 001a:
	??_C@_0BK@IICJAJPK@error_geometry_file?$DN?$DNNULL?$AA@ (0000)
00279D68 0025:
	??_C@_0CF@CMPOCLNK@c?3?2halo?2SOURCE?2tool?2error_geomet@ (0000)
00279D90 0005:
	??_C@_04CPPJJJDH@?4wrl?$AA@ (0000)
00279D98 0003:
	??_C@_02KGHIDFGE@?$HN?6?$AA@ (0000)
00279D9C 0028:
	??_C@_0CI@GNMPMJMJ@?7IndexedLineSet?5?$HL?5coordIndex?$FL0?01@ (0000)
00279DC8 0045:
	??_C@_0EF@LBJBEPME@?7Material?5?$HL?5diffuseColor?$FL?$CFf?5?$CFf?5?$CF@ (0000)
00279E10 0027:
	??_C@_0CH@FLODMBAF@?7MaterialBinding?5?$HL?5value?5PER_VER@ (0000)
00279E38 002c:
	??_C@_0CM@FJEGEIKC@?7Coordinate3?5?$HL?5point?$FL?$CFf?5?$CFf?5?$CFf?0?5?$CF@ (0000)
00279E64 000d:
	??_C@_0N@FJEIPKFG@Separator?6?$HL?6?$AA@ (0000)
00279E74 0003:
	??_C@_02JCBCEJAP@p1?$AA@ (0000)
00279E78 0003:
	??_C@_02ILAJHIEO@p0?$AA@ (0000)
00279E7C 002a:
	??_C@_0CK@FJOLCHNE@?7IndexedFaceSet?5?$HL?5coordIndex?$FL0?01@ (0000)
00279EA8 0037:
	??_C@_0DH@HHENMBBE@?7Material?5?$HL?5diffuseColor?$FL?$CFf?5?$CFf?5?$CF@ (0000)
00279EE0 0025:
	??_C@_0CF@NJNCFAMI@?7MaterialBinding?5?$HL?5value?5PER_FAC@ (0000)
00279F08 0036:
	??_C@_0DG@CMJGEIKG@?7Coordinate3?5?$HL?5point?$FL?$CFf?5?$CFf?5?$CFf?0?5?$CF@ (0000)
00279F40 0003:
	??_C@_02LJDPBKMM@p2?$AA@ (0000)
00279F44 0007:
	??_C@_06MFKIBLBI@?91?$FN?5?$HN?6?$AA@ (0000)
00279F4C 0004:
	??_C@_03DPLIHHLN@?$CFd?0?$AA@ (0000)
00279F50 001e:
	??_C@_0BO@OJFBMEMD@?7IndexedFaceSet?5?$HL?5coordIndex?$FL?$AA@ (0000)
00279F70 000b:
	??_C@_0L@EOBPCNFM@?$CFf?5?$CFf?5?$CFf?$CFs?$AA@ (0000)
00279F7C 0005:
	??_C@_04PPKKLPCP@?$FN?5?$HN?6?$AA@ (0000)
00279F84 0003:
	??_C@_02KEGNLNML@?0?5?$AA@ (0000)
00279F88 0016:
	??_C@_0BG@HEHBNALL@?7Coordinate3?5?$HL?5point?$FL?$AA@ (0000)
00279FA0 0007:
	??_C@_06GPGIDCJA@points?$AA@ (0000)
00279FA8 000f:
	??_C@_0P@LFAAJJAC@point_count?$DO?$DN0?$AA@ (0000)
00279FB8 000a:
	??_C@_09MJKGDFBK@?7?7?$FN?6?7?$HN?6?$HN?6?$AA@ (0000)
00279FC4 000e:
	??_C@_0O@JEOEGOMK@?$CFd?0?$CFd?0?$CFd?0?91?0?5?$AA@ (0000)
00279FD4 0004:
	??_C@_03HNKAOPKP@?7?7?7?$AA@ (0000)
00279FD8 0025:
	??_C@_0CF@LDPPOFCI@?7IndexedFaceSet?6?7?$HL?6?7?7coordIndex?6@ (0000)
0027A000 001b:
	??_C@_0BL@KIFPEEHN@?7?7?$FN?6?7?7transparency?$FL?$CFf?$FN?6?7?$HN?6?$AA@ (0000)
0027A01C 000e:
	??_C@_0O@NOAOBPFG@?7?7?7?$CFf?5?$CFf?5?$CFf?0?5?$AA@ (0000)
0027A02C 0021:
	??_C@_0CB@HPDIJFFH@?7Material?6?7?$HL?6?7?7diffuseColor?6?7?7?$FL?6@ (0000)
0027A050 0029:
	??_C@_0CJ@LJKCEKIK@?7MaterialBinding?6?7?$HL?6?7?7value?5PER_@ (0000)
0027A07C 0008:
	??_C@_07DBGECBOF@?7?7?$FN?6?7?$HN?6?$AA@ (0000)
0027A084 000e:
	??_C@_0O@LBGFNDHO@?7?7?7?$CFf?5?$CFf?5?$CFf?0?6?$AA@ (0000)
0027A094 001d:
	??_C@_0BN@PFBNMPA@?7Coordinate3?6?7?$HL?6?7?7point?6?7?7?$FL?6?$AA@ (0000)
0027A0B4 000d:
	??_C@_0N@CLJKDHIM@point_counts?$AA@ (0000)
0027A0C4 0011:
	??_C@_0BB@MKNONCHH@polygon_count?$DO?$DN0?$AA@ (0000)
0027A0D8 0010:
	??_C@_0BA@BKEBAMII@?$CFd?0?$CFd?0?$CFd?0?$CFd?0?91?0?$AA@ (0000)
0027A0E8 000b:
	??_C@_0L@DNALKCHJ@?7?7?7?$CFf?5?$CFf?0?6?$AA@ (0000)
0027A0F4 0023:
	??_C@_0CD@JGFFBFNH@?7TextureCoordinate?6?7?$HL?6?7?7point?6?7?7@ (0000)
0027A118 000a:
	??_C@_09FIPCNGBA@texcoords?$AA@ (0000)
0027A124 0009:
	??_C@_08NBDFKFMI@height?$DO0?$AA@ (0000)
0027A130 0008:
	??_C@_07LCHAJLOP@width?$DO0?$AA@ (0000)
0027A138 0002:
	??_C@_01IPJKGB@?$CD?$AA@ (0000)
0027A13C 0020:
	??_C@_0CA@HIMELCND@?$CFs?3?5assert_valid_real_matrix4x3?$AA@ (0000)
0027A15C 003f:
	??_C@_0DP@FKDOHHDA@?$CFs?5had?5a?5forward?5?$CI?$CFf?0?$CFf?0?$CFf?$CJ?5not?5@ (0000)
0027A19C 003c:
	??_C@_0DM@IIGMNBAA@?$CFs?5had?5a?5up?5?$CI?$CFf?0?$CFf?0?$CFf?$CJ?5not?5perpe@ (0000)
0027A1D8 0041:
	??_C@_0EB@IEMCMOPH@?$CFs?5had?5a?5forward?5?$CI?$CFf?0?$CFf?0?$CFf?$CJ?5not?5@ (0000)
0027A21C 0021:
	??_C@_0CB@GEBHBJIP@?$CFs?5had?5a?5bad?5position?5?$CI?$CFf?0?$CFf?0?$CFf?$CJ@ (0000)
0027A240 001b:
	??_C@_0BL@PFADAFNL@?$CFs?5had?5a?5bad?5up?5?$CI?$CFf?0?$CFf?0?$CFf?$CJ?$AA@ (0000)
0027A25C 001d:
	??_C@_0BN@IHEDFLPN@?$CFs?5had?5a?5bad?5left?5?$CI?$CFf?0?$CFf?0?$CFf?$CJ?$AA@ (0000)
0027A27C 0020:
	??_C@_0CA@PHDHMACA@?$CFs?5had?5a?5bad?5forward?5?$CI?$CFf?0?$CFf?0?$CFf?$CJ?$AA@ (0000)
0027A29C 0016:
	??_C@_0BG@NNFMPBFP@?$CFs?5had?5a?5bad?5scale?5?$CFf?$AA@ (0000)
00307850 0074:
	_data_00307850 (0000)
004561B4 0004:
	_bss_004561b4 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
