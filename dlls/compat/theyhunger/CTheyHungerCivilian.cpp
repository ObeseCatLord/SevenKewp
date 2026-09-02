#include "../../extdll.h"
#include "../../monster/CScientist.h"

namespace {
constexpr int NUM_CIVILIAN_HEADS = 5;
}

class CEinarCivilian : public CScientist {
public:
	void Spawn() override;
	void Precache() override;
};

LINK_ENTITY_TO_CLASS(einar_civ, CEinarCivilian)

void CEinarCivilian::Spawn() {
	if (pev->body == -1) {
		pev->body = RANDOM_LONG(0, NUM_CIVILIAN_HEADS - 1);
	}

	Precache();
	SET_MODEL(edict(), "models/civ.mdl");
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorHuman();
	if (!pev->health) {
		pev->health = gSkillData.sk_scientist_health;
	}
	pev->view_ofs = Vector(0, 0, 50);
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;

	MonsterInit();
	SetUse(&CEinarCivilian::FollowerUse);
}

void CEinarCivilian::Precache() {
	PRECACHE_MODEL("models/civ.mdl");
	CScientist::Precache();
}

class CEinarDeadCivilian : public CDeadScientist {
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(einar_civ_dead, CEinarDeadCivilian)

void CEinarDeadCivilian::Spawn() {
	PRECACHE_MODEL("models/civ.mdl");
	SET_MODEL(edict(), "models/civ.mdl");

	pev->effects = 0;
	pev->sequence = 0;
	pev->health = 8;
	m_bloodColor = BloodColorHuman();

	if (pev->body == -1) {
		pev->body = RANDOM_LONG(0, NUM_CIVILIAN_HEADS - 1);
	}

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);
	if (pev->sequence == -1) {
		ALERT(at_console, "Dead civilian with bad pose\n");
	}

	MonsterInitDead();
}

class CEinarSittingCivilian : public CSittingScientist {
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(einar_civ_sit, CEinarSittingCivilian)

void CEinarSittingCivilian::Spawn() {
	PRECACHE_MODEL("models/civ.mdl");
	SET_MODEL(edict(), "models/civ.mdl");
	Precache();
	InitBoneControllers();

	SetSize(Vector(-14, -14, 0), Vector(14, 14, 36));
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = 50;
	m_bloodColor = BloodColorHuman();
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD;
	SetBits(pev->spawnflags, SF_MONSTER_PREDISASTER);

	if (pev->body == -1) {
		pev->body = RANDOM_LONG(0, NUM_CIVILIAN_HEADS - 1);
	}

	m_baseSequence = LookupSequence("sitlookleft");
	pev->sequence = m_baseSequence + RANDOM_LONG(0, 4);
	ResetSequenceInfo();

	SetThink(&CEinarSittingCivilian::SittingThink);
	pev->nextthink = gpGlobals->time + 0.1f;
	DROP_TO_FLOOR(edict());
}
