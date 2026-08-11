#pragma once
#include "H2MOD/Variants/VariantSystem.h"

class GunGame : public ICustomGameVariant
{
public:
	GunGame();
	virtual ~GunGame() = default;
	virtual void Initialize() override;
	virtual void Dispose() override;
	virtual CustomVariantId GetVariantId() override;

	// on map load can be used as Initialize
	virtual void OnMapLoad(ExecTime execTime, enum e_game_mode game_mode) override;
	virtual void OnPlayerSpawn(ExecTime execTime, datum playerIdx) override;
	virtual void OnPlayerDeath(ExecTime execTime, datum playerIdx) override;

	virtual bool OnAutoPickupHandler(ExecTime execTime, datum playerIdx, datum objectIdx) override
	{
		return false;
	}

	virtual bool c_game_statborg__adjust_player_stat(ExecTime execTime, c_game_statborg* statborg, datum player_datum, e_statborg_entry statistic, short count, int game_results_statistic, bool adjust_team_stat) override;

	static void ResetPlayerLevels();
};
