#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "hltv.h"
#include "game.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"

#include "debug.h"

#include "pm_shared.h"
#include "utllinkedlist.h"

// CSBOT and Nav
#include "game_shared/GameEvent.h"		// Game event enum used by career mode, tutor system, and bots
#include "game_shared/bot/bot_util.h"
#include "game_shared/bot/simple_state_machine.h"

#include "game_shared/steam_util.h"

#include "game_shared/bot/bot_manager.h"
#include "game_shared/bot/bot_constants.h"
#include "game_shared/bot/bot.h"

#include "game_shared/shared_util.h"
#include "game_shared/bot/bot_profile.h"

#include "game_shared/bot/nav.h"
#include "game_shared/bot/improv.h"
#include "game_shared/bot/nav_node.h"
#include "game_shared/bot/nav_area.h"
#include "game_shared/bot/nav_file.h"
#include "game_shared/bot/nav_path.h"

#include "airtank.h"
#include "h_ai.h"
#include "h_cycler.h"
#include "h_battery.h"

// Hostage
#include "hostage/hostage.h"
#include "hostage/hostage_localnav.h"

#include "bot/cs_bot.h"

// Tutor
#include "tutor.h"
#include "tutor_base_states.h"
#include "tutor_base_tutor.h"
#include "tutor_cs_states.h"
#include "tutor_cs_tutor.h"

#include "gamerules.h"
#include "career_tasks.h"
#include "maprules.h"

bool HasDefaultPistol(CCSBot *me)
{
	CBasePlayerWeapon *pistol = static_cast<CBasePlayerWeapon *>(me->m_rgpPlayerItems[ PISTOL_SLOT ]);

	if (pistol == NULL)
		return false;

	if (me->m_iTeam == TERRORIST && pistol->m_iId == WEAPON_GLOCK18)
		return true;

	if (me->m_iTeam == CT && pistol->m_iId == WEAPON_USP)
		return true;

	return false;
}

// Buy weapons, armor, etc.

void BuyState::OnEnter(CCSBot *me)
{
	CCSBotManager *ctrl = TheCSBots();

	m_retries = 0;
	m_prefRetries = 0;
	m_prefIndex = 0;

	m_doneBuying = false;
	m_isInitialDelay = true;

	// this will force us to stop holding live grenade
	me->EquipBestWeapon();

	m_buyDefuseKit = false;
	m_buyShield = false;

	if (me->m_iTeam == CT)
	{
		if (ctrl->GetScenario() == CCSBotManager::SCENARIO_DEFUSE_BOMB)
		{
			// CT's sometimes buy defuse kits in the bomb scenario (except in career mode, where the player should defuse)
			if (g_pGameRules->IsCareer() == false)
			{
				const float buyDefuseKitChance = 50.0f;	// 100.0f * (me->GetProfile()->GetSkill() + 0.2f);
				if (RANDOM_FLOAT(0.0f, 100.0f) < buyDefuseKitChance)
				{
					m_buyDefuseKit = true;
				}
			}
		}

		// determine if we want a tactical shield
		if (!me->m_bHasPrimary && ctrl->AllowTacticalShield())
		{
			if (me->m_iAccount > 2500)
			{
				if (me->m_iAccount < 4000)
					m_buyShield = (RANDOM_FLOAT(0, 100.0f) < 33.3f) ? true : false;
				else
					m_buyShield = (RANDOM_FLOAT(0, 100.0f) < 10.0f) ? true : false;
			}
		}
	}

	if (ctrl->AllowGrenades())
	{
		m_buyGrenade = (RANDOM_FLOAT(0.0f, 100.0f) < 33.3f) ? true : false;
	}
	else
	{
		m_buyGrenade = false;
	}

	m_buyPistol = false;

	if (ctrl->AllowPistols())
	{
		CBasePlayerWeapon *pistol = static_cast<CBasePlayerWeapon *>(me->m_rgpPlayerItems[ PISTOL_SLOT ]);

		// check if we have a pistol
		if (pistol != NULL)
		{
			// if we have our default pistol, think about buying a different one
			if (HasDefaultPistol(me))
			{
				// if everything other than pistols is disallowed, buy a pistol
				if (ctrl->AllowShotguns() == false &&
					ctrl->AllowSubMachineGuns() == false &&
					ctrl->AllowRifles() == false &&
					ctrl->AllowMachineGuns() == false &&
					ctrl->AllowTacticalShield() == false &&
					ctrl->AllowSnipers() == false)
				{
					m_buyPistol = (RANDOM_FLOAT(0, 100) < 75.0f);
				}
				else if (me->m_iAccount < 1000)
				{
					// if we're low on cash, buy a pistol
					m_buyPistol = (RANDOM_FLOAT(0, 100) < 75.0f);
				}
				else
				{
					m_buyPistol = (RANDOM_FLOAT(0, 100) < 33.3f);
				}
			}
		}
		else
		{
			// we dont have a pistol - buy one
			m_buyPistol = true;
		}
	}
}

enum WeaponType
{
	PISTOL,
	SHOTGUN,
	SUB_MACHINE_GUN,
	RIFLE,
	MACHINE_GUN,
	SNIPER_RIFLE,
	GRENADE,
	NUM_WEAPON_TYPES,
};

struct BuyInfo
{
	WeaponType type;
	bool preferred;		// more challenging bots prefer these weapons
	const char *buyAlias;		// the buy alias for this equipment
};

// These tables MUST be kept in sync with the CT and T buy aliases
static BuyInfo primaryWeaponBuyInfoCT[PRIMARY_WEAPON_BUY_COUNT] =
{
	{ SHOTGUN,		false, "m3" },			
	{ SHOTGUN,		false, "custom33" },		//m1887
	{ SHOTGUN,		false, "xm1014" },		
	{ SHOTGUN,		false, "custom31" },		//zslayer
	{ SHOTGUN,		false, "custom32" },		//spas
	{ SUB_MACHINE_GUN,	false, "tmp" },			
	{ SUB_MACHINE_GUN,	false, "custom29" },	//mp7			
	{ SUB_MACHINE_GUN,	false, "custom27" },	//mp9		
	{ SUB_MACHINE_GUN,	false, "custom23" },	//mp9
	{ SUB_MACHINE_GUN,	false, "mp5" },			
	{ SUB_MACHINE_GUN,	false, "ump45" },		
	{ SUB_MACHINE_GUN,	false, "p90" },			
	{ RIFLE,		true,  "famas" },				
	{ RIFLE,		true,  "custom12" },	//aug sil	
	{ RIFLE,		true,  "custom41" },	//f200	
	{ RIFLE,		true,  "custom50" },	//g36
	{ RIFLE,		true,  "custom58" },	//hk
	{ RIFLE,		true,  "custom77" },	//msbs
	{ RIFLE,		true,  "custom84" },	//sc
	{ RIFLE,		true,  "custom99" },	//sc
	{ SNIPER_RIFLE,		true, "scout" },		
	{ SNIPER_RIFLE,		true, "custom10" },//Cheytac 
	{ SNIPER_RIFLE,		true, "custom3" },//kar 
	{ SNIPER_RIFLE,		true, "custom15" },//rangemas
	{ SNIPER_RIFLE,		true, "custom16" },//pgm
	{ SNIPER_RIFLE,		true, "custom17" },//dragunn
	{ RIFLE,		true,  "m4a1" },		
	{ RIFLE,		false, "aug" },			
	{ SNIPER_RIFLE,		true,  "sg550" },		
	{ SNIPER_RIFLE,		true,  "awp" },			
	{ MACHINE_GUN,		false, "m249" }		
};

static BuyInfo secondaryWeaponBuyInfoCT[ SECONDARY_WEAPON_BUY_COUNT ] =
{
//	{ PISTOL, false,	"glock" },
//	{ PISTOL, false,	"usp" },
	{ PISTOL, true,		"p228" },
	{ PISTOL, true,		"deagle" },
	{ PISTOL, true,		"fn57" }
};

static BuyInfo primaryWeaponBuyInfoT[PRIMARY_WEAPON_BUY_COUNT] =
{
	{ SHOTGUN,		false, "m3" },			
	{ SHOTGUN,		false, "custom33" },		//m1887
	{ SHOTGUN,		false, "xm1014" },		
	{ SHOTGUN,		false, "custom31" },		//zslayer
	{ SHOTGUN,		false, "custom32" },		//spas
	{ SUB_MACHINE_GUN,	false, "tmp" },			
	{ SUB_MACHINE_GUN,	false, "custom29" },	//mp7			
	{ SUB_MACHINE_GUN,	false, "custom27" },	//mp9		
	{ SUB_MACHINE_GUN,	false, "custom23" },	//mp9
	{ SUB_MACHINE_GUN,	false, "mp5" },			
	{ SUB_MACHINE_GUN,	false, "ump45" },		
	{ SUB_MACHINE_GUN,	false, "p90" },			
	{ RIFLE,		true,  "famas" },				
	{ RIFLE,		true,  "custom12" },	//aug sil	
	{ RIFLE,		true,  "custom41" },	//f200	
	{ RIFLE,		true,  "custom50" },	//g36
	{ RIFLE,		true,  "custom58" },	//hk
	{ RIFLE,		true,  "custom77" },	//msbs
	{ RIFLE,		true,  "custom84" },	//sc
	{ RIFLE,		true,  "custom99" },	//sc
	{ SNIPER_RIFLE,		true, "scout" },		
	{ SNIPER_RIFLE,		true, "custom10" },//Cheytac 
	{ SNIPER_RIFLE,		true, "custom3" },//kar 
	{ SNIPER_RIFLE,		true, "custom15" },//rangemas
	{ SNIPER_RIFLE,		true, "custom16" },//pgm
	{ SNIPER_RIFLE,		true, "custom17" },//dragunn
	{ RIFLE,		true,  "m4a1" },		
	{ RIFLE,		false, "aug" },			
	{ SNIPER_RIFLE,		true,  "sg550" },		
	{ SNIPER_RIFLE,		true,  "awp" },			
	{ MACHINE_GUN,		false, "m249" }		
};

static BuyInfo secondaryWeaponBuyInfoT[ SECONDARY_WEAPON_BUY_COUNT ] =
{
//	{ PISTOL, false,	"glock" },
//	{ PISTOL, false,	"usp" },
	{ PISTOL, true,		"p228" },
	{ PISTOL, true,		"deagle" },
	{ PISTOL, true,		"elites" }
};

// Given a weapon alias, return the kind of weapon it is
inline WeaponType GetWeaponType(const char *alias)
{
	int i;
	for (i = 0; i < PRIMARY_WEAPON_BUY_COUNT; ++i)
	{
		if (!Q_stricmp(alias, primaryWeaponBuyInfoCT[i].buyAlias))
			return primaryWeaponBuyInfoCT[i].type;

		if (!Q_stricmp(alias, primaryWeaponBuyInfoT[i].buyAlias))
			return primaryWeaponBuyInfoT[i].type;
	}

	for (i = 0; i < SECONDARY_WEAPON_BUY_COUNT; ++i)
	{
		if (!Q_stricmp(alias, secondaryWeaponBuyInfoCT[i].buyAlias))
			return secondaryWeaponBuyInfoCT[i].type;

		if (!Q_stricmp(alias, secondaryWeaponBuyInfoT[i].buyAlias))
			return secondaryWeaponBuyInfoT[i].type;
	}

	return NUM_WEAPON_TYPES;
}

void BuyState::OnUpdate(CCSBot *me)
{
	// wait for a Navigation Mesh
	if (!TheNavAreaList.size())
		return;

	// apparently we cant buy things in the first few seconds, so wait a bit
	if (m_isInitialDelay)
	{
		const float waitToBuyTime = 2.0f; // 0.25f;
		if (gpGlobals->time - me->GetStateTimestamp() < waitToBuyTime)
			return;

		m_isInitialDelay = false;
	}

	// if we're done buying and still in the freeze period, wait
	if (m_doneBuying)
	{
		if (g_pGameRules->IsMultiplayer () && g_pGameRules->IsFreezePeriod ())
		{
			// make sure we're locked and loaded
			me->EquipBestWeapon (MUST_EQUIP);
			me->Reload ();
			me->ResetStuckMonitor ();
			return;
		}

		me->Idle();
		return;
	}

	// is the bot spawned outside of a buy zone?
	if (!(me->m_signals.GetState() & SIGNAL_BUY))
	{
		m_doneBuying = true;
		UTIL_DPrintf("%s bot spawned outside of a buy zone (%d, %d, %d)\n", (me->m_iTeam == CT) ? "CT" : "Terrorist", me->pev->origin.x, me->pev->origin.y, me->pev->origin.z);
		return;
	}

	CCSBotManager *ctrl = TheCSBots();

	// try to buy some weapons
	const float buyInterval = 0.2f; // 0.02f
	if (gpGlobals->time - me->GetStateTimestamp() > buyInterval)
	{
		me->m_stateTimestamp = gpGlobals->time;

		bool isPreferredAllDisallowed = true;

		// try to buy our preferred weapons first
		if (m_prefIndex < me->GetProfile()->GetWeaponPreferenceCount())
		{
			// need to retry because sometimes first buy fails??
			const int maxPrefRetries = 2;
			if (m_prefRetries >= maxPrefRetries)
			{
				// try to buy next preferred weapon
				++m_prefIndex;
				m_prefRetries = 0;
				return;
			}

			int weaponPreference = me->GetProfile()->GetWeaponPreference(m_prefIndex);

			// don't buy it again if we still have one from last round
			CBasePlayerWeapon *weapon = me->GetActiveWeapon();
			if (weapon != NULL && weapon->m_iId == weaponPreference)
			{
				// done with buying preferred weapon
				m_prefIndex = 9999;
				return;
			}

			if (me->HasShield() && weaponPreference == WEAPON_SHIELDGUN)
			{
				// done with buying preferred weapon
				m_prefIndex = 9999;
				return;
			}

			const char *buyAlias = NULL;

			if (weaponPreference == WEAPON_SHIELDGUN)
			{
				if (ctrl->AllowTacticalShield())
					buyAlias = "shield";
			}
			else
			{
				buyAlias = WeaponIDToAlias(weaponPreference);
				WeaponType type = GetWeaponType(buyAlias);

				switch (type)
				{
				case PISTOL:
					if (!ctrl->AllowPistols())
						buyAlias = NULL;
					break;
				case SHOTGUN:
					if (!ctrl->AllowShotguns())
						buyAlias = NULL;
					break;
				case SUB_MACHINE_GUN:
					if (!ctrl->AllowSubMachineGuns())
						buyAlias = NULL;
					break;
				case RIFLE:
					if (!ctrl->AllowRifles())
						buyAlias = NULL;
					break;
				case MACHINE_GUN:
					if (!ctrl->AllowMachineGuns())
						buyAlias = NULL;
					break;
				case SNIPER_RIFLE:
					if (!ctrl->AllowSnipers())
						buyAlias = NULL;
					break;
				default:
					break;
				}
			}

			if (buyAlias)
			{
				me->ClientCommand(buyAlias);
				me->PrintIfWatched("Tried to buy preferred weapon %s.\n", buyAlias);

				isPreferredAllDisallowed = false;
			}

			++m_prefRetries;

			// bail out so we dont waste money on other equipment
			// unless everything we prefer has been disallowed, then buy at random
			if (isPreferredAllDisallowed == false)
				return;
		}

		// if we have no preferred primary weapon (or everything we want is disallowed), buy at random
		if (!me->m_bHasPrimary && (isPreferredAllDisallowed || !me->GetProfile()->HasPrimaryPreference()))
		{
			if (m_buyShield)
			{
				// buy a shield
				me->ClientCommand("shield");
				me->PrintIfWatched("Tried to buy a shield.\n");
			}
			else
			{
				// build list of allowable weapons to buy
				BuyInfo *masterPrimary = (me->m_iTeam == TERRORIST) ? primaryWeaponBuyInfoT : primaryWeaponBuyInfoCT;
				BuyInfo *stockPrimary[ PRIMARY_WEAPON_BUY_COUNT ];
				int stockPrimaryCount = 0;

				// dont choose sniper rifles as often
				const float sniperRifleChance = 50.0f;
				bool wantSniper = (RANDOM_FLOAT(0, 100) < sniperRifleChance) ? true : false;

				for (int i = 0; i < PRIMARY_WEAPON_BUY_COUNT; ++i)
				{
					if ((masterPrimary[i].type == SHOTGUN && ctrl->AllowShotguns()) ||
						(masterPrimary[i].type == SUB_MACHINE_GUN && ctrl->AllowSubMachineGuns()) ||
						(masterPrimary[i].type == RIFLE && ctrl->AllowRifles()) ||
						(masterPrimary[i].type == SNIPER_RIFLE && ctrl->AllowSnipers() && wantSniper) ||
						(masterPrimary[i].type == MACHINE_GUN && ctrl->AllowMachineGuns()))
					{
						stockPrimary[ stockPrimaryCount++ ] = &masterPrimary[i];
					}
				}

				if (stockPrimaryCount)
				{
					// buy primary weapon if we don't have one
					int which;

					// on hard difficulty levels, bots try to buy preferred weapons on the first pass
					if (m_retries == 0 && ctrl->GetDifficultyLevel() >= BOT_HARD)
					{
						// count up available preferred weapons
						int prefCount = 0;
						for (which = 0; which < stockPrimaryCount; ++which)
						{
							if (stockPrimary[which]->preferred)
								++prefCount;
						}

						if (prefCount)
						{
							int whichPref = RANDOM_LONG(0, prefCount - 1);
							for (which = 0; which < stockPrimaryCount; ++which)
							{
								if (stockPrimary[which]->preferred && whichPref-- == 0)
									break;
							}
						}
						else
						{
							// no preferred weapons available, just pick randomly
							which = RANDOM_LONG(0, stockPrimaryCount - 1);
						}
					}
					else
					{
						which = RANDOM_LONG(0, stockPrimaryCount - 1);
					}

					me->ClientCommand(stockPrimary[ which ]->buyAlias);
					me->PrintIfWatched("Tried to buy %s.\n", stockPrimary[ which ]->buyAlias);
				}
			}
		}

		// If we now have a weapon, or have tried for too long, we're done
		if (me->m_bHasPrimary || m_retries++ > 5)
		{
			// primary ammo
			if (me->m_bHasPrimary)
			{
				me->ClientCommand("primammo");
			}

			// buy armor last, to make sure we bought a weapon first
			me->ClientCommand("vesthelm");
			me->ClientCommand("vest");

			// pistols - if we have no preferred pistol, buy at random
			if (ctrl->AllowPistols() && !me->GetProfile()->HasPistolPreference())
			{
				if (m_buyPistol)
				{
					int which = RANDOM_LONG(0, SECONDARY_WEAPON_BUY_COUNT - 1);

					if (me->m_iTeam == TERRORIST)
						me->ClientCommand(secondaryWeaponBuyInfoT[ which ].buyAlias);
					else
						me->ClientCommand(secondaryWeaponBuyInfoCT[ which ].buyAlias);

					// only buy one pistol
					m_buyPistol = false;
				}

				me->ClientCommand("secammo");
			}

			// buy a grenade if we wish, and we don't already have one
			if (m_buyGrenade && !me->HasGrenade())
			{
				if (UTIL_IsTeamAllBots(me->m_iTeam))
				{
					// only allow Flashbangs if everyone on the team is a bot (dont want to blind our friendly humans)
					float rnd = RANDOM_FLOAT(0, 100);

					if (rnd < 10.0f)
					{
						// smoke grenade
						me->ClientCommand("sgren");
					}
					else if (rnd < 35.0f)
					{
						// flashbang
						me->ClientCommand("flash");
					}
					else
					{
						// he grenade
						me->ClientCommand("hegren");
					}
				}
				else
				{
					if (RANDOM_FLOAT(0, 100) < 10.0f)
					{
						// smoke grenade
						me->ClientCommand("sgren");
					}
					else
					{
						// he grenade
						me->ClientCommand("hegren");
					}
				}
			}

			if (m_buyDefuseKit)
			{
				me->ClientCommand("defuser");
			}

			m_doneBuying = true;
		}
	}
}

void BuyState::OnExit(CCSBot *me)
{
	me->ResetStuckMonitor();
	me->EquipBestWeapon();
}
