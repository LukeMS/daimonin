/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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

/*****************************************************************************/
/* First, the headers. We only include plugin.h, because all other includes  */
/* are done into it, and plugproto.h (which is used only by this file).      */
/*****************************************************************************/
#include <global.h>
#include <plugproto.h>

void (*registerHooksFunc)(struct plugin_hooklist *hooks);

struct plugin_hooklist  hooklist    =
{
    LOG, create_pathname, re_cmp,
    new_draw_info, new_draw_info_format,
    new_info_map, new_info_map_except, map_brightness, wall,
    free_string_shared, add_string, add_refcount,
    fix_player, esrv_send_item, esrv_send_inventory,
    lookup_skill_by_name, look_up_spell_name,
    insert_ob_in_ob, insert_ob_in_map, move_ob,
    free_mempool, create_mempool, nearest_pow_two_exp,
    return_poolchunk_array_real, get_poolchunk_array_real,
    arch_to_object, find_archetype,
    register_npc_known_obj,
    get_rangevector, get_rangevector_from_mapcoords,
    get_archetype,
    play_sound_player_only,
    add_money_to_player,
    drop_ob_inv,
    decrease_ob_nr,
    add_quest_containers, add_quest_trigger,
    set_quest_status, spring_trap,cast_spell,play_sound_map,
    find_skill, find_animation, FindFace,
    get_tod, query_money, query_cost,
    cost_string_from_value,pay_for_item,pay_for_amount,
    get_word_from_string,get_money_from_string,sell_item,
    query_money_type, remove_money_type,insert_money_in_player,
    add_pet, material_repair_cost, material_repair_item,
	query_short_name,
	find_artifact,
	give_artifact_abilities,
	find_string,
	get_nrof_quest_item,
	is_player_inv,

    /* global variables */
    &animations, &new_faces, global_darkness_table, coins_arch,
    &shstr_cons
};

CFPlugin                PlugList[PLUGINS_MAX_NROF];
int                     PlugNR      = 0;

/* get_event_object()
 * browse through the event_obj chain of the given object the
 * get a inserted script object from it.
 * 1: object
 * 2: EVENT_NR
 * return: script object matching EVENT_NR
 */
object * get_event_object(object *op, int event_nr)
{
    register object * tmp;
    /* for this first implementation we simply browse
     * through the inventory of object op and stop
     * when we find a script object from type event_nr.
     */
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
    {
        if (tmp->type == TYPE_EVENT_OBJECT && tmp->sub_type1 == event_nr)
            return tmp;
    }
    return tmp;
}

int trigger_object_plugin_event(
        int event_type,
        /* value[2], value[1], value[3] */
        object *me, object *activator, object *other,
        const char *msg,
        int *parm1, int *parm2, int *parm3,
        int flags)
{
    CFParm  CFP;
    CFParm *CFR;
    object *event_obj;
    int plugin;

    if(me == NULL || !(me->event_flags & EVENT_FLAG(event_type)))
        return 0;

    if((event_obj = get_event_object(me, event_type)) == NULL)
    {
        LOG(llevBug, "BUG: object with event flag and no event object: %s\n", STRING_OBJ_NAME(me));
        me->event_flags &= ~(1 << event_type);
        return 0;
    }

    /* Avoid double triggers and infinite loops */
    /* Currently only enabled for SAY and TRIGGER events, as
     * those are the ones we have experienced problems with */
    if(event_type == EVENT_SAY || event_type == EVENT_TRIGGER)
    {
        if(event_obj->damage_round_tag == pticks)
        {
            LOG(llevDebug, "trigger_object_plugin_event(): event object (type %d) for %s called twice the same round\n", event_type, STRING_OBJ_NAME(me));
            return 0;
        }
        event_obj->damage_round_tag = pticks;
    }

    CFP.Value[0] = &event_type;
    CFP.Value[1] = activator;
    CFP.Value[2] = me;
    CFP.Value[3] = other;
    CFP.Value[4] = (void *)msg;
    CFP.Value[5] = parm1;
    CFP.Value[6] = parm2;
    CFP.Value[7] = parm3;
    CFP.Value[8] = &flags;
    CFP.Value[9] = (char *) event_obj->race;
    CFP.Value[10] = (char *) event_obj->slaying;
    CFP.Value[11] = NULL;

    if (event_obj->name && (plugin = findPlugin(event_obj->name)) >= 0)
    {
#ifdef TIME_SCRIPTS
            int             count   = 0;
            struct timeval  start, stop;
            long long   start_u, stop_u;
            gettimeofday(&start, NULL);

            for (count = 0; count < 10000; count++)
                CFR = ((PlugList[plugin].eventfunc) (&CFP));

            gettimeofday(&stop, NULL);
            start_u = start.tv_sec * 1000000 + start.tv_usec;
            stop_u  = stop.tv_sec * 1000000 + stop.tv_usec;

            LOG(llevDebug, "running time: %2.4f s\n", (stop_u - start_u) / 1000000.0);
#else
        CFR = ((PlugList[plugin].eventfunc) (&CFP));
#endif
        /* TODO: we could really use a more efficient event interface */
        if(CFR && CFR->Value[0])
            return *(int *) (CFR->Value[0]);
    }
    else
    {
        LOG(llevBug, "BUG: event object with unknown plugin: %s, plugin %s\n", STRING_OBJ_NAME(me), STRING_OBJ_NAME(event_obj));
        me->event_flags &= ~(1 << event_type);
    }

    return 0;
}

/*****************************************************************************/
/* Tries to find if a given command is handled by a plugin.                  */
/* Note that find_plugin_command is called *before* the internal commands are*/
/* checked, meaning that you can "overwrite" them.                           */
/*****************************************************************************/
CommArray_s * find_plugin_command(const char *cmd, object *op)
{
    CFParm              CmdParm;
    CFParm             *RTNValue;
    int                 i;
    char                cmdchar[10];
    static CommArray_s  RTNCmd;

    strcpy(cmdchar, "command?");
    CmdParm.Value[0] = cmdchar;
    CmdParm.Value[1] = (char *) cmd;
    CmdParm.Value[2] = op;

    for (i = 0; i < PlugNR; i++)
    {
        RTNValue = (PlugList[i].propfunc(&CmdParm));
        if (RTNValue != NULL)
        {
            RTNCmd.name = (char *) (RTNValue->Value[0]);
            RTNCmd.func = (CommFunc) (RTNValue->Value[1]);
            RTNCmd.time = *(float *) (RTNValue->Value[2]);
            LOG(llevInfo, "RTNCMD: name %s, time %f\n", RTNCmd.name, RTNCmd.time);
            return &RTNCmd;
        }
    }
    return NULL;
}

/*****************************************************************************/
/* Displays a list of loaded plugins (keystrings and description) in the     */
/* game log window.                                                          */
/*****************************************************************************/
void displayPluginsList(object *op)
{
    char    buf[MAX_BUF];
    int     i;

    new_draw_info(NDI_UNIQUE, 0, op, "List of loaded plugins:");
    new_draw_info(NDI_UNIQUE, 0, op, "-----------------------");
    for (i = 0; i < PlugNR; i++)
    {
        strcpy(buf, PlugList[i].id);
        strcat(buf, ", ");
        strcat(buf, PlugList[i].fullname);
        new_draw_info(NDI_UNIQUE, 0, op, buf);
    }
}

/*****************************************************************************/
/* Searches in the loaded plugins list for a plugin with a keyname of id.    */
/* Returns the position of the plugin in the list if a matching one was found*/
/* or -1 if no correct plugin was detected.                                  */
/*****************************************************************************/
int findPlugin(const char *id)
{
    int i;
    for (i = 0; i < PlugNR; i++)
        if (!strcasecmp(id, PlugList[i].id))
            return i;
    return -1;
}

#ifdef WIN32
/*****************************************************************************/
/* WIN32 Plugins initialization. Browses the plugins directory and call      */
/* initOnePlugin for each file found.                                        */
/*****************************************************************************/
void initPlugins(void)
{
    struct dirent  *currentfile;
    DIR            *plugdir;
    int             n;
    char            buf[MAX_BUF];
    char            buf2[MAX_BUF];

    LOG(llevInfo, "Now initializing plugins\n");
    /* strcpy(buf,DATADIR); dlls should not part of DATADIR or LIBDOR */
    /* strcpy(buf,"./plugins/"); */
    strcpy(buf, PLUGINDIR"/");
    LOG(llevInfo, "Plugins directory is %s\n", buf);

    if (!(plugdir = opendir(buf)))
        return;

    n = 0;

    while (currentfile = readdir(plugdir))
    {
        if (strcmp(currentfile->d_name, ".."))
        {
            /* don't load "." marker, CVS directory or all which has a .txt inside */
            if (strcmp(currentfile->d_name, ".") && !strstr(currentfile->d_name, ".txt") && strcmp(currentfile->d_name,
                                                                                                   "CVS"))
            {
                strcpy(buf2, buf);
                strcat(buf2, currentfile->d_name);
                LOG(llevInfo, "Registering plugin %s\n", currentfile->d_name);
                initOnePlugin(buf2);
            }
        }
    }
    closedir(plugdir);
}

/*****************************************************************************/
/* WIN32 Plugin initialization. Initializes a plugin known by its filename.  */
/* The initialization process has several stages:                            */
/* - Loading of the DLL itself;                                              */
/* - Basical plugin information request;                                     */
/* - CF-Plugin specific initialization tasks (call to initPlugin());         */
/* - Hook bindings;                                                          */
/*****************************************************************************/
void initOnePlugin(const char *pluginfile)
{
    int     i   = 0;
    HMODULE DLLInstance;
    void   *ptr = NULL;
    CFParm *HookParm;

    if ((DLLInstance = LoadLibrary(pluginfile)) == NULL)
    {
        LOG(llevBug, "BUG: Error while trying to load %s\n", pluginfile);
        return;
    }
    PlugList[PlugNR].libptr = DLLInstance;
    PlugList[PlugNR].initfunc = (f_plugin) (GetProcAddress(DLLInstance, "initPlugin"));
    if (PlugList[PlugNR].initfunc == NULL)
    {
        LOG(llevBug, "BUG: Plugin init error\n");
        FreeLibrary(ptr);
        return;
    }
    else
    {
        CFParm *InitParm;

        /* We must send the hooks first of all, so the plugin can use the LOG function */
        if ((registerHooksFunc = (void *) GetProcAddress(DLLInstance, "registerHooks")))
        {
            registerHooksFunc(&hooklist);
        }

        InitParm = PlugList[PlugNR].initfunc(NULL);
        LOG(llevInfo, "Plugin name: %s, known as %s\n", (char *) (InitParm->Value[1]), (char *) (InitParm->Value[0]));
        strcpy(PlugList[PlugNR].id, (char *) (InitParm->Value[0]));
        strcpy(PlugList[PlugNR].fullname, (char *) (InitParm->Value[1]));
    }
    PlugList[PlugNR].removefunc = (f_plugin) (GetProcAddress(DLLInstance, "removePlugin"));
    PlugList[PlugNR].hookfunc = (f_plugin) (GetProcAddress(DLLInstance, "registerHook"));
    PlugList[PlugNR].eventfunc = (f_plugin) (GetProcAddress(DLLInstance, "triggerEvent"));
    PlugList[PlugNR].pinitfunc = (f_plugin) (GetProcAddress(DLLInstance, "postinitPlugin"));
    PlugList[PlugNR].propfunc = (f_plugin) (GetProcAddress(DLLInstance, "getPluginProperty"));
    if (PlugList[PlugNR].pinitfunc == NULL)
    {
        LOG(llevBug, "BUG: Plugin postinit error\n");
        FreeLibrary(ptr);
        return;
    }

    for (i = 0; i < NR_EVENTS; i++)
        PlugList[PlugNR].gevent[i] = 0;
    if (PlugList[PlugNR].hookfunc == NULL)
    {
        LOG(llevBug, "BUG: Plugin hook error\n");
        FreeLibrary(ptr);
        return;
    }
    else
    {
        int j;
        i = 0;
        HookParm = (CFParm *) (malloc(sizeof(CFParm)));
        HookParm->Value[0] = (int *) (malloc(sizeof(int)));

        for (j = 1; j <= NR_OF_HOOKS; j++)
        {
            memcpy(HookParm->Value[0], &j, sizeof(int));
            HookParm->Value[1] = HookList[j];

            PlugList[PlugNR].hookfunc(HookParm);
        }
        free(HookParm->Value[0]);
        free(HookParm);
    }
    if (PlugList[PlugNR].eventfunc == NULL)
    {
        LOG(llevBug, "BUG: Event plugin error\n");
        FreeLibrary(ptr);
        return;
    }
    PlugNR++;
    PlugList[PlugNR - 1].pinitfunc(NULL);
    LOG(llevInfo, "Done\n");
}

/*****************************************************************************/
/* Removes one plugin from memory. The plugin is identified by its keyname.  */
/*****************************************************************************/
void removeOnePlugin(const char *id)
{
    int plid;
    int j;

    /* what that warning means MT-2005 */
    /* LOG(llevDebug, "Warning - removeOnePlugin non-canon under Win32\n"); */
    plid = findPlugin(id);
    if (plid < 0)
        return;
    if (PlugList[plid].removefunc != NULL)
        PlugList[plid].removefunc(NULL);
    /* We unload the library... */
    FreeLibrary(PlugList[plid].libptr);
    /* Then we copy the rest on the list back one position */
    PlugNR--;
    if (plid == 31)
        return;
    for (j = plid + 1; j < 32; j++)
    {
        PlugList[j - 1] = PlugList[j];
    }
}

#else

#ifndef HAVE_SCANDIR

extern int  alphasort(struct dirent **a, struct dirent **b);
#endif

/*****************************************************************************/
/* UNIX Plugins initialization. Browses the plugins directory and call       */
/* initOnePlugin for each file found.                                        */
/*****************************************************************************/
void initPlugins(void)
{
    struct dirent  **namelist   = NULL;
    int             n;
    char            buf[MAX_BUF];
    char            buf2[MAX_BUF];

    LOG(llevInfo, "Initializing plugins :\n");
    /*        strcpy(buf,DATADIR);
            strcat(buf,"/../plugins/");*/
    strcpy(buf, PLUGINDIR"/");
    LOG(llevInfo, "Plugins directory is %s\n", buf);
    n = scandir(buf, &namelist, 0, alphasort);
    if (n < 0)
        LOG(llevBug, "BUG: plugins.c: scandir...\n");
    else
        while (n--)
        {
            if (strcmp(namelist[n]->d_name, ".."))
            {
                /* don't load "." marker, CVS directory or all which has a .txt inside */
                if (strcmp(namelist[n]->d_name, ".")
                 && !strstr(namelist[n]->d_name, ".txt")
                 && strcmp(namelist[n]->d_name,
                                                                                                       "CVS"))
                {
                    strcpy(buf2, buf);
                    strcat(buf2, namelist[n]->d_name);
                    LOG(llevInfo, " -> Loading plugin : %s\n", namelist[n]->d_name);
                    initOnePlugin(buf2);
                }
            }
        }
    if (namelist != NULL)
        free(namelist);
}

/*****************************************************************************/
/* Removes one plugin from memory. The plugin is identified by its keyname.  */
/*****************************************************************************/
void removeOnePlugin(const char *id)
{
    int plid;
    int j;
    plid = findPlugin(id);
    if (plid < 0)
        return;
    if (PlugList[plid].removefunc != NULL)
        PlugList[plid].removefunc(NULL);
    /* We unload the library... */
    dlclose(PlugList[plid].libptr);
    /* Then we copy the rest on the list back one position */
    PlugNR--;
    if (plid == 31)
        return;
    LOG(llevInfo, "plid=%i, PlugNR=%i\n", plid, PlugNR);
    for (j = plid + 1; j < 32; j++)
    {
        PlugList[j - 1] = PlugList[j];
    }
}

/*****************************************************************************/
/* UNIX Plugin initialization. Initializes a plugin known by its filename.   */
/* The initialization process has several stages:                            */
/* - Loading of the DLL itself;                                              */
/* - Basical plugin information request;                                     */
/* - CF-Plugin specific initialization tasks (call to initPlugin());         */
/* - Hook bindings;                                                          */
/*****************************************************************************/
void initOnePlugin(const char *pluginfile)
{
    int     i   = 0;
    void   *ptr = NULL;
    CFParm *HookParm;

    if ((ptr = dlopen(pluginfile, RTLD_NOW | RTLD_GLOBAL)) == NULL)
    {
        LOG(llevInfo, "Plugin error: %s\n", dlerror());
        return;
    }
    PlugList[PlugNR].libptr = ptr;
    PlugList[PlugNR].initfunc = (f_plugin) (dlsym(ptr, "initPlugin"));
    if (PlugList[PlugNR].initfunc == NULL)
    {
        LOG(llevInfo, "Plugin init error: %s\n", dlerror());
    }
    else
    {
        CFParm *InitParm;

        /* We must send the hooks first of all, so the plugin can use the LOG function */
        if ((registerHooksFunc = dlsym(ptr, "registerHooks")))
        {
            registerHooksFunc(&hooklist);
        }

        InitParm = PlugList[PlugNR].initfunc(NULL);
        LOG(llevInfo, "    Plugin %s loaded under the name of %s\n", (char *) (InitParm->Value[1]),
            (char *) (InitParm->Value[0]));
        strcpy(PlugList[PlugNR].id, (char *) (InitParm->Value[0]));
        strcpy(PlugList[PlugNR].fullname, (char *) (InitParm->Value[1]));
    }
    PlugList[PlugNR].removefunc = (f_plugin) (dlsym(ptr, "removePlugin"));
    PlugList[PlugNR].hookfunc = (f_plugin) (dlsym(ptr, "registerHook"));
    PlugList[PlugNR].eventfunc = (f_plugin) (dlsym(ptr, "triggerEvent"));
    PlugList[PlugNR].pinitfunc = (f_plugin) (dlsym(ptr, "postinitPlugin"));
    PlugList[PlugNR].propfunc = (f_plugin) (dlsym(ptr, "getPluginProperty"));
    LOG(llevInfo, "Done\n");
    if (PlugList[PlugNR].pinitfunc == NULL)
    {
        LOG(llevInfo, "Plugin postinit error: %s\n", dlerror());
    }

    for (i = 0; i < NR_EVENTS; i++)
    {
        PlugList[PlugNR].gevent[i] = 0;
    }
    if (PlugList[PlugNR].hookfunc == NULL)
    {
        LOG(llevInfo, "Plugin hook error: %s\n", dlerror());
    }
    else
    {
        int j;
        i = 0;
        HookParm = (CFParm *) (malloc(sizeof(CFParm)));
        HookParm->Value[0] = (int *) (malloc(sizeof(int)));

        for (j = 1; j < NR_OF_HOOKS; j++)
        {
            memcpy(HookParm->Value[0], &j, sizeof(int));
            HookParm->Value[1] = HookList[j];
            PlugList[PlugNR].hookfunc(HookParm);
        }
        free(HookParm->Value[0]);
        free(HookParm);
    }

    if (PlugList[PlugNR].eventfunc == NULL)
    {
        LOG(llevBug, "BUG: Event plugin error %s\n", dlerror());
    }
    PlugNR++;
    PlugList[PlugNR - 1].pinitfunc(NULL);
    LOG(llevInfo, "[Done]\n");
}
#endif /*WIN32*/

void removePlugins(void)
{
    if (PlugNR)
    {
        int i;
        char* ids[32];

        LOG(llevInfo, "Unloading plugins:\n");
        for (i = 0; i != PlugNR; ++i)
            ids[i] = PlugList[i].id;
        for (i = 0; i != 32; ++i)
            if (ids[i] != NULL)
                removeOnePlugin(ids[i]);
    }
}
/* TODO: remove all this old style plugin functions and move them to hooks-> 
 * This will also remove this ugly mallocs here . MT-11.2005
 */

/*****************************************************************************/
/* command_rskill wrapper.                                                   */
/*****************************************************************************/
/* 0 - player;                                                               */
/* 1 - parameters.                                                           */
/*****************************************************************************/
CFParm * CFWCmdRSkill(CFParm *PParm)
{
    static int  val;
    CFParm     *CFP;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = command_rskill((object *) (PParm->Value[0]), (char *) (PParm->Value[1]));
    CFP->Value[0] = (void *) (&val);
    return CFP;
}

/*****************************************************************************/
/* become_follower wrapper.                                                  */
/*****************************************************************************/
/* 0 - object to change;                                                     */
/* 1 - new god object.                                                       */
/*****************************************************************************/
CFParm * CFWBecomeFollower(CFParm *PParm)
{
    become_follower((object *) (PParm->Value[0]), (object *) (PParm->Value[1]));
    return NULL;
}

/*****************************************************************************/
/* pick_up wrapper.                                                          */
/*****************************************************************************/
/* 0 - picker object;                                                        */
/* 1 - picked object.                                                        */
/*****************************************************************************/
CFParm * CFWPickup(CFParm *PParm)
{
    pick_up((object *) (PParm->Value[0]), (object *) (PParm->Value[1]));
    return NULL;
}

/*****************************************************************************/
/* pick_up wrapper.                                                          */
/*****************************************************************************/
/* 0 - picker object;                                                        */
/* 1 - picked object.                                                        */
/*****************************************************************************/
CFParm * CFWGetMapObject(CFParm *PParm)
{
    object         *val = NULL;
    static CFParm   CFP;

    mapstruct      *mt  = (mapstruct *) PParm->Value[0];
    int             x   = *(int *) PParm->Value[1];
    int             y   = *(int *) PParm->Value[2];

    /*    CFP = (CFParm*)(malloc(sizeof(CFParm))); */

    /* Gecko: added tiled map check */
    if ((mt = out_of_map(mt, &x, &y)))
        val = get_map_ob(mt, x, y);

    CFP.Value[0] = (void *) (val);
    return &CFP;
}

/*****************************************************************************/
/* out_of_map wrapper .                                                      */
/*****************************************************************************/
/* 0 - start map                                                             */
/* 1 - x                                                                     */
/* 2 - y                                                                     */
/*****************************************************************************/
CFParm * CFWOutOfMap(CFParm *PParm)
{
    static CFParm   CFP;

    mapstruct      *mt  = (mapstruct *) PParm->Value[0];
    int            *x   = (int *) PParm->Value[1];
    int            *y   = (int *) PParm->Value[2];

    CFP.Value[0] = (void *) out_of_map(mt, x, y);

    return &CFP;
}


/*****************************************************************************/
/* find_player wrapper.                                                      */
/*****************************************************************************/
/* 0 - name of the player to find.                                           */
/*****************************************************************************/
CFParm * CFWFindPlayer(CFParm *PParm)
{
    player *pl;
    CFParm *CFP;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    pl = find_player((char *) (PParm->Value[0]));
    CFP->Value[0] = (void *) (pl);
    return CFP;
}

/*****************************************************************************/
/* manual_apply wrapper.                                                     */
/*****************************************************************************/
/* 0 - object applying;                                                      */
/* 1 - object to apply;                                                      */
/* 2 - apply flags.                                                          */
/*****************************************************************************/
CFParm * CFWManualApply(CFParm *PParm)
{
    CFParm     *CFP;
    static int  val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = manual_apply((object *) (PParm->Value[0]), (object *) (PParm->Value[1]), *(int *) (PParm->Value[2]));
    CFP->Value[0] = &val;
    return CFP;
}

/*****************************************************************************/
/* command_drop wrapper.                                                     */
/*****************************************************************************/
/* 0 - player;                                                               */
/* 1 - parameters string.                                                    */
/*****************************************************************************/
CFParm * CFWCmdDrop(CFParm *PParm)
{
    CFParm     *CFP;
    static int  val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = command_drop((object *) (PParm->Value[0]), (char *) (PParm->Value[1]));
    CFP->Value[0] = &val;
    return CFP;
}

/*****************************************************************************/
/* command_take wrapper.                                                     */
/*****************************************************************************/
/* 0 - player;                                                               */
/* 1 - parameters string.                                                    */
/*****************************************************************************/
CFParm * CFWCmdTake(CFParm *PParm)
{
    CFParm     *CFP;
    static int  val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    /*val = command_take((object *)(PParm->Value[0]),(char *)(PParm->Value[1]));*/
    CFP->Value[0] = &val;
    return CFP;
}

/*****************************************************************************/
/* transfer_ob wrapper.                                                      */
/*****************************************************************************/
/* 0 - object to transfer;                                                   */
/* 1 - x position;                                                           */
/* 2 - y position;                                                           */
/* 3 - random param;                                                         */
/* 4 - originator object;                                                    */
/*****************************************************************************/
CFParm * CFWTransferObject(CFParm *PParm)
{
    CFParm     *CFP;
    static int  val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = transfer_ob((object *) (PParm->Value[0]), *(int *) (PParm->Value[1]), *(int *) (PParm->Value[2]),
                      *(int *) (PParm->Value[3]), (object *) (PParm->Value[4]), (object *) (PParm->Value[5]));
    CFP->Value[0] = &val;
    return CFP;
}

/*****************************************************************************/
/* kill_object wrapper.                                                      */
/*****************************************************************************/
/* 0 - killed object;                                                        */
/* 1 - damage done;                                                          */
/* 2 - killer object;                                                        */
/* 3 - type of killing.                                                      */
/*****************************************************************************/
CFParm * CFWKillObject(CFParm *PParm)
{
    CFParm     *CFP;
    static int  val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = kill_object((object *) (PParm->Value[0]), *(int *) (PParm->Value[1]), (object *) (PParm->Value[2]),
                      *(int *) (PParm->Value[3]));
    CFP->Value[0] = &val;
    return CFP;
}


/*****************************************************************************/
/* check_spell_known wrapper.                                                */
/*****************************************************************************/
/* 0 - object to check;                                                      */
/* 1 - spell index to search.                                                */
/* return: 0: op has not this skill; 1: op has this skill                    */
/*****************************************************************************/
CFParm * CFWCheckSpellKnown(CFParm *PParm)
{
    CFParm     *CFP;
    static int  val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = check_spell_known((object *) (PParm->Value[0]), *(int *) (PParm->Value[1]));
    CFP->Value[0] = &val;
    return CFP;
}

/*****************************************************************************/
/* do_learn_spell wrapper.                                                   */
/*****************************************************************************/
/* 0 - object to affect;                                                     */
/* 1 - spell index to learn;                                                 */
/* 2 - mode 0:learn , 1:unlearn                                              */
/*****************************************************************************/
CFParm * CFWDoLearnSpell(CFParm *PParm)
{
    /* if mode = 1, unlearn - if mode =0 learn */
    if (*(int *) (PParm->Value[2]))
    {
        do_forget_spell((object *) (PParm->Value[0]), *(int *) (PParm->Value[1]));
    }
    else
    {
        do_learn_spell((object *) (PParm->Value[0]), *(int *) (PParm->Value[1]), 0);
        /* The 0 parameter is marker for special_prayer - godgiven spells,
             * which will be deleted when player changes god.
             */
    }
    return NULL;
}



/*****************************************************************************/
/* do_learn_skill wrapper.                                                   */
/*****************************************************************************/
/* 0 - object to affect;                                                     */
/* 1 - skill index to learn;                                                 */
/* 3 - mode 0=leanr, 1=unlearn                                               */
/*****************************************************************************/
CFParm * CFWDoLearnSkill(CFParm *PParm)
{
    if (*(int *) (PParm->Value[2]))
    {
        /* TODO: add again unlearn skills. atm we don't need to unlearn them */
    }
    else
    {
        learn_skill((object *) (PParm->Value[0]), NULL, NULL, *(int *) (PParm->Value[1]), 0);
    }
    return NULL;
}


/*****************************************************************************/
/* update_ob_speed wrapper.                                                  */
/*****************************************************************************/
/* 0 - object to update.                                                     */
/*****************************************************************************/
CFParm * CFWUpdateSpeed(CFParm *PParm)
{
    update_ob_speed((object *) (PParm->Value[0]));
    return NULL;
}

/*****************************************************************************/
/* update_object wrapper.                                                    */
/*****************************************************************************/
/* 0 - object to update.                                                     */
/*****************************************************************************/
CFParm * CFWUpdateObject(CFParm *PParm)
{
    update_object((object *) (PParm->Value[0]), *(int *) (PParm->Value[1]));
    return NULL;
}

/*****************************************************************************/
/* find_animation wrapper.                                                   */
/*****************************************************************************/
/* 0 - name of the animation to find.                                        */
/*****************************************************************************/
CFParm * CFWFindAnimation(CFParm *PParm)
{
    CFParm     *CFP;
    static int  val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    LOG(llevInfo, "CFWFindAnimation: %s\n", (char *) (PParm->Value[0]));
    val = find_animation((char *) (PParm->Value[0]));
    LOG(llevInfo, "Returned val: %i\n", val);
    CFP->Value[0] = (void *) (&val);
    return CFP;
}



/*****************************************************************************/
/* ready_map_name wrapper.                                                   */
/*****************************************************************************/
/* 0 - name of the map to ready;                                             */
/* 1 - integer flags.                                                        */
/*****************************************************************************/
CFParm * CFWReadyMapName(CFParm *PParm)
{
    CFParm     *CFP;
    mapstruct  *map     = NULL;
    char        path[2048], *tmp;
    char       *string  = (char *) (PParm->Value[0]);
    object     *op      = (object *) (PParm->Value[2]);
    int         unique = 0, flag = *(int *) (PParm->Value[1]);

    CFP = (CFParm *) (malloc(sizeof(CFParm)));

    if (flag & 3)
        unique = MAP_PLAYER_UNIQUE;

    if ((flag & 2) && op)
    {
        sprintf(path, "%s/%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(op->name), op->name, clean_path(string));
        tmp = path;
    }
    else
        tmp = string;

    if ((flag & 4) && op) /* delete the map */
        unlink(tmp);

    if ((flag & 3) && op)
        map = ready_map_name(tmp, unique);

    /* if we have map here now, we had loaded a before saved unique map */
    if (!map)
    {
        /* if we are here, we have not a unique map OR we must load
             * and generate a unique map.
             */
        map = ready_map_name((flag & 2) ? create_pathname(string) : string, unique);

        if (map && (flag & 3)) /* unique map loaded - be sure unique is set right */
        {
            FREE_AND_COPY_HASH(map->path, path); /* goes to player dir */
            map->map_flags |= MAP_FLAG_UNIQUE;
        }
    }

    CFP->Value[0] = (void *) (map);
    return CFP;
}

/*****************************************************************************/
/* add_exp wrapper.                                                          */
/*****************************************************************************/
/* 0 - object to increase experience of.                                     */
/* 1 - amount of experience to add.                                          */
/* 2 - Skill number to add xp in                                             */
/*****************************************************************************/
CFParm * CFWAddExp(CFParm *PParm)
{
    add_exp((object *) (PParm->Value[0]), *(int *) (PParm->Value[1]), *(int *) (PParm->Value[2]));
    return(PParm);
}

/*****************************************************************************/
/* determine_god wrapper.                                                    */
/*****************************************************************************/
/* 0 - object to determine the god of.                                       */
/*****************************************************************************/
CFParm * CFWDetermineGod(CFParm *PParm)
{
    CFParm     *CFP;
    const char *val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = determine_god((object *) (PParm->Value[0]));
    CFP->Value[0] = (void *) (val);
    return CFP;
}

/*****************************************************************************/
/* find_god wrapper.                                                         */
/*****************************************************************************/
/* 0 - Name of the god to search for.                                        */
/*****************************************************************************/
CFParm * CFWFindGod(CFParm *PParm)
{
    CFParm *CFP;
    object *val;
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = find_god((char *) (PParm->Value[0]));
    CFP->Value[0] = (void *) (val);
    return CFP;
}

/*****************************************************************************/
/* dump_me wrapper.                                                          */
/*****************************************************************************/
/* 0 - object to dump;                                                       */
/*****************************************************************************/
CFParm * CFWDumpObject(CFParm *PParm)
{
    CFParm *CFP;
    char   *val;
    /*    object* ob; not used */
    val = (char *) (malloc(sizeof(char) * 10240));
    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    dump_me((object *) (PParm->Value[0]), val);
    CFP->Value[0] = (void *) (val);
    return CFP;
}

/*****************************************************************************/
/* load_object_str wrapper.                                                  */
/*****************************************************************************/
/* 0 - object dump string to load.                                           */
/*****************************************************************************/
CFParm * CFWLoadObject(CFParm *PParm)
{
    CFParm *CFP;
    object *val;

    CFP = (CFParm *) (malloc(sizeof(CFParm)));
    val = load_object_str((char *) (PParm->Value[0]));
    LOG(llevDebug, "CFWLoadObject: %s\n", query_name(val));
    CFP->Value[0] = (void *) (val);
    return CFP;
}

/*****************************************************************************/
/* remove_ob wrapper.                                                        */
/*****************************************************************************/
/* 0 - object to remove.                                                     */
/*****************************************************************************/
CFParm * CFWRemoveObject(CFParm *PParm)
{
    remove_ob((object *) (PParm->Value[0]));
    return NULL;
}

/*****************************************************************************/
/* destruct_ob wrapper.                                                      */
/*****************************************************************************/
/* 0 - object to destruct.                                                   */
/*****************************************************************************/
CFParm * CFWDestructObject(CFParm *PParm)
{
    destruct_ob((object *) (PParm->Value[0]));
    return NULL;
}

CFParm * CFWSendCustomCommand(CFParm *PParm)
{
    send_plugin_custom_message((object *) (PParm->Value[0]), (char *) (PParm->Value[1]));
    return NULL;
}


/*****************************************************************************/
/* communicate wrapper.                                                      */
/*****************************************************************************/
/* 0 - object                                                                */
/* 1 - string                                                                */
/*****************************************************************************/
CFParm * CFWCommunicate(CFParm *PParm)
{
    /*char buf[MAX_BUF];*/
    object *op      = (object *) PParm->Value[0];
    char   *string  = (char *) PParm->Value[1];
    if ((!op) || (!string))
        return NULL;

    communicate(op, string);
    return NULL;
}

/*****************************************************************************/
/* find_marked_object .                                                      */
/*****************************************************************************/
/* 0 - object                                                                */
/*****************************************************************************/
/* return: object or NULL                                                    */
/*****************************************************************************/
CFParm * CFWFindMarkedObject(CFParm *PParm)
{
    static CFParm   CFP;

    object         *op  = (object *) PParm->Value[0];
    if (op)
        op = find_marked_object(op);

    CFP.Value[0] = (void *) op;

    return(&CFP);
}

/*****************************************************************************/
/* find_marked_object .                                                      */
/*****************************************************************************/
/* 0 - object, 1 - if set, send examine msg to this object                   */
/*****************************************************************************/
/* return: object or NULL                                                    */
/*****************************************************************************/
CFParm * CFWIdentifyObject(CFParm *PParm)
{
    object *caster  = (object *) PParm->Value[0];
    object *target  = (object *) PParm->Value[1];
    object *op      = (object *) PParm->Value[2];


    cast_identify(target, caster->level, op, *(int *) (PParm->Value[3]));

    if (caster)
        play_sound_map(caster->map, caster->x, caster->y, spells[SP_IDENTIFY].sound, SOUND_SPELL);
    else if (target)
        play_sound_map(target->map, target->x, target->y, spells[SP_IDENTIFY].sound, SOUND_SPELL);

    return NULL;
}

/*****************************************************************************/
/* ObjectCreateClone object_copy wrapper.                                    */
/*****************************************************************************/
/* 0 - object                                                                */
/* 1 - type 0 = clone with inventory                                         */
/*          1 = only duplicate the object without it's content and op->more  */
/*****************************************************************************/
CFParm * CFWObjectCreateClone(CFParm *PParm)
{
    CFParm *CFP = (CFParm *) malloc(sizeof(CFParm));
    if (*(int *) PParm->Value[1] == 0)
        CFP->Value[0] = ObjectCreateClone((object *) PParm->Value[0]);
    else if (*(int *) PParm->Value[1] == 1)
    {
        object *tmp;
        tmp = get_object();
        copy_object((object *) PParm->Value[0], tmp);
        CFP->Value[0] = tmp;
    }
    return CFP;
}

/*****************************************************************************/
/* teleport an object to another map                                         */
/*****************************************************************************/
/* 0 - object                                                                */
/* 1 - mapname we use for destination                                        */
/* 2 - mapx                                                                  */
/* 3 - mapy                                                                  */
/* 4 - unique?                                                               */
/* 5 - msg (used for random maps entering. May be NULL)                      */
/*****************************************************************************/
CFParm * CFWTeleportObject(CFParm *PParm)
{
    object *current;
    /*    char * mapname; not used
        int mapx;
        int mapy;
        int unique; not used */
    current = get_object();
    FREE_AND_COPY_HASH(EXIT_PATH(current), (char *) PParm->Value[1]);
    EXIT_X(current) = *(int *) PParm->Value[2];
    EXIT_Y(current) = *(int *) PParm->Value[3];
    if (*(int *) PParm->Value[4])
        current->last_eat = MAP_PLAYER_MAP;
    if (PParm->Value[5])
        FREE_AND_COPY_HASH(current->msg, (char *) PParm->Value[5]);
    enter_exit((object *) PParm->Value[0], current);
    if (((object *) PParm->Value[0])->map)
        play_sound_map(((object *) PParm->Value[0])->map, ((object *) PParm->Value[0])->x,
                       ((object *) PParm->Value[0])->y, SOUND_TELEPORT, SOUND_NORMAL);
    return NULL;
}

/*****************************************************************************/
/* The following is not really a wrapper like the others are.                */
/* It is in fact used to allow the plugin to request the global events it    */
/* wants to be aware of. All events can be seen as global; on the contrary,  */
/* some events can't be used as local: for example, BORN is only global.     */
/*****************************************************************************/
/* 0 - Number of the event to register;                                      */
/* 1 - String ID of the requesting plugin.                                   */
/*****************************************************************************/
CFParm * RegisterGlobalEvent(CFParm *PParm)
{
    int PNR = findPlugin((char *) (PParm->Value[1]));
#ifdef LOG_VERBOSE
    LOG(llevDebug, "Plugin %s (%i) registered the event %i\n", (char *) (PParm->Value[1]), PNR,
        *(int *) (PParm->Value[0]));
#endif
    LOG(llevDebug, "Plugin %s (%i) registered the event %i\n", (char *) (PParm->Value[1]), PNR,
        *(int *) (PParm->Value[0]));
    PlugList[PNR].gevent[*(int *) (PParm->Value[0])] = 1;
    return NULL;
}

/*****************************************************************************/
/* The following unregisters a global event.                                 */
/*****************************************************************************/
/* 0 - Number of the event to unregister;                                    */
/* 1 - String ID of the requesting plugin.                                   */
/*****************************************************************************/
CFParm * UnregisterGlobalEvent(CFParm *PParm)
{
    int PNR = findPlugin((char *) (PParm->Value[1]));
    PlugList[PNR].gevent[*(int *) (PParm->Value[0])] = 0;
    return NULL;
}

/*****************************************************************************/
/* When a specific global event occurs, this function is called.             */
/* Concerns events: BORN, QUIT, LOGIN, LOGOUT, SHOUT for now.                */
/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
void GlobalEvent(CFParm *PParm)
{
    int i;
    for (i = 0; i < PlugNR; i++)
    {
        if (PlugList[i].gevent[*(int *) (PParm->Value[0])] != 0)
        {
            (PlugList[i].eventfunc) (PParm);
        }
    }
}


/*****************************************************************************/
/* create_object wrapper.                                                    */
/*****************************************************************************/
/* 0 - archetype                                                             */
/* 1 - map;                                                                  */
/* 2 - x;                                                                    */
/* 3 - y;                                                                    */
/*****************************************************************************/
CFParm * CFWCreateObject(CFParm *PParm)
{
    static CFParm   CFP;
    archetype      *arch;
    object         *newobj;

    CFP.Value[0] = NULL;

    if (!(arch = find_archetype((char *) (PParm->Value[0]))))
        return(&CFP);

    if (arch->clone.type == PLAYER)
        return(&CFP);

    if (!(newobj = arch_to_object(arch)))
        return(&CFP);

    newobj->x = *(int *) (PParm->Value[2]);
    newobj->y = *(int *) (PParm->Value[3]);

    if(newobj->type == MONSTER)
        fix_monster(newobj);
    
    newobj = insert_ob_in_map(newobj, (mapstruct *) (PParm->Value[1]), NULL, 0);

    CFP.Value[0] = newobj;
    return (&CFP);
}

/*****************************************************************************/
/* transfer map items wrapper.                                               */
/*****************************************************************************/
/* 0 - old_map;                                                              */
/* 1 - new_map;                                                              */
/*****************************************************************************/
CFParm * CFTransferMapItems(CFParm *PParm)
{
    int         x, y, i, j;
    object     *op, *tmp, *tmp2;
    mapstruct  *map_old, *map_new;

    map_old = (mapstruct *) (PParm->Value[0]);
    map_new = (mapstruct *) (PParm->Value[1]);

    if (!map_old || !map_new)
        LOG(llevBug, "BUG:  CFTransferMapItems %s isd NULL\n", map_old ? "map_new" : "map_old");

    x = *(int *) (PParm->Value[2]);
    y = *(int *) (PParm->Value[3]);

    for (i = 0; i < MAP_WIDTH(map_old); i++)
    {
        for (j = 0; j < MAP_HEIGHT(map_old); j++)
        {
            for (op = get_map_ob(map_old, i, j); op; op = tmp2)
            {
                tmp2 = op->above;
                /* if thats true, the player can't get it - no sense to transfer it! */
                if (QUERY_FLAG(op, FLAG_SYS_OBJECT))
                    continue;

                if (!QUERY_FLAG(op, FLAG_NO_PICK))
                {
                    remove_ob(op);
                    op->x = x;
                    op->y = y;
                    insert_ob_in_map(op, map_new, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
                }
                else /* this is a fixed part of the map */
                {
                    /* now we test we have a container type object.
                                 * The player can have items stored in it.
                                 * If so, we remove them too.
                                 * we don't check inv of non container object.
                                 * The player can't store in normal sense items
                                 * in them, so the items in them (perhaps special
                                 * marker of forces) should not be transfered.
                                 */

                    for (tmp = op->inv; tmp; tmp = tmp2)
                    {
                        tmp2 = tmp->below;
                        /* well, non pickup container in non pickup container? no no... */
                        if (QUERY_FLAG(tmp, FLAG_SYS_OBJECT) || QUERY_FLAG(tmp, FLAG_NO_PICK))
                            continue;
                        remove_ob(tmp);
                        tmp->x = x;
                        tmp->y = y;
                        insert_ob_in_map(tmp, map_new, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
                    }
                }
            }
        }
    }

    return NULL;
}

CFParm * CFMapSave(CFParm *PParm)
{
    mapstruct  *map     = (mapstruct *) (PParm->Value[0]);
    int         flags   = *(int *) (PParm->Value[1]);

    if (!map || map->in_memory != MAP_IN_MEMORY)
        return NULL;

    if (new_save_map(map, 0) == -1)
    {
        LOG(llevDebug, "CFMapSave(): failed to save map %s\n", map->path ? map->path : "NO PATH");
    }

    if (flags)
        map->in_memory = MAP_IN_MEMORY;

    return NULL;
}

CFParm * CFMapDelete(CFParm *PParm)
{
    mapstruct  *map     = (mapstruct *) (PParm->Value[0]);
    int         flags   = *(int *) (PParm->Value[1]);

    if (!map || !map->in_memory)
        return NULL;

    if (flags) /* really delete the map */
    {
        free_map(map, 1);
        delete_map(map);
    }
    else /* just swap it out */
        free_map(map, 1);

    return NULL;
}

CFParm * CFInterface(CFParm *PParm)
{
    object               *who        = (object*) (PParm->Value[0]);
    int                    mode    = *(int *) (PParm->Value[1]);
    char               *text    = (char *) (PParm->Value[2]);

    SOCKET_SET_BINARY_CMD(&global_sl, BINARY_CMD_INTERFACE);

    /* NPC_INTERFACE_MODE_NO will send a clear body = remove interface to the client */
    if(mode != NPC_INTERFACE_MODE_NO)
    {
        SockList_AddChar(&global_sl, (char)mode);
        strcpy(global_sl.buf+global_sl.len, text);
        global_sl.len += strlen(text)+1;
    }

    Send_With_Handling(&CONTR(who)->socket, &global_sl);
    return NULL;
}

