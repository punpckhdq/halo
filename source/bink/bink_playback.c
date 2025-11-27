/*
BINK_PLAYBACK.C

symbols in this file:
001B53F0 0040:
	_code_001b53f0 (0000)
001B5430 0020:
	_bink_playback_active (0000)
001B5450 0020:
	_bink_playback_ui_rendering_inhibited (0000)
001B5470 0010:
	_bink_playback_in_progress (0000)
001B5480 00f0:
	_code_001b5480 (0000)
001B5570 0030:
	_code_001b5570 (0000)
001B55A0 01f0:
	_code_001b55a0 (0000)
001B5790 00c0:
	_code_001b5790 (0000)
001B5850 0010:
	_code_001b5850 (0000)
001B5860 0390:
	_code_001b5860 (0000)
001B5BF0 0010:
	_code_001b5bf0 (0000)
001B5C00 0050:
	_code_001b5c00 (0000)
001B5C50 0010:
	_code_001b5c50 (0000)
001B5C60 0020:
	_code_001b5c60 (0000)
001B5C80 0010:
	_code_001b5c80 (0000)
001B5C90 0030:
	_bink_playback_initialize (0000)
001B5CC0 0060:
	_code_001b5cc0 (0000)
001B5D20 0070:
	_code_001b5d20 (0000)
001B5D90 0070:
	_bink_playback_stop (0000)
001B5E00 0030:
	_bink_playback_dispose (0000)
001B5E30 00a0:
	_code_001b5e30 (0000)
001B5ED0 0060:
	_bink_playback_render (0000)
001B5F30 0430:
	_bink_playback_start (0000)
001B6360 0010:
	_bink_playback_update (0000)
002AA374 001e:
	??_C@_0BO@MFHOKAAA@bink_globals?4memory_pool_base?$AA@ (0000)
002AA394 002d:
	??_C@_0CN@BECKFBGA@bink_globals?4memory_pool_size?$DO?$DNs@ (0000)
002AA3C4 0010:
	??_C@_0BA@FPPHECDF@size_in_bytes?$DO0?$AA@ (0000)
002AA3D8 0047:
	??_C@_0EH@BGOHEHFB@alignment_in_bytes?$DO0?5?$CG?$CG?5?$CIalignme@ (0000)
002AA420 0024:
	??_C@_0CE@CFKHDMNJ@c?3?2halo?2SOURCE?2bink?2bink_playbac@ (0000)
002AA448 0040:
	??_C@_0EA@OJNDLELI@bink_globals?4memory_pool_offset?5@ (0000)
002AA488 002f:
	??_C@_0CP@EKHFIEHP@?$CD?$CD?$CD?5FATAL_ERROR?5bink?5needs?5more?5@ (0000)
002AA4B8 0053:
	??_C@_0FD@JEIGGOCE@?$CD?$CD?$CD?5FATAL_ERROR?5bink?5needs?5more?5@ (0000)
002AA50C 002a:
	??_C@_0CK@MEIMCONA@?$CB?$CCbink?5memory?5allocation?5should?5@ (0000)
002AA538 003a:
	??_C@_0DK@JIJLOKKD@?$CD?$CD?$CD?5FATAL_ERROR?5bink?5just?5confus@ (0000)
002AA574 003a:
	??_C@_0DK@JKNNFEPK@?$CD?$CD?$CD?5FATAL_ERROR?5bink?5just?5confus@ (0000)
002AA5B0 002d:
	??_C@_0CN@HECFNEKJ@SkippedFrames?$DN?$HMt?$CFd?5?$CI?$CFd?$CJ?$HMnSkipped@ (0000)
002AA5E0 00c5:
	??_C@_0MF@OOMIEAIL@FramesTime?$DN?$HMt?$CF?402f?$HMnFrameVideoDe@ (0000)
002AA6A8 0040:
	??_C@_0EA@EKIBIHMI@we?5released?5the?5texture?5cache?5bu@ (0000)
002AA6E8 005a:
	??_C@_0FK@BKFMFKML@?$CD?$CD?$CD?5ERROR?5bink?5failed?5to?5fucking@ (0000)
002AA744 0028:
	??_C@_0CI@EPJBNEGJ@?$CD?$CD?$CD?5ERROR?5failed?5to?5open?5bink?5fi@ (0000)
002AA76C 0012:
	??_C@_0BC@DLLDJJJE@end?5event_manager?$AA@ (0000)
002AA780 0014:
	??_C@_0BE@DCEEPBKH@begin?5event_manager?$AA@ (0000)
002AA794 0015:
	??_C@_0BF@DNOFFDBE@end?5XPhysicalProtect?$AA@ (0000)
002AA7AC 0017:
	??_C@_0BH@JFDFIOME@begin?5XPhysicalProtect?$AA@ (0000)
002AA7C4 000d:
	??_C@_0N@POAPBJK@end?5BinkOpen?$AA@ (0000)
002AA7D4 0022:
	??_C@_0CC@FBMLCGMA@?$CD?$CD?$CD?5ERROR?5no?5DirectSound?5for?5bin@ (0000)
002AA7F8 001c:
	??_C@_0BM@OJHAKIIN@end?5BinkSoundUseDirectSound?$AA@ (0000)
002AA814 001e:
	??_C@_0BO@ONDJOKIK@begin?5BinkSoundUseDirectSound?$AA@ (0000)
002AA834 000f:
	??_C@_0P@MDOKKFIJ@begin?5BinkOpen?$AA@ (0000)
002AA844 0035:
	??_C@_0DF@PFMDENHE@0?5?$DN?$DN?5?$CIbink_globals?4memory_pool_s@ (0000)
002AA87C 002b:
	??_C@_0CL@NNCIPKIH@there?5is?5already?5a?5bink?5movie?5be@ (0000)
0031693C 0005:
	_data_0031693c (0000)
004D2AF0 0164:
	_bss_004d2af0 (0000)
	_debug_bink (0084)
	_bink_globals (0088)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
