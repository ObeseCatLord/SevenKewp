#pragma once
#include "../../weapon/CBasePlayerWeapon.h"
class CWeaponTheyHungerSniper : public CBasePlayerWeapon {
public:
  void Spawn();
  void Precache();
  void PrecacheEvents();
  int iItemSlot() { return 3; }
  int GetItemInfo(ItemInfo *);
  BOOL Deploy();
  void Holster(int = 0);
  void PrimaryAttack();
  void SecondaryAttack();
  void Reload();
  void WeaponIdle();
  BOOL ShouldWeaponIdle() { return TRUE; }
  void GetAmmoDropInfo(bool, const char *&, int &) override;
  BOOL UseDecrement() {
#if defined(CLIENT_WEAPONS)
    return TRUE;
#else
    return FALSE;
#endif
  }

private:
  int m_inZoom;
  unsigned short m_fireEvent;
};
class CWeaponTheyHungerTFCSniper : public CBasePlayerWeapon {
public:
  void Spawn();
  void Precache();
  void PrecacheEvents();
  int iItemSlot() { return 3; }
  int GetItemInfo(ItemInfo *);
  BOOL Deploy();
  void Holster(int = 0);
  void PrimaryAttack();
  void SecondaryAttack();
  void Reload();
  void WeaponIdle();
  BOOL ShouldWeaponIdle() { return TRUE; }
  void GetAmmoDropInfo(bool, const char *&, int &) override;
  BOOL UseDecrement() {
#if defined(CLIENT_WEAPONS)
    return TRUE;
#else
    return FALSE;
#endif
  }

private:
  int m_inZoom;
  unsigned short m_fireEvent;
};
