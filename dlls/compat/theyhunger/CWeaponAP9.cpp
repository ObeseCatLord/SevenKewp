#include "../../extdll.h"
#include "../../weapon/weapons.h"
#include "../../player/CBasePlayer.h"
#include "../../weapon/CBasePlayerAmmo.h"
#include "CWeaponAP9.h"
#include "../../../game_shared/compat/theyhunger/ap9.h"

LINK_ENTITY_TO_CLASS(weapon_th_ap9, CWeaponTheyHungerAP9)

#if !defined(CLIENT_DLL)
TYPEDESCRIPTION CWeaponTheyHungerAP9::m_SaveData[] =
{
	DEFINE_FIELD(CWeaponTheyHungerAP9, m_burstShots, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CWeaponTheyHungerAP9, CBasePlayerWeapon)
#endif

namespace
{
Vector AP9AimVector(CBasePlayer* player, BOOL useAutoAim)
{
	if (useAutoAim && !player->HasFreshVRPose())
		return player->GetAutoaimVector(AUTOAIM_10DEGREES);

#if defined(CLIENT_DLL)
	float forward[3];
	gEngfuncs.pfnAngleVectors(player->GetWeaponViewAngles(), forward, NULL, NULL);
	return Vector(forward);
#else
	UTIL_MakeVectors(player->GetWeaponViewAngles());
	return gpGlobals->v_forward;
#endif
}
}

void CWeaponTheyHungerAP9::Spawn()
{
	Precache();
	m_iId = TH_WEAPON_AP9;
	SetWeaponModelW();
	m_iDefaultAmmo = TH_AP9_DEFAULT_GIVE;
	m_burstShots = 0;
	FallInit();
}

void CWeaponTheyHungerAP9::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_ap9.mdl";
	m_defaultModelP = "models/p_ap9.mdl";
	m_defaultModelW = "models/w_ap9.mdl";
	CBasePlayerWeapon::Precache();

	m_shell = PRECACHE_MODEL("models/shell.mdl");
	PRECACHE_MODEL("models/w_ap9clip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("weapons/ap9_bolt.wav");
	PRECACHE_SOUND("weapons/ap9_clipin.wav");
	PRECACHE_SOUND("weapons/ap9_clipout.wav");
	PRECACHE_SOUND("weapons/ap9_fire.wav");
	PrecacheEvents();
}

void CWeaponTheyHungerAP9::PrecacheEvents()
{
	m_fireEvent = PRECACHE_EVENT(1, "events/ap9.sc");
}

int CWeaponTheyHungerAP9::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = TH_AP9_AMMO_NAME;
	p->pszAmmo2 = NULL;
	p->iMaxClip = TH_AP9_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = TH_WEAPON_AP9;
	p->iWeight = TH_AP9_WEIGHT;
	p->fAccuracyDeg = 4;
	return 1;
}

BOOL CWeaponTheyHungerAP9::Deploy()
{
	return DefaultDeploy(GetModelV(), GetModelP(), TH_AP9_DRAW, "onehanded");
}

void CWeaponTheyHungerAP9::PrimaryAttack()
{
	Fire(0.07f, 0.125f, TRUE);
}

void CWeaponTheyHungerAP9::SecondaryAttack()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	m_burstShots += 2;
	Fire(0.035f, 0.6f, FALSE);
	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.04f;
}

void CWeaponTheyHungerAP9::Fire(float spread, float cycleTime, BOOL useAutoAim)
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2f;
		}
		return;
	}

	--m_iClip;
	player->pev->effects |= EF_MUZZLEFLASH;
	player->SetAnimation(PLAYER_ATTACK1);
	player->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	player->m_iWeaponFlash = NORMAL_GUN_FLASH;

	const Vector source = player->GetWeaponPosition();
	const Vector aiming = AP9AimVector(player, useAutoAim);

	lagcomp_begin(player);
#if defined(CLIENT_DLL)
	const int bulletType = bIsMultiplayer() ? BULLET_PLAYER_9MM : BULLET_PLAYER_MP5;
#else
	const int bulletType = g_pGameRules->IsMultiplayer() ? BULLET_PLAYER_9MM : BULLET_PLAYER_MP5;
#endif
	const Vector direction = player->FireBulletsPlayer(1, source, aiming,
		Vector(spread, spread, spread), 8192, bulletType, 0, 0, player->pev,
		player->random_seed, 0, BULLETPRED_EVENT);
	lagcomp_end();

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL(flags, player->edict(), m_fireEvent, 0.0,
		(float*)&g_vecZero, (float*)&g_vecZero, direction.x, direction.y, 0, 0, useAutoAim, 0);

	if (!m_iClip && player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		player->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(cycleTime);
	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + cycleTime;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
}

void CWeaponTheyHungerAP9::Reload()
{
	CBasePlayer* player = GetPlayer();
	if (!player || player->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == TH_AP9_MAX_CLIP)
		return;

	if (DefaultReload(TH_AP9_MAX_CLIP, TH_AP9_RELOAD, 1.8f))
	{
#if defined(CLIENT_DLL)
		if (!bIsMultiplayer())
#else
		if (!g_pGameRules->IsMultiplayer())
#endif
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.9f;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
	}
}

void CWeaponTheyHungerAP9::WeaponIdle()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	ResetEmptySound();
	player->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_burstShots >= 2)
	{
		if (player->m_flNextAttack > UTIL_WeaponTimeBase())
			return;
		SecondaryAttack();
		if (m_burstShots >= 6)
			m_burstShots = 0;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		SendWeaponAnim(TH_AP9_IDLE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0f;
	}
}

void CWeaponTheyHungerAP9::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount)
{
	ammoEntName = secondary ? NULL : "ammo_th_ap9";
	dropAmount = secondary ? 0 : TH_AMMO_AP9_GIVE;
}

class CAmmoTheyHungerAP9 : public CBasePlayerAmmo
{
public:
	void Precache()
	{
		m_defaultModel = "models/w_ap9clip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* other)
	{
		const int result = other->GiveAmmo(TH_AMMO_AP9_GIVE, TH_AP9_AMMO_NAME);
		if (result != -1)
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		return result != -1;
	}
};

LINK_ENTITY_TO_CLASS(ammo_th_ap9, CAmmoTheyHungerAP9)
