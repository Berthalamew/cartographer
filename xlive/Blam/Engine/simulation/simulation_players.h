#pragma once
#include "game/game.h"
#include "game/player_control.h"
#include "game/players.h"
#include "networking/network_game_definitions.h"

/* enums */

enum e_simulation_player_type
{
	_simulation_player_type_local_authoritative = 0,
	_simulation_player_type_local_predicted,
	_simulation_player_type_local_zombie,
	_simulation_player_type_remote_synchronous,
	_simulation_player_type_remote_predicted,
	_simulation_player_type_remote_replicated,
	k_simulation_player_type_count,
};

enum e_simulation_player_update_type
{
	_simulation_player_update_type_left_game = 0,
	_simulation_player_update_type_swap,
	_simulation_player_update_type_remove,
	_simulation_player_update_type_added,
	_simulation_player_update_type_configuration,
	k_simulation_player_update_type_count
};

/* structures */

struct simulation_player_update
{
	int32 player_index;
	s_player_identifier player_identifier;
	int32 update_type;
	s_machine_identifier machine_identifier;
	int32 user_index;
	int32 controller_index;
	bool player_left_game;
	int8 pad[3];
	s_player_configuration properties;
	int32 swap_player_index;
	s_player_identifier swap_player_identifier;
};
ASSERT_STRUCT_SIZE(simulation_player_update, 0xB4);

struct s_player_collection_player
{
	s_player_identifier identifier;
	bool left_game;
	int32 left_game_time;
	s_machine_identifier machine_identifier;
	int32 user_index;
	e_controller_index controller_index;
	s_player_configuration configuration_data;
};
ASSERT_STRUCT_SIZE(s_player_collection_player, 0xA4);

struct s_player_collection
{
	uint32 player_valid_mask;
	s_player_collection_player players[k_maximum_players];
};
ASSERT_STRUCT_SIZE(s_player_collection, 0xA44);

/* classes */

#pragma pack(push, 1)
class c_simulation_player
{
public:
	void set_active(bool active);

	void destroy(void);
	void handle_local_input(struct player_action const* action);

	bool exists(
		void) const
	{
		return m_player_index != NONE;
	}

	bool is_local(
		void) const
	{
		ASSERT(exists());
		ASSERT(m_player_type>=0 && m_player_type<k_simulation_player_type_count);

		return
			m_player_type== _simulation_player_type_local_authoritative ||
			m_player_type== _simulation_player_type_local_predicted;
	}

	bool active(
		void) const
	{
		return m_active;
	}

	bool pending_deletion(
		void) const
	{
		ASSERT(exists());

		return m_pending_deletion;
	}

	int32 get_player_index(
		void) const
	{
		return m_player_datum_index;
	}

	void get_identifier(
		struct s_player_identifier* player_identifier) const
	{
		ASSERT(exists());
		ASSERT(player_identifier);
		*player_identifier = m_player_identifier;

		return;
	}

private:
	int32 m_player_index;
	datum m_player_datum_index;
	e_simulation_player_type m_player_type;
	s_player_identifier m_player_identifier;
	s_machine_identifier m_player_machine_identifier;
	int16 pad;
	class c_simulation_world* m_world;
	bool m_pending_deletion;
	bool m_active;
	int16 pad_1;
	uint32 m_current_action_time;
	player_action m_current_action;
};
#pragma pack(pop)
ASSERT_STRUCT_SIZE(c_simulation_player, 0x88);

/* prototypes */

void simulation_players_apply_patches(void);

void simulation_player_collection_clear(struct s_player_collection* collection);

bool __cdecl simulation_players_apply_update(struct simulation_player_update* player_update);

void __cdecl simulation_player_joined_game(datum player_index);

void __cdecl simulation_player_left_game(datum player_index);
