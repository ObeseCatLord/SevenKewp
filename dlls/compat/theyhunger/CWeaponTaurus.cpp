#include "../../extdll.h"
#include "../../weapon/weapons.h"
#include "CWeaponTaurus.h"
#include "../../../game_shared/compat/theyhunger/taurus.h"
#include "../../player/CBasePlayer.h"
#include "../../weapon/CBasePlayerAmmo.h"
LINK_ENTITY_TO_CLASS(weapon_th_taurus, CWeaponTheyHungerTaurus)
namespace {
Vector TaurusAim(CBasePlayer *p) {
#if defined(CLIENT_DLL)
  float f[3];
  gEngfuncs.pfnAngleVectors(p->GetWeaponViewAngles(), f, NULL, NULL);
  return Vector(f);
#else
  UTIL_MakeVectors(p->GetWeaponViewAngles());
  return gpGlobals->v_forward;
#endif
}
} // namespace
void CWeaponTheyHungerTaurus::Spawn() {
  Precache();
  m_iId = TH_WEAPON_TAURUS;
  SetWeaponModelW();
  m_iDefaultAmmo = TH_TAURUS_DEFAULT_GIVE;
  FallInit();
}
void CWeaponTheyHungerTaurus::Precache() {
  m_hasHandModels = true;
  m_defaultModelV = "models/v_taurus.mdl";
  m_defaultModelP = "models/p_taurus.mdl";
  m_defaultModelW = "models/w_taurus.mdl";
  CBasePlayerWeapon::Precache();
  PRECACHE_MODEL("models/shell.mdl");
  PRECACHE_MODEL("models/w_taurusclip.mdl");
  PRECACHE_SOUND("items/9mmclip1.wav");
  PRECACHE_SOUND("weapons/tau_back.wav");
  PRECACHE_SOUND("weapons/tau_clipin.wav");
  PRECACHE_SOUND("weapons/tau_clipout.wav");
  PRECACHE_SOUND("weapons/tau_fire.wav");
  PRECACHE_SOUND("weapons/tau_release.wav");
  PrecacheEvents();
}
void CWeaponTheyHungerTaurus::PrecacheEvents() {
  m_fireEvent = PRECACHE_EVENT(1, "events/taurus.sc");
}
int CWeaponTheyHungerTaurus::GetItemInfo(ItemInfo *p) {
  p->pszName = STRING(pev->classname);
  p->pszAmmo1 = TH_TAURUS_AMMO_NAME;
  p->pszAmmo2 = NULL;
  p->iMaxClip = TH_TAURUS_MAX_CLIP;
  p->iSlot = 1;
  p->iPosition = 3;
  p->iFlags = 0;
  p->iId = m_iId = TH_WEAPON_TAURUS;
  p->iWeight = TH_TAURUS_WEIGHT;
  p->fAccuracyDeg = 4;
  return 1;
}
BOOL CWeaponTheyHungerTaurus::Deploy() {
  return DefaultDeploy(GetModelV(), GetModelP(), TH_TAURUS_DRAW2, "onehanded");
}
void CWeaponTheyHungerTaurus::PrimaryAttack() { Fire(.04f, .25f); }
void CWeaponTheyHungerTaurus::Fire(float spread, float cycle) {
  CBasePlayer *p = GetPlayer();
  if (!p)
    return;
  if (m_iClip <= 0) {
    if (m_fFireOnEmpty) {
      PlayEmptySound();
      m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + .2f;
    }
    return;
  }
  --m_iClip;
  p->pev->effects |= EF_MUZZLEFLASH;
  p->SetAnimation(PLAYER_ATTACK1);
  p->m_iWeaponVolume = NORMAL_GUN_VOLUME;
  p->m_iWeaponFlash = NORMAL_GUN_FLASH;
  Vector aim = TaurusAim(p);
  lagcomp_begin(p);
  Vector dir = p->FireBulletsPlayer(
      1, p->GetWeaponPosition(), aim, Vector(spread, spread, spread), 8192,
      BULLET_PLAYER_9MM, 0, 0, p->pev, p->random_seed, 0, BULLETPRED_EVENT);
  lagcomp_end();
  int flags;
#if defined(CLIENT_WEAPONS)
  flags = FEV_NOTHOST;
#else
  flags = 0;
#endif
  PLAYBACK_EVENT_FULL(flags, p->edict(), m_fireEvent, 0, (float *)&g_vecZero,
                      (float *)&g_vecZero, dir.x, dir.y, 0, 0, m_iClip == 0, 0);
  if (!m_iClip && p->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
    p->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
  m_flNextPrimaryAttack = GetNextAttackDelay(cycle);
  if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + cycle;
  m_flTimeWeaponIdle =
      UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(p->random_seed, 5, 10);
}
void CWeaponTheyHungerTaurus::Reload() {
  CBasePlayer *p = GetPlayer();
  if (p && p->m_rgAmmo[m_iPrimaryAmmoType] > 0 &&
      m_iClip != TH_TAURUS_MAX_CLIP &&
      DefaultReload(TH_TAURUS_MAX_CLIP, TH_TAURUS_RELOAD2, 2.0f))
    m_flTimeWeaponIdle =
        UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(p->random_seed, 5, 10);
}
void CWeaponTheyHungerTaurus::WeaponIdle() {
  CBasePlayer *p = GetPlayer();
  if (!p)
    return;
  ResetEmptySound();
  p->GetAutoaimVector(AUTOAIM_10DEGREES);
  if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() || !m_iClip)
    return;
  float r = UTIL_SharedRandomFloat(p->random_seed, 0, 1);
  SendWeaponAnim(r <= .3f   ? TH_TAURUS_IDLE3
                 : r <= .6f ? TH_TAURUS_IDLE1
                            : TH_TAURUS_IDLE2,
                 1);
  m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 15;
}
void CWeaponTheyHungerTaurus::GetAmmoDropInfo(bool secondary, const char *&name,
                                              int &amount) {
  name = secondary ? NULL : "ammo_th_taurus";
  amount = secondary ? 0 : TH_AMMO_TAURUS_GIVE;
}
class CAmmoTheyHungerTaurus : public CBasePlayerAmmo {
public:
  void Precache() {
    m_defaultModel = "models/w_taurusclip.mdl";
    CBasePlayerAmmo::Precache();
    PRECACHE_SOUND("items/9mmclip1.wav");
  }
  BOOL AddAmmo(CBaseEntity *o) {
    int r = o->GiveAmmo(TH_AMMO_TAURUS_GIVE, TH_TAURUS_AMMO_NAME);
    if (r != -1)
      EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
    return r != -1;
  }
};
LINK_ENTITY_TO_CLASS(ammo_th_taurus, CAmmoTheyHungerTaurus)
