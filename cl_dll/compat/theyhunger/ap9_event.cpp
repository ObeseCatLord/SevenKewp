#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "pm_defs.h"
#include "pm_materials.h"
#include "weapons/eventscripts.h"
#include "in_defs.h"
#include "pm_shared.h"
#include "event_api.h"
#include "weapons/ev_hldm.h"
#include "../../game_shared/compat/theyhunger/ap9.h"

void V_PunchAxis(int axis, float punch);

extern "C" void EV_FireAP9(event_args_t* args)
{
	const int index = args->entindex;
	if (!index)
		return;

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
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);
	AngleVectors(angles, forward, right, up);

	if (EV_IsLocal(index))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(TH_AP9_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2), 0);
		if (args->bparam1)
		{
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-3.5f, 2.0f));
			V_PunchAxis(1, gEngfuncs.pfnRandomFloat(-1.5f, 1.5f));
		}
		else
		{
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-3.5f, -1.5f));
			V_PunchAxis(1, gEngfuncs.pfnRandomFloat(-1.0f, 1.0f));
		}
	}

	const int shell = gEngfuncs.pEventAPI->EV_FindModelIndex(RemapFile("models/shell.mdl"));
	EV_GetDefaultShellInfo(args, origin, velocity, shellVelocity, shellOrigin, forward, right, up, 27, -10, 11);
	EV_EjectBrass(shellOrigin, shellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);
	gEngfuncs.pEventAPI->EV_PlaySound(index, origin, CHAN_WEAPON, RemapFile("weapons/ap9_fire.wav"), 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition(args, source, origin);
	VectorCopy(forward, aiming);
	const float spread = args->bparam1 ? 0.07f : 0.035f;
	const int bulletType = gEngfuncs.GetMaxClients() > 1 ? BULLET_PLAYER_9MM : BULLET_PLAYER_MP5;
	static int tracerCount[32];
	EV_HLDM_FireBullets(index, forward, right, up, 1, source, aiming, 8192, bulletType, 0,
		&tracerCount[index - 1], spread, spread);
}
