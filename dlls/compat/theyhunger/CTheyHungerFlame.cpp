#include "../../extdll.h"
#include "../../game/gamerules.h"
#include "../../weapon/weapons.h"
#include "CTheyHungerFlame.h"

LINK_ENTITY_TO_CLASS(einar_flame, CEinarFlameRocket)

CEinarFlameRocket* CEinarFlameRocket::FlameCreate(const Vector& origin, const Vector& angles, edict_t* owner) {
	CEinarFlameRocket* flame = GetClassPtr((CEinarFlameRocket*)nullptr);
	UTIL_SetOrigin(flame->pev, origin);
	flame->pev->angles = angles;
	flame->pev->owner = owner;
	flame->Spawn();
	return flame;
}

void CEinarFlameRocket::Spawn() {
	Precache();

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;
	pev->effects |= EF_DIMLIGHT;

	SET_MODEL(edict(), "sprites/fthrow.spr");
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("einar_flame");
	pev->speed = g_pGameRules->IsMultiplayer() ? 500 : 400;
	SetTouch(&CEinarFlameRocket::FlameTouch);
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 1.1f;

	UTIL_MakeVectors(pev->angles);
	pev->velocity = gpGlobals->v_forward * pev->speed;
	pev->gravity = 0;

	int ownerIndex = 0;
	if (pev->owner && (pev->owner->v.flags & FL_CLIENT)) {
		ownerIndex = ENTINDEX(pev->owner);
	}

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_PROJECTILE);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(pev->velocity.x);
	WRITE_COORD(pev->velocity.y);
	WRITE_COORD(pev->velocity.z);
	WRITE_SHORT(pev->modelindex);
	WRITE_BYTE(1);
	WRITE_BYTE(ownerIndex);
	MESSAGE_END();
}

void CEinarFlameRocket::FlameTouch(CBaseEntity* other) {
	if (pev->watertype != CONTENTS_WATER && other->pev->takedamage) {
		TraceResult trace = UTIL_GetGlobalTrace();
		entvars_t* owner = pev->owner ? &pev->owner->v : nullptr;

		pev->dmg = g_pGameRules->IsMultiplayer() ? gSkillData.sk_plr_egon_wide : gSkillData.sk_plr_egon_narrow * 1.5f;
		pev->dmg *= 2.0f;

		ClearMultiDamage();
		other->TraceAttack(owner, pev->dmg, pev->velocity.Normalize(), &trace, DMG_BURN);
		ApplyMultiDamage(owner, owner);
	}

	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->velocity = g_vecZero;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove(this);
}

void CEinarFlameRocket::Precache() {
	PRECACHE_MODEL("sprites/fthrow.spr");
}
