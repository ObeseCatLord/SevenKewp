#include "vr_client.h"

#include "../public/vr_usercmd_sidecar.h"

#include "hud.h"
#include "cl_util.h"
#include "ref_params.h"
#include "in_defs.h"
#include "cl_entity.h"
#include "const.h"
#include "entity_types.h"
#include "event_api.h"
#include "pm_defs.h"
#include "r_efx.h"
#include "Exports.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WEAPON_CROWBAR 1
#define WEAPON_CROSSBOW 6
#define WEAPON_RPG 7
#define WEAPON_HANDGRENADE 12
#define WEAPON_TRIPMINE 13
#define WEAPON_SATCHEL 14
#define WEAPON_SNARK 15
#define VR_POSE_SEND_INTERVAL 0.05f

static vr_client_frame_t g_vrFrame;
static vr_client_engine_api_t g_vrEngineAPI;
static long g_vrCurrentWeapon;
static int g_vrInitialised;
static int g_vrScopeButtonDown;
static int g_vrScopeEngaged;
static int g_vrScopeAnglesValid;
static Vector g_vrScopeAngles;
static float g_vrNextPoseSendTime;
static cl_entity_t g_vrTorchEntity;
static struct model_s *g_vrTorchModel;
static int g_vrTorchModelIndex;
static cl_entity_t g_vrHandEntities[VR_CLIENT_MAX_HANDS];
static struct model_s *g_vrHandModels[VR_CLIENT_MAX_HANDS];
static int g_vrHandModelIndices[VR_CLIENT_MAX_HANDS];
static BEAM *g_vrLaserBeam;
static TEMPENTITY *g_vrLaserDot;
static float g_vrLaserExpiry;

typedef struct vr_weapon_pose_s
{
	Vector weaponOffset;
	Vector weaponAngles;
	Vector weaponVelocity;
	Vector supportOffset;
	Vector supportAngles;
	int stabilised;
} vr_weapon_pose_t;

static vr_weapon_pose_t g_vrCachedWeaponPose;
static uint64_t g_vrCachedPoseFrameId;
static float g_vrCachedPoseYaw;
static int g_vrCachedWeaponPoseValid;

void VectorAngles( const float *forward, float *angles );

static float VR_Clamp( float value, float minimum, float maximum )
{
	return value < minimum ? minimum : ( value > maximum ? maximum : value );
}

static int VR_IsFiniteVector( const Vector &vector )
{
	return vector.x == vector.x && vector.x >= -FLT_MAX && vector.x <= FLT_MAX &&
		vector.y == vector.y && vector.y >= -FLT_MAX && vector.y <= FLT_MAX &&
		vector.z == vector.z && vector.z >= -FLT_MAX && vector.z <= FLT_MAX;
}

static void VR_ReleaseScopeButton()
{
	if( g_vrScopeButtonDown && gEngfuncs.pfnClientCmd )
		gEngfuncs.pfnClientCmd( "-alt1" );
	g_vrScopeButtonDown = 0;
}

static void VR_KillLaser()
{
	const float time = gEngfuncs.GetClientTime();
	if( time < g_vrLaserExpiry )
	{
		if( g_vrLaserBeam ) g_vrLaserBeam->die = 0.0f;
		if( g_vrLaserDot ) g_vrLaserDot->die = 0.0f;
	}
	g_vrLaserBeam = NULL;
	g_vrLaserDot = NULL;
	g_vrLaserExpiry = 0.0f;
}

static void VR_ClearFrame()
{
	memset( &g_vrFrame, 0, sizeof( g_vrFrame ) );
	g_vrNextPoseSendTime = 0.0f;
	g_vrScopeEngaged = 0;
	g_vrScopeAnglesValid = 0;
	g_vrCachedWeaponPoseValid = 0;
	CVAR_SET_FLOAT( "vr_weapon_stabilised", 0.0f );
	CVAR_SET_FLOAT( "vr_scope_engaged", 0.0f );
	VR_KillLaser();
}

static void VR_CLIENT_CALL VR_ClientFrame( const vr_client_frame_t *frame )
{
	if( !frame || frame->version != VR_CLIENT_FRAME_VERSION || frame->struct_size < sizeof( g_vrFrame ) )
	{
		VR_ClearFrame();
		return;
	}
	memcpy( &g_vrFrame, frame, sizeof( g_vrFrame ) );
	g_vrCachedWeaponPoseValid = 0;
}

static void VR_CLIENT_CALL VR_ClientShutdown()
{
	VR_ReleaseScopeButton();
	VR_ClearFrame();
	memset( &g_vrEngineAPI, 0, sizeof( g_vrEngineAPI ) );
}

static Vector VR_RotateYaw( const float vector[3], float bodyYaw )
{
	const float radians = bodyYaw * (float)M_PI / 180.0f;
	const float sine = sinf( radians );
	const float cosine = cosf( radians );
	return Vector( vector[0] * cosine - vector[1] * sine,
		vector[0] * sine + vector[1] * cosine, vector[2] );
}

static void VR_BasisAngles( const Vector &forward, const Vector &right, const Vector &up, Vector &angles )
{
	float pitch = -asinf( VR_Clamp( forward.z, -1.0f, 1.0f ) );
	float cpitch = cosf( pitch );
	float yaw, roll;
	if( fabsf( cpitch ) > 0.000001f )
	{
		cpitch = 1.0f / cpitch;
		pitch = pitch * 180.0f / (float)M_PI;
		yaw = atan2f( forward.y * cpitch, forward.x * cpitch ) * 180.0f / (float)M_PI;
		roll = atan2f( -right.z * cpitch, up.z * cpitch ) * 180.0f / (float)M_PI;
	}
	else
	{
		pitch = forward.z > 0.0f ? -90.0f : 90.0f;
		yaw = atan2f( right.x, -right.y ) * 180.0f / (float)M_PI;
		roll = 180.0f;
	}
	angles = Vector( pitch, yaw, roll );
}

static void VR_TransformPose( const vr_client_pose_t &pose, float bodyYaw, Vector &position, Vector &angles )
{
	const Vector forward = VR_RotateYaw( pose.forward, bodyYaw );
	const Vector right = VR_RotateYaw( pose.right, bodyYaw );
	const Vector up = VR_RotateYaw( pose.up, bodyYaw );
	position = VR_RotateYaw( pose.position, bodyYaw );
	VR_BasisAngles( forward, right, up, angles );
}

static float VR_AngleDelta( float value, float reference )
{
	float delta = value - reference;
	while( delta > 180.0f ) delta -= 360.0f;
	while( delta < -180.0f ) delta += 360.0f;
	return delta;
}

static Vector VR_LerpAngles( const Vector &from, const Vector &to, float fraction )
{
	return Vector( from.x + VR_AngleDelta( to.x, from.x ) * fraction,
		from.y + VR_AngleDelta( to.y, from.y ) * fraction,
		from.z + VR_AngleDelta( to.z, from.z ) * fraction );
}

static void VR_SendPoseUpdate( ref_params_s *params, const Vector &weaponPosition,
	const Vector &weaponAngles, const Vector &weaponVelocity, const Vector &offhandPosition,
	const Vector &offhandAngles )
{
	char command[256];
	int length;
	if( !params || !gEngfuncs.pfnClientCmd || params->time < g_vrNextPoseSendTime ||
		!VR_IsFiniteVector( weaponPosition ) || !VR_IsFiniteVector( weaponAngles ) ||
		!VR_IsFiniteVector( weaponVelocity ) || !VR_IsFiniteVector( offhandPosition ) ||
		!VR_IsFiniteVector( offhandAngles ) )
		return;
	length = snprintf( command, sizeof( command ),
		"updatevr %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f",
		weaponPosition.x, weaponPosition.y, weaponPosition.z, weaponAngles.x, weaponAngles.y, weaponAngles.z,
		weaponVelocity.x, weaponVelocity.y, weaponVelocity.z, offhandPosition.x, offhandPosition.y, offhandPosition.z,
		offhandAngles.x, offhandAngles.y, offhandAngles.z );
	if( length < 0 || length >= (int)sizeof( command ) ) return;
	gEngfuncs.pfnClientCmd( command );
	g_vrNextPoseSendTime = params->time + VR_POSE_SEND_INTERVAL;
}

static int VR_IsThrowable( long weapon )
{
	return weapon == WEAPON_HANDGRENADE || weapon == WEAPON_TRIPMINE || weapon == WEAPON_SATCHEL || weapon == WEAPON_SNARK;
}

static int VR_ParseCommandFloat( const char *text, float &value )
{
	char *end;
	double parsed;
	if( !text || !text[0] ) return 0;
	parsed = strtod( text, &end );
	if( end == text || *end || parsed != parsed || parsed < -FLT_MAX || parsed > FLT_MAX ) return 0;
	value = (float)parsed;
	return value == value;
}

static void VR_ServerHapticCommand()
{
	float duration, handValue, amplitude;
	int hand;
	if( gEngfuncs.Cmd_Argc() != 4 || !VR_ParseCommandFloat( gEngfuncs.Cmd_Argv( 1 ), duration ) ||
		!VR_ParseCommandFloat( gEngfuncs.Cmd_Argv( 2 ), handValue ) || !VR_ParseCommandFloat( gEngfuncs.Cmd_Argv( 3 ), amplitude ) ) return;
	hand = (int)handValue;
	if( handValue != (float)hand || hand < -2 || hand >= VR_CLIENT_MAX_HANDS ) return;
	if( hand == -1 ) hand = VR_GetDominantHand();
	else if( hand == -2 ) hand = VR_GetDominantHand() == VR_CLIENT_HAND_LEFT ? VR_CLIENT_HAND_RIGHT : VR_CLIENT_HAND_LEFT;
	VR_Haptic( VR_Clamp( duration, 0.0f, 2000.0f ), (unsigned int)hand, VR_Clamp( amplitude, 0.0f, 1.0f ) );
}

void VR_Init()
{
	if( g_vrInitialised ) return;
	CVAR_CREATE( "vr_weapon_pitchadjust", "-20", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_crowbar_pitchadjust", "-25", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_weapon_recoil", "0", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_weapon_stabilised", "0", 0 );
	CVAR_CREATE( "vr_scope_engaged", "0", 0 );
	CVAR_CREATE( "vr_scope_stabilise", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_lasersight", "0", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_flashlight_model", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_hand_model", "1", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_headtorch", "0", FCVAR_ARCHIVE );
	CVAR_CREATE( "vr_reversetorch", "0", FCVAR_ARCHIVE );
	CVAR_CREATE( "hand", "0", FCVAR_ARCHIVE );
	gEngfuncs.pfnAddCommand( "vr_haptic", VR_ServerHapticCommand );
	g_vrInitialised = 1;
}

void VR_Reset() { VR_ReleaseScopeButton(); VR_ClearFrame(); }
void VR_UpdateCurrentWeapon( long weapon ) { g_vrCurrentWeapon = weapon; }
int VR_IsActive() { return ( g_vrFrame.flags & VR_CLIENT_FRAME_ACTIVE ) != 0; }
int VR_GetDominantHand() { return CVAR_GET_FLOAT( "hand" ) >= 0.5f ? VR_CLIENT_HAND_LEFT : VR_CLIENT_HAND_RIGHT; }
int VR_IsWeaponStabilised() { return VR_IsActive() && CVAR_GET_FLOAT( "vr_weapon_stabilised" ) != 0.0f; }
int VR_ShouldApplyWeaponRecoil() { return !VR_IsActive() || CVAR_GET_FLOAT( "vr_weapon_recoil" ) != 0.0f; }
void VR_Haptic( float durationMilliseconds, unsigned int hand, float amplitude )
{
	if( VR_IsActive() && g_vrEngineAPI.Haptic && hand < VR_CLIENT_MAX_HANDS )
		g_vrEngineAPI.Haptic( hand, durationMilliseconds / 1000.0f, 0.0f, amplitude );
}

static void VR_UpdateScope( float bodyYaw, const Vector &weaponOffset, int stabilised )
{
	Vector headOffset, unusedAngles;
	if( g_vrCurrentWeapon != WEAPON_CROSSBOW )
	{
		VR_ReleaseScopeButton(); g_vrScopeEngaged = 0; g_vrScopeAnglesValid = 0; CVAR_SET_FLOAT( "vr_scope_engaged", 0.0f ); return;
	}
	if( g_vrFrame.flags & VR_CLIENT_FRAME_HEAD_VALID ) VR_TransformPose( g_vrFrame.head, bodyYaw, headOffset, unusedAngles );
	else headOffset = Vector( 0.0f, 0.0f, 0.0f );
	const float scale = g_vrFrame.world_scale > 0.0f ? g_vrFrame.world_scale : 40.0f;
	const int requested = stabilised && ( weaponOffset - headOffset ).Length() < scale * 0.28f;
	if( requested && !g_vrScopeButtonDown && gEngfuncs.pfnClientCmd ) { gEngfuncs.pfnClientCmd( "+alt1" ); g_vrScopeButtonDown = 1; }
	else if( !requested ) VR_ReleaseScopeButton();
	g_vrScopeEngaged = gHUD.m_iFOV > 0 && gHUD.m_iFOV < (int)CVAR_GET_FLOAT( "default_fov" );
	if( !g_vrScopeEngaged ) g_vrScopeAnglesValid = 0;
	CVAR_SET_FLOAT( "vr_scope_engaged", g_vrScopeEngaged ? 1.0f : 0.0f );
}

static int VR_ResolveWeaponPose( float bodyYaw, float frametime, vr_weapon_pose_t *pose )
{
	const int dominant = VR_GetDominantHand();
	const int offhand = dominant == VR_CLIENT_HAND_LEFT ? VR_CLIENT_HAND_RIGHT : VR_CLIENT_HAND_LEFT;
	const vr_client_hand_t *weapon = &g_vrFrame.hands[dominant];
	const vr_client_hand_t *support = &g_vrFrame.hands[offhand];
	const float scale = g_vrFrame.world_scale > 0.0f ? g_vrFrame.world_scale : 40.0f;

	if( !pose || !VR_IsActive() || !( weapon->flags & VR_CLIENT_HAND_AIM_VALID ) )
	{
		CVAR_SET_FLOAT( "vr_weapon_stabilised", 0.0f );
		VR_ReleaseScopeButton(); g_vrScopeEngaged = 0; g_vrScopeAnglesValid = 0; CVAR_SET_FLOAT( "vr_scope_engaged", 0.0f );
		return 0;
	}
	VR_TransformPose( weapon->aim, bodyYaw, pose->weaponOffset, pose->weaponAngles );
	pose->supportOffset = pose->weaponOffset; pose->supportAngles = pose->weaponAngles;
	if( support->flags & VR_CLIENT_HAND_AIM_VALID ) VR_TransformPose( support->aim, bodyYaw, pose->supportOffset, pose->supportAngles );
	const float separation = ( pose->supportOffset - pose->weaponOffset ).Length();
	pose->stabilised = ( support->flags & VR_CLIENT_HAND_AIM_VALID ) && support->squeeze >= 0.7f && separation > scale * 0.15f && separation < scale * 0.50f;
	CVAR_SET_FLOAT( "vr_weapon_stabilised", pose->stabilised ? 1.0f : 0.0f );
	if( pose->stabilised ) VectorAngles( pose->supportOffset - pose->weaponOffset, pose->weaponAngles );
	if( g_vrCurrentWeapon == WEAPON_CROWBAR ) pose->weaponAngles[PITCH] += CVAR_GET_FLOAT( "vr_crowbar_pitchadjust" );
	else if( !VR_IsThrowable( g_vrCurrentWeapon ) ) pose->weaponAngles[PITCH] += CVAR_GET_FLOAT( "vr_weapon_pitchadjust" );
	VR_UpdateScope( bodyYaw, pose->weaponOffset, pose->stabilised );
	if( g_vrScopeEngaged && CVAR_GET_FLOAT( "vr_scope_stabilise" ) != 0.0f )
	{
		if( g_vrScopeAnglesValid ) pose->weaponAngles = VR_LerpAngles( g_vrScopeAngles, pose->weaponAngles, VR_Clamp( frametime * 8.0f, 0.0f, 1.0f ) );
		g_vrScopeAngles = pose->weaponAngles; g_vrScopeAnglesValid = 1;
	}
	pose->weaponVelocity = VR_RotateYaw( weapon->linear_velocity, bodyYaw );
	return VR_IsFiniteVector( pose->weaponOffset ) && VR_IsFiniteVector( pose->weaponAngles ) && VR_IsFiniteVector( pose->weaponVelocity ) && VR_IsFiniteVector( pose->supportOffset ) && VR_IsFiniteVector( pose->supportAngles );
}

static int VR_GetWeaponPose( float bodyYaw, float frametime, vr_weapon_pose_t *pose )
{
	if( g_vrCachedWeaponPoseValid && g_vrCachedPoseFrameId == g_vrFrame.frame_id && g_vrCachedPoseYaw == bodyYaw )
	{
		*pose = g_vrCachedWeaponPose;
		return 1;
	}
	if( !VR_ResolveWeaponPose( bodyYaw, frametime, pose ) ) return 0;
	g_vrCachedWeaponPose = *pose; g_vrCachedPoseFrameId = g_vrFrame.frame_id; g_vrCachedPoseYaw = bodyYaw; g_vrCachedWeaponPoseValid = 1;
	return 1;
}

static void VR_AddPresentationModel( cl_entity_t *entity, struct model_s *model, int modelIndex,
	const Vector &origin, const Vector &angles, int glow )
{
	memset( entity, 0, sizeof( *entity ) );
	entity->model = model;
	entity->curstate.modelindex = modelIndex;
	entity->curstate.movetype = MOVETYPE_NONE;
	entity->curstate.rendermode = glow ? kRenderGlow : kRenderNormal;
	entity->curstate.renderfx = glow ? kRenderFxNoDissipation : kRenderFxNone;
	entity->curstate.renderamt = glow ? 192 : 0;
	VectorCopy( origin, entity->origin ); VectorCopy( origin, entity->curstate.origin );
	VectorCopy( angles, entity->angles ); VectorCopy( angles, entity->curstate.angles );
	gEngfuncs.CL_CreateVisibleEntity( ET_NORMAL, entity );
}

static void VR_AddHandModel( int hand, const Vector &origin, const Vector &angles )
{
	vec3_t forward;
	if( hand < 0 || hand >= VR_CLIENT_MAX_HANDS || !CVAR_GET_FLOAT( "vr_hand_model" ) || !gEngfuncs.CL_LoadModel || !gEngfuncs.CL_CreateVisibleEntity ) return;
	if( !g_vrHandModels[hand] ) g_vrHandModels[hand] = gEngfuncs.CL_LoadModel( "models/v_hand.mdl", &g_vrHandModelIndices[hand] );
	if( !g_vrHandModels[hand] ) return;
	gEngfuncs.pfnAngleVectors( angles, forward, NULL, NULL );
	VR_AddPresentationModel( &g_vrHandEntities[hand], g_vrHandModels[hand], g_vrHandModelIndices[hand], origin - Vector( forward ) * 5.0f, angles, 1 );
}

static void VR_AddTorchModel( const Vector &origin, const Vector &angles )
{
	if( !gHUD.m_Flash.HasFlashlight() || !CVAR_GET_FLOAT( "vr_flashlight_model" ) || !gEngfuncs.CL_LoadModel || !gEngfuncs.CL_CreateVisibleEntity ) return;
	if( !g_vrTorchModel ) g_vrTorchModel = gEngfuncs.CL_LoadModel( "models/v_torch.mdl", &g_vrTorchModelIndex );
	if( g_vrTorchModel ) VR_AddPresentationModel( &g_vrTorchEntity, g_vrTorchModel, g_vrTorchModelIndex, origin, angles, 0 );
}

static void VR_UpdateLaser( const Vector &origin, const Vector &angles )
{
	const int mode = (int)CVAR_GET_FLOAT( "vr_lasersight" );
	pmtrace_t trace;
	vec3_t forward, start, end;
	const float time = gEngfuncs.GetClientTime();
	if( mode < 1 || mode > 2 || g_vrCurrentWeapon == WEAPON_RPG || !gEngfuncs.pEventAPI || !gEngfuncs.pEfxAPI ) { VR_KillLaser(); return; }
	gEngfuncs.pfnAngleVectors( angles, forward, NULL, NULL );
	VectorCopy( origin, start ); VectorMA( start, mode == 1 ? 2048.0f : 8192.0f, forward, end );
	memset( &trace, 0, sizeof( trace ) ); trace.fraction = 1.0f; VectorCopy( end, trace.endpos );
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true ); gEngfuncs.pEventAPI->EV_PushPMStates();
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	if( player ) { gEngfuncs.pEventAPI->EV_SetSolidPlayers( player->index - 1 ); gEngfuncs.pEventAPI->EV_SetTraceHull( 2 ); gEngfuncs.pEventAPI->EV_PlayerTrace( start, end, PM_NORMAL, -1, &trace ); }
	gEngfuncs.pEventAPI->EV_PopPMStates();
	const int model = gEngfuncs.pEventAPI->EV_FindModelIndex( mode == 1 ? "sprites/laserbeam.spr" : "sprites/hlcoop/laserdot.spr" );
	if( model <= 0 ) { VR_KillLaser(); return; }
	if( mode == 1 ) { if( g_vrLaserDot ) g_vrLaserDot->die = 0.0f; g_vrLaserDot = NULL; if( !g_vrLaserBeam || time >= g_vrLaserExpiry ) g_vrLaserBeam = gEngfuncs.pEfxAPI->R_BeamPoints( origin, trace.endpos, model, 0.1f, 3.0f, 0.0f, 96.0f / 255.0f, 255.0f, 0, 0.0f, 214.0f / 255.0f, 34.0f / 255.0f, 34.0f / 255.0f ); else { g_vrLaserBeam->source = origin; g_vrLaserBeam->target = trace.endpos; g_vrLaserBeam->die = time + 0.1f; } }
	else { if( g_vrLaserBeam ) g_vrLaserBeam->die = 0.0f; g_vrLaserBeam = NULL; if( !g_vrLaserDot || time >= g_vrLaserExpiry ) g_vrLaserDot = gEngfuncs.pEfxAPI->R_TempSprite( trace.endpos, vec3_origin, 0.5f, model, kRenderGlow, kRenderFxNoDissipation, 1.0f, 0.1f, FTENT_SPRCYCLE | FTENT_PERSIST ); else { VectorCopy( trace.endpos, g_vrLaserDot->entity.origin ); g_vrLaserDot->die = time + 0.1f; } }
	g_vrLaserExpiry = time + 0.1f;
}

void VR_UpdateViewmodel( ref_params_s *params )
{
	if( !params || !VR_IsActive() ) { VR_ReleaseScopeButton(); CVAR_SET_FLOAT( "vr_scope_engaged", 0.0f ); VR_KillLaser(); return; }
	cl_entity_t *viewmodel = gEngfuncs.GetViewModel();
	const int dominant = VR_GetDominantHand(); const int offhand = dominant == VR_CLIENT_HAND_LEFT ? VR_CLIENT_HAND_RIGHT : VR_CLIENT_HAND_LEFT;
	const vr_client_hand_t *support = &g_vrFrame.hands[offhand];
	vr_weapon_pose_t pose;
	if( !viewmodel || !VR_GetWeaponPose( params->cl_viewangles[YAW], params->frametime, &pose ) ) { CVAR_SET_FLOAT( "vr_weapon_stabilised", 0.0f ); VR_ReleaseScopeButton(); VR_KillLaser(); return; }
	VR_SendPoseUpdate( params, Vector( params->vieworg ) + pose.weaponOffset, pose.weaponAngles, pose.weaponVelocity, Vector( params->vieworg ) + pose.supportOffset, pose.supportAngles );
	const Vector weaponOrigin = Vector( params->vieworg ) + pose.weaponOffset;
	VectorCopy( weaponOrigin, viewmodel->origin ); VectorCopy( weaponOrigin, viewmodel->curstate.origin ); VectorCopy( weaponOrigin, viewmodel->latched.prevorigin );
	VectorCopy( pose.weaponAngles, viewmodel->angles ); VectorCopy( pose.weaponAngles, viewmodel->curstate.angles ); VectorCopy( pose.weaponAngles, viewmodel->latched.prevangles ); VectorCopy( pose.weaponVelocity, viewmodel->curstate.velocity );
	VR_UpdateLaser( weaponOrigin, pose.weaponAngles );
	if( !g_vrScopeEngaged && !params->intermission && !params->spectator && !params->onlyClientDraw && params->health > 0 && params->viewentity == params->playernum + 1 && !gEngfuncs.IsSpectateOnly() )
	{
		const int hasFlashlight = gHUD.m_Flash.HasFlashlight();
		if( ( support->flags & VR_CLIENT_HAND_AIM_VALID ) && !hasFlashlight && !pose.stabilised ) VR_AddHandModel( offhand, Vector( params->vieworg ) + pose.supportOffset, pose.supportAngles );
		if( !viewmodel->model ) VR_AddHandModel( dominant, weaponOrigin, pose.weaponAngles );
		if( ( support->flags & VR_CLIENT_HAND_AIM_VALID ) && hasFlashlight && !pose.stabilised && CVAR_GET_FLOAT( "vr_headtorch" ) == 0.0f ) { if( CVAR_GET_FLOAT( "vr_reversetorch" ) != 0.0f ) { vec3_t forward; gEngfuncs.pfnAngleVectors( pose.supportAngles, forward, NULL, NULL ); VectorAngles( -Vector( forward ), pose.supportAngles ); } VR_AddTorchModel( Vector( params->vieworg ) + pose.supportOffset, pose.supportAngles ); }
	}
}

static int32_t VR_CLIENT_CALL VR_BuildUsercmdSidecar( const vr_client_usercmd_t *cmd, vr_usercmd_sidecar_t *sample )
{
	vr_weapon_pose_t pose;
	int index;
	if( !cmd || !sample || cmd->struct_size < VR_CLIENT_USERCMD_MIN_SIZE ) return 0;
	memset( sample, 0, sizeof( *sample ) );
	if( !VR_GetWeaponPose( cmd->viewangles[YAW], cmd->frametime, &pose ) ) return 0;
	for( index = 0; index < 3; ++index )
	{
		sample->weapon_position[index] = VR_UsercmdSidecarQuantize( pose.weaponOffset[index], 8.0f );
		sample->weapon_angles[index] = VR_UsercmdSidecarQuantize( pose.weaponAngles[index], 128.0f );
		sample->weapon_velocity[index] = VR_UsercmdSidecarQuantize( pose.weaponVelocity[index], 8.0f );
		sample->offhand_position[index] = VR_UsercmdSidecarQuantize( pose.supportOffset[index], 8.0f );
		sample->offhand_angles[index] = VR_UsercmdSidecarQuantize( pose.supportAngles[index], 128.0f );
	}
	sample->version = VR_USERCMD_SIDECAR_VERSION; sample->flags = VR_USERCMD_SIDECAR_POSE_VALID;
	return 1;
}

extern "C" int VR_CLIENT_CALL CL_DLLEXPORT HUD_GetVRClientAPI( const vr_client_engine_api_t *enginefuncs, vr_client_api_t *callbacks )
{
	uint32_t writableSize;
	if( !enginefuncs || !callbacks || enginefuncs->version != VR_CLIENT_API_VERSION || enginefuncs->struct_size < VR_CLIENT_ENGINE_API_MIN_SIZE ) return 0;
	writableSize = callbacks->struct_size ? callbacks->struct_size : VR_CLIENT_API_MIN_SIZE;
	if( writableSize < VR_CLIENT_API_MIN_SIZE ) return 0;
	if( writableSize > sizeof( *callbacks ) ) writableSize = sizeof( *callbacks );
	memset( &g_vrEngineAPI, 0, sizeof( g_vrEngineAPI ) ); memcpy( &g_vrEngineAPI, enginefuncs, sizeof( g_vrEngineAPI ) );
	memset( callbacks, 0, writableSize ); callbacks->version = VR_CLIENT_API_VERSION; callbacks->struct_size = writableSize; callbacks->Frame = VR_ClientFrame; callbacks->Shutdown = VR_ClientShutdown;
	if( writableSize >= VR_CLIENT_API_SIDECAR_SIZE ) callbacks->BuildUsercmdSidecar = VR_BuildUsercmdSidecar;
	return 1;
}
