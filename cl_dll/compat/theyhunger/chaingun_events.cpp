#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "event_api.h"
#include "in_defs.h"
#include "pm_defs.h"
#include "pm_materials.h"
#include "pm_shared.h"
#include "weapons/ev_hldm.h"
#include "weapons/eventscripts.h"

void V_PunchAxis(int axis, float punch);
void EV_GetWeaponEventPose(event_args_t* args, float* origin, float* angles, float* velocity);

namespace
{
enum chaingun_e
{
	CHAINGUN_IDLE = 0,
	CHAINGUN_IDLE2,
	CHAINGUN_SPINUP,
	CHAINGUN_SPINDOWN,
	CHAINGUN_FIRE,
	CHAINGUN_DRAW,
	CHAINGUN_HOLSTER
};

unsigned int g_iEjectBrass;
int g_tracerCount[32];
}

extern "C" void EV_FireChaingun1(event_args_t* args)
{
	const int index = args->entindex;
	vec3_t origin;
	EV_GetWeaponEventPose(args, origin, NULL, NULL);

	if (args->bparam1)
	{
		if (EV_IsLocal(index))
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CHAINGUN_SPINUP, 0);
		gEngfuncs.pEventAPI->EV_PlaySound(index, origin, CHAN_WEAPON,
			RemapFile("weapons/asscan1.wav"), 1, ATTN_NORM, 0, PITCH_NORM);
	}
	else
	{
		gEngfuncs.pEventAPI->EV_KillEvents(index, "events/chaingun2.sc");
		gEngfuncs.pEventAPI->EV_StopSound(index, CHAN_WEAPON,
			RemapFile("weapons/asscan2.wav"));

		if (EV_IsLocal(index))
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CHAINGUN_SPINDOWN, 0);
		gEngfuncs.pEventAPI->EV_PlaySound(index, origin, CHAN_WEAPON,
			RemapFile("weapons/asscan3.wav"), 1, ATTN_NORM, 0, PITCH_NORM);
	}
}

extern "C" void EV_FireChaingun2(event_args_t* args)
{
	const int index = args->entindex;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t shellVelocity;
	vec3_t shellOrigin;
	vec3_t source;
	vec3_t aiming;
	vec3_t up;
	vec3_t right;
	vec3_t forward;

	EV_GetWeaponEventPose(args, origin, angles, velocity);
	AngleVectors(angles, forward, right, up);

	const int shell = gEngfuncs.pEventAPI->EV_FindModelIndex(RemapFile("models/shell.mdl"));
	if (EV_IsLocal(index))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(CHAINGUN_FIRE, 0);
		V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-0.5f, 0.5f));
		V_PunchAxis(1, gEngfuncs.pfnRandomFloat(-0.5f, 0.5f));
	}

	if ((g_iEjectBrass++) % 2)
	{
		EV_GetDefaultShellInfo(args, origin, velocity, shellVelocity, shellOrigin,
			forward, right, up, 20, -10, 5);
		EV_EjectBrass(shellOrigin, shellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);
	}

	gEngfuncs.pEventAPI->EV_PlaySound(index, origin, CHAN_WEAPON,
		RemapFile("weapons/asscan2.wav"), 1, ATTN_NORM, 0,
		94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, source, origin);
	VectorCopy(forward, aiming);
	const int bulletCount = args->bparam1 ? 4 : 2;
	EV_HLDM_FireBullets(index, forward, right, up, bulletCount, source, aiming,
		8192, BULLET_PLAYER_MP5, 0, &g_tracerCount[index - 1],
		args->fparam1, args->fparam2);
}
