#include "../../extdll.h"
#include "../../weapon/weapons.h"
#include "../../monster/CAGrunt.h"
#include "../../monster/CBullsquid.h"
#include "../../monster/CHeadCrab.h"
#include "../../monster/CISlave.h"

class CEinarBabyKelly : public CISlave {
public:
	void Precache() override { CISlave::Precache(); m_defaultModel = "models/babykelly.mdl"; PRECACHE_MODEL(m_defaultModel); }
	void Spawn() override { CBaseMonster::Spawn(); Precache(); SET_MODEL(edict(), pev->model ? STRING(pev->model) : "models/babykelly.mdl"); SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX); pev->solid = SOLID_SLIDEBOX; pev->movetype = MOVETYPE_STEP; SetBloodColor(DONT_BLEED); pev->health = gSkillData.sk_islave_health; pev->view_ofs = Vector(0,0,32); m_flFieldOfView = VIEW_FIELD_WIDE; m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_RANGE_ATTACK2 | bits_CAP_DOORS_GROUP; MonsterInit(); }
};
LINK_ENTITY_TO_CLASS(monster_th_babykelly, CEinarBabyKelly)
LINK_ENTITY_TO_CLASS(monster_babykelly, CEinarBabyKelly)

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
