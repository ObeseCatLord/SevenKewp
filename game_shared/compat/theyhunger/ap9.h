#pragma once

// They Hunger's original AP9 used ID 17. SevenKewp assigns 17 to the
// pipewrench, so the native compatibility weapon uses the first unassigned
// legacy-safe ID. IDs 1-22 are occupied, 23-30 are unassigned, and 31 is the
// suit. ID 23 remains inside the 32-bit legacy weapon bitfield.
#define TH_WEAPON_AP9 23

#define TH_AP9_AMMO_NAME "ap9"
#define TH_AP9_MAX_CARRY 200
#define TH_AP9_MAX_CLIP 40
#define TH_AP9_DEFAULT_GIVE 80
#define TH_AMMO_AP9_GIVE TH_AP9_MAX_CLIP
#define TH_AP9_WEIGHT 15

enum th_ap9_anim_e
{
	TH_AP9_IDLE = 0,
	TH_AP9_RELOAD,
	TH_AP9_DRAW,
	TH_AP9_SHOOT1,
	TH_AP9_SHOOT2,
	TH_AP9_SHOOT3,
};
