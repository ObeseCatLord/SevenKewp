#include "CWeaponSupport.h"
#include "../../extdll.h"
#include "../../player/CBasePlayer.h"
#include "../../weapon/weapons.h"

enum { CG_IDLE, CG_IDLE2, CG_SPINUP, CG_SPINDOWN, CG_FIRE, CG_DRAW, CG_HOLSTER };
LINK_ENTITY_TO_CLASS(weapon_th_chaingun, CWeaponTheyHungerChaingun)
void CWeaponTheyHungerChaingun::Spawn() { Precache(); m_iId = TH_WEAPON_CHAINGUN; m_defaultModelV="models/v_tfac.mdl"; m_defaultModelP="models/p_tfac.mdl"; m_defaultModelW="models/w_tfac.mdl"; m_hasHandModels=true; SetWeaponModelW(); m_iDefaultAmmo=100; m_spinning=0; FallInit(); }
void CWeaponTheyHungerChaingun::Precache() { CBasePlayerWeapon::Precache(); PRECACHE_MODEL("models/shell.mdl"); PRECACHE_SOUND("items/9mmclip1.wav"); PRECACHE_SOUND("weapons/reload3.wav"); for (int i=1;i<=4;i++) PRECACHE_SOUND(UTIL_VarArgs("weapons/asscan%d.wav",i)); m_spinEvent=PRECACHE_EVENT(1,"events/chaingun1.sc"); m_fireEvent=PRECACHE_EVENT(1,"events/chaingun2.sc"); }
int CWeaponTheyHungerChaingun::GetItemInfo(ItemInfo* p) { p->pszName=STRING(pev->classname); p->pszAmmo1="9mm"; p->pszAmmo2=NULL; p->iMaxClip=100; p->iSlot=3; p->iPosition=4; p->iFlags=0; p->iId=m_iId=TH_WEAPON_CHAINGUN; p->iWeight=15; p->fAccuracyDeg=8; return 1; }
BOOL CWeaponTheyHungerChaingun::Deploy() { return DefaultDeploy(GetModelV(),GetModelP(),CG_DRAW,"egon"); }
void CWeaponTheyHungerChaingun::Holster(int) { SpinDown(); SendWeaponAnim(CG_HOLSTER); GetPlayer()->m_flNextAttack=UTIL_WeaponTimeBase()+.5f; }
void CWeaponTheyHungerChaingun::PrimaryAttack() { if (!m_spinning) { m_spinning=1; SendWeaponAnim(CG_SPINUP); m_flNextPrimaryAttack=m_flNextSecondaryAttack=UTIL_WeaponTimeBase()+.4f; return; } if (m_flNextPrimaryAttack<=UTIL_WeaponTimeBase()) Fire(); }
void CWeaponTheyHungerChaingun::SecondaryAttack() { SpinDown(); }
void CWeaponTheyHungerChaingun::SpinDown() { if (!m_spinning) return; m_spinning=0; SendWeaponAnim(CG_SPINDOWN); m_flNextPrimaryAttack=m_flNextSecondaryAttack=UTIL_WeaponTimeBase()+1.2f; }
void CWeaponTheyHungerChaingun::Fire() { CBasePlayer* p=GetPlayer(); if (!p || p->pev->waterlevel==3 || !m_iClip) { SpinDown(); if (m_fFireOnEmpty) PlayEmptySound(); return; } int shots=m_iClip>1?4:2; m_iClip-=m_iClip>1?2:1; p->m_iWeaponVolume=LOUD_GUN_VOLUME; p->m_iWeaponFlash=NORMAL_GUN_FLASH; p->pev->effects|=EF_MUZZLEFLASH; p->SetAnimation(PLAYER_ATTACK1); UTIL_MakeVectors(p->GetWeaponViewAngles()); lagcomp_begin(p); Vector d=p->FireBulletsPlayer(shots,p->GetWeaponPosition(),gpGlobals->v_forward,Vector(.15,.15,.15),8192,BULLET_PLAYER_MP5,0,0,p->pev,p->random_seed,0,BULLETPRED_EVENT); lagcomp_end(); int flags=0;
#if defined(CLIENT_WEAPONS)
	flags=FEV_NOTHOST;
#endif
	PLAYBACK_EVENT_FULL(flags,p->edict(),m_fireEvent,0,(float*)&g_vecZero,(float*)&g_vecZero,d.x,d.y,0,0,m_iClip>0,0); m_flNextPrimaryAttack=UTIL_WeaponTimeBase()+.1f; m_flTimeWeaponIdle=m_flNextSecondaryAttack=UTIL_WeaponTimeBase()+.1f; }
void CWeaponTheyHungerChaingun::Reload() { if (m_spinning) SpinDown(); else DefaultReload(100,CG_HOLSTER,.5f); }
void CWeaponTheyHungerChaingun::WeaponIdle() { ResetEmptySound(); if (m_spinning && m_flTimeWeaponIdle<=UTIL_WeaponTimeBase()) SpinDown(); else if (m_flTimeWeaponIdle<=UTIL_WeaponTimeBase()) { SendWeaponAnim(CG_IDLE); m_flTimeWeaponIdle=UTIL_WeaponTimeBase()+20; } }

enum { MK_IDLE, MK_LONGIDLE, MK_LONGUSE, MK_SHORTUSE, MK_HOLSTER, MK_DRAW };
LINK_ENTITY_TO_CLASS(weapon_th_medkit, CWeaponTheyHungerMedkit)
void CWeaponTheyHungerMedkit::Spawn() { Precache(); m_iId=TH_WEAPON_MEDKIT; m_defaultModelV="models/v_tfc_medkit.mdl"; m_defaultModelP="models/p_tfc_medkit.mdl"; m_defaultModelW="models/w_tfc_medkit.mdl"; m_hasHandModels=true; SetWeaponModelW(); m_iDefaultAmmo=50; m_using=0; FallInit(); }
void CWeaponTheyHungerMedkit::Precache() { CBasePlayerWeapon::Precache(); PRECACHE_SOUND("items/9mmclip1.wav"); PRECACHE_SOUND("items/medshot4.wav"); PRECACHE_SOUND("items/medshotno1.wav"); }
int CWeaponTheyHungerMedkit::GetItemInfo(ItemInfo* p) { p->pszName=STRING(pev->classname); p->pszAmmo1="health"; p->pszAmmo2=NULL; p->iMaxClip=WEAPON_NOCLIP; p->iSlot=4; p->iPosition=4; p->iFlags=ITEM_FLAG_LIMITINWORLD|ITEM_FLAG_EXHAUSTIBLE; p->iId=m_iId=TH_WEAPON_MEDKIT; p->iWeight=0; return 1; }
BOOL CWeaponTheyHungerMedkit::Deploy() { return DefaultDeploy(GetModelV(),GetModelP(),MK_DRAW,"trip"); }
void CWeaponTheyHungerMedkit::Holster(int) { m_using=0; SendWeaponAnim(MK_HOLSTER); GetPlayer()->m_flNextAttack=UTIL_WeaponTimeBase()+.5f; }
void CWeaponTheyHungerMedkit::PrimaryAttack() { CBasePlayer* p=GetPlayer(); if (!p || p->pev->health>=p->pev->max_health || p->m_rgAmmo[m_iPrimaryAmmoType]<=0) { if(m_fFireOnEmpty) PlayEmptySound(); return; } if(m_using) Heal(); else { m_using=1; SendWeaponAnim(MK_LONGUSE); m_flNextPrimaryAttack=m_flNextSecondaryAttack=m_flTimeWeaponIdle=UTIL_WeaponTimeBase()+1.2f; } }
void CWeaponTheyHungerMedkit::SecondaryAttack() { if(m_using) Heal(); }
void CWeaponTheyHungerMedkit::Heal() { CBasePlayer* p=GetPlayer(); if(!p) return; --p->m_rgAmmo[m_iPrimaryAmmoType]; p->TakeHealth(25,DMG_GENERIC); EMIT_SOUND(p->edict(),CHAN_WEAPON,"items/medshot4.wav",1,ATTN_NORM); m_using=0; m_flNextPrimaryAttack=m_flNextSecondaryAttack=m_flTimeWeaponIdle=UTIL_WeaponTimeBase()+1.2f; }
void CWeaponTheyHungerMedkit::Reload() { SecondaryAttack(); }
void CWeaponTheyHungerMedkit::WeaponIdle() { ResetEmptySound(); if(m_using) Heal(); else if(m_flTimeWeaponIdle<=UTIL_WeaponTimeBase()) { SendWeaponAnim(MK_IDLE); m_flTimeWeaponIdle=UTIL_WeaponTimeBase()+20; } }
BOOL CWeaponTheyHungerMedkit::PlayEmptySound() { if(m_iPlayEmptySound) { EMIT_SOUND(GetPlayer()->edict(),CHAN_WEAPON,"items/medshotno1.wav",.8,ATTN_NORM); m_iPlayEmptySound=0; } return FALSE; }
