/*
GAME_STATISTICS.H
*/

#ifndef __GAME_STATISTICS_H
#define __GAME_STATISTICS_H
#pragma once

/* ---------- prototypes/GAME_STATISTICS.C */

void game_statistics_record_damage(long object_index, real damage, long owner_player_index, long owner_object_index, short owner_team_index);
void game_statistics_record_kill(long object_index, long owner_player_index, long owner_object_index, short owner_team_index);

#endif // __GAME_STATISTICS_H
