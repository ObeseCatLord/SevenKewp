#pragma once

#include "../../monster/CAGrunt.h"

class CEinarCyberFranklin : public CAGrunt {
public:
	void Spawn() override;
	void Precache() override;
	void HandleAnimEvent(MonsterEvent_t* event) override;

	void AlertSound() override;
	void DeathSound() override;
	void PainSound() override;
	void AttackSound() override;
	void IdleSound() override {}

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pDieSounds[];
	static const char* pPainSounds[];
	static const char* pAlertSounds[];
};
