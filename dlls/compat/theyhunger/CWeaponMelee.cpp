#include "CWeaponMelee.h"

#include "../../extdll.h"
#include "../../env/te_effects.h"
#include "../../game/user_messages.h"
#include "../../player/CBasePlayer.h"
#include "../../weapon/weapons.h"

#define TH_MELEE_MIN_PHYSICAL_SWING_SPEED 70.0f

enum shovel_e {
	SHOVEL_IDLE, SHOVEL_DRAW, SHOVEL_HOLSTER, SHOVEL_ATTACK1HIT,
	SHOVEL_ATTACK1MISS, SHOVEL_ATTACK2MISS, SHOVEL_ATTACK2HIT,
	SHOVEL_ATTACK3MISS, SHOVEL_ATTACK3HIT, SHOVEL_IDLE2, SHOVEL_IDLE3,
};
enum spanner_e {
	SPANNER_IDLE, SPANNER_ATTACK1, SPANNER_ATTACK2, SPANNER_USE,
	SPANNER_DRAW, SPANNER_HOLSTER,
};

namespace {
constexpr const char* kHitSounds[] = {
	"weapons/cbar_hitbod1.wav", "weapons/cbar_hitbod2.wav", "weapons/cbar_hitbod3.wav",
};
constexpr const char* kSkeletonHitSounds[] = {
	"kelly/cbar_hitkelly1.wav", "kelly/cbar_hitkelly2.wav", "kelly/cbar_hitkelly3.wav",
};

void PrecacheMeleeSounds(CBaseEntity* weapon) {
	PRECACHE_SOUND_ENT(weapon, "weapons/cbar_hit1.wav");
	PRECACHE_SOUND_ENT(weapon, "weapons/cbar_hit2.wav");
	PRECACHE_SOUND_ENT(weapon, "weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND_ENT(weapon, "weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND_ENT(weapon, "weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND_ENT(weapon, "weapons/cbar_miss1.wav");
	PRECACHE_SOUND_ENT(weapon, "kelly/cbar_hitkelly1.wav");
	PRECACHE_SOUND_ENT(weapon, "kelly/cbar_hitkelly2.wav");
	PRECACHE_SOUND_ENT(weapon, "kelly/cbar_hitkelly3.wav");
}

int AddMeleeToPlayer(CBasePlayerWeapon* weapon, CBasePlayer* player) {
	if (!weapon->CBasePlayerWeapon::AddToPlayer(player))
		return FALSE;
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, g_umsg.WeapPickup, NULL, player->pev);
	WRITE_BYTE(weapon->m_iId);
	MESSAGE_END();
#endif
	return TRUE;
}
} // namespace

void CWeaponTheyHungerMelee::InitializePhysicalSwingState() {
#ifndef CLIENT_DLL
	ClearPhysicalSwingState();
	m_physicalLastWoosh = 0.0f;
#endif
}

void CWeaponTheyHungerMelee::Holster(int) {
	CBasePlayer* player = GetPlayer();
	if (!player)
		return;
#ifndef CLIENT_DLL
	ClearPhysicalSwingState();
#endif
	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
	SendWeaponAnim(HolsterAnimation());
}

void CWeaponTheyHungerMelee::ItemPostFrame() {
	CBasePlayerWeapon::ItemPostFrame();
#ifndef CLIENT_DLL
	CBasePlayer* player = GetPlayer();
	if (!player || !player->HasFreshVRPose()) {
		ClearPhysicalSwingState();
		return;
	}

	const float speed = player->GetWeaponVelocity().Length();
	if (speed < TH_MELEE_MIN_PHYSICAL_SWING_SPEED) {
		ClearPhysicalSwingState();
		return;
	}
	if (!m_physicalSwinging && gpGlobals->time > m_physicalLastWoosh + 0.5f) {
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/cbar_miss1.wav", 1.0f,
			ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		m_physicalLastWoosh = gpGlobals->time;
	}
	m_physicalSwinging = true;
	CheckPhysicalSwing(speed);
#endif
}

void CWeaponTheyHungerMelee::Smack() {
#ifndef CLIENT_DLL
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
#endif
}

int CWeaponTheyHungerMelee::Swing(int firstSwing) {
	CBasePlayer* player = GetPlayer();
	if (!player)
		return FALSE;

	int didHit = FALSE;
	TraceResult tr;
	UTIL_MakeVectors(player->GetWeaponViewAngles());
	Vector src = player->GetWeaponPosition();
	Vector end = src + gpGlobals->v_forward * 32;

#ifndef CLIENT_DLL
	SolidifyNearbyCorpses(false);
	lagcomp_begin(player);
#endif
	UTIL_TraceLine(src, end, dont_ignore_monsters, ENT(player->pev), &tr);
#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0f) {
		UTIL_TraceHull(src, end, dont_ignore_monsters, head_hull, ENT(player->pev), &tr);
		if (tr.flFraction < 1.0f) {
			CBaseEntity* hit = CBaseEntity::Instance(tr.pHit);
			if (!hit || hit->IsBSPModel())
				FindHullIntersection(src, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, player->edict());
			end = tr.vecEndPos;
		}
	}
	player->WaterSplashTrace(src, 32, head_hull, 0.4f);
	lagcomp_end();
	SolidifyNearbyCorpses(true);
#endif

	if (tr.flFraction >= 1.0f) {
		if (firstSwing) {
			SendWeaponAnim(MissAnimation());
			m_flNextPrimaryAttack = GetNextAttackDelay(MissDelay());
			player->SetAnimation(PLAYER_ATTACK1);
			EMIT_SOUND_DYN(ENT(player->pev), CHAN_WEAPON, "weapons/cbar_miss1.wav", 1,
				ATTN_NORM, 0, 94 + RANDOM_LONG(0, 0xF));
		}
	}
	else {
		SendWeaponAnim(HitAnimation());
		player->SetAnimation(PLAYER_ATTACK1);
#ifndef CLIENT_DLL
		didHit = TRUE;
		CBaseEntity* entity = CBaseEntity::Instance(tr.pHit);
		float volume = 1.0f;
		bool hitWorld = true;
		if (entity) {
			ClearMultiDamage();
			const bool firstDesktopSwing = m_flNextPrimaryAttack + 1.0f < UTIL_WeaponTimeBase();
			const float damage = player->GetDamage(gSkillData.sk_plr_crowbar) *
				DamageMultiplier(g_pGameRules->IsMultiplayer(), firstDesktopSwing);
			entity->TraceAttack(player->pev, damage, gpGlobals->v_forward, &tr, DMG_CLUB);
			ApplyMultiDamage(player->pev, player->pev);

			const bool bodyHit = entity->Classify() != CLASS_NONE && !entity->IsMachine() &&
				!entity->IsBSPModel();
			if (bodyHit) {
				// SevenKewp has no CLASS_SKELETON value (its numeric slot is Race X),
				// so preserve Team Beef's CIslave skeleton sound dispatch by classname.
				const char* const* sounds = UsesSkeletonSounds() &&
					FClassnameIs(entity->pev, "monster_islave") ? kSkeletonHitSounds : kHitSounds;
				EMIT_SOUND(ENT(player->pev), CHAN_ITEM, sounds[RANDOM_LONG(0, 2)], 1, ATTN_NORM);
				player->m_iWeaponVolume = BodyHitVolume();
				if (!entity->IsAlive())
					return TRUE; // Team Beef's rapid-melee behavior.
				volume = 0.1f;
				hitWorld = false;
			}
		}
		if (hitWorld) {
			float textureVolume = TEXTURETYPE_PlaySound(&tr, src, src + (end - src) * 2,
				BULLET_PLAYER_CROWBAR);
			if (g_pGameRules->IsMultiplayer())
				textureVolume = 1.0f;
			EMIT_SOUND_DYN(ENT(player->pev), CHAN_ITEM,
				RANDOM_LONG(0, 1) ? "weapons/cbar_hit2.wav" : "weapons/cbar_hit1.wav",
				textureVolume, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			m_trHit = tr;
		}
		player->m_iWeaponVolume = volume * WallHitVolume();
#endif
		m_flNextPrimaryAttack = GetNextAttackDelay(HitDelay());
		SetThink(&CWeaponTheyHungerMelee::Smack);
		pev->nextthink = gpGlobals->time + 0.2f;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
	return didHit;
}

#ifndef CLIENT_DLL
void CWeaponTheyHungerMelee::CheckPhysicalSwing(float speed) {
	CBasePlayer* player = GetPlayer();
	TraceResult tr;
	UTIL_MakeVectors(player->GetWeaponViewAngles());
	const Vector src = player->GetWeaponPosition();
	const Vector end = src + gpGlobals->v_up * PhysicalLength();
	UTIL_TraceLine(src, end, dont_ignore_monsters, ENT(player->pev), &tr);
	if (tr.fStartSolid || tr.fAllSolid || tr.flFraction >= 1.0f)
		return;
	CBaseEntity* entity = CBaseEntity::Instance(tr.pHit);
	if (!entity || HasHitEntityThisSwing(entity))
		return;
	RememberHitEntityThisSwing(entity);
	++m_physicalHitCount;
	ClearMultiDamage();
	entity->TraceAttack(player->pev, player->GetDamage(gSkillData.sk_plr_crowbar) *
		(speed / TH_MELEE_MIN_PHYSICAL_SWING_SPEED) / m_physicalHitCount,
		gpGlobals->v_up, &tr, DMG_CLUB);
	ApplyMultiDamage(player->pev, player->pev);
	CLIENT_COMMAND(player->edict(), "vr_haptic 90 -1 1\n");
}

bool CWeaponTheyHungerMelee::HasHitEntityThisSwing(CBaseEntity* entity) {
	for (int i = 0; i < ARRAYSIZE(m_physicalHitEntities); ++i)
		if ((CBaseEntity*)m_physicalHitEntities[i] == entity) return true;
	return false;
}

void CWeaponTheyHungerMelee::RememberHitEntityThisSwing(CBaseEntity* entity) {
	for (int i = 0; i < ARRAYSIZE(m_physicalHitEntities); ++i)
		if (!m_physicalHitEntities[i]) { m_physicalHitEntities[i] = entity; return; }
}

void CWeaponTheyHungerMelee::ClearPhysicalSwingState() {
	m_physicalSwinging = false;
	m_physicalHitCount = 0.0f;
	for (int i = 0; i < ARRAYSIZE(m_physicalHitEntities); ++i)
		m_physicalHitEntities[i] = NULL;
}
#endif

LINK_ENTITY_TO_CLASS(weapon_th_shovel, CWeaponTheyHungerShovel)

void CWeaponTheyHungerShovel::Spawn() {
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

void CWeaponTheyHungerShovel::Precache() {
	CBasePlayerWeapon::Precache();
	PrecacheMeleeSounds(this);
}

int CWeaponTheyHungerShovel::AddToPlayer(CBasePlayer* player) { return AddMeleeToPlayer(this, player); }

int CWeaponTheyHungerShovel::GetItemInfo(ItemInfo* info) {
	info->pszName = STRING(pev->classname);
	info->pszAmmo1 = info->pszAmmo2 = NULL;
	info->iMaxClip = WEAPON_NOCLIP;
	info->iSlot = 0; info->iPosition = 1; info->iFlags = 0;
	info->iId = m_iId = TH_WEAPON_SHOVEL; info->iWeight = 0;
	return 1;
}

BOOL CWeaponTheyHungerShovel::Deploy() { return DefaultDeploy(GetModelV(), GetModelP(), SHOVEL_DRAW, "crowbar"); }
BOOL CWeaponTheyHungerShovel::UseDecrement() {
#if defined(CLIENT_WEAPONS)
	return TRUE;
#else
	return FALSE;
#endif
}
void CWeaponTheyHungerShovel::PrimaryAttack() {
	if (!Swing(TRUE)) { SetThink(&CWeaponTheyHungerShovel::SwingAgain); pev->nextthink = gpGlobals->time + 0.1f; }
}
void CWeaponTheyHungerShovel::SwingAgain() { Swing(FALSE); }
int CWeaponTheyHungerShovel::MissAnimation() {
	if (RANDOM_FLOAT(0.0f, 1.0f) <= 0.25f) ++m_iSwing;
	return (m_iSwing++ % 2) == 0 ? SHOVEL_ATTACK1MISS : SHOVEL_ATTACK2MISS;
}
int CWeaponTheyHungerShovel::HitAnimation() {
	switch (RANDOM_LONG(0, 2)) { case 0: return SHOVEL_ATTACK1HIT; case 1: return SHOVEL_ATTACK2HIT; default: return SHOVEL_ATTACK3HIT; }
}
int CWeaponTheyHungerShovel::HolsterAnimation() const { return SHOVEL_HOLSTER; }
float CWeaponTheyHungerShovel::MissDelay() const { return 0.5f; }
float CWeaponTheyHungerShovel::HitDelay() const { return 0.25f; }
float CWeaponTheyHungerShovel::DamageMultiplier(bool multiplayer, bool first) const { return multiplayer ? 1.6f : first ? 1.2f : 0.6f; }
float CWeaponTheyHungerShovel::PhysicalLength() const { return 24.0f; }
int CWeaponTheyHungerShovel::BodyHitVolume() const { return 128; }
int CWeaponTheyHungerShovel::WallHitVolume() const { return 512; }
bool CWeaponTheyHungerShovel::UsesSkeletonSounds() const { return true; }
void CWeaponTheyHungerShovel::WeaponIdle() {
	if (m_flTimeWeaponIdle >= UTIL_WeaponTimeBase()) return;
	const float random = UTIL_SharedRandomFloat(GetPlayer()->random_seed, 0, 1);
	if (random > 0.9f) { SendWeaponAnim(SHOVEL_IDLE2); m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 160.0f / 30.0f; }
	else if (random > 0.5f) { SendWeaponAnim(SHOVEL_IDLE); m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0f / 30.0f; }
	else { SendWeaponAnim(SHOVEL_IDLE3); m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 160.0f / 30.0f; }
}

LINK_ENTITY_TO_CLASS(weapon_th_spanner, CWeaponTheyHungerSpanner)

void CWeaponTheyHungerSpanner::Spawn() {
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

void CWeaponTheyHungerSpanner::Precache() {
	CBasePlayerWeapon::Precache();
	PrecacheMeleeSounds(this);
}

int CWeaponTheyHungerSpanner::AddToPlayer(CBasePlayer* player) { return AddMeleeToPlayer(this, player); }

int CWeaponTheyHungerSpanner::GetItemInfo(ItemInfo* info) {
	info->pszName = STRING(pev->classname);
	info->pszAmmo1 = info->pszAmmo2 = NULL;
	info->iMaxClip = WEAPON_NOCLIP;
	info->iSlot = 0; info->iPosition = 2; info->iFlags = 0;
	info->iId = m_iId = TH_WEAPON_SPANNER; info->iWeight = 0;
	return 1;
}

BOOL CWeaponTheyHungerSpanner::Deploy() { return DefaultDeploy(GetModelV(), GetModelP(), SPANNER_DRAW, "crowbar"); }
BOOL CWeaponTheyHungerSpanner::UseDecrement() {
#if defined(CLIENT_WEAPONS)
	return TRUE;
#else
	return FALSE;
#endif
}
void CWeaponTheyHungerSpanner::PrimaryAttack() {
	if (!Swing(TRUE)) { SetThink(&CWeaponTheyHungerSpanner::SwingAgain); pev->nextthink = gpGlobals->time + 0.1f; }
}
void CWeaponTheyHungerSpanner::SwingAgain() { Swing(FALSE); }
int CWeaponTheyHungerSpanner::MissAnimation() {
	if (RANDOM_FLOAT(0.0f, 1.0f) <= 0.25f) ++m_iSwing;
	return (m_iSwing++ % 2) == 0 ? SPANNER_ATTACK1 : SPANNER_ATTACK2;
}
int CWeaponTheyHungerSpanner::HitAnimation() { return (m_iSwing++ % 2) == 0 ? SPANNER_ATTACK1 : SPANNER_ATTACK2; }
int CWeaponTheyHungerSpanner::HolsterAnimation() const { return SPANNER_HOLSTER; }
float CWeaponTheyHungerSpanner::MissDelay() const { return 0.4f; }
float CWeaponTheyHungerSpanner::HitDelay() const { return 0.2f; }
float CWeaponTheyHungerSpanner::DamageMultiplier(bool multiplayer, bool first) const { return multiplayer ? 1.2f : first ? 0.8f : 0.4f; }
float CWeaponTheyHungerSpanner::PhysicalLength() const { return 15.0f; }
int CWeaponTheyHungerSpanner::BodyHitVolume() const { return 128; }
int CWeaponTheyHungerSpanner::WallHitVolume() const { return 512; }
bool CWeaponTheyHungerSpanner::UsesSkeletonSounds() const { return true; }
void CWeaponTheyHungerSpanner::WeaponIdle() {
	if (m_flTimeWeaponIdle >= UTIL_WeaponTimeBase()) return;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(GetPlayer()->random_seed, 5, 10);
	SendWeaponAnim(SPANNER_IDLE);
}
