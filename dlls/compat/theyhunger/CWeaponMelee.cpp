#include "CWeaponMelee.h"
#include "../../extdll.h"
#include "../../weapon/weapons.h"

LINK_ENTITY_TO_CLASS(weapon_th_shovel, CWeaponTheyHungerShovel)
void CWeaponTheyHungerShovel::Spawn() { Precache(); m_iId=TH_WEAPON_SHOVEL; m_defaultModelV="models/v_shovel.mdl"; m_defaultModelP="models/p_shovel.mdl"; m_defaultModelW="models/w_shovel.mdl"; m_hasHandModels=true; SetWeaponModelW(); m_iClip=WEAPON_NOCLIP; FallInit(); }
void CWeaponTheyHungerShovel::Precache() { CBasePlayerWeapon::Precache(); }
int CWeaponTheyHungerShovel::GetItemInfo(ItemInfo* p) { p->pszName=STRING(pev->classname); p->pszAmmo1=NULL; p->pszAmmo2=NULL; p->iMaxClip=WEAPON_NOCLIP; p->iSlot=0; p->iPosition=1; p->iFlags=0; p->iId=m_iId=TH_WEAPON_SHOVEL; p->iWeight=0; return 1; }
BOOL CWeaponTheyHungerShovel::Deploy() { return DefaultDeploy(GetModelV(),GetModelP(),1,"crowbar"); }

LINK_ENTITY_TO_CLASS(weapon_th_spanner, CWeaponTheyHungerSpanner)
void CWeaponTheyHungerSpanner::Spawn() { Precache(); m_iId=TH_WEAPON_SPANNER; m_defaultModelV="models/v_tfc_spanner.mdl"; m_defaultModelP="models/p_spanner.mdl"; m_defaultModelW="models/backpack.mdl"; m_hasHandModels=true; SetWeaponModelW(); m_iClip=WEAPON_NOCLIP; FallInit(); }
void CWeaponTheyHungerSpanner::Precache() { CBasePlayerWeapon::Precache(); }
int CWeaponTheyHungerSpanner::GetItemInfo(ItemInfo* p) { p->pszName=STRING(pev->classname); p->pszAmmo1=NULL; p->pszAmmo2=NULL; p->iMaxClip=WEAPON_NOCLIP; p->iSlot=0; p->iPosition=2; p->iFlags=0; p->iId=m_iId=TH_WEAPON_SPANNER; p->iWeight=0; return 1; }
BOOL CWeaponTheyHungerSpanner::Deploy() { return DefaultDeploy(GetModelV(),GetModelP(),4,"crowbar"); }
