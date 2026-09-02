#include "../../extdll.h"
#include "../../monster/CHeadCrab.h"

class CEinarHand : public CHeadCrab {
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override { return CLASS_ALIEN_MONSTER; }
	float GetDamageAmount() override { return gSkillData.sk_headcrab_dmg_bite * 2.0f; }

	void PainSound() override {}
	void DeathSound() override {}
	void IdleSound() override {}
	void AlertSound() override {}
	void StartAttackSound() {}
	void AttackSound();
	void BiteSound();

	static const char* pAttackSounds[];
	static const char* pBiteSounds[];
};

LINK_ENTITY_TO_CLASS(einar_hand, CEinarHand)

const char* CEinarHand::pAttackSounds[] = {
	"thehand/hnd_attack1.wav",
};

const char* CEinarHand::pBiteSounds[] = {
	"thehand/hnd_headbite.wav",
};

void CEinarHand::Spawn() {
	Precache();
	SET_MODEL(edict(), "models/thehand.mdl");
	SetSize(Vector(-12, -12, 0), Vector(12, 12, 24));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorHuman();
	pev->effects = 0;
	pev->health = gSkillData.sk_headcrab_health * 2.0f;
	pev->view_ofs = Vector(0, 0, 20);
	pev->yaw_speed = 5;
	m_flFieldOfView = 0.5f;
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
}

void CEinarHand::Precache() {
	PRECACHE_MODEL("models/thehand.mdl");
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pBiteSounds);
}

void CEinarHand::SetYawSpeed() {
	switch (m_Activity) {
	case ACT_IDLE:
		pev->yaw_speed = 60;
		break;
	case ACT_RUN:
	case ACT_WALK:
		pev->yaw_speed = 30;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		pev->yaw_speed = 120;
		break;
	case ACT_RANGE_ATTACK1:
	default:
		pev->yaw_speed = 60;
		break;
	}
}

void CEinarHand::AttackSound() {
	EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CEinarHand::BiteSound() {
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pBiteSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}
