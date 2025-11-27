/*
NETWORK_MESSAGES.C

symbols in this file:
0011AE90 0010:
	_initialize_network_game_packets (0000)
0011AEA0 0060:
	_network_event (0000)
0011AF00 0050:
	_code_0011af00 (0000)
0011AF50 05e0:
	_create_network_game_message (0000)
0011B530 00a0:
	_decode_network_game_message (0000)
00284040 001c:
	??_C@_0BM@EBOFJEK@network_game_messages_group?$AA@ (0000)
0028405C 0032:
	??_C@_0DC@BOGDHCMH@message_client_graceful_game_exi@ (0000)
00284090 0028:
	??_C@_0CI@EEPBNHAG@message_client_switch_to_pregame@ (0000)
002840B8 0035:
	??_C@_0DF@EMNEGNHK@message_client_remove_player_req@ (0000)
002840F0 0032:
	??_C@_0DC@MCHEBCFB@message_server_graceful_game_exi@ (0000)
00284124 0028:
	??_C@_0CI@LMKDEEMM@message_server_switch_to_pregame@ (0000)
0028414C 0024:
	??_C@_0CE@ILDHAAGE@message_client_join_new_host_pac@ (0000)
00284170 0030:
	??_C@_0DA@NAFCJNBH@message_client_host_crashed_cry_@ (0000)
002841A0 0033:
	??_C@_0DD@DPFJDDJL@message_client_remove_player_req@ (0000)
002841D4 0030:
	??_C@_0DA@HJJAAOFP@message_client_add_player_reques@ (0000)
00284204 0022:
	??_C@_0CC@JNOAENJF@message_client_game_update_packe@ (0000)
00284228 001d:
	??_C@_0BN@NJKENIOP@message_client_loaded_packet?$AA@ (0000)
00284248 0020:
	??_C@_0CA@NHACOKNP@message_server_game_over_packet?$AA@ (0000)
00284268 002b:
	??_C@_0CL@ONEMLOHO@message_server_remove_player_ing@ (0000)
00284294 0028:
	??_C@_0CI@NCMFMOGB@message_server_add_player_ingame@ (0000)
002842BC 0022:
	??_C@_0CC@EKNLMOAI@message_server_game_update_packe@ (0000)
002842E0 002f:
	??_C@_0CP@ICNCBHBJ@message_client_map_is_precached_@ (0000)
00284310 0031:
	??_C@_0DB@MBNEDKDF@message_client_graceful_game_exi@ (0000)
00284344 0029:
	??_C@_0CJ@DBOKMPL@message_client_game_start_reques@ (0000)
00284370 002e:
	??_C@_0CO@ONAIPAAF@message_client_player_settings_r@ (0000)
002843A0 0027:
	??_C@_0CH@DFKBPHDI@message_client_settings_request_@ (0000)
002843C8 0034:
	??_C@_0DE@BOCJLBIG@message_client_remove_player_req@ (0000)
002843FC 0031:
	??_C@_0DB@ICGGHOBC@message_client_add_player_reques@ (0000)
00284430 0028:
	??_C@_0CI@NJAKJDJG@message_client_join_game_request@ (0000)
00284458 0031:
	??_C@_0DB@LJGDBL@message_server_graceful_game_exi@ (0000)
0028448C 0021:
	??_C@_0CB@GBFOJNFG@message_server_begin_game_packet@ (0000)
002844B0 002a:
	??_C@_0CK@NGOKIJFI@message_server_postgame_keep_ali@ (0000)
002844DC 0029:
	??_C@_0CJ@OOOJECPL@message_server_pregame_keep_aliv@ (0000)
00284508 0028:
	??_C@_0CI@DDGLHHCI@message_server_pregame_countdown@ (0000)
00284530 002b:
	??_C@_0CL@KGHPJAMF@message_server_game_settings_upd@ (0000)
0028455C 0027:
	??_C@_0CH@PPCEIPKP@message_server_machine_rejected_@ (0000)
00284584 0027:
	??_C@_0CH@MHOBBBHN@message_server_machine_accepted_@ (0000)
002845AC 001b:
	??_C@_0BL@OBJNBJFL@message_server_pong_packet?$AA@ (0000)
002845C8 0025:
	??_C@_0CF@LPLCJKKH@message_server_game_advertise_pa@ (0000)
002845F0 001b:
	??_C@_0BL@KJGLBKLM@message_client_ping_packet?$AA@ (0000)
0028460C 002c:
	??_C@_0CM@DBKCBLPI@message_client_broadcast_game_se@ (0000)
00284638 002d:
	??_C@_0CN@IGMHDBNJ@c?3?2halo?2SOURCE?2networking?2networ@ (0000)
00284668 0057:
	??_C@_0FH@LDAHLJDB@message_struct?5?$CG?$CG?5encoded_messag@ (0000)
002846C0 0025:
	??_C@_0CF@FAGKPKLA@encode_network_game_message?$CI?$CJ?5fa@ (0000)
002846E8 0018:
	??_C@_0BI@HDENCOBE@create_message?$CI?$CJ?5failed?$AA@ (0000)
00284700 002c:
	??_C@_0CM@HANECPBB@unknown?5network?5game?5message?5str@ (0000)
00284730 0048:
	??_C@_0EI@LDIICBJE@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284778 003e:
	??_C@_0DO@FGHCKKNA@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
002847B8 004b:
	??_C@_0EL@FPPCOKOP@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284808 0048:
	??_C@_0EI@GOMGGMKH@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284850 003e:
	??_C@_0DO@IBEJCJEN@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284890 003a:
	??_C@_0DK@FANDEEMI@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
002848D0 0046:
	??_C@_0EG@BNKKIHMJ@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284918 0049:
	??_C@_0EJ@DEANEKBI@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284968 0046:
	??_C@_0EG@ENKLANMK@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
002849B0 0038:
	??_C@_0DI@IKHDCGFA@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
002849E8 0033:
	??_C@_0DD@IMEOHJNL@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284A1C 0036:
	??_C@_0DG@HDEEJKMJ@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284A58 0041:
	??_C@_0EB@OBJOGCPO@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284A9C 003e:
	??_C@_0DO@HENBBIFD@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284ADC 0038:
	??_C@_0DI@FLGIACGI@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284B18 0045:
	??_C@_0EF@LBGBNCAB@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284B60 0047:
	??_C@_0EH@FNFCMEAG@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284BA8 003f:
	??_C@_0DP@KKICJIGK@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284BE8 0044:
	??_C@_0EE@CKGGABKC@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284C2C 003d:
	??_C@_0DN@DENDNKBM@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284C70 004a:
	??_C@_0EK@EEGLJJMH@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284CC0 0047:
	??_C@_0EH@CMMPIMHI@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284D08 003e:
	??_C@_0DO@DMPHOMMG@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284D48 0040:
	??_C@_0EA@HDLMPJOG@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284D88 0047:
	??_C@_0EH@BOGDDOGJ@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284DD0 0037:
	??_C@_0DH@NCBFHEAI@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284E08 003f:
	??_C@_0DP@DPJHGIHE@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284E48 003e:
	??_C@_0DO@CMCEHFEG@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284E88 0041:
	??_C@_0EB@IBENFKED@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284ECC 003d:
	??_C@_0DN@OFBIEKEC@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284F0C 003d:
	??_C@_0DN@MAJGGCDN@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284F4C 0031:
	??_C@_0DB@MDKHEKHO@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284F80 003b:
	??_C@_0DL@IBPEMGOC@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284FBC 0031:
	??_C@_0DB@GADNMNKL@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00284FF0 0042:
	??_C@_0EC@OBBNPBFB@message_struct_size?$DN?$DNsizeof?$CImess@ (0000)
00285034 0025:
	??_C@_0CF@MENKCHHP@decode_network_game_message?$CI?$CJ?5fa@ (0000)
00285060 00a4:
	??_C@_0KE@BCMKEKI@message_struct?5?$CG?$CG?5encoded_messag@ (0000)
0030AA68 0844:
	_data_0030aa68 (0000)
004566F0 0604:
	_bss_004566f0 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
