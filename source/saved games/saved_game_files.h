/*
SAVED_GAME_FILES.H

header included in hcex build.
*/

#ifndef __SAVED_GAME_FILES_H
#define __SAVED_GAME_FILES_H
#pragma once

/* ---------- constants */

enum
{
	_saved_game_file_type_player_profile = 0,
	_saved_game_file_type_play_list,
	NUMBER_OF_SAVED_GAME_FILE_TYPES
};

enum
{
	MAXIMUM_ENUMERATED_SAVED_GAME_FILES_ANY_TYPE_PER_MEMORY_UNIT = 100,
	MAXIMUM_SAVED_GAME_NAME_LENGTH = 128
};

enum
{
	_saved_game_file_type_bit0 = 0,
	_saved_game_file_type_bit1,
	_saved_game_file_type_bit2,
	_saved_game_file_type_bit3,
	SAVED_GAME_FILE_TYPE_MASK = MASK(_saved_game_file_type_bit3 + 1),
	SAVED_GAME_FILE_TYPE_SHIFT = 0,

	_unused_bit4 = 4,
	_unused_bit5,
	_unused_bit6,
	_unused_bit7,
	_memory_unit_index_bit8,
	_memory_unit_index_bit9,
	_memory_unit_index_bit10,
	_memory_unit_index_bit11,
	_memory_unit_index_bit12,
	_memory_unit_index_bit13,
	_memory_unit_index_bit14,
	_memory_unit_index_bit15,
	MEMORY_UNIT_INDEX_MASK = MASK(_memory_unit_index_bit15 - _memory_unit_index_bit8 + 1),
	MEMORY_UNIT_INDEX_SHIFT = 8,

	_enumerated_index_bit16 = 16,
	_enumerated_index_bit17,
	_enumerated_index_bit18,
	_enumerated_index_bit19,
	_enumerated_index_bit20,
	_enumerated_index_bit21,
	_enumerated_index_bit22,
	_enumerated_index_bit23,
	_enumerated_index_bit24,
	_enumerated_index_bit25,
	_enumerated_index_bit26,
	_enumerated_index_bit27,
	ENUMERATED_INDEX_MASK = MASK(_enumerated_index_bit27 - _enumerated_index_bit16 + 1),
	ENUMERATED_INDEX_SHIFT = 16,

	_unused_bit28 = 28,
	_unused_bit29,
	_default_profile_bit30,
	_profile_checksum_valid_bit31,
	NUMBER_OF_SAVED_GAME_FILE_INDEX_BITS
};

enum
{
	SAVED_GAME_FILE_BLOCK_SIZE = 512
};

/* ---------- macros */

#define SAVED_GAMES_DIRECTORY "z:\\saved"																/* fake name */
#define PLAYER_PROFILES_DIRECTORY SAVED_GAMES_DIRECTORY "\\player_profiles"								/* fake name */
#define DEFAULT_PLAYER_PROFILES_DIRECTORY PLAYER_PROFILES_DIRECTORY "\\default_profile"					/* fake name */
#define DEFAULT_PLAYER_PROFILE_PATH_FORMAT DEFAULT_PLAYER_PROFILES_DIRECTORY "\\%02d.sav"				/* fake name */
#define PLAYLISTS_DIRECTORY SAVED_GAMES_DIRECTORY "\\playlists"											/* fake name */
#define DEFAULT_PLAYLISTS_DIRECTORY PLAYLISTS_DIRECTORY "\\default_playlist"							/* fake name */
#define DEFAULT_PLAYLIST_DIRECTORY_FORMAT DEFAULT_PLAYLISTS_DIRECTORY "\\%02d"							/* fake name */
#define DEFAULT_PLAYLIST_PATH_FORMAT DEFAULT_PLAYLIST_DIRECTORY_FORMAT "\\" PLAYLIST_PROFILE_FILE_NAME	/* fake name */
#define RECORDINGS_DIRECTORY SAVED_GAMES_DIRECTORY "\\recordings"										/* fake name */
#define LAST_RECORDING_DIRECTORY RECORDINGS_DIRECTORY "\\last_recording"								/* fake name */
#define PLAYER_PROFILE_FILE_NAME "blam.sav"																/* fake name */
#define PLAYLIST_PROFILE_FILE_NAME "blam.lst"															/* fake name */
#define DEFAULT_MULTIPLAYER_GAME_SETTING_NAMES_TAG_NAME "ui\\default_multiplayer_game_setting_names"	/* fake name */

/* ---------- structures */

/* ---------- prototypes/SAVED_GAME_FILES.C */

void saved_game_files_initialize(void);
void saved_game_files_dispose(void);
void saved_game_files_enumerate_available_to_local_player_index(short player_index, word saved_game_file_type, word *number_of_profiles, long *player_profile_indices, boolean include_default_profiles);
wchar_t *saved_game_file_get_display_name(long index);
long create_enumerated_saved_game_file(word saved_game_file_type, short local_player_index, wchar_t *display_name);
boolean delete_enumerated_saved_game_file(long index);
boolean saved_game_file_open(struct file_reference *saved_game_file, long index);
boolean saved_game_file_close(struct file_reference *saved_game_file, long index);
word saved_game_file_get_type(long index);
void saved_game_files_delete_all_custom_profiles(void);
void saved_game_files_notify_memory_units_changed(void);
void saved_game_file_get_useable_untitled_profile_name(wchar_t *display_name);
boolean synchronize_metadata_display_name_with_profile_name(long index, wchar_t *game_display_name);
boolean saved_game_files_take_mutex(void);
void saved_game_files_release_mutex(void);
boolean saved_game_file_get_path_to_enclosing_directory(long index, char *full_path);
short saved_game_perform_file_system_checks(void);
boolean saved_game_file_name_unique(wchar_t const *name);
void saved_game_file_remember_player1_last_used_profile_directory(char const *directory_path);
boolean saved_game_file_retrieve_player1_last_used_profile_directory(char *directory_path);
void saved_game_file_remember_last_used_multiplayer_variant_directory(char const *directory_path);
boolean saved_game_file_retrieve_last_used_multiplayer_variant_directory(char *directory_path);
long saved_game_file_find_profile_index_for_directory_path(char const *directory_path, short saved_game_file_type);
void saved_game_file_remember_last_used_multiplayer_map(char const *map_name);
boolean saved_game_file_retrieve_last_used_multiplayer_map(char *map_name);
void saved_game_file_generate_checksum(void const *buffer, word buffer_size, XCALCSIG_SIGNATURE *checksum);
void enumerate_memory_units_test(void);

/* ---------- prototypes/UI_WIDGET_EVENT_HANDLER_FUNCTIONS.C */

void reset_last_player1_profile_index(void);

/* ---------- globals */

/* ---------- public code */

#endif // __SAVED_GAME_FILES_H
