#pragma once

#include "../../weapon/CBasePlayerWeapon.h"

// SevenKewp reserves different slots than original They Hunger.
#define TH_WEAPON_CHAINGUN 27
#define TH_WEAPON_MEDKIT 28

class CWeaponTheyHungerChaingun : public CBasePlayerWeapon
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
	int iItemSlot() { return 4; }
	int GetItemInfo(ItemInfo* p);
	BOOL Deploy();
	void Holster(int skiplocal = 0);
	void PrimaryAttack();
	void SecondaryAttack();
	void Reload();
	void WeaponIdle();
	BOOL ShouldWeaponIdle() { return TRUE; }
	BOOL UseDecrement()
	{
#if defined(CLIENT_WEAPONS)
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void SpinUp();
	void SpinDown();
	void Fire(float spread, float cycleTime);

	int m_spinning;
	unsigned short m_spinEvent;
	unsigned short m_fireEvent;
};

class CWeaponTheyHungerMedkit : public CBasePlayerWeapon
{
public:
#if !defined(CLIENT_DLL)
	int Save(CSave& save);
	int Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];
#endif
	void Spawn();
	void Precache();
	int ExtractAmmo(CBasePlayerWeapon* target) override;
	int iItemSlot() { return 5; }
	int GetItemInfo(ItemInfo* p);
	BOOL Deploy();
	void Holster(int skiplocal = 0);
	void PrimaryAttack();
	void SecondaryAttack();
	void Reload();
	void WeaponIdle();
	BOOL ShouldWeaponIdle() { return TRUE; }
	BOOL PlayEmptySound();
	BOOL UseDecrement()
	{
#if defined(CLIENT_WEAPONS)
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void StartUse();
	void Heal();

	int m_using;
};
