#include "../../extdll.h"
#include "CTheyHungerBabyKelly.h"

LINK_ENTITY_TO_CLASS(monster_th_babykelly, CEinarBabyKelly)

const char* CEinarBabyKelly::pAttackHitSounds[] = {
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CEinarBabyKelly::pAttackMissSounds[] = {
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CEinarBabyKelly::pPainSounds[] = {
	"babykelly/slv_pain1.wav",
	"babykelly/slv_pain2.wav",
};

const char* CEinarBabyKelly::pDeathSounds[] = {
	"babykelly/slv_die1.wav",
	"babykelly/slv_die2.wav",
};

void CEinarBabyKelly::AlertSound() {
	if (m_hEnemy != nullptr) {
		SENTENCEG_PlayRndSz(edict(), "BKL_ALERT", 0.85, ATTN_NORM, 0, m_voicePitch);

		CallForHelp("monster_alien_slave", 512, m_hEnemy, m_vecEnemyLKP);
		CallForHelp("monster_th_babykelly", 512, m_hEnemy, m_vecEnemyLKP);
	}
}

void CEinarBabyKelly::IdleSound() {
	if (RANDOM_LONG(0, 2) == 0) {
		SENTENCEG_PlayRndSz(edict(), "BKL_IDLE", 0.85, ATTN_NORM, 0, m_voicePitch);
	}
}

void CEinarBabyKelly::PainSound() {
	if (RANDOM_LONG(0, 2) == 0) {
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
	}
}

void CEinarBabyKelly::DeathSound() {
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
}

void CEinarBabyKelly::Spawn() {
	Precache();

	SET_MODEL(edict(), pev->model ? STRING(pev->model) : "models/babykelly.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = DONT_BLEED;
	pev->effects = 0;
	pev->health = gSkillData.sk_islave_health;
	pev->view_ofs = Vector(0, 0, 32);
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_RANGE_ATTACK2 | bits_CAP_DOORS_GROUP;

	m_voicePitch = RANDOM_LONG(85, 110);

	MonsterInit();
}

void CEinarBabyKelly::Precache() {
	CBaseMonster::Precache();

	PRECACHE_MODEL(pev->model ? STRING(pev->model) : "models/babykelly.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_SOUND("debris/zap1.wav");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("hassault/hw_shoot1.wav");
	PRECACHE_SOUND("zombie/zo_pain2.wav");
	PRECACHE_SOUND("headcrab/hc_headbite.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	UTIL_PrecacheOther("test_effect");
}
