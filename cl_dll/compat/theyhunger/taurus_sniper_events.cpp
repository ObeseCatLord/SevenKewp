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
#include "../../vr_client.h"
#include "../../game_shared/compat/theyhunger/taurus.h"
#include "../../game_shared/compat/theyhunger/sniper.h"
void V_PunchAxis(int axis, float punch);
namespace {
void Pose(event_args_t *a, vec3_t o, vec3_t an, vec3_t v) {
  VectorCopy(a->origin, o);
  VectorCopy(a->angles, an);
  VectorCopy(a->velocity, v);
  cl_entity_t *vm = gEngfuncs.GetViewModel();
  if (vm && VR_IsActive()) {
    VectorCopy(vm->curstate.origin, o);
    VectorCopy(vm->curstate.angles, an);
    an[PITCH] = -an[PITCH];
    VectorCopy(vm->curstate.velocity, v);
  }
}
void Haptic(int i, int bullet) {
  if (!EV_IsLocal(i))
    return;
  VR_Haptic(120, (unsigned)VR_GetDominantHand(),
            bullet == BULLET_PLAYER_357 ? .8f : .4f);
}
void Sniper(event_args_t *a, bool tfc) {
  int i = a->entindex;
  if (!i)
    return;
  vec3_t o, an, v, s, aim, u, r, f;
  Pose(a, o, an, v);
  AngleVectors(an, f, r, u);
  int scoped = a->bparam1;
  if (EV_IsLocal(i)) {
    EV_MuzzleFlash();
    gEngfuncs.pEventAPI->EV_WeaponAnimation(
        tfc ? (scoped ? TH_TFC_SNIPER_FIRE : TH_TFC_SNIPER_AUTOFIRE)
			: (int)TH_SNIPER_SHOOT,
        0);
    if (scoped)
      V_PunchAxis(0, -2);
    else {
      V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-2, -4));
      V_PunchAxis(1, gEngfuncs.pfnRandomFloat(-1, 1));
    }
  }
  gEngfuncs.pEventAPI->EV_PlaySound(
      i, o, CHAN_WEAPON, RemapFile("weapons/sniper.wav"),
      gEngfuncs.pfnRandomFloat(.8f, .9f), ATTN_NORM, 0, PITCH_NORM);
  EV_GetGunPosition(a, s, o);
  VectorCopy(f, aim);
  float spread = gEngfuncs.GetMaxClients() > 1 ? (scoped ? .002f : .07846f)
                                               : (scoped ? .004f : .08716f);
  EV_HLDM_FireBullets(i, f, r, u, 1, s, aim, 8192, BULLET_PLAYER_357, 0, NULL,
                      spread, spread);
  Haptic(i, BULLET_PLAYER_357);
}
} // namespace
extern "C" void EV_FireTaurus(event_args_t *a) {
  int i = a->entindex;
  if (!i)
    return;
  vec3_t o, an, v, s, aim, u, r, f;
  Pose(a, o, an, v);
  AngleVectors(an, f, r, u);
  if (EV_IsLocal(i)) {
    EV_MuzzleFlash();
    gEngfuncs.pEventAPI->EV_WeaponAnimation(
        a->bparam1 ? TH_TAURUS_SHOOT_EMPTY
                   : TH_TAURUS_SHOOT1 + gEngfuncs.pfnRandomLong(0, 2),
        1);
    V_PunchAxis(0, -3);
  }
  gEngfuncs.pEventAPI->EV_PlaySound(
      i, o, CHAN_WEAPON, RemapFile("weapons/tau_fire.wav"),
      gEngfuncs.pfnRandomFloat(.92f, 1), ATTN_NORM, 0,
      98 + gEngfuncs.pfnRandomLong(0, 3));
  EV_GetGunPosition(a, s, o);
  VectorCopy(f, aim);
  EV_HLDM_FireBullets(i, f, r, u, 1, s, aim, 8192, BULLET_PLAYER_9MM, 0, NULL,
                      .04f, .04f);
  Haptic(i, BULLET_PLAYER_9MM);
}
extern "C" void EV_FireSniper(event_args_t *a) { Sniper(a, false); }
extern "C" void EV_FireSniper2(event_args_t *a) { Sniper(a, true); }
