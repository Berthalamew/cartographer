#pragma once
#include "user_interface_group_widget.h"
#include "user_interface_screen_widget_definition.h"

#include "game/game_allegiance.h"
#include "game/players.h"

/* enums */

enum e_player_widget_representation_flags
{
	_player_widget_name_bit= 0,
	_player_widget_appearance_bit,
	_player_widget_player_team_bit,
	_player_widget_team_name_bit,
	_player_widget_player_custom_name_bit,
	_player_widget_fake_player_bit,
	_player_widget_player_rank_bit,
	_player_widget_player_is_observing_bit,
	_player_widget_bungie_role_bit,
	_player_widget_change_color_bit,
	k_player_widget_flag_count
};

/* classes */

class c_player_widget : protected c_group_widget
{
public:
	// c_player_widget virtual functions

	virtual ~c_player_widget(void) = default;
	virtual void setup_children(void) override;

protected:
	int32 m_screen_player_index;
	struct s_player_block_reference* m_tag_block;
};
ASSERT_STRUCT_SIZE(c_player_widget, 0x78);


class c_player_widget_representation
{
public:
	c_player_widget_representation(void);

	void set_player_name(wchar_t const* configuration);
	void set_appearance(struct s_player_appearance const* appearance);
	void set_player_team_name(string_id team_name);
	void set_player_custom_name(wchar_t const* player_custom_name);
	void set_player_team(e_game_team team);
	void set_player_is_observer(bool observer);
	void set_fake_player(bool fake);
	void set_player_rank(int32 rank);
	void set_user_role(int32 role);
	void set_change_color(real_rgb_color const* color);

private:
	c_flags<e_player_widget_representation_flags, uint32, k_player_widget_flag_count> m_flags;
	wchar_t const* m_player_name;
	wchar_t m_player_custom_name[32];
	s_player_appearance m_appearance;
	string_id m_team_name;
	e_game_team m_player_team;
	bool m_fake_player;
	bool m_player_is_observing;
	int16 m_player_rank;
	uint8 gap_62[2];
	int32 m_bungie_role;
	real_rgb_color m_change_color;
};
ASSERT_STRUCT_SIZE(c_player_widget_representation, 0x74);
