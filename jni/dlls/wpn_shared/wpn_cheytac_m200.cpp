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
#include "game.h"
#include "wpn_cheytac_m200.h"

enum awp_e
{
	AWP_IDLE,
	AWP_SHOOT1,
	AWP_SHOOT2,
	AWP_SHOOT3,
	AWP_RELOAD,
	AWP_DRAW,
ANIM1,
ANIM2,
ANIM3,
ANIM4,
ANIM5,
ANIM6
};

LINK_ENTITY_TO_CLASS(weapon_cheytac_m200, CCHEYTAC)

void CCHEYTAC::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_cheytac_m200");

	Precache();
	m_iId = WEAPON_AWP;
	SET_MODEL(ENT(pev), "models/w_awp.mdl");

	m_iDefaultAmmo = AWP_DEFAULT_GIVE;
	FallInit();
}

void CCHEYTAC::Precache(void)
{
	PRECACHE_MODEL("models/billflx/v_cheytac_m200.mdl");

	PRECACHE_MODEL("models/p_cheytac_m200.mdl");

	PRECACHE_SOUND("weapons/cheytac_m200-1.wav");
	PRECACHE_SOUND("weapons/boltpull1.wav");
	PRECACHE_SOUND("weapons/boltup.wav");
	PRECACHE_SOUND("weapons/boltdown.wav");
	PRECACHE_SOUND("weapons/zoom_sniper_in.wav");
PRECACHE_SOUND("weapons/zoom_sniper_out.wav");

	PRECACHE_SOUND("weapons/awp_deploy.wav");
	PRECACHE_SOUND("weapons/awp_clipin.wav");
	PRECACHE_SOUND("weapons/awp_clipout.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireCHEYTAC = PRECACHE_EVENT(1, "events/awp.sc");
}

int CCHEYTAC::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "338Magnum";
	p->iMaxAmmo1 = MAX_AMMO_338MAGNUM;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AWP_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = WEAPON_AWP;
	p->iFlags = 0;
	p->iWeight = AWP_WEIGHT;

	return 1;
}


BOOL CCHEYTAC::Deploy(void)
{
if (DefaultDeploy("models/billflx/v_cheytac_m200.mdl", "models/p_cheytac_m200.mdl", AWP_DRAW, "rifle", 0))

{
	m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1;

m_flNextSecondaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;
	
	return TRUE;
	}
	return FALSE;

}

void CCHEYTAC::QuickDeploy()
{
SendWeaponAnim(ANIM5, 0);
m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.3;
m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CCHEYTAC::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
		case 90: 

SetThink(&CCHEYTAC::Fov2);
	pev->nextthink = gpGlobals->time + 0.000001;
break;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;

		default: 
SetThink(&CCHEYTAC::offFov2);
	pev->nextthink = gpGlobals->time + 0.0000009;
break;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
	}

	m_pPlayer->ResetMaxSpeed();
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;

}

void CCHEYTAC::Fov2(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 80; 

SetThink(&CCHEYTAC::Fov3);
	pev->nextthink = gpGlobals->time + 0.000001;

//m_pPlayer->pev->viewmodel = MAKE_STRING("models/sight/v_muzzle_bottom.mdl");

#ifndef CLIENT_DLL
if (weapon_sight_cheytac_m200.value == 0)
CLIENT_COMMAND(m_pPlayer->edict(), "sniper_scope\n");
else if (weapon_sight_cheytac_m200.value == 1) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 1; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 2) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 2; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 3) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 3; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 4) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 4; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 5) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 5; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 6) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 6; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 7) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 7; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 8) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 8; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 9) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 9; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 10) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 10; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 11) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 11; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 12) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 12; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 13) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 13; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 14) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 14; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 15) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 15; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 16) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 16; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 17) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 17; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 18) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 18; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 19) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 19; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 20) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 20; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 21) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 21; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 22) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 22; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 23) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 23; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 24) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 24; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 25) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 25; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 26) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 26; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 27) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 27; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 28) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 28; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 29) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 29; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 30) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 30; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 31) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 31; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 32) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 32; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 33) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 33; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 34) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 34; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 35) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 35; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 36) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 36; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 37) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 37; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 38) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 38; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 39) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 39; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 40) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 40; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 41) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 41; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 42) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 42; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 43) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 43; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 44) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 44; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 45) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 45; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 46) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 46; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 47) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 47; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 48) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 48; Custom_Scope\n");
else if (weapon_sight_cheytac_m200.value == 49) CLIENT_COMMAND(m_pPlayer->edict(), "custom_scope_cmd 49; Custom_Scope\n");
#endif

}

void CCHEYTAC::Fov3(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 70; 

SetThink(&CCHEYTAC::Fov4);
	pev->nextthink = gpGlobals->time + 0.0000009;

}

void CCHEYTAC::Fov4(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 50; 

SetThink(&CCHEYTAC::Fov5);
	pev->nextthink = gpGlobals->time + 0.0000009;

}

void CCHEYTAC::Fov5(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; 

SetThink(&CCHEYTAC::Fov6);
	pev->nextthink = gpGlobals->time + 0.0000009;

}

void CCHEYTAC::Fov6(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 30; 

SetThink(&CCHEYTAC::Fov7);
	pev->nextthink = gpGlobals->time + 0.0000009;

}

void CCHEYTAC::Fov7(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 20;
	
	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom_sniper_in.wav", 0.2, 2.4);
	


SetThink(&CCHEYTAC::breath);
switch (RANDOM_LONG(1, 3))
{
case 1:
	pev->nextthink = gpGlobals->time + 4;
break;
case 2:
	pev->nextthink = gpGlobals->time + 5;
break;
case 3:
	pev->nextthink = gpGlobals->time + 7;
break;
}

}


void CCHEYTAC::breath()
{
switch (m_pPlayer->m_iModelName)
		{
case MODEL_URBAN:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breath\"\n");
			break;
case MODEL_GSG9:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breathf\"\n");
			break;
case MODEL_SAS:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breath\"\n");
			break;
case MODEL_GIGN:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breathf\"\n");
			break;
case MODEL_SPETSNAZ:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breathf\"\n");
			break;
case MODEL_TERROR:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breath\"\n");
			break;
case MODEL_LEET:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breathf\"\n");
			break;
case MODEL_ARCTIC:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breath\"\n");
			break;
case MODEL_GUERILLA:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breathf\"\n");
			break;
case MODEL_MILITIA:
CLIENT_COMMAND(m_pPlayer->edict(), "speak \"breathf\"\n");
			break;
}

}

void CCHEYTAC::Holster(int skiplocal)
{
m_fInReload = FALSE;
SetThink(nullptr);
}

BOOL CCHEYTAC::CanDeploy(void)
{
	return 1;
}

BOOL CCHEYTAC::CanHolster(void)
{
	return 1;
}

//offfov

void CCHEYTAC::offFov2(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 30; 

SetThink(&CCHEYTAC::offFov3);
	pev->nextthink = gpGlobals->time + 0.0000009;
}

void CCHEYTAC::offFov3(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; 

SetThink(&CCHEYTAC::offFov4);
	pev->nextthink = gpGlobals->time + 0.0000009;
}

void CCHEYTAC::offFov4(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 50; 

SetThink(&CCHEYTAC::offFov5);
	pev->nextthink = gpGlobals->time + 0.0000009;
}

void CCHEYTAC::offFov5(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 60; 

SetThink(&CCHEYTAC::offFov6);
	pev->nextthink = gpGlobals->time + 0.0000009;
}

void CCHEYTAC::offFov6(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 70; 

SetThink(&CCHEYTAC::offFov7);
	pev->nextthink = gpGlobals->time + 0.0000009;
}

void CCHEYTAC::offFov7(void)
{
	
m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom_sniper_out.wav", 0.2, 2.4);

//m_pPlayer->pev->viewmodel = MAKE_STRING(m_pPlayer->m_iTeam == CT ? "models/billflx/cityforce/v_cheytac_m200.mdl" : "models/billflx/rebel/v_cheytac_m200.mdl");


m_pPlayer->pev->viewmodel = MAKE_STRING("models/billflx/v_cheytac_m200.mdl");
	

}

void CCHEYTAC::PrimaryAttack(void)
{

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		CHEYTACFire(0.0, 2.5, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		CHEYTACFire(0.0, 2.5, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 10)
		CHEYTACFire(0.0, 2.5, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		CHEYTACFire(0.0, 2.5, FALSE);
	else
		CHEYTACFire(0.0, 2.5, FALSE);


}

void CCHEYTAC::CHEYTACFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;

//SetThink(&CCHEYTAC::offFov7);
	//pev->nextthink = gpGlobals->time + 0.0000009;
	}
	else // not zoomed, so bullets diverts more
		flSpread += 0.1;

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
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.55;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_338MAG, 135, 0.99, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);


Vector vecSrc = m_pPlayer->GetGunPosition();
UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 2000;
#ifndef CLIENT_DLL
	m_pPlayer->Trace(vecEnd, gpGlobals->v_forward, flSpread, 0, 0, m_pPlayer->pev, m_pPlayer->pev);
#endif

SendWeaponAnim(AWP_SHOOT1, 0);
EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/cheytac_m200-1.wav", VOL_NORM, ATTN_NORM);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev),m_usFireCHEYTAC, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;
	m_pPlayer->pev->punchangle.x -= 2;
}

void CCHEYTAC::Reload(void)
{
	if (m_pPlayer->ammo_338mag <= 0)
		return;
if (m_iClip == AWP_MAX_CLIP)
return;
	if (DefaultReload(AWP_MAX_CLIP, NULL, 3))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		
			//m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 10;
			SetThink(&CCHEYTAC::offFov7);
	pev->nextthink = gpGlobals->time + 0.0000009;

		
	}
SendWeaponAnim(AWP_RELOAD,  0);
}

void CCHEYTAC::QuickReload()
{
	if (m_pPlayer->ammo_338mag <= 0)
		return;
if (m_iClip == AWP_MAX_CLIP)
return;
	if (DefaultReloadQuick(AWP_MAX_CLIP, NULL, 2))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		
			//m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 10;
			SetThink(&CCHEYTAC::offFov7);
	pev->nextthink = gpGlobals->time + 0.0000009;

		
	}
SendWeaponAnim(ANIM6,  0);
}

void CCHEYTAC::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;


;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60;
		SendWeaponAnim(AWP_IDLE, 0);
	}
}
