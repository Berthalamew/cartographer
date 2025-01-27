#pragma once

void AchievementUnlock(uint64 xuid, int achievement_id, XOVERLAPPED* pOverlapped);
void GetAchievements(unsigned long long xuid);

extern bool g_achievement_list[42];
extern std::unordered_map<std::string, bool> AchievementMap;
