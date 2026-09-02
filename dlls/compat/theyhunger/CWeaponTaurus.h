#pragma once
#include "../../weapon/CBasePlayerWeapon.h"
class CWeaponTheyHungerTaurus : public CBasePlayerWeapon {
public:
  void Spawn();
  void Precache();
  void PrecacheEvents();
  int iItemSlot() { return 2; }
  int GetItemInfo(ItemInfo *p);
  BOOL Deploy();
  void PrimaryAttack();
  void Reload();
  void WeaponIdle();
  BOOL ShouldWeaponIdle() { return TRUE; }
  void GetAmmoDropInfo(bool secondary, const char *&ammoEntName,
                       int &dropAmount) override;
  BOOL UseDecrement() {
#if defined(CLIENT_WEAPONS)
    return TRUE;
#else
    return FALSE;
#endif
  }

private:
  void Fire(float spread, float cycleTime, BOOL useAutoAim);
  unsigned short m_fireEvent;
};
