#include "stdafx.h"
#include "life_cycle_manager.h"



e_game_life_cycle __cdecl get_game_life_cycle()
{
	c_game_life_cycle_manager* life_cycle_manager = c_game_life_cycle_manager::get();

	if (life_cycle_manager->initialized)
		return life_cycle_manager->life_cycle_state;

	return _life_cycle_none;
}

void c_game_life_cycle_handler::initialize(void* life_cycle_manager, e_game_life_cycle life_cycle, bool a3)
{
	this->life_cycle_manager = life_cycle_manager;
	this->life_cycle = life_cycle;
	this->field_C = a3;
	static_cast<c_game_life_cycle_manager*>(this->life_cycle_manager)->life_cycle_handlers[this->life_cycle] = this;
}

void __cdecl c_game_life_cycle_handler_joining::check_joining_capability()
{
	INVOKE(0x1AD643, 0x1A65C0, check_joining_capability);
	return;
}

bool c_game_life_cycle_manager::get_active_session(c_network_session** out_session)
{
	c_network_session* active_session = NetworkSession::GetActiveNetworkSession();

	bool result = false;
	*out_session = NULL;

	c_game_life_cycle_manager* life_cycle_manager = get();
	if (life_cycle_manager->initialized
		&& IN_RANGE(life_cycle_manager->life_cycle_state, _life_cycle_pre_game, _life_cycle_joining)
		&& !active_session->local_state_none()
		)
	{
		*out_session = active_session;
		result = true;
	}

	return result;
}

c_game_life_cycle_manager* c_game_life_cycle_manager::get()
{
	return Memory::GetAddress<c_game_life_cycle_manager*>(0x420FC0, 0x3C40A8);
}

bool c_game_life_cycle_manager::game_life_cycle_initialized()
{
	return c_game_life_cycle_manager::get()->initialized;
}

e_game_life_cycle c_game_life_cycle_manager::get_life_cycle() const
{
	e_game_life_cycle result = _life_cycle_none;
	if (game_life_cycle_initialized())
	{
		result = life_cycle_state;
	}
	return result;
}

bool c_game_life_cycle_manager::state_is_joining() const
{
	if (!game_life_cycle_initialized())
		return false;
	if (life_cycle_state == _life_cycle_joining)
		return true;

	return false;
}

bool c_game_life_cycle_manager::state_is_in_game(void) const
{
	if (!game_life_cycle_initialized())
		return false;
	if (life_cycle_state == _life_cycle_in_game)
		return true;

	return false;
}

void c_game_life_cycle_manager::request_state_change(e_game_life_cycle requested_state, int a3, void* a4)
{
	this->requested_life_cycle = requested_state;
	this->update_requested = true;
	this->field_3C = a3;
	this->field_40 = 0;
	if (this->field_3C > 0)
	{
		memcpy(&field_40, a4, field_3C);
	}
}
