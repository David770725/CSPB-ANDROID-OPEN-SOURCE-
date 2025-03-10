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
//
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cvardef.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>

#include "ammohistory.h"
#include "eventscripts.h"
#include "com_weapons.h"
#include "draw_util.h"
#include "triangleapi.h"

enum WeaponIdType
{
	WEAPON_NONE,
	WEAPON_P228,
	WEAPON_GLOCK,
	WEAPON_SCOUT,
	WEAPON_HEGRENADE,
	WEAPON_XM1014,
	WEAPON_C4,
	WEAPON_MAC10,
	WEAPON_AUG,
	WEAPON_SMOKEGRENADE,
	WEAPON_ELITE,
	WEAPON_FIVESEVEN,
	WEAPON_UMP45,
	WEAPON_SG550,
	WEAPON_GALIL,
	WEAPON_FAMAS,
	WEAPON_USP,
	WEAPON_GLOCK18,
	WEAPON_AWP,
	WEAPON_MP5N,
	WEAPON_M249,
	WEAPON_M3,
	WEAPON_M4A1,
	WEAPON_TMP,
	WEAPON_G3SG1,
	WEAPON_FLASHBANG,
	WEAPON_DEAGLE,
	WEAPON_SG552,
	WEAPON_AK47,
	WEAPON_KNIFE,
	WEAPON_P90,
	WEAPON_SHIELDGUN = 99
};
//#include "vgui_TeamFortressViewport.h"

#ifndef __linux__

void sincosf (float rad, float *sine, float *cosine)
{
   *sine = sinf (rad);
   *cosine = cosf (rad);
}

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#endif

WEAPON *gpActiveSel;	// NULL means off, 1 means just the menu bar, otherwise
						// this points to the active weapon menu item
WEAPON *gpLastSel;		// Last weapon menu selection 

client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

WeaponsResource gWR;

int g_weaponselect = 0;
int g_iShotsFired;

void WeaponsResource :: LoadAllWeaponSprites( void )
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if ( rgWeapons[i].iId )
			LoadWeaponSprites( &rgWeapons[i] );
	}
}

int WeaponsResource :: CountAmmo( int iId ) 
{ 
	if ( iId < 0 )
		return 0;

	return riAmmo[iId];
}

int WeaponsResource :: HasAmmo( WEAPON *p )
{
	if ( !p )
		return FALSE;

	// weapons with no max ammo can always be selected
	if ( p->iMax1 == -1 )
		return TRUE;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType) 
		|| CountAmmo(p->iAmmo2Type) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
}


void WeaponsResource :: LoadWeaponSprites( WEAPON *pWeapon )
{
	int i, iRes = 640;

	char sz[128];

	if ( !pWeapon )
		return;

	memset( &pWeapon->rcActive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcInactive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo2, 0, sizeof(wrect_t) );
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	sprintf(sz, "sprites/%s.txt", pWeapon->szName);

	if (!strncmp(pWeapon->szName, "knife_", 6))
	{
		char* buffer = (char*)gEngfuncs.COM_LoadFile((char*)sz, 5, nullptr);
		char szSpriteName[32]{};
		int x, y, w, h;
		if (sscanf(buffer, "%s %d %d %d %d", szSpriteName, &x, &y, &w, &h) == 5)
		{
			sprintf(sz, "sprites/%s.spr", szSpriteName);
			pWeapon->hInactive = pWeapon->hActive = SPR_Load(sz);
			pWeapon->rcInactive = pWeapon->rcActive = { x, x + w, y, y + h };

			gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
		}

		gEngfuncs.COM_FreeFile(buffer);
		return;
	}

	client_sprite_t *pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t *p;
	
	/*p = GetSpriteList( pList, "crosshair", iRes, i );
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hCrosshair = SPR_Load(sz);
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = 0;

	p = GetSpriteList(pList, "autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAutoaim = SPR_Load(sz);
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList( pList, "zoom", iRes, i );
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedCrosshair = SPR_Load(sz);
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList(pList, "zoom_autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedAutoaim = SPR_Load(sz);
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;  //default to zoomed crosshair
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}*/

	p = GetSpriteList(pList, "weapon", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hInactive = SPR_Load(sz);
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList(pList, "weapon_s", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hActive = SPR_Load(sz);
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	/*p = GetSpriteList(pList, "ammo", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo = SPR_Load(sz);
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo2 = 0;*/

}

// Returns the first weapon for a given slot.
WEAPON *WeaponsResource :: GetFirstPos( int iSlot )
{
	WEAPON *pret = NULL;

	for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
	{
		if ( rgSlots[iSlot][i] /*&& HasAmmo( rgSlots[iSlot][i] )*/ )
		{
			pret = rgSlots[iSlot][i];
			break;
		}
	}

	return pret;
}


WEAPON* WeaponsResource :: GetNextActivePos( int iSlot, int iSlotPos )
{
	if ( iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS )
		return NULL;

	WEAPON *p = gWR.rgSlots[ iSlot ][ iSlotPos+1 ];
	
	if ( !p || !gWR.HasAmmo(p) )
		return GetNextActivePos( iSlot, iSlotPos + 1 );

	return p;
}


int giBucketHeight, giBucketWidth, giABHeight, giABWidth; // Ammo Bar width and height

HSPRITE ghsprBuckets;					// Sprite for top row of weapons menu

DECLARE_MESSAGE(m_Ammo, CurWeapon ) // Current weapon and clip
DECLARE_MESSAGE(m_Ammo, WeaponList) // new weapon type
DECLARE_MESSAGE(m_Ammo, AmmoX)      // update known ammo type's count
DECLARE_MESSAGE(m_Ammo, AmmoPickup) // flashes an ammo pickup record
DECLARE_MESSAGE(m_Ammo, WeapPickup) // flashes a weapon pickup record
DECLARE_MESSAGE(m_Ammo, HideWeapon) // hides the weapon, ammo, and crosshair displays temporarily
DECLARE_MESSAGE(m_Ammo, ItemPickup)
DECLARE_MESSAGE(m_Ammo, Crosshair)
DECLARE_MESSAGE(m_Ammo, Brass)

DECLARE_COMMAND(m_Ammo, Slot1)
DECLARE_COMMAND(m_Ammo, Slot2)
DECLARE_COMMAND(m_Ammo, Slot3)
DECLARE_COMMAND(m_Ammo, Slot4)
DECLARE_COMMAND(m_Ammo, Slot5)
DECLARE_COMMAND(m_Ammo, Slot6)
DECLARE_COMMAND(m_Ammo, Slot7)
DECLARE_COMMAND(m_Ammo, Slot8)
DECLARE_COMMAND(m_Ammo, Slot9)
DECLARE_COMMAND(m_Ammo, Slot10)
DECLARE_COMMAND(m_Ammo, Close)
DECLARE_COMMAND(m_Ammo, NextWeapon)
DECLARE_COMMAND(m_Ammo, PrevWeapon)
DECLARE_COMMAND(m_Ammo, Adjust_Crosshair)
DECLARE_COMMAND(m_Ammo, Rebuy)
DECLARE_COMMAND(m_Ammo, Autobuy)

// width of ammo fonts
#define AMMO_SMALL_WIDTH 10
#define AMMO_LARGE_WIDTH 20

#define HISTORY_DRAW_TIME	"0"

int CHudAmmo::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(CurWeapon);
	HOOK_MESSAGE(WeaponList);
	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(AmmoX);
	HOOK_MESSAGE(Crosshair);
	HOOK_MESSAGE(Brass);

	HOOK_COMMAND("slot1", Slot1);
	HOOK_COMMAND("slot2", Slot2);
	HOOK_COMMAND("slot3", Slot3);
	HOOK_COMMAND("slot4", Slot4);
	HOOK_COMMAND("slot5", Slot5);
	HOOK_COMMAND("slot6", Slot6);
	HOOK_COMMAND("slot7", Slot7);
	HOOK_COMMAND("slot8", Slot8);
	HOOK_COMMAND("slot9", Slot9);
	HOOK_COMMAND("slot10", Slot10);
	HOOK_COMMAND("cancelselect", Close);
	HOOK_COMMAND("invnext", NextWeapon);
	HOOK_COMMAND("invprev", PrevWeapon);
	HOOK_COMMAND("adjust_crosshair", Adjust_Crosshair);
	HOOK_COMMAND("rebuy", Rebuy);
	HOOK_COMMAND("autobuy", Autobuy);

	Reset();

	m_pHud_DrawHistory_Time = CVAR_CREATE( "hud_drawhistory_time", HISTORY_DRAW_TIME, 0 );
	m_pHud_FastSwitch = CVAR_CREATE( "hud_fastswitch", "0", FCVAR_ARCHIVE );		// controls whether or not weapons can be selected in one keypress
	CVAR_CREATE( "cl_observercrosshair", "1", 0 );
	m_pClCrosshairColor = CVAR_CREATE( "cl_crosshair_color", "50 250 50", FCVAR_ARCHIVE );
	m_pClCrosshairTranslucent = CVAR_CREATE( "cl_crosshair_translucent", "1", FCVAR_ARCHIVE );
	m_pClCrosshairSize = CVAR_CREATE( "cl_crosshair_size", "auto", FCVAR_ARCHIVE );
	m_pClDynamicCrosshair = CVAR_CREATE("cl_dynamiccrosshair", "1", FCVAR_ARCHIVE);
	cl_crosshair_type = CVAR_CREATE("cl_crosshair_type", "0", FCVAR_ARCHIVE);


	m_iFlags = HUD_DRAW | HUD_THINK; //!!!
	m_R = 50;
	m_G = 250;
	m_B = 50;
	m_iAlpha = 200;

	m_cvarB = m_cvarR = m_cvarG = -1;
	m_iCurrentCrosshair = 0;
	m_bAdditive = true;
	m_iCrosshairScaleBase = -1;
	m_bDrawCrosshair = true;

	gWR.Init();
	gHR.Init();

	return 1;
}

void CHudAmmo::Reset(void)
{
	m_fFade = 0;

	gpActiveSel = NULL;
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();

	//	VidInit();

}

int CHudAmmo::VidInit(void)
{
	// Load sprites for buckets (top row of weapon menu)
	m_HUD_bucket0 = gHUD.GetSpriteIndex( "bucket1" );
	m_HUD_selection = gHUD.GetSpriteIndex( "selection" );

	ghsprBuckets = gHUD.GetSprite(m_HUD_bucket0);
	giBucketWidth = gHUD.GetSpriteRect(m_HUD_bucket0).right - gHUD.GetSpriteRect(m_HUD_bucket0).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top;

	gHR.iHistoryGap = max( gHR.iHistoryGap, gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top);

	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

	if (ScreenWidth >= 640)
	{
		giABWidth = 20;
		giABHeight = 4;
	}
	else
	{
		giABWidth = 10;
		giABHeight = 2;
	}

//pb stuff
R_InitTexture(pb_crosshair[0], "gfx/billflx/crosshair/crosshair_laser.tga");
R_InitTexture(pb_crosshair[1], "gfx/billflx/crosshair/crosshair_default.tga");
R_InitTexture(pb_crosshair[2], "gfx/billflx/crosshair/crosshair_grenade.tga");
R_InitTexture(pb_crosshair[3], "gfx/billflx/crosshair/crosshair_default.tga");

R_InitTexture(m_weaponbg, "gfx/billflx/garing");

m_iNum_L = gHUD.GetSpriteIndex("ammo_res");
BuildHudNumberRect(m_iNum_L, m_rcNumber_Large, 13, 13, 1, 1);

m_iNum_L_ammo = gHUD.GetSpriteIndex("upboard_num");
m_iNum_S_ammo = gHUD.GetSpriteIndex("SBNum_S");

BuildHudNumberRect(m_iNum_L_ammo, m_rcNumber_Large_ammo, 30, 55, 1, 1);
BuildHudNumberRect(m_iNum_S_ammo, m_rcNumber_Small_ammo, 10, 10, 1, 1);

R_InitTexture(weaponname_bg, "gfx/billflx/bg_weapon");

R_InitTexture(m_specials[0], "gfx/billflx/equip/gren0.tga");
R_InitTexture(m_specials[1], "gfx/billflx/equip/gren1.tga");
R_InitTexture(m_specials[2], "gfx/billflx/equip/gren2.tga");
R_InitTexture(m_specials[3], "gfx/billflx/equip/med0.tga");
R_InitTexture(m_specials[4], "gfx/billflx/equip/med1.tga");
R_InitTexture(m_specials[5], "gfx/billflx/equip/med2.tga");
R_InitTexture(m_specials[6], "gfx/billflx/equip/smoke0.tga");
R_InitTexture(m_specials[7], "gfx/billflx/equip/smoke1.tga");
R_InitTexture(m_specials[8], "gfx/billflx/equip/smoke2.tga");

	return 1;
}

//
// Think:
//  Used for selection of weapon menu item.
//
void CHudAmmo::Think(void)
{
	if ( gHUD.m_fPlayerDead )
		return;

	if ( gHUD.m_iWeaponBits != gWR.iOldWeaponBits )
	{
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = 0; i < MAX_WEAPONS-1; i++ )
		{
			WEAPON *p = gWR.GetWeapon(i);

			if ( p )
			{
				if ( gHUD.m_iWeaponBits & ( 1 << p->iId ) )
				{
					gWR.PickupWeapon( p );
				}
				else
				{
					gWR.DropWeapon( p );
				}
			}
		}
	}

	if (!gpActiveSel)
		return;

	// has the player selected one?
	/*if (gHUD.m_iKeyBits & IN_ATTACK)
	{*/
		if (gpActiveSel != (WEAPON *)1)
		{
			ServerCmd(gpActiveSel->szName);
			g_weaponselect = gpActiveSel->iId;
		}

		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;

		PlaySound("common/wpn_select.wav", 1);
	//}

}

//
// Helper function to return a Ammo pointer from id
//

HSPRITE* WeaponsResource :: GetAmmoPicFromWeapon( int iAmmoId, wrect_t& rect )
{
	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if ( rgWeapons[i].iAmmoType == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if ( rgWeapons[i].iAmmo2Type == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return NULL;
}


// Menu Selection Code

void WeaponsResource :: SelectSlot( int iSlot, int fAdvance, int iDirection )
{
	if ( gHUD.m_Menu.m_fMenuDisplayed && (fAdvance == FALSE) && (iDirection == 1) )	
	{ // menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem( iSlot + 1 );  // slots are one off the key numbers
		return;
	}

	if ((gHUD.m_ZB2.m_iFlags & HUD_ACTIVE) && (fAdvance == FALSE) && (iDirection == 1))
	{
		if(gHUD.m_ZB2.ActivateSkill(iSlot + 1))
			return;
	}

	if ( iSlot > MAX_WEAPON_SLOTS )
		return;

	if ( gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return;

	if ( ! ( gHUD.m_iWeaponBits & ~(1<<(WEAPON_SUIT)) ))
		return;

	WEAPON *p = NULL;
	bool fastSwitch = gHUD.m_Ammo.m_pHud_FastSwitch->value != 0.0f;

	if ( (gpActiveSel == NULL) || (gpActiveSel == (WEAPON *)1) || (iSlot != gpActiveSel->iSlot) )
	{
		PlaySound( "common/wpn_hudon.wav", 1 );
		p = GetFirstPos( iSlot );

		if ( p && fastSwitch ) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON *p2 = GetNextActivePos( p->iSlot, p->iSlotPos );
			if ( !p2 )
			{	// only one active item in bucket, so change directly to weapon
				ServerCmd( p->szName );
				g_weaponselect = p->iId;
				return;
			}
		}
	}
	else
	{
		PlaySound("common/wpn_moveselect.wav", 1);
		if ( gpActiveSel )
			p = GetNextActivePos( gpActiveSel->iSlot, gpActiveSel->iSlotPos );
		if ( !p )
			p = GetFirstPos( iSlot );
	}

	
	if ( !p )  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it
		if ( !fastSwitch )
			gpActiveSel = (WEAPON *)1;
		else
			gpActiveSel = NULL;
	}
	else 
		gpActiveSel = p;
}

//------------------------------------------------------------------------
// Message Handlers
//------------------------------------------------------------------------

//
// AmmoX  -- Update the count of a known type of ammo
// 
int CHudAmmo::MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	int iIndex = reader.ReadByte();
	// fuck 255 limit
	int iCount;
	if (iSize == 3)
	{
		iCount = reader.ReadShort();
	}
	else
	{
		iCount = reader.ReadByte();
	}

	gWR.SetAmmo( iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	int iIndex = reader.ReadByte();
	int iCount = reader.ReadByte();

	// Add ammo to the history
	gHR.AddToHistory( HISTSLOT_AMMO, iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	int iIndex = reader.ReadByte();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_WEAP, iIndex );

	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	const char *szName = reader.ReadString();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_ITEM, szName );

	return 1;
}


int CHudAmmo::MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	
	gHUD.m_iHideHUDDisplay = reader.ReadByte();

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_FLASHLIGHT | HIDEHUD_ALL ) )
	{
		gpActiveSel = NULL;
		SetCrosshair( 0, nullrc, 0, 0, 0 );
	}
	else
	{
		if ( m_pWeapon )
			SetCrosshair( m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255 );
	}

	return 1;
}

// 
//  CurWeapon: Update hud state with the current weapon and clip count. Ammo
//  counts are updated with AmmoX. Server assures that the Weapon ammo type 
//  numbers match a real ammo type.
//
int CHudAmmo::MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	int iState = reader.ReadByte();
	int iId = reader.ReadChar();
	
	// fuck 128 limit
	int iClip;
	if (iSize == 4)
	{
		iClip = reader.ReadShort();
	}
	else
	{
		iClip = reader.ReadByte();
	}


	if ( iId < 1 )
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 0;
	}

	if ( g_iUser1 != OBS_IN_EYE )
	{
		// Is player dead???
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = TRUE;
			gpActiveSel = NULL;
			return 1;
		}
		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon( iId );

	if ( !pWeapon )
		return 0;

	if ( iClip < -1 )
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;


	if ( iState == 0 )	// we're not the current weapon, so update no more
		return 1;

	m_pWeapon = pWeapon;

	m_fFade = 200.0f; //!!!

	return 1;
}

//
// WeaponList -- Tells the hud about a new weapon type.
//
int CHudAmmo::MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	
	WEAPON Weapon;

	strncpy( Weapon.szName, reader.ReadString(), MAX_WEAPON_NAME );
	Weapon.iAmmoType = (int)reader.ReadChar();
	
	Weapon.iMax1 = reader.ReadByte();
	if (Weapon.iMax1 == 255)
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = reader.ReadChar();
	Weapon.iMax2 = reader.ReadByte();
	if (Weapon.iMax2 == 255)
		Weapon.iMax2 = -1;

	Weapon.iSlot = reader.ReadChar();
	Weapon.iSlotPos = reader.ReadChar();
	Weapon.iId = reader.ReadChar();
	Weapon.iFlags = reader.ReadByte();
	Weapon.iClip = 0;

	gWR.AddWeapon( &Weapon );

	return 1;

}

int CHudAmmo::MsgFunc_Crosshair(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	if( reader.ReadByte() > 0)
	{
		m_bDrawCrosshair = true;
	}
	else
	{
		m_bDrawCrosshair = false;
	}
   return 0;
}

int CHudAmmo::MsgFunc_Brass( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	reader.ReadByte(); // unused!

	Vector origin, velocity;
	origin.x = reader.ReadCoord();
	origin.y = reader.ReadCoord();
	origin.z = reader.ReadCoord();
	reader.ReadCoord(); // unused!
	reader.ReadCoord(); // unused!
	reader.ReadCoord(); // unused!
	velocity.x = reader.ReadCoord();
	velocity.y = reader.ReadCoord();
	velocity.z = reader.ReadCoord();

	float Rotation = M_PI * reader.ReadAngle() / 180.0f;
	int ModelIndex = reader.ReadShort();
	int BounceSoundType = reader.ReadByte();
	int Life = reader.ReadByte();
	int Client = reader.ReadByte();

	float sin, cos, x, y;
	sincosf( fabs(Rotation), &sin, &cos );

	if( gHUD.cl_righthand->value != 0.0f && EV_IsLocal( Client ) )
	{
		velocity.x += sin * -120.0;
		velocity.y += cos * 120.0;
		x = 9.0 * sin;
		y = -9.0 * cos;
	}
	else
	{
		x = -9.0 * sin;
		y = 9.0 * cos;
	}

	origin.x += x;
	origin.y += y;
	EV_EjectBrass( origin, velocity, Rotation, ModelIndex, BounceSoundType, Life );
	return 1;
}

//------------------------------------------------------------------------
// Command Handlers
//------------------------------------------------------------------------
// Slot button pressed
void CHudAmmo::SlotInput( int iSlot )
{
	// Let the Viewport use it first, for menus
	//if ( gViewPort && gViewPort->SlotInput( iSlot ) )
		//return;

	gWR.SelectSlot(iSlot, FALSE, 1);
}

void CHudAmmo::UserCmd_Slot1(void)
{
	SlotInput( 0 );
}

void CHudAmmo::UserCmd_Slot2(void)
{
	SlotInput( 1 );
}

void CHudAmmo::UserCmd_Slot3(void)
{
	SlotInput( 2 );
}

void CHudAmmo::UserCmd_Slot4(void)
{
	SlotInput( 3 );
}

void CHudAmmo::UserCmd_Slot5(void)
{
	SlotInput( 4 );
}

void CHudAmmo::UserCmd_Slot6(void)
{
	SlotInput( 5 );
}

void CHudAmmo::UserCmd_Slot7(void)
{
	SlotInput( 6 );
}

void CHudAmmo::UserCmd_Slot8(void)
{
	SlotInput( 7 );
}

void CHudAmmo::UserCmd_Slot9(void)
{
	SlotInput( 8 );
}

void CHudAmmo::UserCmd_Slot10(void)
{
	SlotInput( 9 );
}

void CHudAmmo::UserCmd_Adjust_Crosshair()
{
	int newCrosshair;
	int oldCrosshair = m_iCurrentCrosshair;

	if ( gEngfuncs.Cmd_Argc() <= 1 )
	{
		newCrosshair = (oldCrosshair + 1) % 5;
	}
	else
	{
		const char *arg = gEngfuncs.Cmd_Argv(1);
		newCrosshair = atoi(arg) % 10;
	}

	m_iCurrentCrosshair = newCrosshair;
	if ( newCrosshair <= 9 )
	{
		switch ( newCrosshair )
		{
		case 0:
		case 5:
			m_R = 50;
			m_G = 250;
			m_B = 50;
			break;
		case 1:
		case 6:
			m_R = 250;
			m_G = 50;
			m_B = 50;
			break;
		case 2:
		case 7:
			m_R = 50;
			m_G = 50;
			m_B = 250;
			break;
		case 3:
		case 8:
			m_R = 250;
			m_G = 250;
			m_B = 50;
			break;
		case 4:
		case 9:
			m_R = 50;
			m_G = 250;
			m_B = 250;
			break;
		}
		m_bAdditive = newCrosshair < 5 ? true: false;
	}
	else
	{
		m_R = 50;
		m_G = 250;
		m_B = 50;
		m_bAdditive = 1;
	}

	char s[16];
	sprintf(s, "%d %d %d", m_R, m_G, m_B);
	gEngfuncs.Cvar_Set("cl_crosshair_color", s);
	gEngfuncs.Cvar_Set("cl_crosshair_translucent", (char*)(m_bAdditive ? "1" : "0"));
}


void CHudAmmo::UserCmd_Close(void)
{
	if (gpActiveSel)
	{
		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		PlaySound("common/wpn_hudoff.wav", 1);
	}
	else
		ClientCmd("escape");
}


// Selects the next item in the weapon menu
void CHudAmmo::UserCmd_NextWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = 0;
	int slot = 0;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos + 1;
		slot = gpActiveSel->iSlot;
	}

	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot < MAX_WEAPON_SLOTS; slot++ )
		{
			for ( ; pos < MAX_WEAPON_POSITIONS; pos++ )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp /*&& gWR.HasAmmo(wsp)*/ )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = 0;
		}

		slot = 0;  // start looking from the first slot again
	}

	gpActiveSel = NULL;
}

// Selects the previous item in the menu
void CHudAmmo::UserCmd_PrevWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = MAX_WEAPON_POSITIONS-1;
	int slot = MAX_WEAPON_SLOTS-1;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos - 1;
		slot = gpActiveSel->iSlot;
	}
	
	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot >= 0; slot-- )
		{
			for ( ; pos >= 0; pos-- )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp /*&& gWR.HasAmmo(wsp)*/ )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS-1;
		}
		
		slot = MAX_WEAPON_SLOTS-1;
	}

	gpActiveSel = NULL;
}

void CHudAmmo::UserCmd_Autobuy()
{
	char *afile = (char*)gEngfuncs.COM_LoadFile("autobuy.txt", 5, NULL);
	char *pfile = afile;
	char token[256];
	char szCmd[1024];

	if( !pfile )
	{
		ConsolePrint( "Can't open autobuy.txt file.\n" );
		return;
	}

	strncpy(szCmd, "cl_setautobuy", sizeof(szCmd));

	while((pfile = gEngfuncs.COM_ParseFile( pfile, token )))
	{
		// append space first
		strcat(szCmd, " ");
		strcat(szCmd, token);
	}

	ConsolePrint( szCmd );
	gEngfuncs.pfnServerCmd( szCmd );

	gEngfuncs.COM_FreeFile( afile );
}

void CHudAmmo::UserCmd_Rebuy()
{
	char *afile = (char*)gEngfuncs.COM_LoadFile("rebuy.txt", 5, NULL);
	char *pfile = afile;
	char token[64];
	char szCmd[1024];
	int lastCh;

	if( !pfile )
	{
		ConsolePrint( "Can't open rebuy.txt file.\n" );
		return;
	}

	// start with \"
	strncpy(szCmd, "cl_setrebuy \"", sizeof(szCmd));



	while((pfile = gEngfuncs.COM_ParseFile( pfile, token )))
	{
		strcat(szCmd, token);
		// append space after token
		strcat(szCmd, " ");
	}
	// replace last space with ", before terminator
	lastCh = strlen(szCmd);
	szCmd[lastCh - 1] = '\"';

	ConsolePrint(szCmd);
	gEngfuncs.pfnServerCmd(szCmd);

	gEngfuncs.COM_FreeFile( afile );
}


//-------------------------------------------------------------------------
// Drawing code
//-------------------------------------------------------------------------

void CHudAmmo::BuildHudNumberRect(int moe, wrect_t *prc, int w, int h, int xOffset, int yOffset)
{
	wrect_t rc = gHUD.GetSpriteRect(moe);
	int x = rc.left;
	int y = rc.top;

	for (int i = 0; i < 10; i++)
	{
		prc[i].left = x;
		prc[i].top = 0;
		prc[i].right = prc[i].left + w + xOffset;
		prc[i].bottom = h + yOffset;

		x += w;
		y += h;
	}
}

int CHudAmmo::DrawHudNumberBill(int moe, wrect_t *prc, int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = prc[0].right - prc[0].left;
	int k;
	wrect_t rc;

	if (iNumber >= 10000)
	{
		k = iNumber / 10000;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 1000)
	{
		k = (iNumber % 10000) / 1000;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 100)
	{
		k = (iNumber % 1000) / 100;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 10)
	{
		k = (iNumber % 100) / 10;
		rc = prc[k];
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &rc);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	k = iNumber % 10;
	SPR_Set(gHUD.GetSprite(moe), r, g, b);
	SPR_DrawAdditive(0, x, y, &prc[k]);
	x += iWidth;

	return x;
}


int CHudAmmo::GetHudNumberWidth(int moe, wrect_t *prc, int iFlags, int iNumber)
{
	int iWidth = prc[0].right - prc[0].left;
	int k;
	wrect_t rc;
	int x = 0;

	if (iNumber >= 10000)
	{
		k = iNumber / 10000;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS))
		x += iWidth;

	if (iNumber >= 1000)
	{
		k = (iNumber % 10000) / 1000;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
		x += iWidth;

	if (iNumber >= 100)
	{
		k = (iNumber % 1000) / 100;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
		x += iWidth;

	if (iNumber >= 10)
	{
		k = (iNumber % 100) / 10;
		rc = prc[k];
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
		x += iWidth;

	k = iNumber % 10;
	x += iWidth;

	return x;
}

char *UTIL_VarArgs(const char *format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	return string;
}

int CHudAmmo::Draw(float flTime)
{
	int a, x, y, r, g, b;
	int AmmoWidth;
	static bool switchCrosshairType = false;

	int x2 = ScreenWidth / 2; 
	int y2 = ScreenHeight / 2;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;

	WEAPON *pw = m_pWeapon;


	// place it here, so pretty dynamic crosshair will work even in spectator!
	if( gHUD.m_iFOV > 40 )
	{
		if( switchCrosshairType )
		{
			SetCrosshair( 0, nullrc, 0, 0, 0);
			switchCrosshairType = false;
		}


// draw pb crosshair


//laser for scope
if(gHUD.reddot_scope)
{

if (gHUD.m_iKeyBits & IN_ATTACK & pw->iClip > 0)
{

if (((int)(flTime * 20) % 2))
{				
pb_crosshair[0]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 6, y2 - 6,x2 + 6,y2 + 6 );
}
else
{
//up
pb_crosshair[0]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 6, y2 - 12,x2 + 6,y2 - 2 );
}	

}
else
{
pb_crosshair[0]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 6, y2 - 6,x2 + 6,y2 + 6 );
}

}


//laser
if (gHUD.m_pb_crosshair->value == 0 && gHUD.m_iFOV > 55)
{

if (gHUD.m_iKeyBits & IN_ATTACK & pw->iClip > 0)
{

if (((int)(flTime * 20) % 2))
{				
pb_crosshair[0]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 4, y2 - 4,x2 + 4,y2 + 4 );
}
else
{
//up
pb_crosshair[0]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 4, y2 - 10,x2 + 4,y2 + 0 );
}	

}
else
{
pb_crosshair[0]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 4, y2 - 4,x2 + 4,y2 + 4 );
}

}
//normal green
else if (gHUD.m_pb_crosshair->value == 1 && gHUD.m_iFOV > 55)
{
pb_crosshair[1]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 5, y2 - 5,x2 + 5,y2 + 5 );
}
//grenade 
else if (gHUD.m_pb_crosshair->value == 2 && gHUD.m_iFOV > 55)
{
pb_crosshair[2]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuadScaled(x2 - 75, y2 - 75,x2 + 75,y2 + 75 );
}
//sniper not visible 
else if (gHUD.m_pb_crosshair->value == 3 && gHUD.m_iFOV > 55)
{
pb_crosshair[0]->Bind();
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 0);
gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
DrawUtils::Draw2DQuad((TrueWidth - TrueHeight)/2, 0, (TrueWidth - TrueHeight)/2 + TrueHeight, TrueHeight);
}

	}
	else
	{
		if( !switchCrosshairType )
		{
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);
			switchCrosshairType = true;
		}
	}

	if ( (gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL )) )
		return 1;

	// Draw Weapon Menu
	DrawWList(flTime);

	// Draw ammo pickup history
	gHR.DrawAmmoHistory( flTime );

	if (!m_pWeapon)
		return 0;

	

	// SPR_Draw Ammo
	if ((pw->iAmmoType < 0) && (pw->iAmmo2Type < 0))
		return 0;

	int iFlags = DHN_DRAWZERO; // draw 0 values
//pb stuff 
int yxx = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight/ 1;

	AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

	a = max( MIN_ALPHA, (int)m_fFade );

	if (m_fFade > 0)
		m_fFade -= (gHUD.m_flTimeDelta * 20);

	// Does this weapon have a clip?
	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight/0.75;

	//pb stuff 
		int iIconWidth = m_pWeapon->rcAmmo.right - m_pWeapon->rcAmmo.left;
		x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;


int xr = ScreenWidth - (8 * AmmoWidth) - iIconWidth;

//garing
gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 200);
m_weaponbg->Bind();
DrawUtils::Draw2DQuadScaled(xr - 5, yxx - gHUD.m_iFontHeight / 1, xr + (10 * AmmoWidth) + iIconWidth, yxx + gHUD.m_iFontHeight );

int iBarWidth =  AmmoWidth/10;

int ytt = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight/10;
int xtt = ScreenWidth - (12 * AmmoWidth) - iIconWidth;

gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
weaponname_bg->Bind();
DrawUtils::Draw2DQuadScaled(xtt, ytt - gHUD.m_iFontHeight / 6, xtt + (12 * AmmoWidth) + iIconWidth, ScreenHeight - 6);

const char *weaponF;
int xt = ScreenWidth - (10 * AmmoWidth) - iIconWidth;
weaponF = CVAR_GET_STRING("weaponName");
gEngfuncs.pfnDrawSetTextColor(255, 255, 255);
gEngfuncs.pfnDrawConsoleString(xt / 1, ytt - gHUD.m_iFontHeight / 15, weaponF);


if (m_pWeapon->iAmmoType > 0) //mag
{
DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
DrawUtils::ScaleColors(r, g, b, a);		
if (pw->iClip >= 0)
{
x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;

//new
int NumAmmo = GetHudNumberWidth(m_iNum_S_ammo, m_rcNumber_Small_ammo, DHN_2DIGITS | DHN_DRAWZERO, pw->iClip);
if (NumAmmo > 0)
x = DrawHudNumberBill(m_iNum_L_ammo, m_rcNumber_Large_ammo, xr - 30, y - 30, iFlags|DHN_3DIGITS, pw->iClip, 255, 255, 255);

x += AmmoWidth/2;
DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);
x += iBarWidth + AmmoWidth/2;;
DrawUtils::ScaleColors(r, g, b, a );
//reserve 
x = DrawUtils::DrawHudNumber2(x, yxx - 10, true,3, gWR.CountAmmo(pw->iAmmoType), 150, 150, 150);
	}
	else//knife 
	{
DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
DrawUtils::ScaleColors(r, g, b, a);

if (pw->iClip < 3)
{
x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;

//new
int NumAmmo = GetHudNumberWidth(m_iNum_S_ammo, m_rcNumber_Small_ammo, DHN_2DIGITS | DHN_DRAWZERO, gWR.CountAmmo(pw->iAmmoType));
if (NumAmmo > 0)
x = DrawHudNumberBill(m_iNum_L_ammo, m_rcNumber_Large_ammo, xr - 30, y - 30, iFlags|DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), 255, 255, 255);

x += AmmoWidth/2;
DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);
x += iBarWidth + AmmoWidth/2;;
DrawUtils::ScaleColors(r, g, b, a );
//reserve 
x = DrawUtils::DrawHudNumber2(x, yxx - 10, true,3, gWR.CountAmmo(pw->iAmmoType), 150, 150, 150);
	}

}




}
	return 1;
}

#define WEST_XPOS (ScreenWidth / 2 - flCrosshairDistance - iLength + 1)
#define EAST_XPOS (flCrosshairDistance + ScreenWidth / 2)
#define EAST_WEST_YPOS (ScreenHeight / 2)

#define NORTH_YPOS (ScreenHeight / 2 - flCrosshairDistance - iLength + 1)
#define SOUTH_YPOS (ScreenHeight / 2 + flCrosshairDistance)
#define NORTH_SOUTH_XPOS (ScreenWidth / 2)

int Distances[30][2] =
{
{ 8, 3 }, // 0
{ 4, 3 }, // 1
{ 5, 3 }, // 2
{ 8, 3 }, // 3
{ 9, 4 }, // 4
{ 6, 3 }, // 5
{ 9, 3 }, // 6
{ 3, 3 }, // 7
{ 8, 3 }, // 8
{ 4, 3 }, // 9
{ 8, 3 }, // 10
{ 6, 3 }, // 11
{ 5, 3 }, // 12
{ 4, 3 }, // 13
{ 4, 3 }, // 14
{ 8, 3 }, // 15
{ 8, 3 }, // 16
{ 8, 3 }, // 17
{ 6, 3 }, // 18
{ 6, 3 }, // 19
{ 8, 6 }, // 20
{ 4, 3 }, // 21
{ 7, 3 }, // 22
{ 6, 4 }, // 23
{ 8, 3 }, // 24
{ 8, 3 }, // 25
{ 5, 3 }, // 26
{ 4, 4 }, // 27
{ 7, 3 }, // 28
{ 7, 3 }, // 29
};

void CHudAmmo::DrawCrosshair( float flTime )
{
	int flags, iDeltaDistance, iDistance, iLength, weaponid;
	float flCrosshairDistance;

	if( !m_pWeapon )
		return;

	weaponid = m_pWeapon->iId;

	if(    weaponid == WEAPON_AWP 
		|| weaponid == WEAPON_SCOUT 
		|| weaponid == WEAPON_SG550 
		|| weaponid == WEAPON_G3SG1 )
		return;

	if ( g_iWeaponFlags & WPNSTATE_SHIELD_DRAWN )
		return;

	if( weaponid <= 30 )
	{
		iDistance = Distances[weaponid - 1][0];
		iDeltaDistance = Distances[weaponid - 1][1];
	}
	else
	{
		iDistance = 4;
		iDeltaDistance = 3;
	}
	
	flags = GetWeaponAccuracyFlags( weaponid );
	if ( flags && m_pClDynamicCrosshair->value && !(gHUD.m_iHideHUDDisplay & 1) )
	{
		if ( g_iPlayerFlags & FL_ONGROUND || !( flags & ACCURACY_AIR ) )
		{
			if ( ( g_iPlayerFlags & FL_DUCKING ) && ( flags & ACCURACY_DUCK ) )
			{
				iDistance *= 0.5;
			}
			else
			{
				int iWeaponSpeed = 0;

				switch( weaponid )
				{
				case WEAPON_P90: // p90
					iWeaponSpeed = 170;
					break;
				case WEAPON_AUG: // aug
				case WEAPON_GALIL: // galil
				case WEAPON_FAMAS: // famas
				case WEAPON_M249: // m249
				case WEAPON_M4A1: // m4a1
				case WEAPON_SG552: // sg552
				case WEAPON_AK47: // ak47
					iWeaponSpeed = 140;
					break;
				}

				if ( (flags & ACCURACY_SPEED) && (g_flPlayerSpeed >= iWeaponSpeed) )
					iDistance *= 1.5;
			}
		}
		else 
		{
			iDistance *= 2;
		}
		if ( flags & ACCURACY_MULTIPLY_BY_14 )
			iDistance *= 1.4;
		if ( flags & ACCURACY_MULTIPLY_BY_14_2 )
			iDistance *= 1.4;
	}

	if ( m_iAmmoLastCheck >= g_iShotsFired )
	{
		m_flCrosshairDistance -= (m_flCrosshairDistance * 0.013 + 0.1 );
		m_iAlpha += 2;
	}
	else
	{
		m_flCrosshairDistance = min( m_flCrosshairDistance + iDeltaDistance, 15.0f );
		m_iAlpha = max( m_iAlpha - 40, 120 );
	}

	if ( g_iShotsFired > 600 )
		g_iShotsFired = 1;

	CalcCrosshairColor();
	CalcCrosshairDrawMode();
	CalcCrosshairSize();

	m_iAmmoLastCheck = g_iShotsFired;
	m_flCrosshairDistance = max( m_flCrosshairDistance, (float)iDistance );
	iLength = (m_flCrosshairDistance - iDistance) * 0.5 + 5;
	
	if ( m_iAlpha > 255 )
		m_iAlpha = 255;
	
	if ( ScreenWidth != m_iCrosshairScaleBase )
	{
		flCrosshairDistance = ScreenWidth * m_flCrosshairDistance / m_iCrosshairScaleBase;
		iLength = ScreenWidth * iLength / m_iCrosshairScaleBase;
	}
	else
	{
		flCrosshairDistance = m_flCrosshairDistance;
	}


	// drawing
	/*if ( gHUD.m_NVG.m_iFlags )
	{
		FillRGBABlend(WEST_XPOS, EAST_WEST_YPOS,	iLength, 1, 250, 50, 50, m_iAlpha);
		FillRGBABlend(EAST_XPOS, EAST_WEST_YPOS,	iLength, 1, 250, 50, 50, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, NORTH_YPOS, 1, iLength, 250, 50, 50, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, SOUTH_YPOS, 1, iLength, 250, 50, 50, m_iAlpha);
	}
	else if ( m_bAdditive )
	{
		FillRGBA(WEST_XPOS, EAST_WEST_YPOS,		iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBA(EAST_XPOS, EAST_WEST_YPOS,		iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBA(NORTH_SOUTH_XPOS,	NORTH_YPOS,	1, iLength, m_R, m_G, m_B, m_iAlpha);
		FillRGBA(NORTH_SOUTH_XPOS, SOUTH_YPOS,	1, iLength, m_R, m_G, m_B, m_iAlpha);
	}
	else
	{
		FillRGBABlend(WEST_XPOS, EAST_WEST_YPOS,	iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBABlend(EAST_XPOS, EAST_WEST_YPOS,	iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, NORTH_YPOS, 1, iLength, m_R, m_G, m_B, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, SOUTH_YPOS, 1, iLength, m_R, m_G, m_B, m_iAlpha);
	}*/
	if (gHUD.m_NVG.m_iFlags)
		DrawCrosshairEx(flTime, weaponid, iLength, flCrosshairDistance, false, 250, 50, 50, m_iAlpha);
	else
		DrawCrosshairEx(flTime, weaponid, iLength, flCrosshairDistance, m_bAdditive, m_R, m_G, m_B, m_iAlpha);
	return;
}



int CHudAmmo::DrawCrosshairEx(float flTime, int weaponid, int iBarSize, float flCrosshairDistance, bool bAdditive, int r, int g, int b, int a)
{
	bool bDrawPoint = false;
	bool bDrawCircle = false;
	bool bDrawCross = false;

	void(*pfnFillRGBA)(int x, int y, int w, int h, int r, int g, int b, int a) = (bAdditive == false) ? gEngfuncs.pfnFillRGBABlend : gEngfuncs.pfnFillRGBA;

	switch ((int)cl_crosshair_type->value)
	{
	case 1:
	{
		bDrawPoint = true;
		bDrawCross = true;
		break;
	}

	case 2:
	{
		bDrawPoint = true;
		bDrawCircle = true;
		break;
	}

	case 3:
	{
		bDrawPoint = true;
		bDrawCircle = true;
		bDrawCross = true;
		break;
	}

	case 4:
	{
		bDrawPoint = true;
		break;
	}

	default:
	{
		bDrawCross = true;
		break;
	}
	}

	if (bDrawCircle)
	{
		float radius = (iBarSize / 2) + flCrosshairDistance;
#if 0
		switch (m_iCrosshairScaleBase)
		{
		case 640: radius += 0.1; break;
		case 800: radius += 0.8; break;
		case 1024: radius += 0.5; break;
		}
#endif
		int count = (int)((cos(0.7853981852531433) * radius) + 0.5);

		for (int i = 0; i < count; i++)
		{
			int size = sqrt((radius * radius) - (float)(i * i));

			pfnFillRGBA((ScreenWidth / 2) + i, (ScreenHeight / 2) + size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + i, (ScreenHeight / 2) - size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - i, (ScreenHeight / 2) + size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - i, (ScreenHeight / 2) - size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + size, (ScreenHeight / 2) + i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + size, (ScreenHeight / 2) - i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - size, (ScreenHeight / 2) + i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - size, (ScreenHeight / 2) - i, 1, 1, r, g, b, a);
		}
	}

	if (bDrawPoint)
		pfnFillRGBA((ScreenWidth / 2) - 1, (ScreenHeight / 2) - 1, 3, 3, r, g, b, a);

	if (bDrawCross)
	{
		pfnFillRGBA((ScreenWidth / 2) - (int)flCrosshairDistance - iBarSize + 1, ScreenHeight / 2, iBarSize, 1, r, g, b, a);
		pfnFillRGBA((ScreenWidth / 2) + (int)flCrosshairDistance, ScreenHeight / 2, iBarSize, 1, r, g, b, a);
		pfnFillRGBA(ScreenWidth / 2, (ScreenHeight / 2) - (int)flCrosshairDistance - iBarSize + 1, 1, iBarSize, r, g, b, a);
		pfnFillRGBA(ScreenWidth / 2, (ScreenHeight / 2) + (int)flCrosshairDistance, 1, iBarSize, r, g, b, a);
	}

	return 1;
}


void CHudAmmo::CalcCrosshairSize()
{
	static char prevSize[64] = { 0 };
	const char *size = m_pClCrosshairSize->string;

	if( !strncmp( prevSize, size, sizeof(prevSize) ) )
		return;
	
	strncpy( prevSize, size, sizeof(prevSize) );
	
	if( !stricmp(size, "auto") )
	{
		if( ScreenWidth <= 640 )
		{
			m_iCrosshairScaleBase = 1024;
		}
		else if( ScreenWidth <= 1024 )
		{
			m_iCrosshairScaleBase = 800;
		}
		else
		{
			m_iCrosshairScaleBase = 640;
		}
	}
	else if( !stricmp( size, "small" ))
	{
		m_iCrosshairScaleBase = 1024;
	}
	else if( !stricmp( size, "medium" ))
	{
		m_iCrosshairScaleBase = 800;
	}
	else if( !stricmp( size, "large" ))
	{
		m_iCrosshairScaleBase = 640;
	}
	return;
}

void CHudAmmo::CalcCrosshairDrawMode()
{
	static float prevDrawMode = -1;
	float drawMode = m_pClCrosshairTranslucent->value;
	
	if( drawMode == prevDrawMode )
		return;
	
	if ( drawMode == 0.0f )
	{
		m_bAdditive = 0;
	}
	else if ( drawMode == 1.0f )
	{
		m_bAdditive = 1;
	}
	else
	{
		gEngfuncs.Con_Printf("usage: cl_crosshair_translucent <1|0>\n");
		gEngfuncs.Cvar_Set("cl_crosshair_translucent", (char*)"1");
	}
	
	prevDrawMode = drawMode;
}

void CHudAmmo::CalcCrosshairColor()
{
	static char prevColors[64] = { 0 };
	const char *colors = m_pClCrosshairColor->string;

	if( strncmp( prevColors, colors, 64 ) )
	{
		strncpy( prevColors, colors, 64 );
	
		sscanf( colors, "%d %d %d", &m_cvarR, &m_cvarG, &m_cvarB);

		m_R = m_cvarR = bound( 0, m_cvarR, 255 );
		m_G = m_cvarG = bound( 0, m_cvarG, 255 );
		m_B = m_cvarB = bound( 0, m_cvarB, 255 );
	}
}

//
// Draws the ammo bar on the hud
//
int DrawBar(int x, int y, int width, int height, float f)
{
	int r, g, b;

	f = bound( 0, f, 1 );
	
	if (f)
	{
		int w = f * width;

		// Always show at least one pixel if we have ammo.
		if (w <= 0)
			w = 1;
		DrawUtils::UnpackRGB(r, g, b, RGB_GREENISH);
		FillRGBA(x, y, w, height, r, g, b, 255);
		x += w;
		width -= w;
	}

	DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);

	FillRGBA(x, y, width, height, r, g, b, 128);

	return (x + width);
}



void DrawAmmoBar(WEAPON *p, int x, int y, int width, int height)
{
	if ( !p )
		return;
	
	if (p->iAmmoType != -1)
	{
		if (!gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gWR.CountAmmo(p->iAmmoType)/(float)p->iMax1;
		
		x = DrawBar(x, y, width, height, f);


		// Do we have secondary ammo too?

		if (p->iAmmo2Type != -1)
		{
			f = (float)gWR.CountAmmo(p->iAmmo2Type)/(float)p->iMax2;

			x += 5; //!!!

			DrawBar(x, y, width, height, f);
		}
	}
}




//
// Draw Weapon Menu
//
int CHudAmmo::DrawWList(float flTime)
{
	int r,g,b,x,y,a,i;

	if ( !gpActiveSel )
		return 0;

	int iActiveSlot;

	if ( gpActiveSel == (WEAPON *)1 )
		iActiveSlot = -1;	// current slot has no weapons
	else 
		iActiveSlot = gpActiveSel->iSlot;

	x = gHUD.m_Radar.GetRadarSize() + 10; //!!!
	y = 10; //!!!
	

	// Ensure that there are available choices in the active slot
	if ( iActiveSlot > 0 )
	{
		if ( !gWR.GetFirstPos( iActiveSlot ) )
		{
			gpActiveSel = (WEAPON *)1;
			iActiveSlot = -1;
		}
	}
		
	// Draw top line
	for ( i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		int iWidth;

		DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);
	
		if ( iActiveSlot == i )
			a = 255;
		else
			a = 192;

		DrawUtils::ScaleColors(r, g, b, 255);
		SPR_Set(gHUD.GetSprite(m_HUD_bucket0 + i), r, g, b );

		// make active slot wide enough to accommodate gun pictures
		if ( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos(iActiveSlot);
			if ( p )
				iWidth = p->rcActive.right - p->rcActive.left;
			else
				iWidth = giBucketWidth;
		}
		else
			iWidth = giBucketWidth;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_bucket0 + i));
		
		x += iWidth + 5;
	}


	a = 128; //!!!
	x = gHUD.m_Radar.GetRadarSize() + 10; //!!!;

	// Draw all of the buckets
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		y = giBucketHeight + 10;

		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		if ( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos( i );
			int iWidth = giBucketWidth;
			if ( p )
				iWidth = p->rcActive.right - p->rcActive.left;

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				p = gWR.GetWeaponSlot( i, iPos );

				if ( !p || !p->iId )
					continue;

			
				// if active, then we must have ammo.
				if ( gWR.HasAmmo(p) )
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH );
					DrawUtils::ScaleColors(r, g, b, 192);
				}
				else
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_REDISH);
					DrawUtils::ScaleColors(r, g, b, 128);
				}


				if ( gpActiveSel == p )
				{
					SPR_Set(p->hActive, r, g, b );
					SPR_DrawAdditive(0, x, y, &p->rcActive);

					SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b );
					SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
				}
				else
				{
					// Draw Weapon if Red if no ammo
					SPR_Set( p->hInactive, r, g, b );
					SPR_DrawAdditive( 0, x, y, &p->rcInactive );
				}

				// Draw Ammo Bar

				DrawAmmoBar(p, x + giABWidth/2, y, giABWidth, giABHeight);
				
				y += p->rcActive.bottom - p->rcActive.top + 5;
			}

			x += iWidth + 5;

		}
		else
		{
			// Draw Row of weapons.

			DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				WEAPON *p = gWR.GetWeaponSlot( i, iPos );
				
				if ( !p || !p->iId )
					continue;

				if ( gWR.HasAmmo(p) )
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);
					a = 128;
				}
				else
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_REDISH);
					a = 96;
				}

				FillRGBA( x, y, giBucketWidth, giBucketHeight, r, g, b, a );

				y += giBucketHeight + 5;
			}

			x += giBucketWidth + 5;
		}
	}	

	return 1;

}


/*
=================================
	GetSpriteList

Finds and returns the matching 
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
=================================
*/
client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount)
{
	if (!pList)
		return NULL;

	int i = iCount;
	client_sprite_t *p = pList;

	while(i--)
	{
		if ((!strcmp(psz, p->szName)) && (p->iRes == iRes))
			return p;
		p++;
	}

	return NULL;
}
