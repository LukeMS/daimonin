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

    The author can be reached via e-mail to info@daimonin.org
*/

#include "include.h"

_sound_system       SoundSystem; /* THE SoundSystem status*/

#ifdef INSTALL_SOUND

music_data          music;           /* thats the music we just play - if NULL, no music */
music_data          music_new;       /* when we get a new piece of music, we store it here and
                                             * give the current music the command to fade out or
                                             * to break (if new music parameter force it).
                                             * then we copy new_music to music and start it
                                             */
static music_data   music_buffer;        /* when we got a "special" music like a map position
                                            * music command (for example a NPC shout or something
                                            * like this) then we need to play it ASAP - and when
                                            * we are finished with it, we need to restore the
                                            * old music we perhaps had interrupted. the old music
                                            * is stored here
                                            */

static int          special_sounds[SPECIAL_SOUND_INIT];
#endif

#define POW2(x) ((x) * (x))


#ifdef INSTALL_SOUND

static _sounds sounds = {0, NULL};

#endif

/* this value is defined in server too - change only both at once */
#define MAX_SOUND_DISTANCE 12

static void musicDone(void); /* callback function for background music */
static void sound_start_music(char *fname, int vol, int fade, int loop);

// Helper string function
// Duplicate string
char *str_dup(const char *str)
{
    char *ret;

    MALLOC_STRING(ret, str);

    return ret;
}

void read_sounds(void)
{
#ifdef INSTALL_SOUND
    FILE *stream;

    srv_client_files[SRV_CLIENT_SOUNDS].len = 0;
    srv_client_files[SRV_CLIENT_SOUNDS].crc = 0;
    LOG(LOG_DEBUG, "Reading %s....", FILE_CLIENT_SOUNDS);

    if ((stream = fopen_wrapper(FILE_CLIENT_SOUNDS, "rb")) != NULL)
    {
        struct stat    statbuf;
        int            i;
        unsigned char *temp_buf;
        size_t         dummy; // purely to avoid GCC's warn_unused_result warning

        /* temp load the file and get the data we need for compare with
         * server. */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_SOUNDS].len = i;
        MALLOC(temp_buf, i);
        dummy = fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_SOUNDS].crc = crc32(1L, temp_buf, i);
        FREE(temp_buf);
        fclose(stream);
        LOG(LOG_DEBUG, " found file!(%d/%x)",
            srv_client_files[SRV_CLIENT_SOUNDS].len,
            srv_client_files[SRV_CLIENT_SOUNDS].crc);
    }

    LOG(LOG_DEBUG, " done.\n");
#endif
}

/* Load the sounds file */
void load_sounds(void)
{
#ifdef INSTALL_SOUND
    char    buf[64];
    FILE    *stream;
    int     state = 0;
    char    name[32];
    char    file[64];
    int     type_count  = 0;
    int     type_index  = -1;
    int     sound_count = 0;
    int     sound_index = -1;

    if (!(stream = fopen_wrapper(FILE_CLIENT_SOUNDS, "rb")))
    {
        LOG(LOG_ERROR,"ERROR: Can't find file %s.\n", FILE_CLIENT_SOUNDS);
        return;
    }
    while (fgets(buf, sizeof(buf), stream) != NULL)
    {
        // Strip trailing newline character(s) (allow for \r\n or \n)
        buf[strcspn(buf, "\r\n")] = '\0';

        if ((strlen(buf) == 0) || (buf[0] == '#'))
            continue;

        if (!strcmp(buf, "*end"))
            break;

        switch (state)
        {
        case 0:
            // Looking for start line
            if (strncmp(buf, "*start", 6) == 0)
            {
                strtok(buf, "|"); // discard *start
                sscanf(strtok(NULL, "|"), "%d", &type_count); // count of soundtypes
                sounds.count = type_count;

                // Allocate memory
                MALLOC(sounds.types, type_count * sizeof(_soundtype));

                state++;
            }
            break;

        case 1:
            // Looking for soundtype introducer
            if ((type_count > 0) && (buf[0] == '*'))
            {
                // New soundtype
                type_count--;
                type_index++;
                sscanf(strtok(buf, "|"), "*%d", &sounds.types[type_index].id);
                strcpy(name, strtok(NULL, "|"));
                strtok(NULL, "|");  // discard prefix
                sscanf(strtok(NULL, "|"), "%d", &sound_count);
                sounds.types[type_index].count = sound_count;
                sounds.types[type_index].name = str_dup(name);
                MALLOC(sounds.types[type_index].sounds, sound_count * sizeof(_sound)); // space for sounds
                sound_index = -1;
                state++;
            }
            break;

        case 2:
            // Process sound
            if ((sound_count > 0) && (buf[0] == '+'))
            {
                // Process sound
                sound_count--;
                sound_index++;
                sscanf(strtok(buf, "|"), "+%d", &sounds.types[type_index].sounds[sound_index].id);
                strcpy(name, strtok(NULL, "|"));
                strcpy(file, strtok(NULL, "|"));
                sounds.types[type_index].sounds[sound_index].name = str_dup(name);
                sounds.types[type_index].sounds[sound_index].file = str_dup(file);
            }

            if (sound_count == 0)
                state--;            // Look for next soundtype
            break;
        }
    }

    fclose(stream);
#endif
}

void sound_init(void)
{
#ifdef INSTALL_SOUND
    int i;

    /* we want no sound*/
    if (SoundSystem == SOUND_SYSTEM_NONE)
        return;

    music.flag = 0;
    music.data = NULL;
    music_new.flag = 0;
    music_new.data = NULL;
    music_buffer.flag = 0;
    music_buffer.data = NULL;

    SoundSystem = SOUND_SYSTEM_OFF;

    for (i=0;i < SPECIAL_SOUND_INIT;i++)
        special_sounds[i] = -1;

    /* Open the audio device */
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 1024) < 0)
    {
        LOG(LOG_MSG, "Warning: Couldn't set sound device. Reason: %s\n", SDL_GetError());
        return;
    }

#endif
    SoundSystem = SOUND_SYSTEM_ON;
}

void sound_deinit(void)
{
#ifdef INSTALL_SOUND
    if (SoundSystem == SOUND_SYSTEM_ON)
        Mix_CloseAudio();
#endif
}

/* we are loading here all different sound groups in one array.
 */
void sound_loadall(void)
{
#ifdef INSTALL_SOUND
    int     i, j;

    if (SoundSystem != SOUND_SYSTEM_ON)
        return;

    load_sounds();

    for (i = 0; i < sounds.count; i++)
    {
        for (j = 0; j < sounds.types[i].count; j++)
        {
            const char *fname = sounds.types[i].sounds[j].file;
            char        buf[MEDIUM_BUF];
            SDL_RWops  *rw;

            sprintf(buf, "%s%s", GetSfxDirectory(), fname);

            if ((rw = PHYSFSRWOPS_openRead(buf)))
            {
                sounds.types[i].sounds[j].sound = Mix_LoadWAV_RW(rw, 1);
            }

            if (!sounds.types[i].sounds[j].sound)
            {
                LOG(LOG_ERROR, "sound_loadall: missing sound file %s\n",
                    fname);
            }
        }
    }

#endif
}

void sound_freeall(void)
{
#ifdef INSTALL_SOUND

    register int i, j;

    if (SoundSystem != SOUND_SYSTEM_ON)
        return;

    for (i = 0; i < sounds.count; i++)
    {
        FREE(sounds.types[i].name);
        for (j = 0; j < sounds.types[i].count; j++)
        {
            FREE(sounds.types[i].sounds[j].name);
            FREE(sounds.types[i].sounds[j].file);
            Mix_FreeChunk(sounds.types[i].sounds[j].sound);
        }
        FREE(sounds.types[i].sounds);
    }
    FREE(sounds.types);
#endif
}

#ifdef INSTALL_SOUND
Mix_Chunk *find_sound_by_id(int type_id, int sound_id)
{
    _soundtype  *type = NULL;
    Mix_Chunk   *sound = NULL;
    int          i;

    for (i = 0; i < sounds.count; i++)
    {
        if (sounds.types[i].id == type_id)
        {
            type = &sounds.types[i];
            break;
        }
    }

    if (type)
    {
        for (i = 0; i < type->count; i++)
        {
            if (type->sounds[i].id == sound_id)
            {
                sound = type->sounds[i].sound;
                break;
            }
        }
    }
    return sound;
}
#endif

void calculate_map_sound(int type_id, int sound_id, int xoff, int yoff, int flags)
{
    /* we got xoff/yoff relative to 0, when this will change, exchange 0
     * with the right default position */
#ifdef INSTALL_SOUND
    int pane, distance = isqrt(POW2(0 - xoff) + POW2(0 - yoff)) - 1;

    if (SoundSystem != SOUND_SYSTEM_ON)
        return;

    if (distance < 0)
        distance = 0;
    distance = 100 - distance * (100 / MAX_SOUND_DISTANCE); /* thats our real volume in % */

    /* now we set the panning.
     * Because reducing volume from one to another reduce volume too,
     * we used only yoff real volume.
     */
    pane = isqrt(POW2(0 - xoff)) - 1;
    if (pane < 0)
        pane = 0;
    pane = pane * (100 / MAX_SOUND_DISTANCE); /* thats "% use of left or right speaker" */
    /* note that as higher is the xoff distance, so more we use one direction
       only */

    pane = (int) ((double) pane * ((double) 255 / (double) 100));
    if (xoff < 0) /* now mark this is left or right pane. left is *(-1) */
        pane *= -1;

    sound_play_effect(type_id, sound_id, 0, pane, distance);
#endif
}

/* Play a sound.
   we define panning as -255 (total left) or +255 (total right)
   Return: Channel (id) of the sound. -1 = error
   */
int sound_play_effect(int type_id, int sound_id, uint32 flag, int pan, int vol)
{
#ifdef INSTALL_SOUND
    int tmp;

    if (SoundSystem != SOUND_SYSTEM_ON)
        return -1;

    tmp = Mix_PlayChannel(-1, find_sound_by_id(type_id, sound_id), 0);
    if (tmp != -1)
    {
        int l = 255, r = 255;

        Mix_Volume(tmp,
                   (int)
                   (((double) options.sound_volume / (double) 100) * ((double)
                           vol * ((double) MIX_MAX_VOLUME / (double) 100))));
        if (pan < 0)
        {
            l = 255;
            r = 255 + pan;
        }
        else if (pan > 0)
        {
            l = 255 - pan;
            r = 255;
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

    if (SoundSystem != SOUND_SYSTEM_ON)
        return 0;
    return Mix_Playing(channel);

#else
    return 0;
#endif
}

void sound_play_one_repeat(int type_id, int sound_id, int special_id)
{
#ifdef INSTALL_SOUND

    int tmp;

    if (SoundSystem != SOUND_SYSTEM_ON)
        return;

    if (special_sounds[special_id] != -1)
    {
        if (Mix_Playing(special_sounds[special_id]))
            return;
    }
    tmp = Mix_PlayChannel(-1, find_sound_by_id(type_id, sound_id), 0);
    if (tmp == -1)
    {
        /* we failed... */
        special_sounds[special_id] = -1;
        return;
    }
    special_sounds[special_id] = tmp;
    Mix_Volume(tmp, (int) (((double) options.sound_volume / (double) 100) * (double) MIX_MAX_VOLUME));

#endif
}

void sound_play_music(char *fname, int vol, int fade, int loop, int flags, int mode)
{
#ifdef INSTALL_SOUND
    int vol2    = vol;

    if (SoundSystem != SOUND_SYSTEM_ON)
        return;

    if (mode & MUSIC_MODE_DIRECT)
        fade = 0;
    else if (mode & MUSIC_MODE_FORCED)
        vol2 = 100;

    /* same sound? */
    if (music.data && !strcmp(fname, music.name))
    {
        music.fade = fade;
        music.loop = loop;
        if (vol != music.vol)
        {
            music.vol = vol;
            Mix_VolumeMusic(vol);
        }
        return;
    }

    if (music.flag && !(mode & MUSIC_MODE_DIRECT)) /* only when set, we still play something */
    {
        music_new.flag = 1;
        music_new.loop = loop;
        music_new.fade = fade;
        music_new.vol = vol;
        strcpy(music_new.name, fname);
        sound_fadeout_music(music.flag);        /* lets fade out old music */
        /* the music_new will be fired in the music hook function after fadeout */
    }
    else /* no playing music, we fire our new music up */
    {
        /* we don't care about the old buffer data - when we overwrite is always right */
        music_new.flag = 0;
        sound_start_music(fname, vol2, fade, loop);
    }

#endif
}


static void sound_start_music(char *fname, int vol, int fade, int loop)
{
#ifdef INSTALL_SOUND
    char       buf[MEDIUM_BUF];
    SDL_RWops *rw;

    if (SoundSystem != SOUND_SYSTEM_ON ||
        !fname)
    {
        return;
    }

    if (music.data)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music.data);
        music.data = NULL;
        music.flag = 0;
    }

    sprintf(buf, "%s%s", GetMediaDirectory(), fname);

    /* try to load the music */
    if ((rw = PHYSFSRWOPS_openRead(buf)))
    {
        music.data = Mix_LoadMUS_RW(rw);
    }

    if (!music.data)
    {
#ifdef DAI_DEVELOPMENT
        textwin_showstring(COLOR_GREEN, "mix_loadmus() failed (%s).", buf);
#endif
        return;
    }
    music.fade = fade;
    music.loop = loop;
    music.vol = vol;
    music.flag = 1;
    strcpy(music.name, fname);

    Mix_VolumeMusic(vol);
    if (fade)
        Mix_FadeInMusic(music.data, loop, fade);
    else
        Mix_PlayMusic(music.data, loop);
    Mix_HookMusicFinished(musicDone);

#endif
}

void sound_fadeout_music(int i)
{
#ifdef INSTALL_SOUND
    if (SoundSystem != SOUND_SYSTEM_ON)
        return;

    if (music.flag)
    {
        if (!Mix_FadeOutMusic(4000)) /* give a fadeout cmd to the music buffer */
        {
            /* fadeout has failed - buffer is busy or a different fade is still on the way */
            music_new.flag = i;
            /* now set the global main loop marker - we poll it for the hard way */
            music_global_fade = 1;
            return;
        }
        /* all ok, we fade out and the callback will do the rest */
        music_new.flag = i;
    }
    music_global_fade = 0;

#endif
}

/* callback function from current played music sound */
static void musicDone(void)
{
#ifdef INSTALL_SOUND
    if (music.data)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music.data);
        music.data = NULL;
        music.flag = 0;
    }

    if (music_new.flag)
    {
        sound_start_music(music_new.name, options.music_volume, music_new.fade, music_new.loop);
        music_new.flag = 0;
    }
    music_global_fade = 0;
#endif
}

