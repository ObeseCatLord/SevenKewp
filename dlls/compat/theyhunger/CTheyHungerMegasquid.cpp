#include "../../extdll.h"
#include "../../util/util.h"
#include "../../monster/monsters.h"
#include "../../env/te_effects.h"
#include "CTheyHungerMegasquid.h"

extern int iSquidSpitSprite;

namespace {
constexpr int MEGASQUID_SPIT_SIZE = 2;
constexpr int MSQUID_AE_SPIT = 1;
constexpr int MSQUID_AE_BITE = 2;
constexpr int MSQUID_AE_THROW = 6;
}

LINK_ENTITY_TO_CLASS(monster_th_megasquid, CEinarMegasquid)

int CEinarMegasquid::Classify() {
	return CLASS_ALIEN_MONSTER;
}

void CEinarMegasquid::Spawn() {
	Precache();

	InitModel();
	SetSize(Vector(-160, -160, 0), Vector(160, 160, 256));
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorHuman();
	pev->effects = 0;
	pev->health = gSkillData.sk_bullsquid_health * 10;
	m_flFieldOfView = 0.2;
	m_MonsterState = MONSTERSTATE_NONE;
	SetBits(pev->spawnflags, SF_MONSTER_FADECORPSE);
	m_fCanThreatDisplay = FALSE;
	m_flNextSpitTime = gpGlobals->time;
	pev->view_ofs = Vector(0, 0, 128);

	MonsterInit();
	m_afCapability &= ~bits_CAP_MELEE_ATTACK1;
}

void CEinarMegasquid::Precache() {
	CBullsquid::Precache();
	m_defaultModel = "models/megasquid.mdl";
	PRECACHE_MODEL(GetModel());
}

BOOL CEinarMegasquid::CheckRangeAttack1(float dot, float distance) {
	if (distance > 450 && distance <= 1256 && dot >= 0.5f && gpGlobals->time >= m_flNextSpitTime) {
		if (m_hEnemy != nullptr && fabs(pev->origin.z - m_hEnemy->pev->origin.z) > 256) {
			return FALSE;
		}

		m_flNextSpitTime = gpGlobals->time + (IsMoving() ? 5.0f : 0.5f);
		return TRUE;
	}

	return FALSE;
}

BOOL CEinarMegasquid::CheckMeleeAttack2(float dot, float distance) {
	return distance <= 330 && dot >= 0.7f;
}

void CEinarMegasquid::HandleAnimEvent(MonsterEvent_t* event) {
	switch (event->event) {
	case MSQUID_AE_SPIT: {
		UTIL_MakeVectors(pev->angles);
		Vector spitOffset = pev->origin + gpGlobals->v_right * 8 + gpGlobals->v_forward * 37 + gpGlobals->v_up * 23;
		Vector spitDir = ((m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs) - spitOffset).Normalize();
		spitDir.x += RANDOM_FLOAT(-0.05, 0.05);
		spitDir.y += RANDOM_FLOAT(-0.05, 0.05);
		spitDir.z += RANDOM_FLOAT(-0.05, 0);

		AttackSound();
		UTIL_SpriteSpray(spitOffset, spitDir, iSquidSpitSprite, 15, 210, 25);
		CSquidSpit::Shoot(pev, spitOffset, spitDir * 900, MEGASQUID_SPIT_SIZE);
		break;
	}
	case MSQUID_AE_BITE: {
		CBaseEntity* hurt = CheckTraceHullAttack(450, gSkillData.sk_bullsquid_dmg_bite, DMG_SLASH);
		if (hurt) {
			hurt->pev->velocity = hurt->pev->velocity - gpGlobals->v_forward * 200;
			hurt->pev->velocity = hurt->pev->velocity + gpGlobals->v_up * 200;
		}
		break;
	}
	case MSQUID_AE_THROW: {
		CBaseEntity* hurt = CheckTraceHullAttack(450, 0, 0);
		if (hurt) {
			int pitch = RANDOM_FLOAT(90, 110);
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_LONG(0, 1) ? "bullchicken/bc_bite3.wav" : "bullchicken/bc_bite2.wav", 1, ATTN_NORM, 0, pitch);
			UTIL_ScreenShake(hurt->pev->origin, 25.0, 1.5, 0.7, 2);
			if (hurt->IsPlayer()) {
				UTIL_MakeVectors(pev->angles);
				hurt->pev->velocity = hurt->pev->velocity + gpGlobals->v_forward * 900 + gpGlobals->v_up * 900;
			}
		}
		break;
	}
	default:
		CBullsquid::HandleAnimEvent(event);
		break;
	}
}
