#pragma once
#include "../../weapon/CCrowbar.h"

#define TH_WEAPON_SHOVEL 29
#define TH_WEAPON_SPANNER 30

// The reference weapons share crowbar hit timing.  Deriving here retains the
// maintained VR physical-swing and haptics state machine instead of a copy.
class CWeaponTheyHungerShovel : public CCrowbar {
public:
	void Spawn(); void Precache(); int GetItemInfo(ItemInfo* p); BOOL Deploy();
};
class CWeaponTheyHungerSpanner : public CCrowbar {
public:
	void Spawn(); void Precache(); int GetItemInfo(ItemInfo* p); BOOL Deploy();
};
