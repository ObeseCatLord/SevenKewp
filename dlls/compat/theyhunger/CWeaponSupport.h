#pragma once
#include "../../weapon/CBasePlayerWeapon.h"

#define TH_WEAPON_CHAINGUN 27
#define TH_WEAPON_MEDKIT 28

class CWeaponTheyHungerChaingun : public CBasePlayerWeapon {
public:
	void Spawn(); void Precache(); int GetItemInfo(ItemInfo* p); BOOL Deploy();
	void Holster(int = 0); void PrimaryAttack(); void SecondaryAttack(); void Reload(); void WeaponIdle();
	BOOL UseDecrement() {
#if defined(CLIENT_WEAPONS)
		return TRUE;
#else
		return FALSE;
#endif
	}
private: void Fire(); void SpinDown(); int m_spinning; unsigned short m_spinEvent; unsigned short m_fireEvent;
};
class CWeaponTheyHungerMedkit : public CBasePlayerWeapon {
public:
	void Spawn(); void Precache(); int GetItemInfo(ItemInfo* p); BOOL Deploy();
	void Holster(int = 0); void PrimaryAttack(); void SecondaryAttack(); void Reload(); void WeaponIdle(); BOOL PlayEmptySound();
	BOOL UseDecrement() {
#if defined(CLIENT_WEAPONS)
		return TRUE;
#else
		return FALSE;
#endif
	}
private: void Heal(); int m_using;
};
