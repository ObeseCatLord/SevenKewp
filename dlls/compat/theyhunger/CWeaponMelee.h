#pragma once

#include "../../weapon/CCrowbar.h"

#define TH_WEAPON_SHOVEL 29
#define TH_WEAPON_SPANNER 30

class CWeaponTheyHungerMelee : public CCrowbar
{
protected:
	bool ExclusivePhysicalVRPose() const override { return true; }
	float PhysicalSwingMinSpeed() const override { return 70.0f; }
	size_t PhysicalSwingDedupeLimit() const override { return 128; }
	bool UsesCrowbarSwingEvent() const override { return false; }
	bool IsPhysicalBodyHit(CBaseEntity* entity) const override;
	const char* PhysicalBodyHitSound(CBaseEntity* entity) const override;
	bool StopPhysicalFeedbackOnKill() const override { return true; }
	float DesktopDamage() override;
	virtual float DesktopDamageMultiplier(bool multiplayer, bool firstSwing) const = 0;
	void SetDesktopSwingIdle() override;
	Vector DesktopSwingOrigin(CBasePlayer* player) const override;
	void PrecacheTheyHungerMelee();

public:
	int MergedModelBody() override { return -1; }
};

class CWeaponTheyHungerShovel : public CWeaponTheyHungerMelee
{
public:
	void Spawn();
	void Precache();
	int iItemSlot() { return 1; }
	int GetItemInfo(ItemInfo* info);
	BOOL Deploy();
	void WeaponIdle() override;
	BOOL ShouldWeaponIdle() override { return TRUE; }

protected:
	float PhysicalSwingReach() const override { return 24.0f; }
	float PhysicalLivingBodyVolumeScale() const override { return 0.1f; }
	int HolsterAnimation() const override;
	int DesktopMissAnimation() override;
	int DesktopHitAnimation() override;
	float DesktopDamageMultiplier(bool multiplayer, bool firstSwing) const override;
};

class CWeaponTheyHungerSpanner : public CWeaponTheyHungerMelee
{
public:
	void Spawn();
	void Precache();
	int iItemSlot() { return 1; }
	int GetItemInfo(ItemInfo* info);
	BOOL Deploy();
	void WeaponIdle() override;
	BOOL ShouldWeaponIdle() override { return TRUE; }

protected:
	float PhysicalSwingReach() const override { return 15.0f; }
	float PhysicalLivingBodyVolumeScale() const override { return 1.0f; }
	int HolsterAnimation() const override;
	int DesktopMissAnimation() override;
	int DesktopHitAnimation() override;
	float DesktopMissDelay() const override { return 0.4f; }
	float DesktopHitDelay() const override { return 0.2f; }
	float DesktopDamageMultiplier(bool multiplayer, bool firstSwing) const override;
};
