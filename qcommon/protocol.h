

// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, frame animations)
// that happen constantly on the given entity.
// An entity that has effects will be sent to the client
// even if it has a zero index model.
#define	EF_ROTATE			0x00000001		// rotate (bonus items)
#define	EF_GIB				0x00000002		// blood trail
#define	EF_BLASTER			0x00000008		// light + trail; when EF_TRACKER set - different trail == EF_BLASTER2
#define	EF_ROCKET			0x00000010		// light + trail
#define	EF_GRENADE			0x00000020		// trail
#define	EF_HYPERBLASTER		0x00000040		// light (when EF_TRACKER set -- neg. light)
#define	EF_BFG				0x00000080		// light + trail
#define EF_COLOR_SHELL		0x00000100
#define EF_POWERSCREEN		0x00000200		// special
#define	EF_ANIM01			0x00000400		// automatically cycle between frames 0 and 1 at 2 hz
#define	EF_ANIM23			0x00000800		// automatically cycle between frames 2 and 3 at 2 hz
#define EF_ANIM_ALL			0x00001000		// automatically cycle through all frames at 2hz
#define EF_ANIM_ALLFAST		0x00002000		// automatically cycle through all frames at 10hz
#define	EF_FLIES			0x00004000		// draw flies around entity origin
#define	EF_QUAD				0x00008000		// == blue shell
#define	EF_PENT				0x00010000		// == red shell
#define	EF_TELEPORTER		0x00020000		// particle fountain
#define EF_FLAG1			0x00040000		// red flag: red light + trail
#define EF_FLAG2			0x00080000		// blue flag: blue light + trail
// XATRIX
#define EF_IONRIPPER		0x00100000		// light + trail
#define EF_GREENGIB			0x00200000		// green blood trail
#define	EF_BLUEHYPERBLASTER 0x00400000		// blue light
#define EF_SPINNINGLIGHTS	0x00800000		// red light, rotating around entity origin
#define EF_PLASMA			0x01000000		// light + (if EF_ANIM_ALLFAST set) blaster trail
#define EF_TRAP				0x02000000		// light + trail
// ROGUE
#define EF_TRACKER			0x04000000		// trail + neg. light ; see EF_BLASTER too
#define	EF_DOUBLE			0x08000000		// color shell
#define	EF_SPHERETRANS		0x10000000		// special (+ EF_TRACKERTRAIL flag usage for alpha control)
#define EF_TAGTRAIL			0x20000000		// light + trail
#define EF_HALF_DAMAGE		0x40000000		// color shell
#define EF_TRACKERTRAIL		0x80000000		// when EF_TRACKER set - pulse neg. light; when not set - neg. light + tracker shell


// entity_state_t->renderfx flags
#define	RF_MINLIGHT			0x00000001		// allways have some light (viewmodel)
#define	RF_VIEWERMODEL		0x00000002		// don't draw through eyes, only mirrors
#define	RF_WEAPONMODEL		0x00000004		// only draw through eyes
#define	RF_FULLBRIGHT		0x00000008		// allways draw full intensity
#define	RF_DEPTHHACK		0x00000010		// for view weapon Z crunching
#define	RF_TRANSLUCENT		0x00000020
#define	RF_FRAMELERP		0x00000040		// set but game, but ignored by engine
#define RF_BEAM				0x00000080
#define	RF_CUSTOMSKIN		0x00000100		// skin is an index in image_precache
#define	RF_GLOW				0x00000200		// pulse lighting for bonus items
#define RF_SHELL_RED		0x00000400
#define	RF_SHELL_GREEN		0x00000800
#define RF_SHELL_BLUE		0x00001000
//ROGUE
#define RF_IR_VISIBLE		0x00008000
#define	RF_SHELL_DOUBLE		0x00010000
#define	RF_SHELL_HALF_DAM	0x00020000
#define RF_USE_DISGUISE		0x00040000
// 4.00
#define RF_BBOX				0x40000000


// muzzle flashes / player effects
enum {
	MZ_BLASTER,
	MZ_MACHINEGUN,
	MZ_SHOTGUN,
	MZ_CHAINGUN1, MZ_CHAINGUN2, MZ_CHAINGUN3,
	MZ_RAILGUN,
	MZ_ROCKET,
	MZ_GRENADE,
	MZ_LOGIN,
	MZ_LOGOUT,
	MZ_RESPAWN,
	MZ_BFG,
	MZ_SSHOTGUN,
	MZ_HYPERBLASTER,
	MZ_ITEMRESPAWN,
	// XATRIX
	MZ_IONRIPPER,
	MZ_BLUEHYPERBLASTER,
	MZ_PHALANX,
	MZ_SILENCED =128,		// bit flag OR'ed with one of the above numbers
	// ROGUE
	MZ_ETF_RIFLE = 30,
	MZ_UNUSED,
	MZ_SHOTGUN2,
	MZ_HEATBEAM,
	MZ_BLASTER2,
	MZ_TRACKER,
	MZ_NUKE1, MZ_NUKE2, MZ_NUKE4, MZ_NUKE8
};


// monster muzzle flashes
enum {
	MZ2_TANK_BLASTER_1 = 1, MZ2_TANK_BLASTER_2, MZ2_TANK_BLASTER_3,
	MZ2_TANK_MACHINEGUN_1, MZ2_TANK_MACHINEGUN_2, MZ2_TANK_MACHINEGUN_3,
	MZ2_TANK_MACHINEGUN_4, MZ2_TANK_MACHINEGUN_5, MZ2_TANK_MACHINEGUN_6,
	MZ2_TANK_MACHINEGUN_7, MZ2_TANK_MACHINEGUN_8, MZ2_TANK_MACHINEGUN_9,
	MZ2_TANK_MACHINEGUN_10, MZ2_TANK_MACHINEGUN_11, MZ2_TANK_MACHINEGUN_12,
	MZ2_TANK_MACHINEGUN_13, MZ2_TANK_MACHINEGUN_14, MZ2_TANK_MACHINEGUN_15,
	MZ2_TANK_MACHINEGUN_16, MZ2_TANK_MACHINEGUN_17, MZ2_TANK_MACHINEGUN_18,
	MZ2_TANK_MACHINEGUN_19,
	MZ2_TANK_ROCKET_1, MZ2_TANK_ROCKET_2, MZ2_TANK_ROCKET_3,

	MZ2_INFANTRY_MACHINEGUN_1, MZ2_INFANTRY_MACHINEGUN_2,
	MZ2_INFANTRY_MACHINEGUN_3, MZ2_INFANTRY_MACHINEGUN_4,
	MZ2_INFANTRY_MACHINEGUN_5, MZ2_INFANTRY_MACHINEGUN_6,
	MZ2_INFANTRY_MACHINEGUN_7, MZ2_INFANTRY_MACHINEGUN_8,
	MZ2_INFANTRY_MACHINEGUN_9, MZ2_INFANTRY_MACHINEGUN_10,
	MZ2_INFANTRY_MACHINEGUN_11, MZ2_INFANTRY_MACHINEGUN_12,
	MZ2_INFANTRY_MACHINEGUN_13,

	MZ2_SOLDIER_BLASTER_1, MZ2_SOLDIER_BLASTER_2,
	MZ2_SOLDIER_SHOTGUN_1, MZ2_SOLDIER_SHOTGUN_2,
	MZ2_SOLDIER_MACHINEGUN_1, MZ2_SOLDIER_MACHINEGUN_2,

	MZ2_GUNNER_MACHINEGUN_1, MZ2_GUNNER_MACHINEGUN_2,
	MZ2_GUNNER_MACHINEGUN_3, MZ2_GUNNER_MACHINEGUN_4,
	MZ2_GUNNER_MACHINEGUN_5, MZ2_GUNNER_MACHINEGUN_6,
	MZ2_GUNNER_MACHINEGUN_7, MZ2_GUNNER_MACHINEGUN_8,
	MZ2_GUNNER_GRENADE_1, MZ2_GUNNER_GRENADE_2,
	MZ2_GUNNER_GRENADE_3, MZ2_GUNNER_GRENADE_4,

	MZ2_CHICK_ROCKET_1,

	MZ2_FLYER_BLASTER_1, MZ2_FLYER_BLASTER_2,

	MZ2_MEDIC_BLASTER_1,

	MZ2_GLADIATOR_RAILGUN_1,

	MZ2_HOVER_BLASTER_1,

	MZ2_ACTOR_MACHINEGUN_1,

	MZ2_SUPERTANK_MACHINEGUN_1, MZ2_SUPERTANK_MACHINEGUN_2, MZ2_SUPERTANK_MACHINEGUN_3,
	MZ2_SUPERTANK_MACHINEGUN_4, MZ2_SUPERTANK_MACHINEGUN_5, MZ2_SUPERTANK_MACHINEGUN_6,
	MZ2_SUPERTANK_ROCKET_1, MZ2_SUPERTANK_ROCKET_2, MZ2_SUPERTANK_ROCKET_3,

	MZ2_BOSS2_MACHINEGUN_L1, MZ2_BOSS2_MACHINEGUN_L2, MZ2_BOSS2_MACHINEGUN_L3,
	MZ2_BOSS2_MACHINEGUN_L4, MZ2_BOSS2_MACHINEGUN_L5,
	MZ2_BOSS2_ROCKET_1, MZ2_BOSS2_ROCKET_2, MZ2_BOSS2_ROCKET_3, MZ2_BOSS2_ROCKET_4,

	MZ2_FLOAT_BLASTER_1,

	MZ2_SOLDIER_BLASTER_3, MZ2_SOLDIER_SHOTGUN_3, MZ2_SOLDIER_MACHINEGUN_3,
	MZ2_SOLDIER_BLASTER_4, MZ2_SOLDIER_SHOTGUN_4, MZ2_SOLDIER_MACHINEGUN_4,
	MZ2_SOLDIER_BLASTER_5, MZ2_SOLDIER_SHOTGUN_5, MZ2_SOLDIER_MACHINEGUN_5,
	MZ2_SOLDIER_BLASTER_6, MZ2_SOLDIER_SHOTGUN_6, MZ2_SOLDIER_MACHINEGUN_6,
	MZ2_SOLDIER_BLASTER_7, MZ2_SOLDIER_SHOTGUN_7, MZ2_SOLDIER_MACHINEGUN_7,
	MZ2_SOLDIER_BLASTER_8, MZ2_SOLDIER_SHOTGUN_8, MZ2_SOLDIER_MACHINEGUN_8,

	// --- Xian shit below ---
	MZ2_MAKRON_BFG,
	MZ2_MAKRON_BLASTER_1, MZ2_MAKRON_BLASTER_2, MZ2_MAKRON_BLASTER_3,
	MZ2_MAKRON_BLASTER_4, MZ2_MAKRON_BLASTER_5, MZ2_MAKRON_BLASTER_6,
	MZ2_MAKRON_BLASTER_7, MZ2_MAKRON_BLASTER_8, MZ2_MAKRON_BLASTER_9,
	MZ2_MAKRON_BLASTER_10, MZ2_MAKRON_BLASTER_11, MZ2_MAKRON_BLASTER_12,
	MZ2_MAKRON_BLASTER_13, MZ2_MAKRON_BLASTER_14, MZ2_MAKRON_BLASTER_15,
	MZ2_MAKRON_BLASTER_16, MZ2_MAKRON_BLASTER_17,
	MZ2_MAKRON_RAILGUN_1,
	MZ2_JORG_MACHINEGUN_L1, MZ2_JORG_MACHINEGUN_L2, MZ2_JORG_MACHINEGUN_L3,
	MZ2_JORG_MACHINEGUN_L4, MZ2_JORG_MACHINEGUN_L5, MZ2_JORG_MACHINEGUN_L6,
	MZ2_JORG_MACHINEGUN_R1, MZ2_JORG_MACHINEGUN_R2, MZ2_JORG_MACHINEGUN_R3,
	MZ2_JORG_MACHINEGUN_R4, MZ2_JORG_MACHINEGUN_R5, MZ2_JORG_MACHINEGUN_R6,
	MZ2_JORG_BFG_1,
	MZ2_BOSS2_MACHINEGUN_R1, MZ2_BOSS2_MACHINEGUN_R2, MZ2_BOSS2_MACHINEGUN_R3,
	MZ2_BOSS2_MACHINEGUN_R4, MZ2_BOSS2_MACHINEGUN_R5,

	//ROGUE
	MZ2_CARRIER_MACHINEGUN_L1, MZ2_CARRIER_MACHINEGUN_R1,
	MZ2_CARRIER_GRENADE,
	MZ2_TURRET_MACHINEGUN,
	MZ2_TURRET_ROCKET,
	MZ2_TURRET_BLASTER,
	MZ2_STALKER_BLASTER,
	MZ2_DAEDALUS_BLASTER,
	MZ2_MEDIC_BLASTER_2,
	MZ2_CARRIER_RAILGUN,
	MZ2_WIDOW_DISRUPTOR, MZ2_WIDOW_BLASTER, MZ2_WIDOW_RAIL,
	MZ2_WIDOW_PLASMABEAM,			// PMM - not used
	MZ2_CARRIER_MACHINEGUN_L2,
	MZ2_CARRIER_MACHINEGUN_R2,
	MZ2_WIDOW_RAIL_LEFT,
	MZ2_WIDOW_RAIL_RIGHT,
	MZ2_WIDOW_BLASTER_SWEEP1, MZ2_WIDOW_BLASTER_SWEEP2, MZ2_WIDOW_BLASTER_SWEEP3,
	MZ2_WIDOW_BLASTER_SWEEP4, MZ2_WIDOW_BLASTER_SWEEP5, MZ2_WIDOW_BLASTER_SWEEP6,
	MZ2_WIDOW_BLASTER_SWEEP7, MZ2_WIDOW_BLASTER_SWEEP8, MZ2_WIDOW_BLASTER_SWEEP9,
	MZ2_WIDOW_BLASTER_100, MZ2_WIDOW_BLASTER_90, MZ2_WIDOW_BLASTER_80,
	MZ2_WIDOW_BLASTER_70, MZ2_WIDOW_BLASTER_60, MZ2_WIDOW_BLASTER_50,
	MZ2_WIDOW_BLASTER_40, MZ2_WIDOW_BLASTER_30, MZ2_WIDOW_BLASTER_20,
	MZ2_WIDOW_BLASTER_10, MZ2_WIDOW_BLASTER_0,
	MZ2_WIDOW_BLASTER_10L, MZ2_WIDOW_BLASTER_20L, MZ2_WIDOW_BLASTER_30L,
	MZ2_WIDOW_BLASTER_40L, MZ2_WIDOW_BLASTER_50L, MZ2_WIDOW_BLASTER_60L,
	MZ2_WIDOW_BLASTER_70L,
	MZ2_WIDOW_RUN_1, MZ2_WIDOW_RUN_2, MZ2_WIDOW_RUN_3,
	MZ2_WIDOW_RUN_4, MZ2_WIDOW_RUN_5, MZ2_WIDOW_RUN_6,
	MZ2_WIDOW_RUN_7, MZ2_WIDOW_RUN_8,
	MZ2_CARRIER_ROCKET_1, MZ2_CARRIER_ROCKET_2,
	MZ2_CARRIER_ROCKET_3, MZ2_CARRIER_ROCKET_4,
	MZ2_WIDOW2_BEAMER_1, MZ2_WIDOW2_BEAMER_2, MZ2_WIDOW2_BEAMER_3,
	MZ2_WIDOW2_BEAMER_4, MZ2_WIDOW2_BEAMER_5,
	MZ2_WIDOW2_BEAM_SWEEP_1, MZ2_WIDOW2_BEAM_SWEEP_2, MZ2_WIDOW2_BEAM_SWEEP_3,
	MZ2_WIDOW2_BEAM_SWEEP_4, MZ2_WIDOW2_BEAM_SWEEP_5, MZ2_WIDOW2_BEAM_SWEEP_6,
	MZ2_WIDOW2_BEAM_SWEEP_7, MZ2_WIDOW2_BEAM_SWEEP_8, MZ2_WIDOW2_BEAM_SWEEP_9,
	MZ2_WIDOW2_BEAM_SWEEP_10, MZ2_WIDOW2_BEAM_SWEEP_11
};



// temp entity events
// Temp entity events are for things that happen at a location seperate from any existing entity.
// Temporary entity messages are explicitly constructed and broadcast.
enum
{
	TE_GUNSHOT,
	TE_BLOOD,
	TE_BLASTER,
	TE_RAILTRAIL,
	TE_SHOTGUN,
	TE_EXPLOSION1,
	TE_EXPLOSION2,
	TE_ROCKET_EXPLOSION,
	TE_GRENADE_EXPLOSION,
	TE_SPARKS,
	TE_SPLASH,
	TE_BUBBLETRAIL,
	TE_SCREEN_SPARKS,
	TE_SHIELD_SPARKS,
	TE_BULLET_SPARKS,
	TE_LASER_SPARKS,
	TE_PARASITE_ATTACK,
	TE_ROCKET_EXPLOSION_WATER,
	TE_GRENADE_EXPLOSION_WATER,
	TE_MEDIC_CABLE_ATTACK,
	TE_BFG_EXPLOSION,
	TE_BFG_BIGEXPLOSION,
	TE_BOSSTPORT,			// used as '22' in a map, so DON'T RENUMBER !
	TE_BFG_LASER,
	TE_GRAPPLE_CABLE,
	TE_WELDING_SPARKS,
	TE_GREENBLOOD,
	TE_BLUEHYPERBLASTER,
	TE_PLASMA_EXPLOSION,
	TE_TUNNEL_SPARKS,
	// ROGUE
	TE_BLASTER2,
	TE_RAILTRAIL2,
	TE_FLAME,
	TE_LIGHTNING,
	TE_DEBUGTRAIL,
	TE_PLAIN_EXPLOSION,
	TE_FLASHLIGHT,
	TE_FORCEWALL,
	TE_HEATBEAM,
	TE_MONSTER_HEATBEAM,
	TE_STEAM,
	TE_BUBBLETRAIL2,
	TE_MOREBLOOD,
	TE_HEATBEAM_SPARKS,
	TE_HEATBEAM_STEAM,
	TE_CHAINFIST_SMOKE,
	TE_ELECTRIC_SPARKS,
	TE_TRACKER_EXPLOSION,
	TE_TELEPORT_EFFECT,
	TE_DBALL_GOAL,
	TE_WIDOWBEAMOUT,
	TE_NUKEBLAST,
	TE_WIDOWSPLASH,
	TE_EXPLOSION1_BIG,
	TE_EXPLOSION1_NP,
	TE_FLECHETTE,
	// extended protocol constants
	TE_RAILTRAIL_EXT = 128		// railgun trail with specified color and type
};



enum {
	SPLASH_UNKNOWN,
	SPLASH_SPARKS,
	SPLASH_BLUE_WATER,
	SPLASH_BROWN_WATER,
	SPLASH_SLIME,
	SPLASH_LAVA,
	SPLASH_BLOOD,
	// extended protocol
	SPLASH_BULLET_BLUE_WATER,
	SPLASH_BULLET_BROWN_WATER
};


// player_state->stats[] indexes
enum {
	STAT_HEALTH_ICON,
	STAT_HEALTH,
	STAT_AMMO_ICON,
	STAT_AMMO,
	STAT_ARMOR_ICON,
	STAT_ARMOR,
	STAT_SELECTED_ICON,
	STAT_PICKUP_ICON,
	STAT_PICKUP_STRING,
	STAT_TIMER_ICON,
	STAT_TIMER,
	STAT_HELPICON,
	STAT_SELECTED_ITEM,
	STAT_LAYOUTS,
	STAT_FRAGS,
	STAT_FLASHES,					// cleared each frame, 1 = health, 2 = armor
	STAT_CHASE,
	STAT_SPECTATOR
};



// entity_state_t->event values
// ertity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.
// All muzzle flashes really should be converted to events...
#define NUM_CAMPER_EVENTS	9

enum
{
	EV_NONE,
	EV_ITEM_RESPAWN,
	EV_FOOTSTEP,
	EV_FALLSHORT,
	EV_FALL,
	EV_FALLFAR,
	EV_PLAYER_TELEPORT,
	EV_OTHER_TELEPORT,

	// added since 4.00

	// ext protocol v0
	EV_FOOTSTEP0 = 100,
	EV_FALLSHORT0 = EV_FOOTSTEP0 + MATERIAL_COUNT,
	EV_SPECTATOR0 = EV_FALLSHORT0 + MATERIAL_COUNT,		//?? unused in v0
	EV_CAMPER0 = EV_SPECTATOR0 + 4,
	//?? for extending (rename EV_NEXT to something new):
	EV_NEXT = EV_CAMPER0 + NUM_CAMPER_EVENTS
};



// config strings are a general means of communication from the server to
// all connected clients. Each config string can be at most MAX_QPATH characters.
enum
{
	CS_NAME,
	CS_CDTRACK,
	CS_SKY,
	CS_SKYAXIS,			// %f %f %f format
	CS_SKYROTATE,
	CS_STATUSBAR,		// display program string

	CS_AIRACCEL = 29,	// air acceleration control
	CS_MAXCLIENTS,
	CS_MAPCHECKSUM,		// for catching cheater maps

	CS_MODELS,
	CS_SOUNDS = CS_MODELS+MAX_MODELS,
	CS_IMAGES = CS_SOUNDS+MAX_SOUNDS,
	CS_LIGHTS = CS_IMAGES+MAX_IMAGES,
	CS_ITEMS = CS_LIGHTS+MAX_LIGHTSTYLES,
	CS_PLAYERSKINS = CS_ITEMS+MAX_ITEMS,
	CS_GENERAL = CS_PLAYERSKINS+MAX_CLIENTS,

	MAX_CONFIGSTRINGS = CS_GENERAL+MAX_GENERAL
};
