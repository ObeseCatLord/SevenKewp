#include "../../extdll.h"
#include "../../weapon/weapons.h"
#include "../../monster/CApache.h"

class CEinarBoss : public CApache {
public:
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;
	BOOL FireGun() override;

private:
	int m_shots = 0;
};

TYPEDESCRIPTION CEinarBoss::m_SaveData[] = {
	DEFINE_FIELD(CEinarBoss, m_shots, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEinarBoss, CApache)

LINK_ENTITY_TO_CLASS(monster_th_boss, CEinarBoss)
LINK_ENTITY_TO_CLASS(monster_boss, CEinarBoss)

void CEinarBoss::Spawn() {
	if (!pev->model) {
		pev->model = MAKE_STRING("models/boss.mdl");
	}
	if (!pev->health) {
		pev->health = gSkillData.sk_apache_health * 2.0f;
	}

	CApache::Spawn();
}

void CEinarBoss::Precache() {
	if (!pev->model) {
		pev->model = MAKE_STRING("models/boss.mdl");
	}

	CApache::Precache();
}

BOOL CEinarBoss::FireGun() {
	UTIL_MakeAimVectors(pev->angles);

	Vector posGun;
	Vector angGun;
	GetAttachment(1, posGun, angGun);

	Vector target = (m_posTarget - posGun).Normalize();
	Vector out;
	out.x = DotProduct(-gpGlobals->v_right, target);
	out.y = -DotProduct(gpGlobals->v_forward, target);
	out.z = DotProduct(gpGlobals->v_up, target);

	Vector angles = UTIL_VecToAngles(out);
	angles.x = -angles.x;
	if (angles.y > 180) angles.y -= 360;
	if (angles.y < -180) angles.y += 360;
	if (angles.x > 180) angles.x -= 360;
	if (angles.x < -180) angles.x += 360;

	if (angles.x > m_angGun.x) m_angGun.x = V_min(angles.x, m_angGun.x + 12);
	if (angles.x < m_angGun.x) m_angGun.x = V_max(angles.x, m_angGun.x - 12);
	if (angles.y > m_angGun.y) m_angGun.y = V_min(angles.y, m_angGun.y + 12);
	if (angles.y < m_angGun.y) m_angGun.y = V_max(angles.y, m_angGun.y - 12);

	m_angGun.y = SetBoneController(0, m_angGun.y);
	m_angGun.x = SetBoneController(1, m_angGun.x);

	Vector posBarrel;
	Vector angBarrel;
	GetAttachment(0, posBarrel, angBarrel);
	Vector gun = (posBarrel - posGun).Normalize();

	if (DotProduct(gun, target) > 0.98f && !(++m_shots % 3)) {
		FireBullets(1, posGun, gun, VECTOR_CONE_4DEGREES, 8192, BULLET_MONSTER_9MM, 1);
		EMIT_SOUND(edict(), CHAN_WEAPON, "turret/tu_fire1.wav", 1, 0.3f);
		return TRUE;
	}

	if (m_shots % 3) {
		++m_shots;
	}
	else {
		pev->sequence = 0;
	}

	return FALSE;
}
