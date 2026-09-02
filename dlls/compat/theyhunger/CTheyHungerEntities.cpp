#include "../../extdll.h"
#include "../../weapon/weapons.h"
#include "../../monster/CAGrunt.h"
#include "../../monster/CApache.h"
#include "../../monster/CBullsquid.h"
#include "../../monster/CHeadCrab.h"
#include "../../monster/CISlave.h"
#include "../../monster/CScientist.h"

// These are intentionally small derived entities: the stock SevenKewp AI is
// shared, while They Hunger supplies the model, hull, faction, and tuning.
class CEinarCivilian : public CScientist {
public:
	void Spawn() override {
		if (pev->body == -1) pev->body = RANDOM_LONG(0, 4);
		m_defaultModel = "models/civ.mdl";
		CScientist::Spawn();
		SetUse(&CEinarCivilian::FollowerUse);
	}
	void Precache() override { m_defaultModel = "models/civ.mdl"; CScientist::Precache(); }
};
LINK_ENTITY_TO_CLASS(einar_civ, CEinarCivilian)

class CEinarDeadCivilian : public CDeadScientist {
public:
	void Spawn() override {
		m_defaultModel = "models/civ.mdl";
		if (pev->body == -1) pev->body = RANDOM_LONG(0, 4);
		CDeadScientist::Spawn();
	}
};
LINK_ENTITY_TO_CLASS(einar_civ_dead, CEinarDeadCivilian)

class CEinarSittingCivilian : public CSittingScientist {
public:
	void Spawn() override {
		m_defaultModel = "models/civ.mdl";
		if (pev->body == -1) pev->body = RANDOM_LONG(0, 4);
		CSittingScientist::Spawn();
	}
};
LINK_ENTITY_TO_CLASS(einar_civ_sit, CEinarSittingCivilian)

class CEinarHand : public CHeadCrab {
public:
	int Classify() override { return CLASS_ALIEN_MONSTER; }
	float GetDamageAmount() override { return gSkillData.sk_headcrab_dmg_bite * 2.0f; }
	void Spawn() override {
		Precache();
		SET_MODEL(edict(), "models/thehand.mdl");
		SetSize(Vector(-12, -12, 0), Vector(12, 12, 24));
		pev->solid = SOLID_SLIDEBOX;
		pev->movetype = MOVETYPE_STEP;
		SetBloodColor(BloodColorHuman());
		pev->health = gSkillData.sk_headcrab_health * 2.0f;
		pev->view_ofs = Vector(0, 0, 20);
		m_flFieldOfView = 0.5f;
		m_MonsterState = MONSTERSTATE_NONE;
		MonsterInit();
	}
	void Precache() override {
		m_defaultModel = "models/thehand.mdl";
		CHeadCrab::Precache();
		PRECACHE_SOUND("thehand/hnd_attack1.wav");
		PRECACHE_SOUND("thehand/hnd_headbite.wav");
	}
	void AttackSound() { EMIT_SOUND(edict(), CHAN_VOICE, "thehand/hnd_attack1.wav", 1, ATTN_IDLE); }
	void BiteSound() { EMIT_SOUND(edict(), CHAN_WEAPON, "thehand/hnd_headbite.wav", 1, ATTN_IDLE); }
	void PainSound() override {}
	void DeathSound() override {}
	void AlertSound() override {}
	void IdleSound() override {}
	void StartAttackSound() {}
};
LINK_ENTITY_TO_CLASS(einar_hand, CEinarHand)

class CEinarFlameRocket : public CBaseEntity {
public:
	void Spawn() override {
		Precache();
		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_BBOX;
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0;
		pev->effects |= EF_DIMLIGHT;
		SET_MODEL(edict(), "sprites/fthrow.spr");
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
		pev->classname = MAKE_STRING("einar_flame");
		pev->speed = g_pGameRules->IsMultiplayer() ? 500 : 400;
		SetTouch(&CEinarFlameRocket::FlameTouch);
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 1.1f;
		UTIL_MakeVectors(pev->angles);
		pev->velocity = gpGlobals->v_forward * pev->speed;
	}
	void Precache() override { PRECACHE_MODEL("sprites/fthrow.spr"); }
	void FlameTouch(CBaseEntity* other) {
		if (pev->watertype != CONTENTS_WATER && other->pev->takedamage) {
			TraceResult tr = UTIL_GetGlobalTrace();
			entvars_t* owner = pev->owner ? &pev->owner->v : pev;
			float damage = g_pGameRules->IsMultiplayer() ? gSkillData.sk_plr_egon_wide : gSkillData.sk_plr_egon_narrow * 1.5f;
			ClearMultiDamage(); other->TraceAttack(owner, damage * 2, pev->velocity.Normalize(), &tr, DMG_BURN); ApplyMultiDamage(owner, owner);
		}
		UTIL_Remove(this);
	}
};
LINK_ENTITY_TO_CLASS(einar_flame, CEinarFlameRocket)

class CEinarBabyKelly : public CISlave {
public:
	void Precache() override { CISlave::Precache(); m_defaultModel = "models/babykelly.mdl"; PRECACHE_MODEL(m_defaultModel); }
	void Spawn() override { CBaseMonster::Spawn(); Precache(); SET_MODEL(edict(), pev->model ? STRING(pev->model) : "models/babykelly.mdl"); SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX); pev->solid = SOLID_SLIDEBOX; pev->movetype = MOVETYPE_STEP; SetBloodColor(DONT_BLEED); pev->health = gSkillData.sk_islave_health; pev->view_ofs = Vector(0,0,32); m_flFieldOfView = VIEW_FIELD_WIDE; m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_RANGE_ATTACK2 | bits_CAP_DOORS_GROUP; MonsterInit(); }
};
LINK_ENTITY_TO_CLASS(monster_th_babykelly, CEinarBabyKelly)
LINK_ENTITY_TO_CLASS(monster_babykelly, CEinarBabyKelly)

class CEinarBoss : public CBaseMonster {
public:
	int Classify() override { return CLASS_MACHINE; }
	void Spawn() override { Precache(); SET_MODEL(edict(), "models/boss.mdl"); UTIL_SetSize(pev, Vector(-32, -32, -64), Vector(32, 32, 0)); pev->movetype = MOVETYPE_FLY; pev->solid = SOLID_SLIDEBOX; pev->health = pev->health ? pev->health : gSkillData.sk_apache_health * 2; MonsterInit(); }
	void Precache() override { PRECACHE_MODEL("models/boss.mdl"); }
};
LINK_ENTITY_TO_CLASS(monster_th_boss, CEinarBoss)
LINK_ENTITY_TO_CLASS(monster_boss, CEinarBoss)

class CEinarChicken : public CHeadCrab {
public:
	int Classify() override { return CLASS_ALIEN_PREY; }
	void Precache() override { m_defaultModel = "models/chicken.mdl"; CHeadCrab::Precache(); }
	void Spawn() override { CHeadCrab::Spawn(); SET_MODEL(edict(), "models/chicken.mdl"); }
};
LINK_ENTITY_TO_CLASS(monster_th_chicken, CEinarChicken)
LINK_ENTITY_TO_CLASS(monster_chicken, CEinarChicken)

class CEinarCyberFranklin : public CAGrunt {
public:
	void Precache() override { CAGrunt::Precache(); m_defaultModel = "models/cyberfranklin.mdl"; PRECACHE_MODEL(m_defaultModel); }
};
LINK_ENTITY_TO_CLASS(monster_th_cyberfranklin, CEinarCyberFranklin)
LINK_ENTITY_TO_CLASS(monster_cyberfranklin, CEinarCyberFranklin)

class CEinarMegasquid : public CBullsquid {
public:
	int Classify() override { return CLASS_ALIEN_MONSTER; }
	void Precache() override { CBullsquid::Precache(); m_defaultModel = "models/megasquid.mdl"; PRECACHE_MODEL(m_defaultModel); }
};
LINK_ENTITY_TO_CLASS(monster_th_megasquid, CEinarMegasquid)
LINK_ENTITY_TO_CLASS(monster_megasquid, CEinarMegasquid)

class CEinarZombieBull : public CBullsquid {
public:
	int Classify() override { return CLASS_ALIEN_MONSTER; }
	void Precache() override { CBullsquid::Precache(); m_defaultModel = "models/zombiebull.mdl"; PRECACHE_MODEL(m_defaultModel); }
};
LINK_ENTITY_TO_CLASS(monster_th_zombiebull, CEinarZombieBull)
LINK_ENTITY_TO_CLASS(monster_zombiebull, CEinarZombieBull)
