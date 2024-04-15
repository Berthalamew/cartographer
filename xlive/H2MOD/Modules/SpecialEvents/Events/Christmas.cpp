#include "stdafx.h"

#include "Christmas.h"
#include "../SpecialEventHelpers.h"


#include "items/weapon_definitions.h"
#include "game/game_globals.h"
#include "models/models.h"
#include "scenario/scenario.h"
#include "units/biped_definitions.h"

#include "H2MOD/Tags/MetaExtender.h"
#include "H2MOD/Tags/MetaLoader/tag_loader.h"

void christmas_event_map_load()
{
	// Halo 2 tags
	datum sword_weapon_datum = tags::find_tag(_tag_group_weapon, "objects\\weapons\\melee\\energy_blade\\energy_blade");
	datum ghost_datum = tags::find_tag(_tag_group_vehicle, "objects\\vehicles\\ghost\\ghost");
	datum frag_model_datum = tags::find_tag(_tag_group_model, "objects\\weapons\\grenade\\frag_grenade\\frag_grenade_projectile");
	datum plasma_model_datum = tags::find_tag(_tag_group_model, "objects\\weapons\\grenade\\plasma_grenade\\plasma_grenade");
	datum ball_weapon_datum = tags::find_tag(_tag_group_weapon, "objects\\weapons\\multiplayer\\ball\\ball");
	datum bomb_weapon_datum = tags::find_tag(_tag_group_weapon, "objects\\weapons\\multiplayer\\assault_bomb\\assault_bomb");

	// Carto Shared tags
	datum santa_hat_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\christmas_hat_map\\hat\\hat", _tag_group_scenery, "carto_shared");
	datum beard_datum = tag_loader::Get_tag_datum("objects\\multi\\stpat_hat\\beard\\santa_beard", _tag_group_scenery, "carto_shared");
	datum snow_datum = tag_loader::Get_tag_datum("scenarios\\multi\\lockout\\lockout_big", _tag_group_weather_system, "carto_shared");
	datum candy_cane_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\candy_cane\\candy_cane", _tag_group_render_model, "carto_shared");
	datum deer_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\reindeer_ghost\\reindeer_ghost", _tag_group_render_model, "carto_shared");
	datum ornament_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\ornament\\ornament", _tag_group_render_model, "carto_shared");
	datum present_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\present\\present", _tag_group_render_model, "carto_shared");
	datum fp_present_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\present\\fp_present", _tag_group_render_model, "carto_shared");

	if (!DATUM_IS_NONE(santa_hat_datum) && !DATUM_IS_NONE(beard_datum))
	{
		tag_loader::Load_tag(santa_hat_datum, true, "carto_shared");
		tag_loader::Load_tag(beard_datum, true, "carto_shared");
		tag_loader::Push_Back();

		santa_hat_datum = tag_loader::ResolveNewDatum(santa_hat_datum);
		beard_datum = tag_loader::ResolveNewDatum(beard_datum);

		// Give Santa Hat and Beard to Chief & Friends
		if (datum hlmt_chief_datum = tags::find_tag(_tag_group_model, "objects\\characters\\masterchief\\masterchief");
			hlmt_chief_datum != NONE) 
		{
			add_hat_and_beard_to_model(hlmt_chief_datum, santa_hat_datum, beard_datum);
		}
		if (datum hlmt_chief_mp_datum = tags::find_tag(_tag_group_model, "objects\\characters\\masterchief\\masterchief_mp");
			hlmt_chief_mp_datum != NONE) 
		{
			add_hat_and_beard_to_model(hlmt_chief_mp_datum, santa_hat_datum, beard_datum);
		}
		if (datum hlmt_elite_datum = tags::find_tag(_tag_group_model, "objects\\characters\\elite\\elite_mp");
			hlmt_elite_datum != NONE)
		{
			add_hat_and_beard_to_model(hlmt_elite_datum, santa_hat_datum, beard_datum, true);
		}

		if (datum flood_datum = game_globals_get_representation(_character_type_flood)->third_person_unit.index;
			flood_datum != NONE)
		{
			auto flood_biped = tags::get_tag<_tag_group_biped, _biped_definition>(flood_datum, true);
			add_hat_and_beard_to_model(flood_biped->unit.object.model.index, santa_hat_datum, beard_datum, false);
		}
	}
	if (!DATUM_IS_NONE(snow_datum))
	{
		tag_loader::Load_tag(snow_datum, true, "carto_shared");
		tag_loader::Push_Back();

		snow_datum = tag_loader::ResolveNewDatum(snow_datum);

		if (!DATUM_IS_NONE(snow_datum))
		{
			auto bsp_definition = tags::get_tag_fast<structure_bsp>(get_global_scenario()->structure_bsps[0]->structure_bsp.index);

			auto weat_block = MetaExtender::add_tag_block2<structure_weather_palette_entry>((unsigned long)std::addressof(bsp_definition->weather_palette));
			weat_block->name.set("snow_cs");
			weat_block->weather_system.group.group = _tag_group_weather_system;
			weat_block->weather_system.index = snow_datum;

			for (auto& cluster : bsp_definition->clusters)
			{
				cluster.weather_index = (short)bsp_definition->weather_palette.count - 1;
			}
		}
	}
	if (!DATUM_IS_NONE(candy_cane_datum) && !DATUM_IS_NONE(sword_weapon_datum))
	{
		tag_loader::Load_tag(candy_cane_datum, true, "carto_shared");
		tag_loader::Push_Back();

		candy_cane_datum = tag_loader::ResolveNewDatum(candy_cane_datum);

		auto sword_weapon = tags::get_tag_fast<_weapon_definition>(sword_weapon_datum);

		datum sword_model_datum = sword_weapon->item.object.model.index;
		auto sword_model = tags::get_tag_fast<s_model_definition>(sword_model_datum);

		sword_model->render_model.index = candy_cane_datum;

		for (auto& first_person : sword_weapon->player_interface.first_person)
			first_person.model.index = candy_cane_datum;

		for (auto& attachment : sword_weapon->item.object.attachments)
		{
			attachment.type.index = NONE;
			attachment.type.group = { (e_tag_group)NONE };
			attachment.marker = 0;
			attachment.primary_scale = 0;
		}
	}
	if (!DATUM_IS_NONE(deer_datum) && !DATUM_IS_NONE(ghost_datum))
	{
		tag_loader::Load_tag(deer_datum, true, "carto_shared");
		tag_loader::Push_Back();

		deer_datum = tag_loader::ResolveNewDatum(deer_datum);

		auto ghost_vehicle = tags::get_tag<_tag_group_vehicle, _unit_definition>(ghost_datum);
		ghost_vehicle->object.attachments.data = 0;
		ghost_vehicle->object.attachments.count = 0;

		datum ghost_model_datum = ghost_vehicle->object.model.index;
		auto ghost_model = tags::get_tag<_tag_group_model, s_model_definition>(ghost_model_datum);
		ghost_model->render_model.index = deer_datum;
	}
	if (!DATUM_IS_NONE(ornament_datum) && !DATUM_IS_NONE(frag_model_datum) && !DATUM_IS_NONE(plasma_model_datum))
	{
		tag_loader::Load_tag(ornament_datum, true, "carto_shared");
		tag_loader::Push_Back();

		ornament_datum = tag_loader::ResolveNewDatum(ornament_datum);

		auto frag_model = tags::get_tag<_tag_group_model, s_model_definition>(frag_model_datum);
		frag_model->render_model.index = ornament_datum;

		auto plasma_model = tags::get_tag<_tag_group_model, s_model_definition>(plasma_model_datum);
		plasma_model->render_model.index = ornament_datum;
	}
	if (!DATUM_IS_NONE(present_datum) && !DATUM_IS_NONE(fp_present_datum) && !DATUM_IS_NONE(ball_weapon_datum) && !DATUM_IS_NONE(bomb_weapon_datum))
	{
		tag_loader::Load_tag(present_datum, true, "carto_shared");
		tag_loader::Load_tag(fp_present_datum, true, "carto_shared");
		tag_loader::Push_Back();

		present_datum = tag_loader::ResolveNewDatum(present_datum);
		fp_present_datum = tag_loader::ResolveNewDatum(fp_present_datum);

		replace_fp_and_3p_models_from_weapon(ball_weapon_datum, fp_present_datum, present_datum);
		replace_fp_and_3p_models_from_weapon(bomb_weapon_datum, fp_present_datum, present_datum);
	}
}
