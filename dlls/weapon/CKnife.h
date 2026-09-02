#pragma once
#include "CBasePlayerWeapon.h"

class CKnife : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void ItemPostFrame( void );
	int Swing( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	TraceResult m_trHit;

	virtual const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }
	virtual int MergedModelBody() { return MERGE_MDL_W_KNIFE; }

	BOOL IsClientWeapon() { return FALSE; }

	BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

#ifndef CLIENT_DLL
private:
	void CheckPhysicalSwing( float speed );
	bool HasHitEntityThisSwing( CBaseEntity* entity );
	void RememberHitEntityThisSwing( CBaseEntity* entity );
	void ClearPhysicalSwingState();
	bool m_physicalSwinging;
	float m_physicalHitCount;
	EHANDLE m_physicalHitEntities[32];
#endif
};
