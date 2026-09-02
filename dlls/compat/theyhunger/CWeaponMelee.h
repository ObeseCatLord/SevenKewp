#pragma once

#include "../../weapon/CBasePlayerWeapon.h"

#define TH_WEAPON_SHOVEL 29
#define TH_WEAPON_SPANNER 30

// Shared plumbing keeps both Team Beef weapons on SevenKewp's base weapon,
// prediction, and VR-pose APIs; each weapon supplies its original behavior.
class CWeaponTheyHungerMelee : public CBasePlayerWeapon {
public:
	void Holster(int skiplocal = 0) override;
	void ItemPostFrame() override;
	void Smack();
	int Swing(int firstSwing);

protected:
	virtual int MissAnimation() = 0;
	virtual int HitAnimation() = 0;
	virtual int HolsterAnimation() const = 0;
	virtual float MissDelay() const = 0;
	virtual float HitDelay() const = 0;
	virtual float DamageMultiplier(bool multiplayer, bool firstSwing) const = 0;
	virtual float PhysicalLength() const = 0;
	virtual int BodyHitVolume() const = 0;
	virtual int WallHitVolume() const = 0;
	virtual bool UsesSkeletonSounds() const = 0;
	void InitializePhysicalSwingState();

	int m_iSwing = 0;
	TraceResult m_trHit;

#ifndef CLIENT_DLL
	void CheckPhysicalSwing(float speed);
	bool HasHitEntityThisSwing(CBaseEntity* entity);
	void RememberHitEntityThisSwing(CBaseEntity* entity);
	void ClearPhysicalSwingState();
	bool m_physicalSwinging = false;
	float m_physicalLastWoosh = 0.0f;
	float m_physicalHitCount = 0.0f;
	EHANDLE m_physicalHitEntities[128];
#endif
};

class CWeaponTheyHungerShovel : public CWeaponTheyHungerMelee {
public:
	void Spawn();
	void Precache();
	int iItemSlot() { return 1; }
	int AddToPlayer(CBasePlayer* player);
	int GetItemInfo(ItemInfo* info);
	BOOL Deploy();
	void PrimaryAttack() override;
	void SwingAgain();
	void WeaponIdle() override;
	BOOL ShouldWeaponIdle() override { return TRUE; }
	BOOL UseDecrement() override;

private:
	int MissAnimation() override;
	int HitAnimation() override;
	int HolsterAnimation() const override;
	float MissDelay() const override;
	float HitDelay() const override;
	float DamageMultiplier(bool multiplayer, bool firstSwing) const override;
	float PhysicalLength() const override;
	int BodyHitVolume() const override;
	int WallHitVolume() const override;
	bool UsesSkeletonSounds() const override;
};

class CWeaponTheyHungerSpanner : public CWeaponTheyHungerMelee {
public:
	void Spawn();
	void Precache();
	int iItemSlot() { return 1; }
	int AddToPlayer(CBasePlayer* player);
	int GetItemInfo(ItemInfo* info);
	BOOL Deploy();
	void PrimaryAttack() override;
	void SwingAgain();
	void WeaponIdle() override;
	BOOL ShouldWeaponIdle() override { return TRUE; }
	BOOL UseDecrement() override;

private:
	int MissAnimation() override;
	int HitAnimation() override;
	int HolsterAnimation() const override;
	float MissDelay() const override;
	float HitDelay() const override;
	float DamageMultiplier(bool multiplayer, bool firstSwing) const override;
	float PhysicalLength() const override;
	int BodyHitVolume() const override;
	int WallHitVolume() const override;
	bool UsesSkeletonSounds() const override;
};
