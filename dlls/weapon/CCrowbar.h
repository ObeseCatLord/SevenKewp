#pragma once
#include "CBasePlayerWeapon.h"

#define	CROWBAR_BODYHIT_VOLUME 128
#define	CROWBAR_WALLHIT_VOLUME 512

class EXPORT CCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	void PrecacheEvents();
	int iItemSlot( void ) { return 1; }
	void SwingAgain( void );
	void Smack( void );
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void ItemPostFrame( void );
	int Swing( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	TraceResult m_trHit;

	virtual int MergedModelBody() { return MERGE_MDL_W_CROWBAR; }

	virtual BOOL UseDecrement( void )
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

protected:
	/* Narrow behavior hooks for compatible physical melee weapons. */
	virtual bool ExclusivePhysicalVRPose() const { return false; }
	virtual float PhysicalSwingMinSpeed() const;
	virtual float PhysicalSwingReach() const;
	virtual size_t PhysicalSwingDedupeLimit() const;
	virtual bool UsesCrowbarSwingEvent() const { return true; }
	virtual int HolsterAnimation() const;
	virtual int DesktopMissAnimation();
	virtual int DesktopHitAnimation();
	virtual float DesktopMissDelay() const { return 0.5f; }
	virtual float DesktopHitDelay() const { return 0.25f; }
	virtual float DesktopDamage();
	virtual void SetDesktopSwingIdle() {}
	virtual Vector DesktopSwingOrigin(CBasePlayer* player) const;
	virtual bool IsPhysicalBodyHit(CBaseEntity* entity) const;
	virtual const char* PhysicalBodyHitSound(CBaseEntity* entity) const;
	virtual float PhysicalLivingBodyVolumeScale() const { return 0.25f; }
	virtual bool StopPhysicalFeedbackOnKill() const { return false; }
	void InitializePhysicalSwingState();
	void PrecacheSharedMeleeSounds();

	unsigned short m_usCrowbar;
#ifndef CLIENT_DLL
	void CheckPhysicalSwing( float speed );
	bool HasHitEntityThisSwing( CBaseEntity* entity );
	void RememberHitEntityThisSwing( CBaseEntity* entity );
	void ClearPhysicalSwingState();
	bool m_physicalSwinging;
	float m_physicalHitCount;
	float m_physicalLastWoosh;
	EHANDLE m_physicalHitEntities[128];
#endif
};
