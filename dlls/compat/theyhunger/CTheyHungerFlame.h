#pragma once

#include "CBaseEntity.h"

class CEinarFlameRocket : public CBaseEntity {
public:
	void Spawn() override;
	void Precache() override;
	void FlameTouch(CBaseEntity* other);
	static CEinarFlameRocket* FlameCreate(const Vector& origin, const Vector& angles, edict_t* owner);
};
