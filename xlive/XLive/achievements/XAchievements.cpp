#include "stdafx.h"
#include "XAchievements.h"

#include "H2MOD/Modules/Achievements/Achievements.h"

/* constants */

static char const* k_achievement_strings[] =
{
	"Cairo Station|Complete Cairo Station.",
	"Outskirts|Complete Outskirts.",
	"Metropolis|Complete Metropolis.",
	"The Arbiter|Complete The Arbiter.",
	"Oracle|Complete The Oracle.",
	"Delta Halo|Complete Delta Halo.",
	"Regret|Complete Regret.",
	"Sacred Icon|Complete Sacred Icon.",
	"Quarantine Zone|Complete Quarantine Zone.",
	"Gravemind|Complete Gravemind.",
	"Uprising|Complete Uprising.",
	"High Charity|Complete High Charity.",
	"The Great Journey|Complete The Great Journey.",
	"Warrior|Complete the game on Normal.",
	"Hero|Complete the game on Heroic.",
	"Legend|Complete the game on Legendary.",
	"King of the Scarab|Acquire the Scarab Gun.",
	"Silent But Deadly|Kill 7 opponents from behind.",
	"Demon|Complete any level without dying.",
	"Go Ape Shit|Kill an enraged, berserk Brute by melee.",
	"Stick It|Stick someone!",
	"Counterpoint|Kill the sword carrier.",
	"Carjacking|Steal an occupied vehicle.",
	"Violent Cartographer|Play every default map.",
	"Rainman|Play every variant with at least 3 people.",
	"Double Kill|Kill 2 opponents within 4 seconds.",
	"Triple Kill|Kill 3 opponents within 4 seconds.",
	"Killtacular|Kill 4 opponents within 4 seconds.",
	"Killing Spree|Kill 5 opponents in a row.",
	"Running Riot|Kill 10 opponents in a row.",
	"Sniper Kill|Get a sniper kill.",
	"Roadkill|Run over and kill an opponent.",
	"Bonecracker|Kill an opponent with a melee.",
	"Assassin|Melee an opponent from behind.",
	"Skewer Stopper|Kill sword carrier on Spree.",
	"Vigilante|Stop another player's Killing Spree.",
	"Air Traffic Controller|Blow up a Banshee.",
	"Decorated Soldier|Get awarded at least 8 medals in non-team game.",
	"Ninja|In a non-team game, kill 5 people by melee, from behind.",
	"Flaming Ninja|Kill an opponent that has the Ninja achievement.",
	"Hired Gun|Kill an opponent who has the Legend achievement."
};

/* globals */

static DWORD achievementCount = 0;
static DWORD achievementEnumeratorFlags = 0;
static DWORD achievementEnumeratorIndex = 0;

HANDLE g_dwFakeAchievementContent = INVALID_HANDLE_VALUE;

/* public code */

// #5278: XUserWriteAchievements
DWORD WINAPI XUserWriteAchievements(
	DWORD dwNumAchievements,
	PXUSER_ACHIEVEMENT pAchievement,
	PXOVERLAPPED pOverlapped)
{
	LOG_TRACE_XLIVE(
		"XUserWriteAchievements  (count = {0:#x}, buffer = {1:p}, pOverlapped = {2:p})",
		dwNumAchievements,
		(void*)pAchievement,
		(void*)pOverlapped);

	XUSER_SIGNIN_INFO* signedInUser = XUserGetSignInInfo(pAchievement->dwUserIndex);

	if (dwNumAchievements > 0)
	{
		for (DWORD i = 0u; i < dwNumAchievements; i++)
		{
			int achievementID = pAchievement[i].dwAchievementId;

			LOG_TRACE_GAME("Achievement {0} unlock attempt by Player {1} - id2: {2}", pAchievement[i].dwAchievementId, pAchievement[i].dwUserIndex, achievementID);

			if (g_achievement_list[achievementID] == false)
			{
				g_achievement_list[achievementID] = true;				

				if (VALID_INDEX(achievementID-1, 41))
				{
					AchievementMap[k_achievement_strings[achievementID - 1]] = false;
				}
				else
				{
					AchievementMap["Unknown"] = false;
				}

				std::thread(AchievementUnlock, signedInUser->xuid, achievementID, pOverlapped).detach();
			}
			else
			{
				LOG_TRACE_GAME("Achievement {} was already unlocked", achievementID);
			}
		}
	}

	if (pOverlapped)
	{
		pOverlapped->InternalLow = ERROR_SUCCESS;
		pOverlapped->InternalHigh = dwNumAchievements;
		pOverlapped->dwExtendedError = 0;

		return ERROR_IO_PENDING;
	}

	return ERROR_SUCCESS;
}

// #5280: XUserCreateAchievementEnumerator
DWORD WINAPI XUserCreateAchievementEnumerator(
	DWORD dwTitleId,
	DWORD dwUserIndex,
	XUID xuid,
	DWORD dwDetailFlags,
	DWORD dwStartingIndex, 
	DWORD cItem,
	PDWORD pchBuffer,
	PHANDLE phEnum)
{
	LOG_TRACE_XLIVE("XUserCreateAchievementEnumerator (dwStartingIndex = {0}, MaxEnumerator = {1})", dwStartingIndex, cItem);

	achievementCount = cItem;
	achievementEnumeratorFlags = dwDetailFlags;
	achievementEnumeratorIndex = dwStartingIndex;

	if (pchBuffer) *pchBuffer = cItem * sizeof(XACHIEVEMENT_DETAILS);
	if (phEnum) *phEnum = g_dwFakeAchievementContent = CreateMutex(NULL, NULL, NULL);

	LOG_TRACE_XLIVE("- Handle = {}, pchBuffer = {}", (void*)g_dwFakeAchievementContent, pchBuffer ? *pchBuffer : 0);

	return ERROR_SUCCESS;
}

int AchievementEnumerator(
	DWORD cbBuffer, 
	CHAR* pvBuffer,
	PDWORD pcItemsReturned,
	XOVERLAPPED* pOverlapped)
{
	memset(pOverlapped, 0, sizeof(XOVERLAPPED));

	for (; achievementEnumeratorIndex < achievementCount; achievementEnumeratorIndex++)
	{
		XACHIEVEMENT_DETAILS aaa;
		memset(&aaa, 0, sizeof(XACHIEVEMENT_DETAILS));
		bool achieved = g_achievement_list[achievementEnumeratorIndex + 1];

		// check max
		if ((achievementCount - (achievementEnumeratorIndex + 1)) >= cbBuffer / sizeof(XACHIEVEMENT_DETAILS))
			break;

		FILETIME fileTime;
		SYSTEMTIME systemTime;

		GetSystemTime(&systemTime);
		SystemTimeToFileTime(&systemTime, &fileTime);

		aaa.dwId = achievementEnumeratorIndex + 1;
		aaa.pwszLabel = L"";
		aaa.pwszDescription = L"";
		aaa.pwszUnachieved = L"";
		aaa.dwImageId = 0;

		if (achieved)
		{
			aaa.dwCred = 1000;
			aaa.ftAchieved = fileTime;
		}

		aaa.dwFlags = achieved == true ? XACHIEVEMENT_DETAILS_ACHIEVED_ONLINE | XACHIEVEMENT_DETAILS_ACHIEVED : 0;

		if (pOverlapped == NULL)
		{
			(*pcItemsReturned)++;
		}
		else
		{
			pOverlapped->InternalHigh++;
		}

		if (pvBuffer)
		{
			memcpy(pvBuffer, &aaa, sizeof(XACHIEVEMENT_DETAILS));
			pvBuffer += sizeof(XACHIEVEMENT_DETAILS);
		}
	}

	if (pOverlapped == NULL)
	{
		return ERROR_SUCCESS;
	}
	else
	{
		pOverlapped->InternalLow = ERROR_SUCCESS;
		pOverlapped->dwExtendedError = 0;

		return ERROR_IO_PENDING;
	}
}
