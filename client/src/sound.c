/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#include "include.h"

_sound_system SoundSystem; /* THE SoundSystem status*/

#ifdef INSTALL_SOUND

music_data music;	  		 /* thats the music we just play - if NULL, no music */
music_data music_new;		 /* when we get a new piece of music, we store it here and
									 * give the current music the command to fade out or
									 * to break (if new music parameter force it).
									 * then we copy new_music to music and start it
									 */
static music_data music_buffer;		 /* when we got a "special" music like a map position
									   * music command (for example a NPC shout or something
									   * like this) then we need to play it ASAP - and when
									   * we are finished with it, we need to restore the
									   * old music we perhaps had interrupted. the old music
									   * is stored here
									   */

static int special_sounds[SPECIAL_SOUND_INIT];
#endif

#define POW2(x) ((x) * (x))


#ifdef INSTALL_SOUND
_wave Sounds[SOUND_MAX+SPELL_SOUND_MAX];

static char* sound_files[SOUND_MAX] = {
	"event01.wav"
	,"bow1.wav"
	,"learnspell.wav"
	,"missspell.wav"
	,"rod.wav"
	,"door.wav"
	,"push.wav"

	,"hit_impact.wav" /* 8 */
	,"hit_cleave.wav"
	,"hit_slash.wav"
	,"hit_pierce.wav"
	,"hit_block.wav"
    ,"hit_hand.wav"
    ,"miss_mob1.wav"
    ,"miss_player1.wav"
    
	,"petdead.wav" /* 16 */
	,"playerdead.wav"
    ,"explosion.wav"
    ,"explosion.wav"
    ,"kill.wav"
	,"pull.wav"
	,"fallhole.wav"
	,"poison.wav"
	
	,"drop.wav" /* 24 */
	,"lose_some.wav"
	,"throw.wav"
	,"gate_open.wav"
	,"gate_close.wav"
	,"open_container.wav"
	,"growl.wav"
	,"arrow_hit.wav"
	,"door_close.wav"
	,"teleport.wav"

	,"step1.wav" /* here starts client side sounds */
	,"step2.wav"
	,"pray.wav"
	,"console.wav"
	,"click_fail.wav"
	,"change1.wav"
	,"scroll.wav"
	,"warning_food.wav"
	,"warning_drain.wav"
	,"warning_statup.wav"
	,"warning_statdown.wav"
	,"warning_hp.wav"
	,"warning_hp2.wav"
	,"weapon_attack.wav"
	,"weapon_hold.wav"
	,"get.wav"
};

static char* spell_sound_files[SPELL_SOUND_MAX] = {

	"magic_default.wav"
	,"magic_acid.wav"
	,"magic_animate.wav"
	,"magic_avatar.wav"
	,"magic_bomb.wav"
	,"magic_bullet1.wav"
	,"magic_bullet2.wav"
	,"magic_cancel.wav"
	,"magic_comet.wav"
	,"magic_confusion.wav"
	,"magic_create.wav"
	,"magic_dark.wav"
	,"magic_death.wav"
	,"magic_destruction.wav"
	,"magic_elec.wav"
	,"magic_fear.wav"
	,"magic_fire.wav"
	,"magic_fireball1.wav"
	,"magic_fireball2.wav"
	,"magic_hword.wav"
	,"magic_ice.wav"
	,"magic_invisible.wav"
	,"magic_invoke.wav"
	,"magic_invoke2.wav"
	,"magic_magic.wav"
	,"magic_manaball.wav"
	,"magic_missile.wav"
	,"magic_mmap.wav"
	,"magic_orb.wav"
	,"magic_paralyze.wav"
	,"magic_poison.wav"
	,"magic_protection.wav"
	,"magic_rstrike.wav"
	,"magic_rune.wav"
	,"magic_sball.wav"
	,"magic_slow.wav"
	,"magic_snowstorm.wav"
	,"magic_stat.wav"
	,"magic_steambolt.wav"
	,"magic_summon1.wav"
	,"magic_summon2.wav"
	,"magic_summon3.wav"
	,"magic_teleport.wav"
	,"magic_turn.wav"
	,"magic_wall.wav"
	,"magic_walls.wav"
	,"magic_wound.wav"
};

#endif

/* this value is defined in server too - change only both at once */
#define MAX_SOUND_DISTANCE 12

static void musicDone(void); /* callback function for background music */
static void sound_start_music(char *fname, int vol, int fade, int loop);


void sound_init(void)
{
#ifdef INSTALL_SOUND
	/* we want no sound*/
	if(SoundSystem == SOUND_SYSTEM_NONE)
		return;
	
	music.flag = 0;
	music.data = NULL;
	music_new.flag = 0;
	music_new.data = NULL;
	music_buffer.flag = 0;
	music_buffer.data = NULL;

	SoundSystem = SOUND_SYSTEM_OFF;

	/* Open the audio device */
    if ( Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16,MIX_DEFAULT_CHANNELS, 1024) < 0 )
	{
            LOG(LOG_MSG,"Warning: Couldn't set sound device. Reason: %s\n", SDL_GetError());
			return;
	}

#endif
	SoundSystem = SOUND_SYSTEM_ON;
}

void sound_deinit(void)
{
#ifdef INSTALL_SOUND
    if(SoundSystem == SOUND_SYSTEM_ON)
		Mix_CloseAudio();
#endif
}

/* we are loading here all different sound groups in one array.
 */
void sound_loadall(void)
{
#ifdef INSTALL_SOUND
    register int i,ii;
        char buf[2048];

        if(SoundSystem != SOUND_SYSTEM_ON)
                return;

        for(i=0;i<SOUND_MAX;i++)
        {
            sprintf(buf,"%s%s", GetSfxDirectory(), sound_files[i]);
			Sounds[i].sound = NULL;
            Sounds[i].sound = Mix_LoadWAV(buf);
			if(!Sounds[i].sound)
				LOG(LOG_ERROR,"sound_loadall: missing sound file %s\n", buf);
        }
        for(ii=0;ii<SPELL_SOUND_MAX;ii++)
        {
            sprintf(buf,"%s%s", GetSfxDirectory(), spell_sound_files[ii]);
            Sounds[i+ii].sound = NULL;
            Sounds[i+ii].sound = Mix_LoadWAV(buf);
			if(!Sounds[i+ii].sound)
				LOG(LOG_ERROR,"sound_loadall: missing sound file %s\n", buf);
        }
#endif
}

void sound_freeall(void)
{
#ifdef INSTALL_SOUND
    
        register int i;

        if(SoundSystem != SOUND_SYSTEM_ON)
                return;

        for(i=0;i<SOUND_MAX+SPELL_SOUND_MAX;i++)
        {
                Mix_FreeChunk(Sounds[i].sound);
        }
#endif
}


void calculate_map_sound(int soundnr, int xoff, int yoff, int flags)
{
    /* we got xoff/yoff relative to 0, when this will change, exchange 0
     * with the right default position */
#ifdef INSTALL_SOUND
    int pane,distance=isqrt(POW2(0 - xoff)+POW2(0 - yoff))-1;

    if(SoundSystem != SOUND_SYSTEM_ON)
		return;
    
    if(distance <0)
        distance = 0;
    distance = 100-distance * (100/MAX_SOUND_DISTANCE); /* thats our real volume in % */

    /* now we set the panning.
     * Because reducing volume from one to another reduce volume too,
     * we used only yoff real volume.
     */
    pane = isqrt(POW2(0 - xoff))-1;
    if(pane <0)
        pane = 0;
    pane = pane * (100/MAX_SOUND_DISTANCE); /* thats "% use of left or right speaker" */
    /* note that as higher is the xoff distance, so more we use one direction
       only */
    
    pane = (int)((double) pane * ((double)255/(double)100));
    if(xoff<0) /* now mark this is left or right pane. left is *(-1) */
        pane *=-1;

    sound_play_effect(soundnr,0, pane, distance);
#endif
}

/* Play a sound.
   we define panning as -255 (total left) or +255 (total right)
   Return: Channel (id) of the sound. -1 = error
   */
int sound_play_effect(int soundid,uint32 flag, int pan, int vol)
{
#ifdef INSTALL_SOUND
    int tmp;

        if(SoundSystem != SOUND_SYSTEM_ON)
                return -1;

        tmp = Mix_PlayChannel(-1,Sounds[soundid].sound,0);
        if(tmp != -1)
        {
            int l=255,r=255;

            Mix_Volume(tmp, (int)(((double)options.sound_volume/(double)100) *((double) vol * ((double)MIX_MAX_VOLUME/(double)100 ))));
            if(pan <0)
            {
                l=255;
                r=255+pan; 
            }
            else if(pan >0)
            {
                l=255-pan;
                r=255; 
            }

            Mix_SetPanning(tmp, (Uint8) l, (Uint8) r);
        }
        return tmp;
#else
    return -1;
#endif
}

/* Test for playing. 
   0: Sound is not playing, 1: sound is still playing
   */
int sound_test_playing(int channel)
{
#ifdef INSTALL_SOUND

    if(SoundSystem != SOUND_SYSTEM_ON)
        return 0;
    return Mix_Playing(channel);

#else
return 0;
#endif
}

void sound_play_one_repeat(int soundid, int special_id)
{
#ifdef INSTALL_SOUND
    
    int tmp,s;

    if(SoundSystem != SOUND_SYSTEM_ON)
        return;

    if(special_sounds[special_id]!=-1)
    {
        if(Mix_Playing(special_sounds[special_id]))
            return;
    }
    tmp = Mix_PlayChannel(-1,Sounds[soundid].sound,0);
    if(tmp == -1)
    {
        /* we failed... */
        special_sounds[special_id]=-1;
        return;
    }
    Mix_Volume(tmp, (int) ( ((double)options.sound_volume/(double)100) *(double) MIX_MAX_VOLUME) );
    
    /* thats the wild part: when we got a channel, we must delete every same old entry */
    for (s=0;s<SPECIAL_SOUND_INIT;s++)
    {
        if(special_sounds[s]==tmp)
            special_sounds[s]=-1;
    }
    special_sounds[special_id]=tmp;
    
#endif
}

void sound_play_music(char *fname, int vol, int fade, int loop, int flags, int mode)
{
#ifdef INSTALL_SOUND
	int vol2=vol;

	if(SoundSystem != SOUND_SYSTEM_ON)
		return;

	if(mode & MUSIC_MODE_DIRECT)
		fade = 0;
	else if(mode & MUSIC_MODE_FORCED)
		vol2 = 100;

	if(music.flag && !(mode & MUSIC_MODE_DIRECT)) /* only when set, we still play something */
	{
		music_new.flag=1;
		music_new.loop=loop;
		music_new.fade=fade;
		music_new.vol=vol;
		strcpy(music_new.name,fname);
		sound_fadeout_music(music_new.flag);        /* lets fade out old music */
		/* the music_new will be fired in the music hook function after fadeout */
	}
	else /* no playing music, we fire our new music up */
	{
		/* we don't care about the old buffer data - when we overwrite is always right */
		music_new.flag=0;
		sound_start_music(fname,vol2 ,fade,loop);
	}

#endif
}


static void sound_start_music(char *fname, int vol, int fade, int loop)
{
#ifdef INSTALL_SOUND
    char buf[4096];

    if(SoundSystem != SOUND_SYSTEM_ON)
		return;
    /* try to load the mp3 */
    sprintf(buf,"%s%s",GetMediaDirectory(), fname);

	if(music.data)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(music.data);
		music.data = NULL;
		music.flag = 0;
	}

	music.data = Mix_LoadMUS(buf);
	if(!music.data)
	{
		draw_info("mix_loadmus() failed.",4);
		return;
	}
	music.fade =fade;
	music.loop = loop;
	music.vol = vol;
	music.flag = 1;
	strcpy(music.name, fname);

	Mix_VolumeMusic(vol);
	if(fade)
		Mix_FadeInMusic(music.data, loop, fade);
	else
		Mix_PlayMusic(music.data, loop);
	Mix_HookMusicFinished(musicDone);

 #endif
}

void sound_fadeout_music(int i)
{
#ifdef INSTALL_SOUND
    if(SoundSystem != SOUND_SYSTEM_ON)
		return;

	if(music.flag)
	{
		if(!Mix_FadeOutMusic(4000)) /* give a fadeout cmd to the music buffer */
		{
			/* fadeout has failed - buffer is busy or a different fade is still on the way */
			music_new.flag = i;
			/* now set the global main loop marker - we poll it for the hard way */
			music_global_fade = TRUE;
			return;
		}
		/* all ok, we fade out and the callback will do the rest */
		music_new.flag = i;
	}
	music_global_fade = FALSE;

#endif
}

/* callback function from current played music sound */
static void musicDone(void)
{
#ifdef INSTALL_SOUND
	if(music.data)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(music.data);
		music.data = NULL;
		music.flag = 0;
	}

  if(music_new.flag)
  {
	  sound_start_music(music_new.name, options.music_volume,music_new.fade,music_new.loop);
	  music_new.flag=0;
  }
	music_global_fade = FALSE;
#endif
}

