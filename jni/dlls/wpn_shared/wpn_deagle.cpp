/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_deagle.h"

enum deagle_e
{
	DEAGLE_IDLE1,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD,
	DEAGLE_DRAW, 
DEAGLE_IDLE2,
ANIM8,
ANIM9,
ANIM10,
ANIM11,
ANIM12
};

LINK_ENTITY_TO_CLASS(weapon_deagle, CDEAGLE)

void CDEAGLE::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_deagle");

	Precache();
	m_iId = WEAPON_USP;
	SET_MODEL(ENT(pev), "models/w_desert_eagle.mdl");

	m_iDefaultAmmo = DEAGLE_DEFAULT_GIVE;
	m_flAccuracy = 0.9;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_fMaxSpeed = 250;

	FallInit();
}

void CDEAGLE::Precache(void)
{
	
PRECACHE_MODEL("models/billflx/v_desert_eagle.mdl");

	PRECACHE_MODEL("models/p_desert_eagle.mdl");

	PRECACHE_SOUND("weapons/desert_eagle-1.wav");
	PRECACHE_SOUND("weapons/deagle-2.wav");
	PRECACHE_SOUND("weapons/de_clipout.wav");
	PRECACHE_SOUND("weapons/de_clipin.wav");
	PRECACHE_SOUND("weapons/de_deploy.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireDeagle = PRECACHE_EVENT(1, "events/deagle.sc");
}

int CDEAGLE::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50AE";
	p->iMaxAmmo1 = MAX_AMMO_50AE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = WEAPON_USP;
	p->iFlags = 0;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}


BOOL CDEAGLE::Deploy(void)
{

	m_flAccuracy = 0.9;
	

return DefaultDeploy("models/billflx/v_desert_eagle.mdl", "models/p_desert_eagle.mdl", DEAGLE_DRAW, "onehanded", 0);
			
}


void CDEAGLE::QuickDeploy()
{
SendWeaponAnim(ANIM11, 0);
m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.3;
m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CDEAGLE::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		DEAGLEFire((1.0) * (1 - m_flAccuracy), 0.3, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		DEAGLEFire((0.0) * (1 - m_flAccuracy), 0.3, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		DEAGLEFire((0.0) * (1 - m_flAccuracy), 0.3, FALSE);
	else
		DEAGLEFire((0.0) * (1 - m_flAccuracy), 0.3, FALSE);


}

void CDEAGLE::SecondaryAttack(void)
{
	ShieldSecondaryFire(SHIELDGUN_UP, SHIELDGUN_DOWN);
}

void CDEAGLE::DEAGLEFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	flCycleTime -= 0.075;
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_flLastFire)
	{
		m_flAccuracy -= (0.4 - (gpGlobals->time - m_flLastFire)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.55)
			m_flAccuracy = 0.55;
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	SetPlayerShieldAnim();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, 54, 0.81, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);


//Vector vecSrc = m_pPlayer->GetGunPosition();

if (m_iClip > 0)
{
SendWeaponAnim(DEAGLE_SHOOT1, 0);
}
else if (m_iClip = 0)
{
SendWeaponAnim(DEAGLE_IDLE1, 0);
}

EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/desert_eagle-1.wav", VOL_NORM, ATTN_NORM);


	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(),m_usFireDeagle, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8;
	m_pPlayer->pev->punchangle.x -= 2;
	ResetPlayerShieldAnim();
}

void CDEAGLE::Reload(void)
{
	if (m_pPlayer->ammo_50ae <= 0)
		return;
if (m_iClip == DEAGLE_MAX_CLIP)
return;
	if (DefaultReload(DEAGLE_MAX_CLIP, NULL, 2.2))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.9;

	}
SendWeaponAnim(DEAGLE_RELOAD, 0);
}

void CDEAGLE::QuickReload()
{
	if (m_pPlayer->ammo_50ae <= 0)
		return;
if (m_iClip == DEAGLE_MAX_CLIP)
return;
	if (DefaultReloadQuick(DEAGLE_MAX_CLIP, NULL, 1.2))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.9;

	}
SendWeaponAnim(ANIM12, 0);
}

void CDEAGLE::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		SendWeaponAnim(SHIELDGUN_DRAWN_IDLE, UseDecrement() != FALSE);
}
