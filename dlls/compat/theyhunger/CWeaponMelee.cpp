#include "../../extdll.h"
#include "../../player/CBasePlayer.h"
#include "../../weapon/weapons.h"
#include "CWeaponMelee.h"

enum shovel_e
{
	SHOVEL_IDLE = 0,
	SHOVEL_DRAW,
	SHOVEL_HOLSTER,
	SHOVEL_ATTACK1HIT,
	SHOVEL_ATTACK1MISS,
	SHOVEL_ATTACK2MISS,
	SHOVEL_ATTACK2HIT,
	SHOVEL_ATTACK3MISS,
	SHOVEL_ATTACK3HIT,
	SHOVEL_IDLE2,
	SHOVEL_IDLE3,
};

enum spanner_e
{
	SPANNER_IDLE = 0,
	SPANNER_ATTACK1,
	SPANNER_ATTACK2,
	SPANNER_USE,
	SPANNER_DRAW,
	SPANNER_HOLSTER,
};

namespace
{
#ifndef CLIENT_DLL
const char* const kSkeletonHitSounds[] = {
	"kelly/cbar_hitkelly1.wav",
	"kelly/cbar_hitkelly2.wav",
	"kelly/cbar_hitkelly3.wav",
};
#endif

}

bool CWeaponTheyHungerMelee::IsPhysicalBodyHit(CBaseEntity* entity) const
{
	return entity && entity->Classify() != CLASS_NONE && !entity->IsMachine() &&
		!entity->IsBSPModel();
}

const char* CWeaponTheyHungerMelee::PhysicalBodyHitSound(CBaseEntity* entity) const
{
#ifndef CLIENT_DLL
	if (FClassnameIs(entity->pev, "monster_alien_slave") ||
		FClassnameIs(entity->pev, "monster_th_babykelly"))
		return kSkeletonHitSounds[RANDOM_LONG(0, 2)];
#endif
	return CCrowbar::PhysicalBodyHitSound(entity);
}

float CWeaponTheyHungerMelee::DesktopDamage()
{
#ifdef CLIENT_DLL
	return 0.0f;
#else
	const bool multiplayer = g_pGameRules->IsMultiplayer();
	const bool firstSwing = m_flNextPrimaryAttack + 1.0f < UTIL_WeaponTimeBase();
	return GetDamage(gSkillData.sk_plr_crowbar) *
		DesktopDamageMultiplier(multiplayer, firstSwing);
#endif
}

void CWeaponTheyHungerMelee::SetDesktopSwingIdle()
{
	CBasePlayer* player = GetPlayer();
	if (player)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() +
			UTIL_SharedRandomFloat(player->random_seed, 10, 15);
}

Vector CWeaponTheyHungerMelee::DesktopSwingOrigin(CBasePlayer* player) const
{
	return player->GetWeaponPosition();
}

void CWeaponTheyHungerMelee::PrecacheTheyHungerMelee()
{
	CBasePlayerWeapon::Precache();
	PrecacheSharedMeleeSounds();
	PRECACHE_SOUND("kelly/cbar_hitkelly1.wav");
	PRECACHE_SOUND("kelly/cbar_hitkelly2.wav");
	PRECACHE_SOUND("kelly/cbar_hitkelly3.wav");
}

LINK_ENTITY_TO_CLASS(weapon_th_shovel, CWeaponTheyHungerShovel)

void CWeaponTheyHungerShovel::Spawn()
{
	m_defaultModelV = "models/v_shovel.mdl";
	m_defaultModelP = "models/p_shovel.mdl";
	m_defaultModelW = "models/w_shovel.mdl";
	m_hasHandModels = true;
	Precache();
	m_iId = TH_WEAPON_SHOVEL;
	SetWeaponModelW();
	m_iClip = WEAPON_NOCLIP;
	FallInit();
	InitializePhysicalSwingState();
}

void CWeaponTheyHungerShovel::Precache()
{
	PrecacheTheyHungerMelee();
}

int CWeaponTheyHungerShovel::GetItemInfo(ItemInfo* info)
{
	info->pszName = STRING(pev->classname);
	info->pszAmmo1 = info->pszAmmo2 = NULL;
	info->iMaxClip = WEAPON_NOCLIP;
	info->iSlot = 0;
	info->iPosition = 1;
	info->iFlags = 0;
	info->iId = m_iId = TH_WEAPON_SHOVEL;
	info->iWeight = 0;
	return 1;
}

BOOL CWeaponTheyHungerShovel::Deploy()
{
	return DefaultDeploy(GetModelV(), GetModelP(), SHOVEL_DRAW, "crowbar");
}

int CWeaponTheyHungerShovel::HolsterAnimation() const { return SHOVEL_HOLSTER; }
int CWeaponTheyHungerShovel::DesktopMissAnimation()
{
	CBasePlayer* player = GetPlayer();
	if (player && UTIL_SharedRandomFloat(player->random_seed, 0, 1) <= 0.25f)
		++m_iSwing;
	return (m_iSwing++ % 2) == 0 ? SHOVEL_ATTACK1MISS : SHOVEL_ATTACK2MISS;
}
int CWeaponTheyHungerShovel::DesktopHitAnimation()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0: return SHOVEL_ATTACK1HIT;
	case 1: return SHOVEL_ATTACK2HIT;
	default: return SHOVEL_ATTACK3HIT;
	}
}

float CWeaponTheyHungerShovel::DesktopDamageMultiplier(bool multiplayer, bool firstSwing) const
{
	return multiplayer ? 1.6f : firstSwing ? 1.2f : 0.6f;
}

void CWeaponTheyHungerShovel::WeaponIdle()
{
	CBasePlayer* player = GetPlayer();
	if (!player || m_flTimeWeaponIdle >= UTIL_WeaponTimeBase())
		return;
	const float random = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
	if (random > 0.9f)
	{
		SendWeaponAnim(SHOVEL_IDLE2);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 160.0f / 30.0f;
	}
	else if (random > 0.5f)
	{
		SendWeaponAnim(SHOVEL_IDLE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0f / 30.0f;
	}
	else
	{
		SendWeaponAnim(SHOVEL_IDLE3);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 160.0f / 30.0f;
	}
}

LINK_ENTITY_TO_CLASS(weapon_th_spanner, CWeaponTheyHungerSpanner)

void CWeaponTheyHungerSpanner::Spawn()
{
	m_defaultModelV = "models/v_tfc_spanner.mdl";
	m_defaultModelP = "models/p_spanner.mdl";
	m_defaultModelW = "models/backpack.mdl";
	m_hasHandModels = true;
	Precache();
	m_iId = TH_WEAPON_SPANNER;
	SetWeaponModelW();
	m_iClip = WEAPON_NOCLIP;
	FallInit();
	InitializePhysicalSwingState();
}

void CWeaponTheyHungerSpanner::Precache()
{
	PrecacheTheyHungerMelee();
}

int CWeaponTheyHungerSpanner::GetItemInfo(ItemInfo* info)
{
	info->pszName = STRING(pev->classname);
	info->pszAmmo1 = info->pszAmmo2 = NULL;
	info->iMaxClip = WEAPON_NOCLIP;
	info->iSlot = 0;
	info->iPosition = 2;
	info->iFlags = 0;
	info->iId = m_iId = TH_WEAPON_SPANNER;
	info->iWeight = 0;
	return 1;
}

BOOL CWeaponTheyHungerSpanner::Deploy()
{
	return DefaultDeploy(GetModelV(), GetModelP(), SPANNER_DRAW, "crowbar");
}

int CWeaponTheyHungerSpanner::HolsterAnimation() const { return SPANNER_HOLSTER; }
int CWeaponTheyHungerSpanner::DesktopMissAnimation()
{
	CBasePlayer* player = GetPlayer();
	if (player && UTIL_SharedRandomFloat(player->random_seed, 0, 1) <= 0.25f)
		++m_iSwing;
	return (m_iSwing++ % 2) == 0 ? SPANNER_ATTACK1 : SPANNER_ATTACK2;
}
int CWeaponTheyHungerSpanner::DesktopHitAnimation()
{
	return (m_iSwing++ % 2) == 0 ? SPANNER_ATTACK1 : SPANNER_ATTACK2;
}

float CWeaponTheyHungerSpanner::DesktopDamageMultiplier(bool multiplayer, bool firstSwing) const
{
	return multiplayer ? 1.2f : firstSwing ? 0.8f : 0.4f;
}

void CWeaponTheyHungerSpanner::WeaponIdle()
{
	CBasePlayer* player = GetPlayer();
	if (!player || m_flTimeWeaponIdle >= UTIL_WeaponTimeBase())
		return;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() +
		UTIL_SharedRandomFloat(player->random_seed, 5, 10);
	SendWeaponAnim(SPANNER_IDLE);
}
