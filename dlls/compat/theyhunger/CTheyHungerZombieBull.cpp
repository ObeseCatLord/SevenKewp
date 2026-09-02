#include "../../extdll.h"
#include "../../util/util.h"
#include "../../monster/monsters.h"
#include "CTheyHungerZombieBull.h"

namespace {
constexpr int ZBULL_AE_BITE = 2;
constexpr int ZBULL_AE_TAILWHIP = 4;
constexpr int ZBULL_AE_THROW = 6;
}

LINK_ENTITY_TO_CLASS(monster_th_zombiebull, CEinarZombieBull)

const char* CEinarZombieBull::pAttackSounds[] = {
	"bull/bu_whip1.wav", "bull/bu_whip2.wav", "bull/bu_whip3.wav",
};
const char* CEinarZombieBull::pBiteSounds[] = {
	"bull/bu_gore1.wav", "bull/bu_gore2.wav", "bull/bu_gore3.wav",
};
const char* CEinarZombieBull::pIdleSounds[] = {
	"bull/bu_idle1.wav", "bull/bu_idle2.wav", "bull/bu_idle3.wav",
};
const char* CEinarZombieBull::pAlertSounds[] = {
	"bull/bu_alert1.wav", "bull/bu_alert2.wav", "bull/bu_alert3.wav", "bull/bu_alert4.wav",
};
const char* CEinarZombieBull::pPainSounds[] = {
	"bull/bu_pain1.wav", "bull/bu_pain2.wav", "bull/bu_pain3.wav",
};
const char* CEinarZombieBull::pDeathSounds[] = {
	"bull/bu_die1.wav", "bull/bu_die2.wav", "bull/bu_die3.wav",
};

int CEinarZombieBull::Classify() {
	return CLASS_ALIEN_MONSTER;
}

int CEinarZombieBull::IgnoreConditions() {
	int ignore = CBaseMonster::IgnoreConditions();
	if (gpGlobals->time - m_flLastHurtTime <= 20) {
		ignore = bits_COND_SMELL | bits_COND_SMELL_FOOD;
	}
	return ignore;
}

int CEinarZombieBull::IRelationship(CBaseEntity* target) {
	return CBaseMonster::IRelationship(target);
}

int CEinarZombieBull::TakeDamage(entvars_t* inflictor, entvars_t* attacker, float damage, int damageType) {
	if (m_hEnemy != nullptr && IsMoving() && attacker == m_hEnemy->pev && gpGlobals->time - m_flLastHurtTime > 3) {
		float distance = (pev->origin - m_hEnemy->pev->origin).Length2D();
		if (distance > SQUID_SPRINT_DIST) {
			distance = (pev->origin - m_Route[m_iRouteIndex].vecLocation).Length2D();
			Vector apex;
			if (FTriangulate(pev->origin, m_Route[m_iRouteIndex].vecLocation, distance * 0.5f, m_hEnemy, &apex)) {
				InsertWaypoint(apex, bits_MF_TO_DETOUR | bits_MF_DONT_SIMPLIFY);
			}
		}
	}

	m_flLastHurtTime = gpGlobals->time;
	return CBaseMonster::TakeDamage(inflictor, attacker, damage, damageType);
}

BOOL CEinarZombieBull::CheckMeleeAttack1(float dot, float distance) {
	return m_hEnemy->pev->health <= gSkillData.sk_bullsquid_dmg_whip && distance <= 110 && dot >= 0.7f;
}

BOOL CEinarZombieBull::CheckMeleeAttack2(float dot, float distance) {
	return distance <= 110 && dot >= 0.7f && !HasConditions(bits_COND_CAN_MELEE_ATTACK1);
}

void CEinarZombieBull::IdleSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1, ATTN_NORM);
}

void CEinarZombieBull::PainSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1, ATTN_NORM);
}

void CEinarZombieBull::DeathSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1, ATTN_NORM);
}

void CEinarZombieBull::AlertSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1, ATTN_NORM);
}

void CEinarZombieBull::AttackSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1, ATTN_NORM);
}

void CEinarZombieBull::BiteSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBiteSounds), 1, ATTN_NORM);
}

void CEinarZombieBull::SetYawSpeed() {
	pev->yaw_speed = 95;
}

void CEinarZombieBull::HandleAnimEvent(MonsterEvent_t* event) {
	switch (event->event) {
	case ZBULL_AE_BITE: {
		CBaseEntity* hurt = CheckTraceHullAttack(110, gSkillData.sk_bullsquid_dmg_bite, DMG_SLASH);
		if (hurt) {
			hurt->pev->velocity = hurt->pev->velocity - gpGlobals->v_forward * 50;
			hurt->pev->velocity = hurt->pev->velocity + gpGlobals->v_up * 30;
		}
		break;
	}
	case ZBULL_AE_TAILWHIP: {
		CBaseEntity* hurt = CheckTraceHullAttack(110, gSkillData.sk_bullsquid_dmg_whip, DMG_CLUB | DMG_ALWAYSGIB);
		if (hurt) {
			hurt->pev->velocity = hurt->pev->velocity + gpGlobals->v_forward * 500;
			hurt->pev->velocity = hurt->pev->velocity + gpGlobals->v_up * 30;
		}
		break;
	}
	case ZBULL_AE_THROW: {
		CBaseEntity* hurt = CheckTraceHullAttack(110, 0, 0);
		if (hurt) {
			int pitch = RANDOM_FLOAT(90, 110);
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pBiteSounds), 1, ATTN_NORM, 0, pitch);
			UTIL_ScreenShake(hurt->pev->origin, 25.0, 1.5, 0.7, 2);
			if (hurt->IsPlayer()) {
				UTIL_MakeVectors(pev->angles);
				hurt->pev->velocity = hurt->pev->velocity + gpGlobals->v_forward * 500 + gpGlobals->v_up * 30;
			}
		}
		break;
	}
	default:
		CBullsquid::HandleAnimEvent(event);
		break;
	}
}

void CEinarZombieBull::Spawn() {
	Precache();

	InitModel();
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorHuman();
	pev->effects = 0;
	pev->health = gSkillData.sk_bullsquid_health * 5;
	m_flFieldOfView = 0.2;
	m_MonsterState = MONSTERSTATE_NONE;
	m_fCanThreatDisplay = FALSE;

	MonsterInit();
}

void CEinarZombieBull::Precache() {
	CBaseMonster::Precache();
	m_defaultModel = "models/zombiebull.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_SOUND("zombie/claw_miss2.wav");
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pBiteSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND("bull/bu_paw1.wav");
	PRECACHE_SOUND("bull/bu_runstep1.wav");
	PRECACHE_SOUND("bull/bu_runstep2.wav");
	PRECACHE_SOUND("bull/bu_walkstep1.wav");
	PRECACHE_SOUND("bull/bu_walkstep2.wav");
}

void CEinarZombieBull::RunAI() {
	CBaseMonster::RunAI();
	if (m_hEnemy != nullptr && m_Activity == ACT_RUN && (pev->origin - m_hEnemy->pev->origin).Length2D() < SQUID_SPRINT_DIST) {
		pev->framerate = 1.25;
	}
}

Schedule_t* CEinarZombieBull::GetSchedule() {
	switch (m_MonsterState) {
	case MONSTERSTATE_ALERT:
		return CBaseMonster::GetSchedule();
	case MONSTERSTATE_COMBAT:
		if (HasConditions(bits_COND_ENEMY_DEAD)) {
			return CBaseMonster::GetSchedule();
		}
		if (HasConditions(bits_COND_NEW_ENEMY)) {
			return GetScheduleOfType(SCHED_WAKE_ANGRY);
		}
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1)) {
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK2)) {
			return GetScheduleOfType(SCHED_MELEE_ATTACK2);
		}
		return GetScheduleOfType(SCHED_CHASE_ENEMY);
	default:
		return CBaseMonster::GetSchedule();
	}
}

Schedule_t* CEinarZombieBull::GetScheduleOfType(int type) {
	switch (type) {
	case SCHED_RANGE_ATTACK1:
	case SCHED_SQUID_SNIFF_AND_EAT:
		return CBaseMonster::GetScheduleOfType(type);
	default:
		return CBullsquid::GetScheduleOfType(type);
	}
}

void CEinarZombieBull::StartTask(Task_t* task) {
	m_iTaskStatus = TASKSTATUS_RUNNING;
	if (task->iTask == TASK_MELEE_ATTACK2) {
		AttackSound();
		CBaseMonster::StartTask(task);
	}
	else {
		CBullsquid::StartTask(task);
	}
}
