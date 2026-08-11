#include "stdafx.h"
#include "VariantSystem.h"

#include "GraveRobber/GraveRobber.h"
#include "GunGame/GunGame.h"
#include "Infection/Infection.h"

/* constants */

const wchar_t* k_custom_variant_strings[5]
{
	L"infection",
	L"zombies",
	L"gungame",
	L"headhunter",
	L"graverobber"
};

const CustomVariantId k_custom_variant_ids[5]
{
	CustomVariantId::_id_infection,
	CustomVariantId::_id_infection,
	CustomVariantId::_id_gungame,
	CustomVariantId::_id_graverobber,
	CustomVariantId::_id_graverobber
};

/* globals */

ICustomGameVariant* g_custom_variants[3];
ICustomGameVariant* g_current_running_variant;

void CustomVariantHandler::RegisterCustomVariants()
{
	g_custom_variants[0] = new Infection();
	g_custom_variants[1] = new GunGame();
	g_custom_variants[2] = new GraveRobber();
}

ICustomGameVariant* CustomVariantHandler::GetGameVariant(CustomVariantId variantId)
{
	for (auto& variant : g_custom_variants)
	{
		if (variant->GetVariantId() == variantId)
			return variant;
	}

	return nullptr;
}

ICustomGameVariant* CustomVariantHandler::GetCurrentGameVariant()
{
	return g_current_running_variant;
}

void CustomVariantHandler::GameVarianEnable(const wchar_t* variant)
{
	CustomVariantId variantId = _id_unknown;

	for (size_t i = 0; i < NUMBEROF(k_custom_variant_strings); ++i)
	{
		if (StrStrIW(variant, k_custom_variant_strings[i]))
		{
			variantId = k_custom_variant_ids[i];
			break;
		}
	}

	g_current_running_variant = GetGameVariant(variantId);
}

bool CustomVariantHandler::ContainsGameVariant(const wchar_t* variant, CustomVariantId variantId)
{
	for (size_t i = 0; i < NUMBEROF(k_custom_variant_strings); ++i)
	{
		if (StrStrIW(variant, k_custom_variant_strings[i]) && k_custom_variant_ids[i] == variantId)
		{
			return true;
		}
	}

	return false;
}

void CustomVariantHandler::DisposeGameVariant()
{
	if (g_current_running_variant != nullptr)
		g_current_running_variant->Dispose();
	g_current_running_variant = nullptr;
}

bool CustomVariantHandler::VariantEnabled(CustomVariantId variantId)
{
	if (GetCurrentGameVariant() != nullptr)
		return GetCurrentGameVariant()->GetVariantId() == variantId;

	return false;
}

void CustomVariantHandler::OnMapLoad(ExecTime execTime, e_game_mode game_mode)
{
	if (GetCurrentGameVariant() != nullptr)
	{
		GetCurrentGameVariant()->OnMapLoad(execTime, game_mode);
	}
}

void CustomVariantHandler::OnPlayerSpawn(ExecTime execTime, datum playerIdx)
{
	if (GetCurrentGameVariant() != nullptr)
		GetCurrentGameVariant()->OnPlayerSpawn(execTime, playerIdx);
}

void CustomVariantHandler::OnPlayerDeath(ExecTime execTime, datum playerIdx)
{
	if (GetCurrentGameVariant() != nullptr)
		GetCurrentGameVariant()->OnPlayerDeath(execTime, playerIdx);
}

bool CustomVariantHandler::OnAutoPickupHandler(ExecTime execTime, datum playerIdx, datum objectIdx)
{
	if (GetCurrentGameVariant() != nullptr)
		return GetCurrentGameVariant()->OnAutoPickupHandler(execTime, playerIdx, objectIdx);

	return false;
}

bool CustomVariantHandler::c_game_statborg__adjust_player_stat(ExecTime execTime, class c_game_statborg* statborg, datum player_datum, enum e_statborg_entry statistic, short count, int game_results_statistic, bool adjust_team_stat)
{
	if (GetCurrentGameVariant() != nullptr)
		return GetCurrentGameVariant()->c_game_statborg__adjust_player_stat(execTime, statborg, player_datum, statistic, count, game_results_statistic, adjust_team_stat);

	return false;
}