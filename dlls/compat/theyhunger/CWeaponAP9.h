#pragma once

#include "../../weapon/CBasePlayerWeapon.h"

class CWeaponTheyHungerAP9 : public CBasePlayerWeapon
{
public:
#if !defined(CLIENT_DLL)
	int Save(CSave& save);
	int Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];
#endif
	void Spawn();
	void Precache();
	void PrecacheEvents();
	int iItemSlot() { return 2; }
	int GetItemInfo(ItemInfo* p);

	void PrimaryAttack();
	void SecondaryAttack();
	BOOL Deploy();
	void Reload();
	void WeaponIdle();
	BOOL ShouldWeaponIdle() { return TRUE; }
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) override;

	BOOL UseDecrement()
	{
#if defined(CLIENT_WEAPONS)
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void Fire(float spread, float cycleTime, BOOL useAutoAim);

	int m_burstShots;
	int m_shell;
	unsigned short m_fireEvent;
};
