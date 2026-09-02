#pragma once

#include "../../monster/CBullsquid.h"

class CEinarZombieBull : public CBullsquid {
public:
	void Spawn() override;
	void Precache() override;
	int Classify() override;
	void SetYawSpeed() override;
	void HandleAnimEvent(MonsterEvent_t* event) override;
	void IdleSound() override;
	void PainSound() override;
	void DeathSound() override;
	void AlertSound() override;
	void AttackSound();
	void BiteSound();
	void StartTask(Task_t* task) override;
	int IgnoreConditions() override;
	void RunAI() override;
	int IRelationship(CBaseEntity* target) override;
	int TakeDamage(entvars_t* inflictor, entvars_t* attacker, float damage, int damageType) override;
	BOOL CheckMeleeAttack1(float dot, float distance) override;
	BOOL CheckMeleeAttack2(float dot, float distance) override;
	BOOL CheckRangeAttack1(float dot, float distance) override { return FALSE; }
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int type) override;

	static const char* pAttackSounds[];
	static const char* pBiteSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
};
