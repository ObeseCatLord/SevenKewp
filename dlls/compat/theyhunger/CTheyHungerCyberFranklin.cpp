#include "../../extdll.h"
#include "../../util/util.h"
#include "../../monster/monsters.h"
#include "../../env/te_effects.h"
#include "CTheyHungerCyberFranklin.h"

namespace {
constexpr float FRANKLIN_MELEE_DIST = 100.0f;
constexpr int CF_AE_KICK = 3;
constexpr int CF_AE_SHOOT1 = 4;
constexpr int CF_AE_SHOOT2 = 5;
constexpr int CF_AE_SHOOT3 = 6;
constexpr int CF_AE_DROP_GUN = 11;

int g_franklinMuzzleFlash;
}

LINK_ENTITY_TO_CLASS(monster_th_cyberfranklin, CEinarCyberFranklin)

const char* CEinarCyberFranklin::pAttackHitSounds[] = {
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CEinarCyberFranklin::pAttackMissSounds[] = {
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CEinarCyberFranklin::pAttackSounds[] = {
	"franklin/attack1.wav",
};

const char* CEinarCyberFranklin::pDieSounds[] = {
	"franklin/death1.wav",
	"franklin/death2.wav",
	"franklin/death3.wav",
};

const char* CEinarCyberFranklin::pPainSounds[] = {
	"franklin/pain1.wav",
	"franklin/pain2.wav",
};

const char* CEinarCyberFranklin::pAlertSounds[] = {
	"franklin/alert1.wav",
};

void CEinarCyberFranklin::DeathSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDieSounds), 1.0, ATTN_NORM);
}

void CEinarCyberFranklin::AlertSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM);
}

void CEinarCyberFranklin::AttackSound() {
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM);
}

void CEinarCyberFranklin::PainSound() {
	if (m_flNextPainTime > gpGlobals->time) {
		return;
	}

	m_flNextPainTime = gpGlobals->time + 2.0f;
	EMIT_SOUND(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM);
}

void CEinarCyberFranklin::HandleAnimEvent(MonsterEvent_t* event) {
	switch (event->event) {
	case CF_AE_SHOOT1:
	case CF_AE_SHOOT2:
	case CF_AE_SHOOT3: {
		Vector armPos, armDir;
		Vector dirToEnemy;
		Vector angDir;

		if (HasConditions(bits_COND_SEE_ENEMY)) {
			dirToEnemy = m_vecEnemyLKP - pev->origin;
			angDir = UTIL_VecToAngles(dirToEnemy);
			dirToEnemy = dirToEnemy.Normalize();
		}
		else {
			angDir = pev->angles;
			UTIL_MakeAimVectors(angDir);
			dirToEnemy = gpGlobals->v_forward;
		}

		pev->effects = EF_MUZZLEFLASH;
		if (angDir.x > 180) {
			angDir.x -= 360;
		}

		SetBlending(0, angDir.x);
		GetAttachment(0, armPos, armDir);
		armPos = armPos + dirToEnemy * 32;
		UTIL_Sprite(armPos, g_franklinMuzzleFlash, 6, 128);

		CBaseEntity* hornet = CBaseEntity::Create("hornet", armPos, UTIL_VecToAngles(dirToEnemy), true, edict());
		UTIL_MakeVectors(hornet->pev->angles);
		hornet->pev->velocity = gpGlobals->v_forward * 300;

		switch (RANDOM_LONG(0, 2)) {
		case 0: EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "agrunt/ag_fire1.wav", 1.0, ATTN_NORM, 0, 100); break;
		case 1: EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "agrunt/ag_fire2.wav", 1.0, ATTN_NORM, 0, 100); break;
		case 2: EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "agrunt/ag_fire3.wav", 1.0, ATTN_NORM, 0, 100); break;
		}

		CBaseMonster* hornetMonster = hornet->MyMonsterPointer();
		if (hornetMonster) {
			hornetMonster->m_hEnemy = m_hEnemy;
		}
		break;
	}
	case CF_AE_KICK: {
		CBaseEntity* hurt = CheckTraceHullAttack(FRANKLIN_MELEE_DIST, gSkillData.sk_agrunt_dmg_punch, DMG_CLUB);
		if (hurt) {
			hurt->pev->punchangle.y = RANDOM_LONG(-5, 5);
			hurt->pev->punchangle.x = 8;
			if (hurt->IsPlayer()) {
				hurt->pev->velocity = hurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_right * RANDOM_LONG(-50, 50);
			}

			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			Vector armPos, armAng;
			GetAttachment(0, armPos, armAng);
			SpawnBlood(armPos, hurt->BloodColor(), 25);
		}
		else {
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		break;
	}
	case CF_AE_DROP_GUN:
		break;
	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CEinarCyberFranklin::Spawn() {
	Precache();

	InitModel();
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorHuman();
	pev->effects = 0;
	pev->health = gSkillData.sk_agrunt_health * 6;
	m_flFieldOfView = -1.0;
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD;
	m_HackedGunPos = Vector(24, 64, 48);

	MonsterInit();
}

void CEinarCyberFranklin::Precache() {
	CBaseMonster::Precache();

	m_defaultModel = "models/franklin2.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND("player/pl_tile1.wav");
	PRECACHE_SOUND("franklin/franklin_step.wav");
	PRECACHE_SOUND("hassault/hw_shoot1.wav");
	g_franklinMuzzleFlash = PRECACHE_MODEL("sprites/muz4.spr");
	UTIL_PrecacheOther("hornet");
}
