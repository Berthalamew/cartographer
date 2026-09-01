#include "stdafx.h"
#include "game_results.h"

#include "players.h"

#include "game/game.h"
#include "game/game_engine.h"
#include "game/game_options.h"
#include "interface/screens/screen_postgame_statistics.h"
#include "networking/tools/network_webstats_submit.h"
#include "networking/network_event.h"
#include "objects/objects.h"
#include "simulation/simulation.h"

/* constants */

static const real_point3d g_game_results_invalid_player_location{ 0, 0, 500.f };

/* prototypes */

static c_game_results* game_results_get(void);
static s_game_results_globals* game_results_globals_get(void);
static s_integer_statistic_definition* game_results_player_statistic_definition_get();
static s_integer_statistic_definition* game_results_damage_statistic_definition_get();
static s_integer_statistic_definition* game_results_pvp_statistic_definition_get();
static s_integer_statistic_definition* game_results_medal_statistic_definition_get();

/* public code */

void game_results_initialize_for_new_map(
	void)
{
	event(_event_status, "game:results: game results initialized for new map");
	
	game_results_clear();

	if (game_engine_running())
	{
		c_game_results* game_results = game_results_get();

		s_game_options const* game_options = game_options_get();

		ASSERT(!game_results->initialized);

		game_results->game_instance = game_options->game_instance;
		game_results->game_variant = game_options->game_variant;
		game_results->map_id = game_options->map_id;
		csmemcpy(game_results->scenario_path, &game_options->scenario_path, sizeof(game_results->scenario_path));

		for (int32 player_index = 0; player_index<NUMBEROF(game_options->players); ++player_index)
		{
			game_player_options const* option_player = &game_options->players[player_index];
			s_game_results_player_data* result_player = &game_results->players[player_index];

			result_player->exists = option_player->valid;

			if (option_player->valid)
			{
				csmemcpy(&result_player->identifier, &option_player->player_identifier, sizeof(result_player->identifier));
				result_player->player_configuration = option_player->properties;
				result_player->machine_index = (int8)game_results->get_machine_index(&option_player->machine_identifier);

				if (result_player->machine_index==NONE)
				{
					result_player->machine_index = (int8)game_results->add_machine(&option_player->machine_identifier);
				}

				ASSERT(result_player->machine_index!=NONE);

				event(
					_event_message,
					"game:results: player #%d id %s clan %s team %d ",
					player_index,
					player_identifier_get_string(&result_player->identifier),
					clan_identifier_get_string(&result_player->player_configuration.clan_identifiers),
					result_player->player_configuration.team_index
				);
			}
		}

		game_results->is_matchmade_game = game_results->game_variant.game_engine_flags.test(_game_engine_teams_bit);

		if (game_results->is_matchmade_game)
		{
			for (int32 player_index = 0; player_index < NUMBEROF(game_options->players); ++player_index)
			{
				s_game_results_player_data const* result_player = &game_results->players[player_index];
				
				if (result_player->exists)
				{
					e_game_team team_index = (e_game_team)result_player->player_configuration.team_index;
				
					if (team_index!=_game_team_observer)
					{
						ASSERT(team_index>=0 && team_index<k_maximum_teams);

						if (!game_results->teams[team_index].exists)
						{
							game_results->teams[team_index].exists = true;
							game_results->teams[team_index].standing = NONE;
							game_results->teams[team_index].score = 0;
							clan_identifier_clear(&game_results->teams[team_index].clan);
							game_results->teams[team_index].experience = NONE;
							game_results->teams[team_index].skill = NONE;
						}
					}
				}
			}

			for (int32 team_index = 0; team_index < NUMBEROF(game_options->players); ++team_index)
			{
				s_game_results_team_data const* result_team_data = &game_results->teams[team_index];
				
				if (result_team_data->exists)
				{
					event(
						_event_message,
						"game:results: team #%d clan %s exp %d skill %d",
						team_index,
						clan_identifier_get_string(&result_team_data->clan),
						result_team_data->experience,
						result_team_data->skill
					);
				}
			}
		}

		s_transport_unique_identifier* local_unique_identifier = transport_security_get_local_unique_identifier();
		bool local_machine_is_host = simulation_get_machine_is_host((s_machine_identifier const *)local_unique_identifier);

		for (int32 machine_index = 0; machine_index <NUMBEROF(game_results->machines); ++machine_index)
		{
			s_game_results_machine_data *result_machine_data = &game_results->machines[machine_index];

			if (result_machine_data->exists)
			{
				bool machine_is_host = simulation_get_machine_is_host(&result_machine_data->machine);

				result_machine_data->host = machine_is_host;
				result_machine_data->initial_host = machine_is_host;

				if (!local_machine_is_host && !machine_is_host)
				{
					result_machine_data->connected_to_host = false;
				}
				else if (!local_machine_is_host || !machine_is_host)
				{
					result_machine_data->connected_to_host = simulation_get_machine_connectivity(&result_machine_data->machine);
				}
				else
				{
					result_machine_data->connected_to_host = true;
				}
			}
		}

		game_results->initialized = true;
		
		ASSERT(game_results->validate());

		event(_event_status, "game:results: game results initialized from game options");

		if (game_is_authoritative())
		{
			game_results_start_recording();
		}
	}

	return;
}

void game_results_dispose_from_old_map(
	void)
{
	event(_event_status, "game:results: game results disposed from old map");

	if (game_engine_running())
	{
		s_game_results_globals* game_results_globals = game_results_globals_get();
		s_game_results* game_results = game_results_get();
		
		ASSERT(game_results->initialized);



		ASSERT(!game_results_get_game_updating());

		if (game_results_globals->recording)
		{
			game_results_stop_recording();
		}

		if (!game_results->finalized)
		{
			event(_event_warning, "game:results: game_results_dispose_from_old_map() found game results were not finalized, finalizing them");
			
			game_results_finalize();
		}
	}

	return;
}


bool game_results_get_game_finalized(
	void)
{
	c_game_results* game_results = game_results_get();
	
	ASSERT(game_results->initialized);
	
	return game_results->finalized;
}

void game_results_clear(
	void)
{
	c_game_results* game_results = game_results_get();
	csmemset(game_results, 0, sizeof(*game_results));

	event(_event_message, "game:results: game results cleared");

	return;
}

void game_results_finalize(
	void)
{
	s_game_options const* game_options;
	c_game_results* game_results = game_results_get();

	ASSERT(game_results->initialized);
	ASSERT(!game_results->finalized);

	game_options = game_options_get();
	
	if (game_results->game_instance != game_options->game_instance)
	{
		event(_event_warning, "game:results: game results game instance (%I64d) does not match game options game instance (%I64d)", game_results->game_instance, game_options->game_instance);
	}

	if (csmemcmp(&game_results->game_variant, &game_options->game_variant, sizeof(game_results->game_variant)))
	{
		event(_event_warning, "game:results: game results game variant does not match game options game variant");
	}

	if (game_results->map_id != game_options->map_id)
	{
		event(_event_warning, "game:results: game results map id (%d) does not match game options map id (%d)", game_results->map_id, game_options->map_id);
	}

	game_results->finalized = true;
	
	event(_event_message, "game:results: game results finalized");

	if (game_is_multiplayer())
	{
		int32 local_machine_index = game_results->get_local_machine_index();
		int32 host_machine_index = game_results->get_host_machine_index();

		if (host_machine_index!=NONE && local_machine_index==host_machine_index && !game_results->unreliable)
		{
			event(_event_message, "game:results: game_results_finalize() game results submiting to webstats");

			network_webstats_submit(game_results);
		}
	}

	screen_postgame_statistics_finalize();

	for (int32 absolute_player_index= 0; absolute_player_index<NUMBEROF(game_results->players); ++absolute_player_index)
	{
		s_game_results_player_data const* result_player_data = &game_results->players[absolute_player_index];
		s_game_results_player_statistics const* result_player_statistics = &game_results->statistics.player_statistics[absolute_player_index];

		if (result_player_data->exists)
		{
			if (player_identifier_is_guest(&result_player_data->identifier))
			{
				event(
					_event_message,
					"game:results: player %d: id %s: standing %d: kills %d: guest",
					absolute_player_index,
					player_identifier_get_string(&result_player_data->identifier),
					result_player_data->player_place,
					result_player_statistics->statistics[_game_results_player_statistic_kills]
				);
			}
			else
			{
				event(
					_event_message,
					"game:results: player %d: id %s: name %ls: clan %s: standing %d: kills %d: ",
					absolute_player_index,
					player_identifier_get_string(&result_player_data->identifier),
					clan_identifier_get_string(&result_player_data->player_configuration.clan_identifiers),
					result_player_data->player_place,
					result_player_statistics->statistics[_game_results_player_statistic_kills]
				);
			}
		}
	}

	for (int32 absolute_player_index = 0; absolute_player_index < NUMBEROF(game_results->players); ++absolute_player_index)
	{
		s_game_results_player_data const* result_player_data = &game_results->players[absolute_player_index];

		if (result_player_data->exists)
		{
			if (VALID_INDEX(result_player_data->machine_index, NUMBEROF(game_results->machines)))
			{
				s_game_results_machine_data const* result_machine_data = &game_results->machines[result_player_data->machine_index];
				s_game_results_machine_bandwidth_event const* result_machine_bandwitdth_event = &game_results->machine_bandwidth_events[result_player_data->machine_index];

				if (result_machine_data->exists)
				{
					event(
						_event_message,
						"game:results: player %d: id %s: machine #%d '%s': bandwidth events n/d/i/c/l %d/%d/%d/%d/%d",
						absolute_player_index,
						player_identifier_get_string(&result_player_data->identifier),
						result_player_data->machine_index,
						transport_unique_identifier_get_string((s_transport_unique_identifier const*)&result_machine_data->machine),
						result_machine_bandwitdth_event->bandwidth_events[0],
						result_machine_bandwitdth_event->bandwidth_events[1],
						result_machine_bandwitdth_event->bandwidth_events[2],
						result_machine_bandwitdth_event->bandwidth_events[3],
						result_machine_bandwitdth_event->bandwidth_events[4]
					);
				}
			}
		}
	}

	if (game_results->is_matchmade_game)
	{
		for (int32 team_index= 0; team_index<NUMBEROF(game_results->players); ++team_index)
		{
			s_game_results_team_data const* result_team_data = &game_results->teams[team_index];
			
			event(
				_event_message,
				"game:results: team #%d clan %s exp %d skill %d standing %d score %d",
				team_index,
				clan_identifier_get_string(&result_team_data->clan),
				result_team_data->experience,
				result_team_data->skill,
				result_team_data->standing,
				result_team_data->score
			);
		}
	}

	return;
}

void game_results_start_recording(
	void)
{
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	event(_event_message, "game:results: game results start recording");

	ASSERT(!game_results_get_game_updating());
	ASSERT(!game_results_get_game_recording());
	ASSERT(game_results->initialized);

	game_results_globals->recording = true;
	game_results_globals->recording_paused = false;

	game_results_notify_player_indices_changed();
	game_results_notify_active_teams_changed();

	return;
}

void game_results_stop_recording(void)
{
	event(_event_message, "game:results: game results stop recording");

	game_results_globals_get()->recording = false;

	return;
}

void game_results_set_recording_pause(bool pause)
{
	game_results_globals_get()->recording_paused = pause;
	return;
}

bool game_results_get_game_recording(void)
{
	return game_results_globals_get()->recording;
}

bool game_results_get_game_updating(void)
{
	return game_results_globals_get()->updating;
}

void __cdecl game_results_notify_player_indices_changed(
	void)
{
	INVOKE(0x6962F, 0x0, game_results_notify_player_indices_changed);

	return;
}

void game_results_notify_active_teams_changed(
	void)
{
	if (game_results_get_game_recording() && !game_results_get_game_finalized())
	{
		c_game_results* game_results = game_results_get();

		ASSERT(game_results->validate());

		for (int32 team_index = 0; team_index<k_maximum_teams; ++team_index)
		{
			s_game_results_team_data* result_team_data = &game_results->teams[team_index];
			bool team_ever_active = game_engine_is_team_ever_active((e_game_team)team_index);

			if (team_ever_active && !result_team_data->exists)
			{
				result_team_data->exists = true;
			}
		}

		ASSERT(game_results->validate());
	}

	return;
}

void game_results_start_updating(void)
{
	game_results_globals_get()->updating = true;
	return;
}

void game_results_stop_updating(void)
{
	game_results_globals_get()->updating = false;
	return;
}

void __cdecl game_results_update(void)
{
	INVOKE(0x692CC, 0x68CE4, game_results_update);
	return;
}

int32 game_results_get_recording_statistic(int32 player_index, int32 team_index, e_game_results_player_statistic statistic)
{
	//return INVOKE(0x66D3C, 0, game_results_get_recording_statistic, player_index, team_index, statistic);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording)
	{
		if (player_index != NONE)
			result = game_results->statistics.player_statistics[player_index].statistics[statistic].value & SHRT_MAX;
		if (team_index != NONE)
			result = game_results->statistics.team_statistics[team_index].statistics[statistic].value & SHRT_MAX;
	}

	return result;
}

int32 game_results_get_finalized_statistic(int32 player_index, int32 team_index, e_game_results_player_statistic statistic)
{
	//return INVOKE(0x66D88, 0, game_results_get_finalized_statistic, player_index, team_index, statistic);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized && game_results->finalized)
	{
		if (player_index != NONE)
			result = game_results->statistics.player_statistics[player_index].statistics[statistic].value & SHRT_MAX;
		if (team_index != NONE)
			result = game_results->statistics.team_statistics[team_index].statistics[statistic].value & SHRT_MAX;
	}

	return result;
}

int32 game_results_get_finalized_damage_statistic(int32 player_index, e_game_results_damage_statistic statistic, e_damage_reporting_type damage_type)
{
	//return INVOKE(0x66DDD, 0, game_results_get_finalized_damage_statistic, player_index, statistic, damage_type);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized && game_results->finalized)
	{
		result = game_results->statistics.player_statistics[player_index].damage[damage_type].statistics[statistic].value & SHRT_MAX;
	}

	return result;
}

int32 game_results_get_finalized_medal_statistic(int32 player_index, e_game_results_medal_statistic medal)
{
	//return INVOKE(0x66E15, 0, game_results_get_finalized_medal_statistic, player_index, medal);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized && game_results->finalized)
	{
		result = game_results->statistics.player_statistics[player_index].medal_statistics[medal].value & SHRT_MAX;
	}

	return result;
}

int32 game_results_get_finalized_pvp_statistic(int32 player_index, int32 vs_player_index, e_game_results_player_vs_player_statistic statistic)
{
	//return INVOKE(0x66E46, 0, game_results_get_finalized_pvp_statistic, player_index, vs_player_index, statistic);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized && game_results->finalized)
	{
		result = game_results->statistics.pvp_statistics[player_index][vs_player_index].statistic[statistic].value & SHRT_MAX;
	}

	return result;
}

int32 game_results_get_finalized_player_score(int32 player_index)
{
	//return INVOKE(0x66F43, 0, game_results_get_finalized_player_score, player_index);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized &&
		game_results->finalized &&
		game_results->players[player_index].exists)
	{
		result = game_results->players[player_index].score;
	}

	return result;
}

int32 game_results_get_finalized_player_place(int32 player_index)
{
	//return INVOKE(0x699BD, 0, game_results_get_finalized_player_place, player_index);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized &&
		game_results->finalized &&
		game_results->players[player_index].exists)
	{
		result = game_results->players[player_index].player_place;
	}

	return result;
}

s_player_configuration* game_results_get_finalized_player_configuration(int32 player_index)
{
	//return INVOKE(0x66FDC, 0, game_results_get_finalized_player_configuration, player_index);
	s_player_configuration* result = nullptr;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized &&
		game_results->finalized &&
		game_results->players[player_index].exists)
	{
		result = &game_results->players[player_index].player_configuration;
	}

	return result;
}

s_player_identifier* game_results_get_finalized_player_identifier(int32 player_index)
{
	//return INVOKE(0x67012, 0, game_results_get_finalized_player_unknown_02, player_index);
	
	s_player_identifier* result = NULL;

	c_game_results* game_results = game_results_get();

	if (game_results->initialized &&
		game_results->finalized &&
		game_results->players[player_index].exists)
	{
		result = &game_results->players[player_index].identifier;
	}

	return result;
}

e_game_team game_results_get_finalized_player_team(int32 player_index)
{
	//return INVOKE(0x67042, 0, game_results_get_finalized_player_team, player_index);
	e_game_team result = _game_team_observer;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized &&
		game_results->finalized &&
		game_results->players[player_index].exists)
	{
		result = (e_game_team)game_results->players[player_index].player_configuration.team_index;
	}

	return result;
}

void game_results_get_finalized_player_profile_traits(
	int32 player_index,
	s_player_appearance* appearance)
{
	//INVOKE(0x6706D, 0, game_results_get_finalized_player_profile_traits, player_index, profile_traits);
	c_game_results* game_results = game_results_get();

	if (game_results->initialized && player_index != NONE && game_results->players[player_index].exists)
	{
		*appearance = game_results->players[player_index].player_configuration.appearance;
	}
	else
	{
		player_appearance_initialize(appearance);
	}

	return;
}

bool game_results_get_player_position(
	real_point3d* position,
	int32 player_index)
{
	//usercall 0x6928E
	player_datum const* player = player_get(player_index);

	if (player->unit_index != NONE && player->dead_unit_index != NONE)
	{
		object_get_origin(player->unit_index, position, false);
		return true;
	}

	return false;
}

int32 game_results_get_finalized_team_score(e_game_team team)
{
	//return INVOKE(0x66F74, 0, game_results_get_finalized_team_score, team);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized && VALID_INDEX(team, k_game_multiplayer_team_count) && game_results->finalized)
	{
		result = game_results->teams[team].score;
	}

	return result;
}

int32 game_results_get_finalized_team_place(e_game_team team)
{
	//return INVOKE(0x66FA8, 0, game_results_get_finalized_team_place, team);
	int32 result = NONE;
	c_game_results* game_results = game_results_get();

	if (game_results->initialized && VALID_INDEX(team, k_game_multiplayer_team_count) && game_results->finalized)
	{
		result = game_results->teams[team].standing;
	}

	return result;
}

void game_results_set_statistic(int32 player_index, e_game_team team, e_game_results_player_statistic statistic, int32 value)
{
	//INVOKE(0x66CEE, 0, game_results_set_statistic, player_index, team, statistic, value);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		s_integer_statistic_definition* definition = &game_results_player_statistic_definition_get()[statistic];

		int32 clean_value = PIN(value, definition->minimum_value, definition->maximum_value);

		if (player_index != NONE)
		{
			uint16* stat = (uint16*)&game_results->statistics.player_statistics[player_index].statistics[statistic];
			*stat = ((*stat & ~SHRT_MAX) | ((clean_value) & SHRT_MAX));
		}

		if (team != _game_team_observer)
		{
			uint16* stat = (uint16*)&game_results->statistics.team_statistics[team].statistics[statistic];
			*stat = ((*stat & ~SHRT_MAX) | ((clean_value)&SHRT_MAX));
		}
	}
}

void game_results_increment_statistic(int32 player_index, e_game_team team, e_game_results_player_statistic statistic, int32 amount)
{
	//INVOKE(0x66BD2, 0, game_results_increment_statistic, player_index, team, statistic, amount);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		s_integer_statistic_definition* definition = &game_results_player_statistic_definition_get()[statistic];

		if (player_index != NONE)
		{
			int32 new_value = game_results->statistics.player_statistics[player_index].statistics[statistic].value + amount;

			int32 clean_value = PIN(
				new_value, 
				definition->minimum_value, 
				definition->maximum_value);

			uint16* stat = (uint16*)&game_results->statistics.player_statistics[player_index].statistics[statistic];
			*stat = ((*stat & ~SHRT_MAX) | ((clean_value)&SHRT_MAX));
		}

		if (team != _game_team_observer)
		{
			int32 new_value = game_results->statistics.team_statistics[team].statistics[statistic].value + amount;

			int32 clean_value = PIN(
				new_value,
				definition->minimum_value,
				definition->maximum_value);

			uint16* stat = (uint16*)&game_results->statistics.team_statistics[team].statistics[statistic];
			*stat = ((*stat & ~SHRT_MAX) | ((clean_value)&SHRT_MAX));
		}
	}
}

void game_results_increment_pvp_statistic(int32 player_index, int32 vs_player_index, e_game_results_player_vs_player_statistic statistic, int32 amount)
{
	//INVOKE(0x670C2, 0, game_results_increment_pvp_statistic, player_index, vs_player_index, statistic, amount);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		s_integer_statistic_definition* definition = &game_results_pvp_statistic_definition_get()[statistic];

		int32 new_value = game_results->statistics.pvp_statistics[player_index][vs_player_index].statistic[statistic].value + amount;

		int32 clean_value = PIN(
			new_value,
			definition->maximum_value,
			definition->maximum_value
		);

		uint16* stat = (uint16*)&game_results->statistics.pvp_statistics[player_index][vs_player_index].statistic[statistic];
		*stat = ((*stat & ~SHRT_MAX) | ((clean_value)&SHRT_MAX));
	}
}

void game_results_increment_damage_statistic(int32 player_index, e_game_results_damage_statistic statistic, e_damage_reporting_type damage_type, int32 amount)
{
	//INVOKE(0x67149, 0, game_results_increment_damage_statistic, player_index, statistic, damage_type, amount);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		s_integer_statistic_definition* definition = &game_results_damage_statistic_definition_get()[statistic];

		int32 new_value = game_results->statistics.player_statistics[player_index].damage[damage_type].statistics[statistic].value + amount;

		int32 clean_value = PIN(
			new_value,
			definition->minimum_value,
			definition->maximum_value
		);

		uint16* stat = (uint16*)&game_results->statistics.player_statistics[player_index].damage[damage_type].statistics[statistic];
		*stat = ((*stat & ~SHRT_MAX) | ((clean_value)&SHRT_MAX));
	}
}

void game_results_increment_medal_statistic(int32 player_index, e_game_results_medal_statistic medal, int32 amount)
{
	//INVOKE(0x6738C, 0, game_results_increment_medal_statistic, player_index, medal, amount);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		s_integer_statistic_definition* definition = &game_results_medal_statistic_definition_get()[medal];

		int32 new_value = game_results->statistics.player_statistics[player_index].medal_statistics[medal].value + amount;

		int32 clean_value = PIN(
			new_value,
			definition->minimum_value,
			definition->maximum_value
		);

		uint16* stat = (uint16*)&game_results->statistics.player_statistics[player_index].medal_statistics[medal];
		*stat = ((*stat & ~SHRT_MAX) | ((clean_value)&SHRT_MAX));
	}
}

void game_results_insert_event(const s_game_results_event* event)
{
	//INVOKE(0x67411, 0, game_results_insert_event, event);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		int32 event_index = game_results_globals->next_game_event_index;
		csmemcpy(&game_results->game_events[event_index], event, sizeof(s_game_results_event));
		game_results_globals->next_game_event_index = (++event_index) % 1000;
	}
}

void game_results_insert_kill_event(int16 player_index, int16 killed_player_index, int8 damage_reporting_info)
{
	//INVOKE(0x69A27, 0, game_results_insert_kill_event, player_index, killed_player_index, damage_reporting_info);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		s_game_results_event event{};

		event.type = _game_results_event_type_kill;
		event.player_references[0] = (int8)player_index;
		event.player_references[1] = (int8)killed_player_index;
		event.time = system_seconds() - game_results->start_time;
		event.data.kill_event.damage_reporting_type = (int32)damage_reporting_info;

		real_point3d player_position = g_game_results_invalid_player_location;
		real_point3d killed_player_position = g_game_results_invalid_player_location;

		if (game_results_get_player_position(&player_position, player_index) &&
			game_results_get_player_position(&killed_player_position, killed_player_index))
		{
			event.data.kill_event.killer_position = player_position;
			event.data.kill_event.killed_position = killed_player_position;

			game_results_insert_event(&event);
		}
	}
}

void game_results_insert_score_event(int16 player_index, int32 score_type, datum weapon_index)
{
	//INVOKE(0x69AF1, 0, game_results_insert_score_event, player_index, score_type, weapon_index);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		s_game_results_event event{};

		event.type = _game_results_event_type_score;
		event.player_references[0] = (int8)player_index;
		event.player_references[1] = NONE;
		event.time = system_seconds() - game_results->start_time;
		event.data.score_event.score_type = score_type;
		event.data.score_event.weapon_index = weapon_index;

		real_point3d player_position = g_game_results_invalid_player_location;

		if (game_results_get_player_position(&player_position, player_index))
		{
			event.data.score_event.scorer_position = player_position;

			game_results_insert_event(&event);
		}
	}
}

void game_results_insert_carry_event(int16 player_index, datum weapon_index, int32 carry_type)
{
	//INVOKE(0x69C5F, 0, game_results_insert_carry_event, player_index, weapon_index, carry_type);
	c_game_results* game_results = game_results_get();
	s_game_results_globals* game_results_globals = game_results_globals_get();

	if (game_results_globals->recording && !game_results_globals->recording_paused)
	{
		uint32 current_time = system_seconds();
		if (game_results_globals->game_event_timer - current_time > k_game_results_event_cooldown)
		{
			s_game_results_event event{};

			event.type = _game_results_event_type_carry;
			event.player_references[0] = (int8)player_index;
			event.player_references[1] = NONE;
			event.time = system_seconds() - game_results->start_time;
			event.data.carry_event.weapon_index = weapon_index;
			event.data.carry_event.carry_type = carry_type;

			real_point3d player_position = g_game_results_invalid_player_location;

			if (game_results_get_player_position(&player_position, player_index))
			{
				event.data.carry_event.carrier_position = player_position;

				game_results_insert_event(&event);
			}

			game_results_globals->game_event_timer = current_time;
		}
	}
}

void game_results_populate_incremental_update(
	s_game_results_incremental* update)
{
	s_game_results &game_results = *game_results_get();

	csmemset(update, 0, sizeof(*update));
	
	update->started = game_results.started;
	if (game_results.started)
	{
		update->start_time = game_results.start_time;
	}

	update->finalized = game_results.finished;
	if (game_results.finished)
	{
		update->finish_time = game_results.finish_time;
	}

	update->initialized = game_results.finalized;
	csmemcpy(update->players, game_results.players, sizeof(update->players));
	csmemcpy(&update->statistics, &game_results.statistics, sizeof(update->statistics));
	csmemcpy(update->teams, game_results.teams, sizeof(update->teams));
	csmemcpy(update->machines, game_results.machines, sizeof(update->machines));

	return;
}

void __cdecl game_results_calculate_incremental_update(
	struct s_game_results_incremental* previous_state,
	struct s_game_results_incremental* current_state,
	struct s_game_results_incremental_update* incremental_update)
{
	INVOKE(0x67CE3, 0x0, game_results_calculate_incremental_update, previous_state, current_state, incremental_update);
	return;
}

int32 c_game_results::get_machine_index(s_machine_identifier const* machine_identifier) const
{
	int32 machine_index = NONE;

	for (int32 test_machine_index = 0; test_machine_index<NUMBEROF(machines) && machine_index==NONE; ++test_machine_index)
	{
		if (machines[test_machine_index].exists)
		{
			if (!csmemcmp(&machines[test_machine_index], machine_identifier, sizeof(*machine_identifier)))
			{
				machine_index = test_machine_index;
			}
		}
	}

	return machine_index;
}

int32 c_game_results::get_host_machine_index(
	void) const
{
	int32 host_machine_index = NONE;

	for (int32 test_machine_index = 0; test_machine_index<NUMBEROF(machines) && host_machine_index==NONE; ++test_machine_index)
	{
		if (machines[test_machine_index].exists)
		{
			if (machines[test_machine_index].host)
			{
				host_machine_index = test_machine_index;
			}
		}
	}

	return host_machine_index;
}

int32 c_game_results::get_local_machine_index(
	void) const
{
	return get_machine_index((const s_machine_identifier*)transport_security_get_local_unique_identifier());
}


int32 c_game_results::add_machine(
	s_machine_identifier const* machine_identifier)
{
	ASSERT(get_machine_index(machine_identifier)==NONE);

	int32 machine_index = 0;

	for (int32 test_machine_index = 0; test_machine_index<NUMBEROF(machines); ++test_machine_index)
	{
		if (!machines[test_machine_index].exists)
		{
			machine_index = test_machine_index;
			break;
		}
	}

	csmemset(&machines[machine_index], 0, sizeof(machines[machine_index]));
	machines[machine_index].exists = true;
	machines[machine_index].machine = *machine_identifier;

	return machine_index;
}

bool c_game_results::validate(
	void) const
{
	bool valid = initialized;
	uint32 valid_machine_mask = 0;

	for (int32 player_index = 0; player_index<NUMBEROF(players); ++player_index)
	{
		s_game_results_player_data const* player_data = &players[player_index];
		if (player_data->exists)
		{
			int32 machine_index = player_data->machine_index;
			if (machine_index!=NONE)
			{
				SET_BIT(valid_machine_mask, machine_index, true);
			}
		}
	}

	int32 host_count = 0;

	for (int32 machine_index = 0; machine_index<NUMBEROF(machines); ++machine_index)
	{
		s_game_results_machine_data const* machine_data = &machines[machine_index];

		valid = valid && machine_data->exists == TEST_BIT(valid_machine_mask, machine_index);

		if (machine_data->exists && machine_data->host)
		{
			++host_count;
		}
	}

	return valid && host_count<=1;
}

/* private code */

static c_game_results* game_results_get(
	void)
{
	return Memory::GetAddress<c_game_results*>(0x4B1C90, 0x4DC3C0);
}

static s_game_results_globals* game_results_globals_get(
	void)
{
	return Memory::GetAddress<s_game_results_globals*>(0x4B1C80, 0x4DC3B0);
}

static s_integer_statistic_definition* game_results_player_statistic_definition_get(
	void)
{
	return Memory::GetAddress<s_integer_statistic_definition*>(0x412CF8, 0x3B62D0);
}

static s_integer_statistic_definition* game_results_damage_statistic_definition_get(
	void)
{
	return Memory::GetAddress<s_integer_statistic_definition*>(0x412FC8, 0x3B65A0);
}

static s_integer_statistic_definition* game_results_pvp_statistic_definition_get(
	void)
{
	return Memory::GetAddress<s_integer_statistic_definition*>(0x413038, 0x3B6610);
}

static s_integer_statistic_definition* game_results_medal_statistic_definition_get(
	void)
{
	return Memory::GetAddress<s_integer_statistic_definition*>(0x413058, 0x3B6630);
}
