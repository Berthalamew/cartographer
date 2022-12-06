#include "stdafx.h"

#include "Birthday.h"
#include "../SpecialEventHelpers.h"

#include "Blam/Cache/TagGroups/weapon_definition.hpp"
#include "Blam/Engine/game/game.h"
#include "H2MOD/Tags/MetaExtender.h"
#include "H2MOD/Tags/MetaLoader/tag_loader.h"

void BirthdayOnMapLoad()
{
	if (s_game_globals::game_is_multiplayer())
	{
		// Carto Shared Tags
		datum bday_hat_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\birthday_hat\\birthday_hat", blam_tag::tag_group_type::scenery, "carto_shared");
		datum bday_cake_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\birthday_cake\\birthday_cake", blam_tag::tag_group_type::rendermodel, "carto_shared");
		datum fp_bday_cake_datum = tag_loader::Get_tag_datum("scenarios\\objects\\multi\\carto_shared\\birthday_cake\\fp\\fp", blam_tag::tag_group_type::rendermodel, "carto_shared");

		// Halo 2 Tags
		datum ball_weapon_datum = tags::find_tag(blam_tag::tag_group_type::weapon, "objects\\weapons\\multiplayer\\ball\\ball");
		datum bomb_weapon_datum = tags::find_tag(blam_tag::tag_group_type::weapon, "objects\\weapons\\multiplayer\\assault_bomb\\assault_bomb");

		if (!DATUM_IS_NONE(bday_hat_datum))
		{
			tag_loader::Load_tag(bday_hat_datum, true, "carto_shared");
			tag_loader::Push_Back();

			bday_hat_datum = tag_loader::ResolveNewDatum(bday_hat_datum);


			datum hlmt_chief_datum = tags::find_tag(blam_tag::tag_group_type::model, "objects\\characters\\masterchief\\masterchief");
			if (hlmt_chief_datum != DATUM_INDEX_NONE) {
				AddHat(hlmt_chief_datum, bday_hat_datum);
			}
			datum hlmt_chief_mp_datum = tags::find_tag(blam_tag::tag_group_type::model, "objects\\characters\\masterchief\\masterchief_mp");
			if (hlmt_chief_mp_datum != DATUM_INDEX_NONE) {
				AddHat(hlmt_chief_mp_datum, bday_hat_datum);
			}
			datum hlmt_elite_datum = tags::find_tag(blam_tag::tag_group_type::model, "objects\\characters\\elite\\elite_mp");
			if (hlmt_elite_datum != DATUM_INDEX_NONE)
			{
				AddHat(hlmt_elite_datum, bday_hat_datum, true);
			}
		}

		if (!DATUM_IS_NONE(bday_cake_datum) && !DATUM_IS_NONE(fp_bday_cake_datum) && !DATUM_IS_NONE(ball_weapon_datum) && !DATUM_IS_NONE(bomb_weapon_datum))
		{
			tag_loader::Load_tag(bday_cake_datum, true, "carto_shared");
			tag_loader::Load_tag(fp_bday_cake_datum, true, "carto_shared");
			tag_loader::Push_Back();

			bday_cake_datum = tag_loader::ResolveNewDatum(bday_cake_datum);
			fp_bday_cake_datum = tag_loader::ResolveNewDatum(fp_bday_cake_datum);

			ReplaceFirstAndThirdPersonModelFromWeapon(ball_weapon_datum, fp_bday_cake_datum, bday_cake_datum);
			ReplaceFirstAndThirdPersonModelFromWeapon(bomb_weapon_datum, fp_bday_cake_datum, bday_cake_datum);
		}

	}
}
