#pragma once

#include "../../monster/CBullsquid.h"

class CEinarMegasquid : public CBullsquid {
public:
	void Spawn() override;
	void Precache() override;
	void HandleAnimEvent(MonsterEvent_t* event) override;
	int Classify() override;
	BOOL CheckRangeAttack1(float dot, float distance) override;
	BOOL CheckMeleeAttack1(float dot, float distance) override { return FALSE; }
	BOOL CheckMeleeAttack2(float dot, float distance) override;
};
