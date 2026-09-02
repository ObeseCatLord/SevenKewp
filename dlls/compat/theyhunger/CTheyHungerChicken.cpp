#include "../../extdll.h"
#include "CTheyHungerChicken.h"

LINK_ENTITY_TO_CLASS(monster_th_chicken, CEinarChicken)

const char* CEinarChicken::pIdleSounds[] = {
	"chicken/ch_idle1.wav",
	"chicken/ch_idle2.wav",
};

const char* CEinarChicken::pAlertSounds[] = {
	"chicken/ch_alert1.wav",
	"chicken/ch_alert2.wav",
};

const char* CEinarChicken::pPainSounds[] = {
	"chicken/ch_pain1.wav",
	"chicken/ch_pain2.wav",
};

const char* CEinarChicken::pAttackSounds[] = {
	"chicken/ch_attack1.wav",
	"chicken/ch_attack2.wav",
};

const char* CEinarChicken::pDeathSounds[] = {
	"chicken/ch_die1.wav",
	"chicken/ch_die2.wav",
};

int CEinarChicken::Classify() {
	return CLASS_ALIEN_MONSTER;
}

void CEinarChicken::Spawn() {
	Precache();

	SET_MODEL(edict(), "models/chicken.mdl");
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorHuman();
	pev->effects = 0;
	pev->health = gSkillData.sk_headcrab_health;
	pev->view_ofs = Vector(0, 0, 20);
	pev->yaw_speed = 5;
	m_flFieldOfView = 0.5;
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
}

void CEinarChicken::Precache() {
	CBaseMonster::Precache();

	PRECACHE_SOUND("thehand/hnd_attack1.wav");
	PRECACHE_SOUND("thehand/hnd_headbite.wav");
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	PRECACHE_MODEL("models/chicken.mdl");
}

void CEinarChicken::SetYawSpeed() {
	int yawSpeed;

	switch (m_Activity) {
	case ACT_IDLE:
		yawSpeed = 60;
		break;
	case ACT_RUN:
	case ACT_WALK:
		yawSpeed = 30;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		yawSpeed = 120;
		break;
	case ACT_RANGE_ATTACK1:
	default:
		yawSpeed = 60;
		break;
	}

	pev->yaw_speed = yawSpeed;
}

// SevenKewp's CHeadCrab inlines its sounds in these callbacks, unlike the
// They Hunger base class. Keep the original virtual sound dispatch here.
void CEinarChicken::HandleAnimEvent(MonsterEvent_t* event) {
	switch (event->event) {
	case HC_AE_JUMPATTACK:
	{
		ClearBits(pev->flags, FL_ONGROUND);
		UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, 1));
		UTIL_MakeVectors(pev->angles);

		Vector jumpDirection;
		if (m_hEnemy != nullptr) {
			float gravity = g_psv_gravity->value;
			if (gravity <= 1) {
				gravity = 1;
			}

			float height = m_hEnemy->pev->origin.z + m_hEnemy->pev->view_ofs.z - pev->origin.z;
			if (height < 16) {
				height = 16;
			}
			float speed = sqrt(2 * gravity * height);
			float time = speed / gravity;

			jumpDirection = m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - pev->origin;
			jumpDirection = jumpDirection * (1.0f / time);
			jumpDirection.z = speed;

			float distance = jumpDirection.Length();
			if (distance > 650.0f) {
				jumpDirection = jumpDirection * (650.0f / distance);
			}
		}
		else {
			jumpDirection = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z) * 350.0f;
		}

		AttackSound();
		pev->velocity = jumpDirection;
		m_flNextAttack = gpGlobals->time + 2.0f;
		break;
	}
	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CEinarChicken::StartTask(Task_t* task) {
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (task->iTask) {
	case TASK_RANGE_ATTACK1:
		StartAttackSound();
		m_IdealActivity = ACT_RANGE_ATTACK1;
		SetTouch(&CEinarChicken::LeapTouch);
		break;
	default:
		CBaseMonster::StartTask(task);
		break;
	}
}

void CEinarChicken::RunTask(Task_t* task) {
	switch (task->iTask) {
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
		if (m_fSequenceFinished) {
			TaskComplete();
			SetTouch(nullptr);
			m_IdealActivity = ACT_IDLE;
		}
		break;
	default:
		CBaseMonster::RunTask(task);
		break;
	}
}

void CEinarChicken::LeapTouch(CBaseEntity* other) {
	if (!other->pev->takedamage || other->Classify() == Classify()) {
		return;
	}

	if (!FBitSet(pev->flags, FL_ONGROUND)) {
		BiteSound();
		other->TakeDamage(pev, pev, GetDamageAmount(), DMG_SLASH);
	}

	SetTouch(nullptr);
}

int CEinarChicken::TakeDamage(entvars_t* inflictor, entvars_t* attacker, float damage, int damageTypes) {
	if (damageTypes & DMG_ACID) {
		damage = 0;
	}

	return CBaseMonster::TakeDamage(inflictor, attacker, damage, damageTypes);
}

void CEinarChicken::AlertSound() {
	EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CEinarChicken::IdleSound() {
	EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CEinarChicken::BiteSound() {
	if (!RANDOM_LONG(0, 9)) {
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, "chicken/ch_alert1.wav", GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
	}

	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "thehand/hnd_headbite.wav", GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CEinarChicken::PainSound() {
	EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CEinarChicken::DeathSound() {
	EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CEinarChicken::StartAttackSound() {
	if (!RANDOM_LONG(0, 9)) {
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
	}
}

void CEinarChicken::AttackSound() {
	if (!RANDOM_LONG(0, 1)) {
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
	}

	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "thehand/hnd_attack1.wav", GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}
