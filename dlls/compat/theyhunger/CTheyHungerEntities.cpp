#include "../../extdll.h"
#include "../../weapon/weapons.h"
#include "../../monster/CAGrunt.h"
#include "../../monster/CBullsquid.h"

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
