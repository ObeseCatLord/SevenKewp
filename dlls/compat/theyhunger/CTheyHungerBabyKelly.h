#pragma once

#include "../../monster/CISlave.h"

class CEinarBabyKelly : public CISlave {
public:
	void Spawn() override;
	void Precache() override;

	void DeathSound() override;
	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
};
