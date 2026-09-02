#include "CWeaponSupport.h"

#include "../../extdll.h"
#include "../../player/CBasePlayer.h"
#include "../../weapon/weapons.h"

namespace
{
enum th_chaingun_e
{
	TH_CHAINGUN_IDLE = 0,
	TH_CHAINGUN_IDLE2,
	TH_CHAINGUN_SPINUP,
	TH_CHAINGUN_SPINDOWN,
	TH_CHAINGUN_FIRE,
	TH_CHAINGUN_DRAW,
	TH_CHAINGUN_HOLSTER,
};

enum th_medkit_e
{
	TH_MEDKIT_IDLE = 0,
	TH_MEDKIT_LONGIDLE,
	TH_MEDKIT_LONGUSE,
	TH_MEDKIT_SHORTUSE,
	TH_MEDKIT_HOLSTER,
	TH_MEDKIT_DRAW,
};

constexpr int TH_CHAINGUN_MAX_CLIP = 100;
constexpr int TH_CHAINGUN_DEFAULT_GIVE = 100;
constexpr int TH_CHAINGUN_WEIGHT = 20;
constexpr int TH_MEDKIT_DEFAULT_GIVE = 8;
constexpr int TH_MEDKIT_WEIGHT = -1;

Vector ChaingunAimVector(CBasePlayer* player)
{
#if defined(CLIENT_DLL)
	float forward[3];
	gEngfuncs.pfnAngleVectors(player->GetWeaponViewAngles(), forward, NULL, NULL);
	return Vector(forward);
#else
	UTIL_MakeVectors(player->GetWeaponViewAngles());
	return gpGlobals->v_forward;
#endif
}

int WeaponEventFlags()
{
#if defined(CLIENT_WEAPONS)
	return FEV_NOTHOST;
#else
	return 0;
#endif
}
}

LINK_ENTITY_TO_CLASS(weapon_th_chaingun, CWeaponTheyHungerChaingun)

#if !defined(CLIENT_DLL)
TYPEDESCRIPTION CWeaponTheyHungerChaingun::m_SaveData[] =
{
	DEFINE_FIELD(CWeaponTheyHungerChaingun, m_spinning, FIELD_INTEGER),
	DEFINE_FIELD(CWeaponTheyHungerChaingun, m_fInSpecialReload, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CWeaponTheyHungerChaingun, CBasePlayerWeapon)
#endif

void CWeaponTheyHungerChaingun::Spawn()
{
	pev->classname = MAKE_STRING("weapon_th_chaingun");
	Precache();
	m_iId = TH_WEAPON_CHAINGUN;
	SetWeaponModelW();
	m_iDefaultAmmo = TH_CHAINGUN_DEFAULT_GIVE;
	m_spinning = 0;
	m_fInSpecialReload = 0;
	FallInit();
}

void CWeaponTheyHungerChaingun::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_tfac.mdl";
	m_defaultModelP = "models/p_tfac.mdl";
	m_defaultModelW = "models/w_tfac.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_MODEL("models/shell.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("weapons/reload3.wav");
	PRECACHE_SOUND("weapons/asscan1.wav");
	PRECACHE_SOUND("weapons/asscan2.wav");
	PRECACHE_SOUND("weapons/asscan3.wav");
	PRECACHE_SOUND("weapons/asscan4.wav");
	PrecacheEvents();
}

void CWeaponTheyHungerChaingun::PrecacheEvents()
{
	m_spinEvent = PRECACHE_EVENT(1, "events/chaingun1.sc");
	m_fireEvent = PRECACHE_EVENT(1, "events/chaingun2.sc");
}

int CWeaponTheyHungerChaingun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->pszAmmo2 = NULL;
	p->iMaxClip = TH_CHAINGUN_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = TH_WEAPON_CHAINGUN;
	p->iWeight = TH_CHAINGUN_WEIGHT;
	p->fAccuracyDeg = 8;
	return 1;
}

BOOL CWeaponTheyHungerChaingun::Deploy()
{
	return DefaultDeploy(GetModelV(), GetModelP(), TH_CHAINGUN_DRAW, "egon");
}

void CWeaponTheyHungerChaingun::Holster(int)
{
	m_fInSpecialReload = 0;
	if (m_spinning)
		SpinDown();
	else
		SendWeaponAnim(TH_CHAINGUN_HOLSTER);

	CBasePlayer* player = GetPlayer();
	if (player)
		player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CWeaponTheyHungerChaingun::PrimaryAttack()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	if (player->pev->waterlevel == 3 || m_iClip <= 0)
	{
		if (m_spinning)
			SpinDown();
		else
		{
			PlayEmptySound();
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2f;
		}
		return;
	}

	if (!m_spinning)
		SpinUp();
	else if (m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
	{
#if defined(CLIENT_DLL)
		if (!bIsMultiplayer())
#else
		if (!g_pGameRules->IsMultiplayer())
#endif
			Fire(0.1f, 0.1f);
		else
			Fire(0.15f, 0.15f);
	}
}

void CWeaponTheyHungerChaingun::SecondaryAttack()
{
	if (m_spinning)
		SpinDown();
}

void CWeaponTheyHungerChaingun::SpinUp()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	player->m_iWeaponVolume = LOUD_GUN_VOLUME;
	PLAYBACK_EVENT_FULL(WeaponEventFlags(), player->edict(), m_spinEvent, 0.0,
		(float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, 0, 0, TRUE, 0);
	m_spinning = 1;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
}

void CWeaponTheyHungerChaingun::SpinDown()
{
	CBasePlayer* player = GetPlayer();
	if (!player || !m_spinning)
		return;

	PLAYBACK_EVENT_FULL(WeaponEventFlags(), player->edict(), m_spinEvent, 0.0,
		(float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, 0, 0, FALSE, 0);
	player->m_flNextAttack = UTIL_WeaponTimeBase() + 1.2f;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = player->m_flNextAttack;
	m_spinning = 0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;
}

void CWeaponTheyHungerChaingun::Fire(float spread, float cycleTime)
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	int clipNotEmpty = 0;
	player->m_iWeaponVolume = LOUD_GUN_VOLUME;
	player->m_iWeaponFlash = NORMAL_GUN_FLASH;
	--m_iClip;
	if (m_iClip)
	{
		--m_iClip;
		clipNotEmpty = 1;
	}
	player->pev->effects |= EF_MUZZLEFLASH;
	player->SetAnimation(PLAYER_ATTACK1);

	lagcomp_begin(player);
	const Vector direction = player->FireBulletsPlayer(clipNotEmpty ? 4 : 2,
		player->GetWeaponPosition(), ChaingunAimVector(player),
		Vector(spread, spread, spread), 8192, BULLET_PLAYER_MP5, 0, 0,
		player->pev, player->random_seed, 0, BULLETPRED_EVENT);
	lagcomp_end();

	PLAYBACK_EVENT_FULL(WeaponEventFlags(), player->edict(), m_fireEvent, 0.0,
		(float*)&g_vecZero, (float*)&g_vecZero, direction.x, direction.y, 0, 0,
		clipNotEmpty, 0);
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + cycleTime;
	if (!m_iClip && player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		player->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	m_flTimeWeaponIdle = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1f;
}

void CWeaponTheyHungerChaingun::Reload()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	if (m_spinning)
		SpinDown();
	else if (player->m_rgAmmo[m_iPrimaryAmmoType] > 0 && m_iClip != TH_CHAINGUN_MAX_CLIP &&
		DefaultReload(TH_CHAINGUN_MAX_CLIP, TH_CHAINGUN_HOLSTER, 0.5f))
	{
		m_fInSpecialReload = 2;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0f;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 5, 10);
	}
}

void CWeaponTheyHungerChaingun::WeaponIdle()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	ResetEmptySound();
	if (player->m_flNextAttack < UTIL_WeaponTimeBase())
	{
		if (m_fInSpecialReload == 3)
		{
			SendWeaponAnim(TH_CHAINGUN_DRAW, 0);
			m_fInSpecialReload = 1;
		}
		else if (m_fInSpecialReload == 2)
		{
			EMIT_SOUND_DYN(ENT(player->pev), CHAN_ITEM, "weapons/reload3.wav", 1,
				ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0x1f));
			player->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0f;
			m_fInSpecialReload = 3;
		}
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_spinning)
		SpinDown();
	else
	{
		const int animation = UTIL_SharedRandomFloat(player->random_seed, 0, 1) <= 0.5f
			? TH_CHAINGUN_IDLE : TH_CHAINGUN_IDLE2;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
		SendWeaponAnim(animation, 1);
	}
}

LINK_ENTITY_TO_CLASS(weapon_th_medkit, CWeaponTheyHungerMedkit)

#if !defined(CLIENT_DLL)
TYPEDESCRIPTION CWeaponTheyHungerMedkit::m_SaveData[] =
{
	DEFINE_FIELD(CWeaponTheyHungerMedkit, m_using, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CWeaponTheyHungerMedkit, CBasePlayerWeapon)
#endif

void CWeaponTheyHungerMedkit::Spawn()
{
	Precache();
	m_iId = TH_WEAPON_MEDKIT;
	SetWeaponModelW();
	m_iDefaultAmmo = TH_MEDKIT_DEFAULT_GIVE;
	m_using = 0;
	FallInit();
}

void CWeaponTheyHungerMedkit::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_tfc_medkit.mdl";
	m_defaultModelP = "models/p_tfc_medkit.mdl";
	m_defaultModelW = "models/w_tfc_medkit.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
}

int CWeaponTheyHungerMedkit::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "health";
	p->pszAmmo2 = NULL;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 4;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = TH_WEAPON_MEDKIT;
	p->iWeight = TH_MEDKIT_WEIGHT;
	return 1;
}

BOOL CWeaponTheyHungerMedkit::Deploy()
{
	return DefaultDeploy(GetModelV(), GetModelP(), TH_MEDKIT_DRAW, "trip");
}

void CWeaponTheyHungerMedkit::Holster(int)
{
	m_using = 0;
	SendWeaponAnim(TH_MEDKIT_HOLSTER);

	CBasePlayer* player = GetPlayer();
	if (player)
		player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CWeaponTheyHungerMedkit::PrimaryAttack()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	if (player->pev->health >= player->pev->max_health || player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;
		}
		return;
	}

	if (m_using)
	{
		if (m_flNextSecondaryAttack < UTIL_WeaponTimeBase())
			Heal();
		return;
	}

	StartUse();
}

void CWeaponTheyHungerMedkit::SecondaryAttack()
{
	if (m_using)
		Heal();
}

void CWeaponTheyHungerMedkit::StartUse()
{
	SendWeaponAnim(TH_MEDKIT_LONGUSE, 0);
	m_using = 1;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2f;
}

void CWeaponTheyHungerMedkit::Heal()
{
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;

	--player->m_rgAmmo[m_iPrimaryAmmoType];
	player->TakeHealth(25, DMG_GENERIC);
	player->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	EMIT_SOUND(ENT(player->pev), CHAN_WEAPON, "items/medshot4.wav", 1.0f, ATTN_NORM);
	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		player->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_using = 0;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2f;
}

void CWeaponTheyHungerMedkit::Reload()
{
	SecondaryAttack();
}

void CWeaponTheyHungerMedkit::WeaponIdle()
{
	ResetEmptySound();
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_using)
	{
		Heal();
		return;
	}

	SendWeaponAnim(TH_MEDKIT_IDLE, 0);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0f;
}

BOOL CWeaponTheyHungerMedkit::PlayEmptySound()
{
	CBasePlayer* player = GetPlayer();
	if (player && m_iPlayEmptySound)
	{
		EMIT_SOUND(ENT(player->pev), CHAN_WEAPON, "items/medshotno1.wav", 0.8f, ATTN_NORM);
		m_iPlayEmptySound = 0;
	}
	return FALSE;
}
