/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_dma.cpp -- main control for any streaming sound output device

#include "client.h"
#include "snd_loc.h"
#include "cmodel.h"			// for bspfile.type

static void S_Play_f(int argc, char **argv);
static void S_SoundList_f(bool usage, int argc, char **argv);
void S_Update_();


/* NOTES:
 *	1) sounds, which names started with '*' are placeholders - used by game, and engine will
 *		automatically change name to a corresponding model- (or gender-) specific sound
 *	2) sounds, which names started with '#' are searched from base game directory, others -
 *		- from "sounds/..."
 */

// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range
#define		SOUND_FULLVOLUME		80

#define		SOUND_LOOPATTENUATE2	0.003		// for Q2 maps
#define		SOUND_LOOPATTENUATE3	0.0008		// for Q3 maps

static int s_registration_sequence;

channel_t   channels[MAX_CHANNELS];

static bool sound_started = false;
static bool s_registering;
dma_t		dma;

static CVec3 listener_origin, listener_right;

static int soundtime;		// sample PAIRS
int paintedtime; 			// sample PAIRS; used in snd_mix.c

// during registration it is possible to have more sounds
// than could actually be referenced during gameplay,
// because we don't want to free anything until we are
// sure we won't need it.
#define		MAX_SFX		(MAX_SOUNDS*2)
static sfx_t known_sfx[MAX_SFX];
static int	num_sfx;

#define		MAX_PLAYSOUNDS	128
static playsound_t	s_playsounds[MAX_PLAYSOUNDS];
static playsound_t	s_freeplays;
playsound_t	s_pendingplays;

static int	s_beginofs;

cvar_t	*s_volume;
cvar_t	*s_testsound;
cvar_t	*s_loadas8bit;
cvar_t	*s_khz;
cvar_t	*s_primary;
static cvar_t	*s_reverse_stereo;
static cvar_t	*s_show;
static cvar_t	*s_mixahead;


int		s_rawend;
portable_samplepair_t	s_rawsamples[MAX_RAW_SAMPLES];

// ====================================================================
// User-setable variables
// ====================================================================


void S_SoundInfo_f(void)
{
	if (!sound_started)
	{
		appPrintf("sound system not started\n");
		return;
	}

    appPrintf("%5d stereo\n", dma.channels - 1);
    appPrintf("%5d samples\n", dma.samples);
    appPrintf("%5d samplepos\n", dma.samplepos);
    appPrintf("%5d samplebits\n", dma.samplebits);
    appPrintf("%5d submission_chunk\n", dma.submission_chunk);
    appPrintf("%5d speed\n", dma.speed);
}


void S_Init(void)
{
	cvar_t *cv = Cvar_Get("nosound", "0", 0);
	if (cv->integer)
		appPrintf(S_CYAN"Sound disabled\n");
	else
	{
CVAR_BEGIN(vars)
		CVAR_VAR(s_volume, 0.7, CVAR_ARCHIVE),
		CVAR_VAR(s_khz, 22, CVAR_ARCHIVE),
		CVAR_VAR(s_loadas8bit, 0, CVAR_ARCHIVE),
		CVAR_VAR(s_reverse_stereo, 0, CVAR_ARCHIVE),
		CVAR_VAR(s_mixahead, 0.2, CVAR_ARCHIVE),
		CVAR_VAR(s_show, 0, 0),
		CVAR_VAR(s_testsound, 0, 0),
		CVAR_VAR(s_primary, 0, CVAR_ARCHIVE)		//?? win32 specific
CVAR_END

		Cvar_GetVars(ARRAY_ARG(vars));

		appPrintf("\n------- Sound initialization -------\n");

		if (!SNDDMA_Init()) return;

		RegisterCommand("play", S_Play_f);
		RegisterCommand("stopsound", S_StopAllSounds_f);
		RegisterCommand("soundlist", S_SoundList_f);
		RegisterCommand("soundinfo", S_SoundInfo_f);

		S_InitScaletable();

		sound_started = true;
		num_sfx = 0;

		soundtime   = 0;
		paintedtime = 0;

		appPrintf("sound sampling rate: %d\n", dma.speed);

		S_StopAllSounds_f();

		appPrintf("------------------------------------\n");
	}
}


// =======================================================================
// Shutdown sound engine
// =======================================================================

void S_Shutdown(void)
{
	if (!sound_started)
		return;

	SNDDMA_Shutdown();

	sound_started = false;

	UnregisterCommand("play");
	UnregisterCommand("stopsound");
	UnregisterCommand("soundlist");
	UnregisterCommand("soundinfo");

	// free all sounds
	int		i;
	sfx_t	*sfx;
	for (i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++)
	{
		if (!sfx->Name[0])
			continue;
		if (sfx->cache)
			appFree(sfx->cache);
		memset(sfx, 0, sizeof(*sfx));
	}

	num_sfx = 0;
}


// =======================================================================
// Load a sound
// =======================================================================


static sfx_t *S_FindName(const char *name, bool create)
{
	guard(S_FindName);

	int		i;
	sfx_t	*sfx;

	if (!name)		appError("NULL name\n");
	if (!name[0])	appError("empty name\n");
	if (strlen(name) >= MAX_QPATH) appError("Sound name too long: %s", name);

	TString<MAX_QPATH> Filename;
	Filename.filename(name);

	// see if already loaded
	for (i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++)
		if (sfx->Name == Filename)
			return sfx;

	if (!create)
		return NULL;

	// find a free sfx
	for (i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++)
		if (!sfx->Name[0])
			break;

	if (i == num_sfx)
	{
		// not found
		if (num_sfx == MAX_SFX)
		{
#if 0
			appError("too much sfx records");
#else
			Com_DPrintf("too much sfx records\n");
			return NULL;
#endif
		}
		// alloc next free record
		num_sfx++;
	}

	memset(sfx, 0, sizeof(*sfx));
	sfx->Name = Filename;
	sfx->registration_sequence = s_registration_sequence;
	return sfx;

	unguard;
}


static sfx_t *S_AliasName(const char *aliasname, const char *truename)
{
	guard(S_AliasName);

	// find a free sfx
	sfx_t	*sfx;
	int		i;
	for (i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++)
		if (!sfx->Name[0])
			break;

	if (i == num_sfx)
	{
		if (num_sfx == MAX_SFX)
		{
#if 0
			appError("too much sfx records");
#else
			Com_DPrintf("too much sfx records\n");
			return NULL;
#endif
		}
		num_sfx++;
	}

	memset(sfx, 0, sizeof(*sfx));
	sfx->Name.filename(aliasname);
	sfx->TrueName = truename;
	sfx->registration_sequence = s_registration_sequence;
	return sfx;

	unguard;
}


void S_BeginRegistration(void)
{
	s_registration_sequence++;
	s_registering = true;
}


sfx_t *S_RegisterSound(const char *name)
{
	if (!sound_started) return NULL;

	sfx_t *sfx = S_FindName(name, true);
	sfx->registration_sequence = s_registration_sequence;

	if (!s_registering)
		S_LoadSound(sfx);
	else
		if (name[0] != '#' && name[0] != '*' && !GFileSystem->FileExists(va("sound/%s", name)))
			sfx->absent = true;

	return sfx;
}


void S_EndRegistration(void)
{
	int		i;
	sfx_t	*sfx;

	// free any sounds not from this registration sequence
	for (i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++)
	{
		if (!sfx->Name[0])
			continue;
		if (sfx->registration_sequence != s_registration_sequence)
		{
			// don't need this sound
			if (sfx->cache)	// it is possible to have a leftover
				appFree(sfx->cache);	// from a server that didn't finish loading
			memset(sfx, 0, sizeof(*sfx));
		}
	}

	// load everything in
	for (i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++)
	{
		if (!sfx->Name[0])
			continue;
		S_LoadSound(sfx);
	}

	s_registering = false;
}


//=============================================================================

channel_t *S_PickChannel(int entnum, int entchannel)
{
    int			ch_idx;
    int			first_to_die;
    int			life_left;
	channel_t	*ch;

	if (entchannel<0)
		Com_DropError("S_PickChannel: entchannel<0");

// Check for replacement sound, or find the best one to replace
    first_to_die = -1;
    life_left = 0x7fffffff;
    for (ch_idx=0 ; ch_idx < MAX_CHANNELS ; ch_idx++)
    {
		if (entchannel != 0		// channel 0 never overrides
		&& channels[ch_idx].entnum == entnum
		&& channels[ch_idx].entchannel == entchannel)
		{	// always override sound from same entity
			first_to_die = ch_idx;
			break;
		}

		// don't let monster sounds override player sounds
		if (channels[ch_idx].entnum == cl.playernum+1 && entnum != cl.playernum+1 && channels[ch_idx].sfx)
			continue;

		if (channels[ch_idx].end - paintedtime < life_left)
		{
			life_left = channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
   }

	if (first_to_die == -1)
		return NULL;

	ch = &channels[first_to_die];
	memset(ch, 0, sizeof(*ch));

    return ch;
}


// Used for spatializing channels and autosounds
static void S_SpatializeOrigin(const CVec3 &origin, float master_vol, float dist_mult, int *left_vol, int *right_vol)
{
	if (cls.state != ca_active)
	{
		*left_vol = *right_vol = 255;
		return;
	}

	// calculate stereo seperation and distance attenuation
	CVec3 source_vec;
	VectorSubtract(origin, listener_origin, source_vec);

	float dist = source_vec.NormalizeFast();
	dist -= SOUND_FULLVOLUME;
	if (dist < 0)
		dist = 0;			// close enough to be at full volume
	dist *= dist_mult;		// different attenuation levels

	float d = dot(listener_right, source_vec);

	float lscale, rscale;
	if (dma.channels == 1 || !dist_mult)
	{	// no attenuation = no spatialization
		rscale = 1.0f;
		lscale = 1.0f;
	}
	else
	{
		rscale = 0.5f * (1.0f + d);
		lscale = 0.5f * (1.0f - d);
	}

	// swap left and right volumes
	if (s_reverse_stereo->integer)
		Exchange(lscale, rscale);

	float scale;
	// add in distance effect
	scale = (1.0f - dist) * rscale;
	*right_vol = appRound(master_vol * scale);
	if (*right_vol < 0)
		*right_vol = 0;

	scale = (1.0f - dist) * lscale;
	*left_vol = appRound(master_vol * scale);
	if (*left_vol < 0)
		*left_vol = 0;
}


// Called to get the sound spatialization origin
static void GetEntitySoundOrigin(int ent, CVec3 &org)
{
	if (ent < 0 || ent >= MAX_EDICTS)
		Com_DropError("CL_GetEntitySoundOrigin: bad ent");
	centity_t *e = &cl_entities[ent];
	Lerp(e->prev.center, e->current.center, cl.lerpfrac, org);
}

void S_Spatialize(channel_t *ch)
{
	// anything coming from the view entity will always be full volume
	if (ch->entnum == cl.playernum+1)
	{
		ch->leftvol  = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}

	CVec3	origin;
	if (ch->fixed_origin)
	{
		origin = ch->origin;
	}
	else
		GetEntitySoundOrigin(ch->entnum, origin);

	S_SpatializeOrigin(origin, ch->master_vol, ch->dist_mult, &ch->leftvol, &ch->rightvol);
}


playsound_t *S_AllocPlaysound(void)
{
	playsound_t	*ps = s_freeplays.next;
	if (ps == &s_freeplays)
		return NULL;		// no free playsounds

	// unlink from freelist
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	return ps;
}


void S_FreePlaysound(playsound_t *ps)
{
	// unlink from channel
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	// add to free list
	ps->next = s_freeplays.next;
	s_freeplays.next->prev = ps;
	ps->prev = &s_freeplays;
	s_freeplays.next = ps;
}



/*
===============
S_IssuePlaysound

Take the next playsound and begin it on the channel
This is never called directly by S_Play*, but only
by the update loop.
===============
*/
void S_IssuePlaysound(playsound_t *ps)
{
	channel_t	*ch;
	sfxcache_t	*sc;

	if (s_show->integer)
		appPrintf("Sound: %s begin:%d ent:%d chn:%d\n", *ps->sfx->Name, ps->begin, ps->entnum, ps->entchannel);
	// pick a channel to play on
	ch = S_PickChannel(ps->entnum, ps->entchannel);
	if (!ch)
	{
		S_FreePlaysound(ps);
		return;
	}

	// spatialize
	if (ps->attenuation == ATTN_STATIC)
		ch->dist_mult = ps->attenuation * 0.001f;		// will be fixed value: 0.003f
	else
		ch->dist_mult = ps->attenuation * 0.0005f;
	ch->master_vol = ps->volume;
	ch->entnum = ps->entnum;
	ch->entchannel = ps->entchannel;
	ch->sfx = ps->sfx;
	ch->origin = ps->origin;
	ch->fixed_origin = ps->fixed_origin;

	S_Spatialize(ch);

	ch->pos = 0;
	sc = S_LoadSound(ch->sfx);
	ch->end = paintedtime + sc->length;

	// free the playsound
	S_FreePlaysound(ps);
}

static sfx_t *GetPlayerSound(clEntityState_t *ent, const char *base)
{
	clientInfo_t &ci = cl.clientInfo[ent->number - 1];	// may use ent->skinnum & 0xFF too ...
	bool isFemale = ci.modelGender == 'f';

	// see if we already know of the model specific sound
	TString<MAX_QPATH> LocalFilename;
	LocalFilename.sprintf("#players/%s/%s", *ci.ModelName, base+1);
	sfx_t *sfx = S_FindName(LocalFilename, false);
	if (sfx) return sfx;

	// try sound in Quake2 model directory ("players/[model_name]/[sound]")
	if (GFileSystem->FileExists(LocalFilename + 1))
	{
		sfx = S_RegisterSound(LocalFilename);
		if (sfx) return sfx;
	}

	// try sound in Quake3 model directory ("sound/player/[model_name]/[sound]")
	static const struct {
		const char *q2name, *q3name;
	} convert[] = {
		{"death4.wav",	 "death1.wav"},
		{"fall2.wav",	 "fall1.wav"},
		{"pain25_2.wav", "pain25_1.wav"},
		{"pain50_2.wav", "pain50_1.wav"},
		{"pain75_2.wav", "pain75_1.wav"},
		{"pain100_2.wav","pain100_1.wav"}
	};
	// Quake3 models have no some Quake2 sounds
	const char *newName = base + 1;
	for (int i = 0; i < ARRAY_COUNT(convert); i++)
		if (!stricmp(newName, convert[i].q2name))
		{
			newName = convert[i].q3name;
			break;
		}
	TString<MAX_QPATH> FileName2;
	FileName2.sprintf("sound/player/%s/%s", *ci.ModelName, newName);	// NOTE: used below as filename2+6 too
	if (GFileSystem->FileExists(FileName2))
	{
		sfx = S_AliasName(LocalFilename, FileName2 + 6 /* skip "sound/" */);
		if (sfx) return sfx;
	}

	// setup as default sound
	return S_AliasName(LocalFilename, va("player/%smale/%s", isFemale ? "fe" : "", base+1));
}


// =======================================================================
// Start a sound effect
// =======================================================================

/*
====================
S_StartSound

Validates the parms and ques the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound
====================
*/
void S_StartSound(const CVec3 *origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs)
{
	if (!sound_started) return;
	if (!sfx) return;

	if (sfx->Name[0] == '*')
		sfx = GetPlayerSound(&cl_entities[entnum].current, sfx->Name);

	// make sure the sound is loaded
	sfxcache_t *sc = S_LoadSound(sfx);
	if (!sc)
		return;		// couldn't load the sound's data

	int vol = appRound(fvol*255);

	// make the playsound_t
	playsound_t *ps = S_AllocPlaysound();
	if (!ps) return;

	if (origin)
	{
		ps->origin = *origin;
		ps->fixed_origin = true;
	}
	else
		ps->fixed_origin = false;

	ps->entnum      = entnum;
	ps->entchannel  = entchannel;
	ps->attenuation = attenuation;
	ps->volume      = vol;
	ps->sfx         = sfx;

	// drift s_beginofs
	int start = appRound(cl.frame.servertime / 1000.0f * dma.speed + s_beginofs);
	if (start < paintedtime)
	{
		start = paintedtime;
		s_beginofs = start - appRound(cl.frame.servertime / 1000.0f * dma.speed);
	}
	else if (start > paintedtime + 0.3 * dma.speed)
	{
		start = appRound(paintedtime + 0.1 * dma.speed);
		s_beginofs = start - appRound(cl.frame.servertime / 1000.0f * dma.speed);
	}
	else
	{
		s_beginofs-=10;
	}

	if (!timeofs)
		ps->begin = paintedtime;
	else
		ps->begin = appRound(start + timeofs * dma.speed);

	// sort into the pending sound list
	playsound_t *sort;
	for (sort = s_pendingplays.next; sort != &s_pendingplays && sort->begin < ps->begin; sort = sort->next)
		; // empty

	ps->next = sort;
	ps->prev = sort->prev;

	ps->next->prev = ps;
	ps->prev->next = ps;
}


void S_StartLocalSound(const char *sound)
{
	if (!sound_started)
		return;

	sfx_t *sfx = S_RegisterSound(sound);
	if (!sfx)
	{
		appPrintf("S_StartLocalSound: can't cache %s\n", sound);
		return;
	}
	S_StartSound(NULL, cl.playernum+1, 0, sfx, 1, 1, 0);
}


void S_ClearBuffer()
{
	if (!sound_started)
		return;

	s_rawend = 0;

	int clear;
	if (dma.samplebits == 8)
		clear = 0x80;
	else
		clear = 0;

	SNDDMA_BeginPainting();
	if (dma.buffer)
		memset(dma.buffer, clear, dma.samples * dma.samplebits/8);
	SNDDMA_Submit();
}


void S_StopAllSounds_f()
{
	if (!sound_started) return;

	// clear all the playsounds
	memset(s_playsounds, 0, sizeof(s_playsounds));
	s_freeplays.next    = s_freeplays.prev    = &s_freeplays;
	s_pendingplays.next = s_pendingplays.prev = &s_pendingplays;

	for (int i = 0; i < MAX_PLAYSOUNDS; i++)
	{
		s_playsounds[i].prev = &s_freeplays;
		s_playsounds[i].next = s_freeplays.next;
		s_playsounds[i].prev->next = &s_playsounds[i];
		s_playsounds[i].next->prev = &s_playsounds[i];
	}

	// clear all the channels
	memset(channels, 0, sizeof(channels));

	S_ClearBuffer();
}

/*
==================
AddLoopSounds

Entities with a ->sound field will generated looped sounds
that are automatically started, stopped, and merged together
as the entities are sent to the client
==================
*/
static void AddLoopSounds()
{
	int		i;
	int		num;
	clEntityState_t	*ent;

	if (cl_paused->integer) return;
	if (cls.state != ca_active) return;
	if (!cl.sound_ambient) return;

	int sounds[MAX_EDICTS];
	for (i = 0; i < cl.frame.num_entities; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];
		sounds[i] = ent->sound;
	}

	float atten = SOUND_LOOPATTENUATE2;
	if (bspfile.type == map_q3)
		atten = SOUND_LOOPATTENUATE3;

	for (i = 0; i < cl.frame.num_entities; i++)
	{
		if (!sounds[i]) continue;
		sfx_t *sfx = cl.sound_precache[sounds[i]];
		if (!sfx) continue;		// bad sound effect
		sfxcache_t *sc = sfx->cache;
		if (!sc) continue;

		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		// find the total contribution of all sounds of this type
		int left_total, right_total;
		S_SpatializeOrigin(ent->origin, 255.0, atten, &left_total, &right_total);
		for (int j = i + 1; j < cl.frame.num_entities; j++)
		{
			if (sounds[j] != sounds[i]) continue;
			sounds[j] = 0;	// don't check this again later

			num = (cl.frame.parse_entities + j)&(MAX_PARSE_ENTITIES-1);
			ent = &cl_parse_entities[num];

			int left, right;
			S_SpatializeOrigin(ent->origin, 255.0, atten, &left, &right);
			left_total += left;
			right_total += right;
		}

		if (left_total == 0 && right_total == 0)
			continue;		// not audible

		// allocate a channel
		channel_t *ch = S_PickChannel(0, 0);
		if (!ch) return;

		if (left_total > 255)
			left_total = 255;
		if (right_total > 255)
			right_total = 255;
		ch->leftvol   = left_total;
		ch->rightvol  = right_total;
		ch->autosound = true;	// remove next frame
		ch->sfx       = sfx;
		ch->pos       = paintedtime % sc->length;
		ch->end       = paintedtime + sc->length - ch->pos;
	}
}

//=============================================================================

// Cinematic streaming and voice over network
void S_RawSamples(int samples, int rate, int width, int channels, byte *data)
{
	int		i;
	int		src, dst;

	if (!sound_started)
		return;

	if (s_rawend < paintedtime)
		s_rawend = paintedtime;
	float scale = (float)rate / dma.speed;

	if (channels == 2 && width == 2)
	{
		if (scale == 1.0)
		{	// optimized case
			for (i=0 ; i<samples ; i++)
			{
				dst = s_rawend&(MAX_RAW_SAMPLES-1);
				s_rawend++;
				s_rawsamples[dst].left  = ((short *)data)[i*2]   << 8;
				s_rawsamples[dst].right = ((short *)data)[i*2+1] << 8;
			}
		}
		else
		{
			for (i=0 ; ; i++)
			{
				src = appRound(i*scale);
				if (src >= samples)
					break;
				dst = s_rawend&(MAX_RAW_SAMPLES-1);
				s_rawend++;
				s_rawsamples[dst].left  = ((short *)data)[src*2]   << 8;
				s_rawsamples[dst].right = ((short *)data)[src*2+1] << 8;
			}
		}
	}
	else if (channels == 1 && width == 2)
	{
		for (i=0 ; ; i++)
		{
			src = appRound(i*scale);
			if (src >= samples)
				break;
			dst = s_rawend&(MAX_RAW_SAMPLES-1);
			s_rawend++;
			s_rawsamples[dst].left  = ((short *)data)[src] << 8;
			s_rawsamples[dst].right = ((short *)data)[src] << 8;
		}
	}
	else if (channels == 2 && width == 1)
	{
		for (i=0 ; ; i++)
		{
			src = appRound(i*scale);
			if (src >= samples)
				break;
			dst = s_rawend&(MAX_RAW_SAMPLES-1);
			s_rawend++;
			s_rawsamples[dst].left  = ((char *)data)[src*2]   << 16;
			s_rawsamples[dst].right = ((char *)data)[src*2+1] << 16;
		}
	}
	else if (channels == 1 && width == 1)
	{
		for (i=0 ; ; i++)
		{
			src = appRound(i*scale);
			if (src >= samples)
				break;
			dst = s_rawend&(MAX_RAW_SAMPLES-1);
			s_rawend++;
			s_rawsamples[dst].left  = (((byte *)data)[src]-128) << 16;
			s_rawsamples[dst].right = (((byte *)data)[src]-128) << 16;
		}
	}
}

//=============================================================================

// Called once each time through the main loop
void S_Update(const CVec3 &origin, const CVec3 &right)
{
	int		i;

	if (!sound_started) return;

	// rebuild scale tables if volume is modified
	if (s_volume->modified)
		S_InitScaletable();

	listener_origin = origin;
	listener_right  = right;

	// update spatialization for dynamic sounds
	channel_t *ch = channels;
	for (i = 0; i < MAX_CHANNELS; i++, ch++)
	{
		if (!ch->sfx)
			continue;
		if (ch->autosound)
		{	// autosounds are regenerated fresh each frame
			memset(ch, 0, sizeof(*ch));
			continue;
		}
		S_Spatialize(ch);         // respatialize channel
		if (!ch->leftvol && !ch->rightvol)
		{
			memset(ch, 0, sizeof(*ch));
			continue;
		}
	}

	// add loopsounds
	AddLoopSounds();

	// debugging output
	if (s_show->integer == 2)
	{
		int total = 0;
		for (i = 0, ch = channels; i < MAX_CHANNELS; i++, ch++)
			if (ch->sfx && (ch->leftvol || ch->rightvol) )
			{
				appPrintf("%3i %3i %s\n", ch->leftvol, ch->rightvol, *ch->sfx->Name);
				total++;
			}

		appPrintf("----(%i)---- painted: %i\n", total, paintedtime);
	}

// mix some sound
	S_Update_();
}

void GetSoundtime()
{
	static int buffers;
	static int oldsamplepos;

	int fullsamples = dma.samples / dma.channels;

	// it is possible to miscount buffers if it has wrapped twice between
	// calls to S_Update.  Oh well.
	int samplepos = SNDDMA_GetDMAPos();

	if (samplepos < oldsamplepos)
	{
		buffers++;					// buffer wrapped

		if (paintedtime > 0x40000000)
		{	// time to chop things off to avoid 32 bit limits
			buffers = 0;
			paintedtime = fullsamples;
			S_StopAllSounds_f();
		}
	}
	oldsamplepos = samplepos;

	soundtime = buffers*fullsamples + samplepos/dma.channels;
}


void S_Update_()
{
	guard(S_Update_);

	if (!sound_started) return;

	SNDDMA_BeginPainting();

	if (!dma.buffer) return;

	// Updates DMA time
	GetSoundtime();

	// check to make sure that we haven't overshot
	if (paintedtime < soundtime)
	{
		Com_DPrintf("S_Update_ : overflow\n");
		paintedtime = soundtime;
	}

	// mix ahead of current position
	unsigned endtime = soundtime + appRound(s_mixahead->value * dma.speed);

	// mix to an even submission block size
	endtime = Align(endtime, dma.submission_chunk);
	int samps = dma.samples >> (dma.channels-1);
	if (endtime - soundtime > samps)
		endtime = soundtime + samps;

	S_PaintChannels(endtime);

	SNDDMA_Submit();
	unguard;
}

/*
===============================================================================

console functions

===============================================================================
*/

static void S_Play_f(int argc, char **argv)
{
	for (int i = 1; i < argc; i++)
	{
		TString<256> Name;
		if (!strrchr(argv[i], '.'))
			Name.sprintf("%s.wav", argv[i]);
		else
			Name = argv[i];
		sfx_t *sfx = S_RegisterSound(Name);
		S_StartSound(NULL, cl.playernum+1, 0, sfx, 1.0, 1.0, 0);
	}
}

static void S_SoundList_f(bool usage, int argc, char **argv)
{
	if (argc > 2 || usage)
	{
		appPrintf("Usage: soundlist [<mask>]\n");
		return;
	}

	const char *mask = (argc == 2) ? argv[1] : NULL;

	appPrintf("---lp-bit-size----name----------\n");

	int n = 0, total = 0;
	int		i;
	sfx_t	*sfx;
	for (i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++)
	{
		if (!sfx->Name[0])
			continue;

		if (mask && !appMatchWildcard(sfx->Name, mask, true)) continue;
		n++;

		sfxcache_t *sc = sfx->cache;
		if (sc)
		{
			int size = sc->length*sc->width*(sc->stereo+1);
			total += size;
			appPrintf("%-3d %c %2d  %-7d %s\n", i, sc->loopstart >= 0 ? 'L' : ' ', sc->width*8, size, *sfx->Name);
		}
		else
		{
			char *status;

			if (sfx->Name[0] == '*')
				status = "placeholder  ";
			else if (sfx->absent)
				status = S_RED"not found    "S_WHITE;
			else
				status = "not loaded   ";
			appPrintf("%-3d %s %s\n", i, status, *sfx->Name);
		}
	}
	appPrintf("Displayed %d/%d sounds; resident: %d bytes\n", n, num_sfx, total);
}
