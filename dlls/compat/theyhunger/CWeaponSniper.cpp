#include "../../extdll.h"
#include "../../weapon/weapons.h"
#include "../../../engine/shake.h"
#include "../../../game_shared/compat/theyhunger/sniper.h"
#include "CWeaponSniper.h"
#include "../../player/CBasePlayer.h"
#include "../../weapon/CBasePlayerAmmo.h"
LINK_ENTITY_TO_CLASS(weapon_th_sniper, CWeaponTheyHungerSniper)
LINK_ENTITY_TO_CLASS(weapon_einar1, CWeaponTheyHungerTFCSniper)
namespace {
Vector Aim(CBasePlayer *p) {
#if defined(CLIENT_DLL)
  float f[3];
  gEngfuncs.pfnAngleVectors(p->GetWeaponViewAngles(), f, NULL, NULL);
  return Vector(f);
#else
  UTIL_MakeVectors(p->GetWeaponViewAngles());
  return gpGlobals->v_forward;
#endif
}
void Zoom(CBasePlayerWeapon *w, int &z) {
  CBasePlayer *p = w->GetPlayer();
  if (!p)
    return;
  int flags = 0;
  if (z) {
    p->pev->fov = p->m_iFOV = 0;
    z = 0;
    flags |= FFADE_IN;
  } else {
    p->pev->fov = p->m_iFOV = 20;
    z = 1;
    flags |= FFADE_OUT | FFADE_STAYOUT;
  }
#if !defined(CLIENT_DLL)
  UTIL_ScreenFade(p, Vector(0, 255, 0), 0, 0, 70, flags);
#endif
  w->pev->nextthink = UTIL_WeaponTimeBase() + .11f;
  w->m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + .75f;
}
void HolsterSniper(CBasePlayerWeapon *w, int &z) {
  CBasePlayer *p = w->GetPlayer();
  if (!p)
    return;
  w->m_fInReload = FALSE;
  if (z)
    Zoom(w, z);
  p->m_flNextAttack = UTIL_WeaponTimeBase() + .5f;
}
void Fire(CBasePlayerWeapon *w, unsigned short event, int z, int emptyAnimation) {
  CBasePlayer *p = w->GetPlayer();
  if (!p)
    return;
  if (p->pev->waterlevel == 3 || w->m_iClip <= 0) {
    if (w->m_fFireOnEmpty) {
      w->PlayEmptySound();
      w->m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + .25f;
    }
    if (w->m_flTimeWeaponIdle > UTIL_WeaponTimeBase()) {
      if (emptyAnimation >= 0)
        w->SendWeaponAnim(emptyAnimation);
      w->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() +
                              UTIL_SharedRandomFloat(p->random_seed, 15, 20);
    }
    return;
  }
  p->m_iWeaponVolume = LOUD_GUN_VOLUME;
  p->m_iWeaponFlash = BRIGHT_GUN_FLASH;
  --w->m_iClip;
  p->pev->effects |= EF_MUZZLEFLASH;
  p->SetAnimation(PLAYER_ATTACK1);
#if defined(CLIENT_DLL)
  bool mp = bIsMultiplayer();
#else
  bool mp = g_pGameRules->IsMultiplayer();
#endif
  float s = mp ? (z ? .002f : .07846f) : (z ? .004f : .08716f);
  lagcomp_begin(p);
  Vector d = p->FireBulletsPlayer(
      1, p->GetWeaponPosition(), Aim(p), Vector(s, s, s), 8192,
      BULLET_PLAYER_357, 0, 0, p->pev, p->random_seed, 0, BULLETPRED_EVENT);
  lagcomp_end();
  int flags;
#if defined(CLIENT_WEAPONS)
  flags = FEV_NOTHOST;
#else
  flags = 0;
#endif
  PLAYBACK_EVENT_FULL(flags, p->edict(), event, 0, (float *)&g_vecZero,
                      (float *)&g_vecZero, d.x, d.y, 0, 0, z, 0);
  if (!w->m_iClip && p->m_rgAmmo[w->m_iPrimaryAmmoType] <= 0)
    p->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
  if (z)
    w->m_flNextPrimaryAttack = w->m_flNextSecondaryAttack =
        w->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + .75f;
  else {
    w->m_flNextPrimaryAttack = w->m_flNextSecondaryAttack =
        w->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + .11f;
    p->pev->punchangle.x -= RANDOM_FLOAT(0, 2) + 4;
    p->pev->punchangle.y -= 1 - RANDOM_FLOAT(0, 2);
  }
}
void Info(ItemInfo *p, const char *n, int id, int pos) {
  p->pszName = n;
  p->pszAmmo1 = TH_SNIPER_AMMO_NAME;
  p->pszAmmo2 = NULL;
  p->iMaxClip = TH_SNIPER_MAX_CLIP;
  p->iFlags = 0;
  p->iSlot = 2;
  p->iPosition = pos;
  p->iId = id;
  p->iWeight = TH_SNIPER_WEIGHT;
  p->fAccuracyDeg = 1;
}
} // namespace
void CWeaponTheyHungerSniper::Spawn() {
  Precache();
  m_iId = TH_WEAPON_SNIPER;
  SetWeaponModelW();
  m_iDefaultAmmo = TH_SNIPER_DEFAULT_GIVE;
  m_inZoom = 0;
  FallInit();
}
void CWeaponTheyHungerSniper::Precache() {
  m_hasHandModels = true;
  m_defaultModelV = "models/v_hkg36.mdl";
  m_defaultModelP = "models/p_hkg36.mdl";
  m_defaultModelW = "models/w_hkg36.mdl";
  CBasePlayerWeapon::Precache();
  PRECACHE_MODEL("models/w_antidote.mdl");
  PRECACHE_SOUND("items/9mmclip1.wav");
  PRECACHE_SOUND("weapons/sniper.wav");
  PRECACHE_SOUND("weapons/ap9_bolt.wav");
  PRECACHE_SOUND("weapons/ap9_clipin.wav");
  PRECACHE_SOUND("weapons/ap9_clipout.wav");
  PrecacheEvents();
}
void CWeaponTheyHungerSniper::PrecacheEvents() {
  m_fireEvent = PRECACHE_EVENT(1, "events/sniper.sc");
}
int CWeaponTheyHungerSniper::GetItemInfo(ItemInfo *p) {
  Info(p, STRING(pev->classname), TH_WEAPON_SNIPER, 3);
  m_iId = p->iId;
  return 1;
}
BOOL CWeaponTheyHungerSniper::Deploy() {
  return DefaultDeploy(GetModelV(), GetModelP(), TH_SNIPER_DRAW, "bow");
}
void CWeaponTheyHungerSniper::Holster(int) { HolsterSniper(this, m_inZoom); }
void CWeaponTheyHungerSniper::PrimaryAttack() {
  Fire(this, m_fireEvent, m_inZoom, -1);
}
void CWeaponTheyHungerSniper::SecondaryAttack() { Zoom(this, m_inZoom); }
void CWeaponTheyHungerSniper::ToggleScope(BOOL engage) {
  if ((engage != FALSE) != (m_inZoom != 0))
    Zoom(this, m_inZoom);
}
void CWeaponTheyHungerSniper::Reload() {
  CBasePlayer *p = GetPlayer();
  if (p && p->m_rgAmmo[m_iPrimaryAmmoType] > 0 &&
      m_iClip != TH_SNIPER_MAX_CLIP) {
    if (m_inZoom)
      SecondaryAttack();
    DefaultReload(TH_SNIPER_MAX_CLIP, TH_SNIPER_RELOAD, 3.3f);
  }
}
void CWeaponTheyHungerSniper::WeaponIdle() {
  ResetEmptySound();
  m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
}
void CWeaponTheyHungerSniper::GetAmmoDropInfo(bool sec, const char *&n,
                                              int &a) {
  n = sec ? NULL : "ammo_th_sniper";
  a = sec ? 0 : TH_AMMO_SNIPER_GIVE;
}
void CWeaponTheyHungerTFCSniper::Spawn() {
  Precache();
  m_iId = TH_WEAPON_TFC_SNIPER;
  SetWeaponModelW();
  m_iDefaultAmmo = TH_SNIPER_DEFAULT_GIVE;
  m_inZoom = 0;
  FallInit();
}
void CWeaponTheyHungerTFCSniper::Precache() {
  m_hasHandModels = true;
  m_defaultModelV = "models/v_tfc_sniper.mdl";
  m_defaultModelP = "models/p_sniper.mdl";
  m_defaultModelW = "models/w_isotopebox.mdl";
  CBasePlayerWeapon::Precache();
  PRECACHE_SOUND("weapons/sniper.wav");
  PRECACHE_SOUND("weapons/reload3.wav");
  PrecacheEvents();
}
void CWeaponTheyHungerTFCSniper::PrecacheEvents() {
  m_fireEvent = PRECACHE_EVENT(1, "events/sniper2.sc");
}
int CWeaponTheyHungerTFCSniper::GetItemInfo(ItemInfo *p) {
  Info(p, STRING(pev->classname), TH_WEAPON_TFC_SNIPER, 4);
  m_iId = p->iId;
  return 1;
}
BOOL CWeaponTheyHungerTFCSniper::Deploy() {
  m_fInSpecialReload = 0;
  return DefaultDeploy(GetModelV(), GetModelP(), TH_TFC_SNIPER_AUTODRAW, "bow");
}
void CWeaponTheyHungerTFCSniper::Holster(int) { HolsterSniper(this, m_inZoom); }
void CWeaponTheyHungerTFCSniper::PrimaryAttack() {
  Fire(this, m_fireEvent, m_inZoom, TH_TFC_SNIPER_AUTOIDLE);
}
void CWeaponTheyHungerTFCSniper::SecondaryAttack() { Zoom(this, m_inZoom); }
void CWeaponTheyHungerTFCSniper::ToggleScope(BOOL engage) {
  if ((engage != FALSE) != (m_inZoom != 0))
    Zoom(this, m_inZoom);
}
void CWeaponTheyHungerTFCSniper::Reload() {
  CBasePlayer *p = GetPlayer();
  if (p && p->m_rgAmmo[m_iPrimaryAmmoType] > 0 &&
      m_iClip != TH_SNIPER_MAX_CLIP) {
    if (m_inZoom)
      SecondaryAttack();
    if (DefaultReload(TH_SNIPER_MAX_CLIP, TH_TFC_SNIPER_AUTOHOLSTER, .5f)) {
      m_fInSpecialReload = 2;
      m_flNextPrimaryAttack = m_flNextSecondaryAttack =
          UTIL_WeaponTimeBase() + 2;
      m_flTimeWeaponIdle =
          UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(p->random_seed, 5, 10);
    }
  }
}
void CWeaponTheyHungerTFCSniper::WeaponIdle() {
  CBasePlayer *p = GetPlayer();
  if (!p)
    return;
  ResetEmptySound();
  if (p->m_flNextAttack >= UTIL_WeaponTimeBase())
    return;
  if (m_fInSpecialReload == 3) {
    SendWeaponAnim(TH_TFC_SNIPER_AUTODRAW, 0);
    m_fInSpecialReload = 1;
  }
  if (m_fInSpecialReload == 2) {
    EMIT_SOUND_DYN(ENT(p->pev), CHAN_ITEM, "weapons/reload3.wav",
                   RANDOM_FLOAT(.8f, .9f), ATTN_NORM, 0,
                   93 + RANDOM_LONG(0, 0x1f));
    p->m_flNextAttack = UTIL_WeaponTimeBase() + 1;
    m_fInSpecialReload = 3;
  }
  if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase()) {
    SendWeaponAnim(TH_TFC_SNIPER_AUTOIDLE, 1);
    m_flTimeWeaponIdle =
        UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(p->random_seed, 15, 20);
  }
}
void CWeaponTheyHungerTFCSniper::GetAmmoDropInfo(bool sec, const char *&n,
                                                 int &a) {
  n = sec ? NULL : "ammo_einar1";
  a = sec ? 0 : TH_AMMO_SNIPER_GIVE;
}
class CAmmoTheyHungerSniper : public CBasePlayerAmmo {
public:
  void Precache() {
    m_defaultModel = "models/w_antidote.mdl";
    CBasePlayerAmmo::Precache();
    PRECACHE_SOUND("items/9mmclip1.wav");
  }
  BOOL AddAmmo(CBaseEntity *o) {
    int r = o->GiveAmmo(TH_AMMO_SNIPER_GIVE, TH_SNIPER_AMMO_NAME);
    if (r != -1)
      EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
    return r != -1;
  }
};
LINK_ENTITY_TO_CLASS(ammo_th_sniper, CAmmoTheyHungerSniper)
LINK_ENTITY_TO_CLASS(ammo_einar1, CAmmoTheyHungerSniper)
