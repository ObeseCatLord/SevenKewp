#pragma once

#include "../../monster/CHeadCrab.h"

class CEinarChicken : public CHeadCrab {
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* event) override;
	void StartTask(Task_t* task) override;
	void RunTask(Task_t* task) override;
	int TakeDamage(entvars_t* inflictor, entvars_t* attacker, float damage, int damageTypes) override;
	void LeapTouch(CBaseEntity* other);

	void PainSound() override;
	void DeathSound() override;
	void IdleSound() override;
	void AlertSound() override;
	void AttackSound();
	void StartAttackSound();
	void BiteSound();

	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pDeathSounds[];
};
