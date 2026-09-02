#ifndef WEAPON_IDS_H
#define WEAPON_IDS_H

// Shared network weapon identifiers. Keep this dependency-free so client-only
// systems can consume the canonical IDs without server weapon dependencies.
#define WEAPON_NONE 0
#define WEAPON_CROWBAR 1
#define WEAPON_GLOCK 2
#define WEAPON_PYTHON 3
#define WEAPON_MP5 4
#define WEAPON_GRAPPLE 5
#define WEAPON_CROSSBOW 6
#define WEAPON_SHOTGUN 7
#define WEAPON_RPG 8
#define WEAPON_GAUSS 9
#define WEAPON_EGON 10
#define WEAPON_HORNETGUN 11
#define WEAPON_HANDGRENADE 12
#define WEAPON_TRIPMINE 13
#define WEAPON_SATCHEL 14
#define WEAPON_SNARK 15
#define WEAPON_DISPLACER 16
#define WEAPON_PIPEWRENCH 17
#define WEAPON_SHOCKRIFLE 18
#define WEAPON_SPORELAUNCHER 19
#define WEAPON_INVENTORY 20
#define WEAPON_MEDKIT 21
#define WEAPON_KNIFE 22

// Leave a gap for custom weapons that vanilla Half-Life clients can pick up.
#define WEAPON_SUIT 31

#endif
