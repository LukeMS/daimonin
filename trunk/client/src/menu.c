#include <include.h>

_media_file media_file[MEDIA_MAX];

/* keybind menu */
int keybind_entry=0;
int keybind_status;
Boolean keybind_repeat=TRUE;
struct _keybind_key keybind_key;

struct _spell_list spell_list[SPELL_LIST_MAX]; /* skill list entries */
struct _skill_list skill_list[SKILL_LIST_MAX]; /* skill list entries */
struct _spell_list_set spell_list_set;
struct _skill_list_set skill_list_set;

int media_count;        /* buffered media files*/
int media_show; /* show this media file*/
int media_show_update;
static char *get_range_item_name(int id);

static int group_pos[MAX_GROUP_MEMBER][2] ={
	{34,1},
	{34,19},
	{34,37},
	{34,55},
	{143,1},
	{143,19},
	{143,37},
	{143,55}
};

int quick_slots[MAX_QUICK_SLOTS];
int quickslots_pos[MAX_QUICK_SLOTS][2] = {
	{4,10},
	{39,10},
	{74,10},
	{109,10},
	{144,10},
	{179,10},
	{214,10},
	{249,10}
};

void do_console(int x, int y)
{
	show_help_screen=0;
    if(InputStringEscFlag==TRUE)
    {
        sound_play_effect(SOUND_CONSOLE,0,0,100);
            reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag=2;
    }
    /* if set, we got a finished input!*/
    if(InputStringFlag==FALSE
        &&InputStringEndFlag==TRUE)
    {
        sound_play_effect(SOUND_CONSOLE,0,0,100);
        if(InputString[0])
        {
			char buf[MAX_INPUT_STRING+32];
			sprintf(buf,":%s",InputString);
			draw_info(buf,COLOR_DGOLD);

			if(*InputString != '/') /* if not a command ... its chat */
			{
				sprintf(buf,"/say %s",InputString);
			}
			else
			{
				if(client_command_check(InputString) )
					goto no_send_cmd;

				strcpy(buf,InputString);
			}
            send_command(buf, -1, SC_NORMAL);


        }
		no_send_cmd:
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag=2;
    }
    else
        show_console(x,y);
}

/* client_command_check() 
 * Analyze /<cmd> type commands the player has typed in the console
 * or bound to a key. Sort out the "client intern" commands and
 * expand or pre process them for the server.
 * Return: TRUE=don't send command to server
 * FALSE: send command to server
 */
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

int client_command_check(char *cmd)
{
	if(!strncmp(cmd,"/keybind",strlen("/keybind")) )
	{
		map_udate_flag=2;
		if(cpl.menustatus != MENU_KEYBIND)
		{
			keybind_status = KEYBIND_STATUS_NO;
			cpl.menustatus = MENU_KEYBIND;
		}
		else
		{
			save_keybind_file(KEYBIND_FILE);
			cpl.menustatus = MENU_NO;           
		}
		sound_play_effect(SOUND_SCROLL,0,0,100);
		reset_keys();
		return TRUE;
	}
	else if(!strncmp(cmd,"/target",strlen("/target")) )
	{
		/* logic is: if first parameter char is a digit, is enemy,friend or self.
		 * if first char a character - then its a name of a living object.
		 */
		if(!strncmp(cmd,"/target friend",strlen("/target friend")) )
			strcpy(cmd,"/target 1");
		else if(!strncmp(cmd,"/target enemy",strlen("/target enemy")) )
			strcpy(cmd,"/target 0");
		else if(!strncmp(cmd,"/target self",strlen("/target self")) )
			strcpy(cmd,"/target 2");
	}

	return FALSE;
}

void show_console(int x, int y)
{
/*        sprite_blt(Bitmaps[BITMAP_CONSOLE],x, y, NULL, NULL);*/
        StringBlt(ScreenSurface, &SystemFont,
                show_input_string(InputString,&SystemFont,239)
                , x, y, COLOR_WHITE, NULL, NULL);
}

void do_number(int x, int y)
{
	show_help_screen=0;
    if(InputStringEscFlag==TRUE)
    {
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag=2;
    }
    /* if set, we got a finished input!*/
    if(InputStringFlag==FALSE
        &&InputStringEndFlag==TRUE)
    {
        if(InputString[0])
        {
            int tmp;
            char buf[300];
            tmp = atoi(InputString);
            if(tmp>0 && tmp <= cpl.nrof)
            {
                client_send_move (cpl.loc, cpl.tag, tmp);            
                sprintf(buf,"%s %d from %d %s", cpl.nummode == NUM_MODE_GET?"get":"drop", tmp, cpl.nrof, cpl.num_text);
                draw_info(buf,COLOR_DGOLD);
                
            }
        }
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag=2;
    }
    else
        show_number(x,y);
}

void do_keybind_input(void)
{
	show_help_screen=0;
    if(InputStringEscFlag==TRUE)
    {
        reset_keys();
        sound_play_effect(SOUND_CLICKFAIL,0,0,100);
        cpl.input_mode = INPUT_MODE_NO;
        keybind_status = KEYBIND_STATUS_NO;
        map_udate_flag=2;
    }
    /* if set, we got a finished input!*/
    if(InputStringFlag==FALSE
        &&InputStringEndFlag==TRUE)
    {
        if(InputString[0]) 
        {
            strcpy(keybind_key.macro, InputString);
            keybind_key.repeat_flag = keybind_repeat;
            if(keybind_status == KEYBIND_STATUS_NEW)
            {
                keybind_key.entry = -1;
                keybind_status = KEYBIND_STATUS_NEWKEY;            
            }
            else
            {
                keybind_key.entry = keybind_entry;
                keybind_status = KEYBIND_STATUS_EDITKEY;            
            }
        }
        else /* cleared string - delete entry when edit mode or ignore */
        {
            if(keybind_status == KEYBIND_STATUS_EDIT)
            {
                keybind_key.entry = keybind_entry;
                keybind_key.macro[0]=0;
                add_keybind_macro(&keybind_key);
            }
            keybind_status = KEYBIND_STATUS_NO;            
        }
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag=2;
    }
}


void show_number(int x, int y)
{
	SDL_Rect tmp;
	char buf[512];

	tmp.w = 238;

    sprite_blt(Bitmaps[BITMAP_NUMBER],x, y, NULL, NULL);
    sprintf(buf,"%s how much from %d %s", cpl.nummode == NUM_MODE_GET?"get":"drop", cpl.nrof, cpl.num_text);
    StringBlt(ScreenSurface, &SystemFont, buf, x+8, y+6, COLOR_HGOLD, &tmp, NULL);
    StringBlt(ScreenSurface, &SystemFont,
			show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_NUMBER]->bitmap->w-22),
			x+8, y+25, COLOR_WHITE, &tmp, NULL);
}

void show_resist(int x, int y)
{
    char buf[62];
    
    StringBlt(ScreenSurface, &Font6x3Out,"Armour Protection Table",x, y+1, COLOR_HGOLD, NULL, NULL);

    StringBlt(ScreenSurface, &Font6x3Out,"Physical",x, y+16, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"I",x+43, y+17, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[0]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+53, y+17, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont,"S",x+73, y+17, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[1]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+83, y+17, COLOR_WHITE, NULL, NULL);


    StringBlt(ScreenSurface, &SystemFont,"C",x+103, y+17, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[2]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+113, y+17, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"P",x+133, y+17, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[3]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+143, y+17, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"W",x+163, y+17, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[4]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+173, y+17, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &Font6x3Out,"Elemental",x, y+31, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"F",x+43, y+32, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[5]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+53, y+32, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"C",x+73, y+32, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[6]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+83, y+32, COLOR_WHITE, NULL, NULL);
    
    StringBlt(ScreenSurface, &SystemFont,"E",x+103, y+32, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[7]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+113, y+32, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"P",x+133, y+32, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[8]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+143, y+32, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"A",x+163, y+32, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[9]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+173, y+32, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &Font6x3Out,"Magical",x, y+46, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"M",x+43, y+47, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[10]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+53, y+47, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"Mi",x+70, y+47, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[11]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+83, y+47, COLOR_WHITE, NULL, NULL);
    
    StringBlt(ScreenSurface, &SystemFont,"B",x+103, y+47, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[12]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+113, y+47, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"P",x+133, y+47, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[13]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+143, y+47, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"F",x+163, y+47, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[14]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+173, y+47, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &Font6x3Out,"Spherical",x, y+61, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"N",x+43, y+62, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[15]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+53, y+62, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"Ch",x+69, y+62, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[16]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+83, y+62, COLOR_WHITE, NULL, NULL);
    
    StringBlt(ScreenSurface, &SystemFont,"D",x+103, y+62, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[17]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+113, y+62, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"Sp",x+130, y+62, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[18]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+143, y+62, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont,"Co",x+160, y+62, COLOR_HGOLD, NULL, NULL);
    sprintf(buf,"%02d", cpl.stats.protection[19]);
    StringBlt(ScreenSurface, &SystemFont,buf,x+173, y+62, COLOR_WHITE, NULL, NULL);
	
}

#define ICONDEFLEN 32
Boolean blt_face_centered(int face, int x, int y)
{
    register int temp;
    SDL_Rect box;
    
    if(!FaceList[face].sprite)
        return FALSE;
    
    if(FaceList[face].sprite->status != SPRITE_STATUS_LOADED)
        return FALSE;
    
    box.x = FaceList[face].sprite->border_left;
    box.w = FaceList[face].sprite->bitmap->w;
    temp = box.w-FaceList[face].sprite->border_left-FaceList[face].sprite->border_right;
    if(temp > 32)
    {
        box.w = 32;
        temp-=32;
        temp>>=1;
        box.x+=temp;
    }
    else if(temp < 32)
    {
        temp = 32-temp;
        box.x -= (temp>>1);
    }
    
    box.y = -FaceList[face].sprite->border_up;
    box.h = FaceList[face].sprite->bitmap->h;        
    temp = box.h-FaceList[face].sprite->border_up-FaceList[face].sprite->border_down;
    if(temp > 32)
    {
        box.h = 32;
        temp-=32;
        temp>>=1;
        box.y+=temp;
    }
    else if(temp < 32)
    {
        temp = 32-temp;
        box.y = -(temp>>1)+FaceList[face].sprite->border_up;
    }
    sprite_blt(FaceList[face].sprite,x,y, &box, NULL);

    return TRUE;
}

void show_range(int x, int y)
{
    char buf[256];
	SDL_Rect rec_range;
	SDL_Rect rec_item;
    item *tmp;
    
	rec_range.w=160;
	rec_item.w=185;
    examine_range_inv();
        
        sprite_blt(Bitmaps[BITMAP_RANGE],x-2, y, NULL, NULL);

        switch(RangeFireMode)
        {
            case FIRE_MODE_BOW:
                if(fire_mode_tab[FIRE_MODE_BOW].item != FIRE_ITEM_NO)
                {
                    sprintf(buf,"using %s", 
                        get_range_item_name(fire_mode_tab[FIRE_MODE_BOW].item));
                    blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_BOW].item,x+3,y+2);
                                                
                    StringBlt(ScreenSurface, &SystemFont,buf,x+3, y+38, COLOR_WHITE, &rec_range, NULL);
                    if(fire_mode_tab[FIRE_MODE_BOW].amun != FIRE_ITEM_NO)
                    {
						
                        tmp = locate_item_from_item(cpl.ob, fire_mode_tab[FIRE_MODE_BOW].amun);
                        if(tmp)
                        {
                            if(tmp->itype == TYPE_ARROW)
                                sprintf(buf,"ammo %s (%d)", 
                                    get_range_item_name(fire_mode_tab[FIRE_MODE_BOW].amun),tmp->nrof);
                            else
                                sprintf(buf,"ammo %s", 
                                get_range_item_name(fire_mode_tab[FIRE_MODE_BOW].amun));                            
                        }
                        else
                            strcpy(buf,"ammo not selected");
                        blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_BOW].amun,x+43,y+2);
                    }
                    else
                    {
                        sprintf(buf,"ammo not selected");   
                    }
                    StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                }
                else
                {
                    sprintf(buf,"no range weapon applied");   
                    StringBlt(ScreenSurface, &SystemFont,buf,x+3, y+38, COLOR_WHITE, &rec_range, NULL);
                }

                sprite_blt(Bitmaps[BITMAP_RANGE_MARKER],x+3, y+2, NULL, NULL);
                break;

            /* wands, staffs, rods and horns */
            case FIRE_MODE_WAND:
                if(!locate_item_from_item(cpl.ob, fire_mode_tab[FIRE_MODE_WAND].item))
                    fire_mode_tab[FIRE_MODE_WAND].item = FIRE_ITEM_NO;
                if(fire_mode_tab[FIRE_MODE_WAND].item != FIRE_ITEM_NO)
                {
                    sprintf(buf,"%s", 
                        get_range_item_name(fire_mode_tab[FIRE_MODE_WAND].item));
                    StringBlt(ScreenSurface, &SystemFont,buf,x+3, y+49, COLOR_WHITE, &rec_item, NULL);
                    sprite_blt(Bitmaps[BITMAP_RANGE_TOOL],x+3, y+2, NULL, NULL);
                    blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_WAND].item,x+43,y+2);
                }
                else
                {
                    sprite_blt(Bitmaps[BITMAP_RANGE_TOOL_NO],x+3, y+2, NULL, NULL);
                    sprintf(buf,"nothing applied");   
                    StringBlt(ScreenSurface, &SystemFont,buf,x+3, y+49, COLOR_WHITE, &rec_item, NULL);
                }
                
                sprintf(buf,"use range tool");   
                StringBlt(ScreenSurface, &SystemFont,buf,x+3, y+38, COLOR_WHITE, &rec_range, NULL);
                break;

            /* the summon range ctrl will come from server only after the player casted a summon spell */
            case FIRE_MODE_SUMMON:
                if(fire_mode_tab[FIRE_MODE_SUMMON].item != FIRE_ITEM_NO)
                {
                    sprite_blt(Bitmaps[BITMAP_RANGE_CTRL],x+3, y+2, NULL, NULL);
                    StringBlt(ScreenSurface, &SystemFont,fire_mode_tab[FIRE_MODE_SUMMON].name , x+3, y+49, COLOR_WHITE, NULL, NULL);                
					blt_face_centered(fire_mode_tab[FIRE_MODE_SUMMON].item , x+43, y+2);
                }
                else
                {
                    sprite_blt(Bitmaps[BITMAP_RANGE_CTRL_NO],x+3, y+2, NULL, NULL);
                    sprintf(buf,"no golem summoned");
                    StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                }
                sprintf(buf,"mind control");
                StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+38, COLOR_WHITE, &rec_item, NULL);                
                break;
            
            /* these are client only, no server signal needed */
            case FIRE_MODE_SKILL:
                if(fire_mode_tab[FIRE_MODE_SKILL].skill)
                {
                    sprite_blt(Bitmaps[BITMAP_RANGE_SKILL],x+3, y+2, NULL, NULL);
                    if(fire_mode_tab[FIRE_MODE_SKILL].skill->flag != -1)
                    {
                        sprite_blt(fire_mode_tab[FIRE_MODE_SKILL].skill->icon ,x+43, y+2, NULL, NULL);
                        StringBlt(ScreenSurface, &SystemFont,fire_mode_tab[FIRE_MODE_SKILL].skill->name , x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                    }
                    else
                        fire_mode_tab[FIRE_MODE_SKILL].skill=NULL;
                }
                else
                {
                    sprite_blt(Bitmaps[BITMAP_RANGE_SKILL_NO],x+3, y+2, NULL, NULL);
                    sprintf(buf,"no skill selected");
                    StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                }
                sprintf(buf,"use skill");
                StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+38, COLOR_WHITE, &rec_range, NULL);                

            break;
            case FIRE_MODE_SPELL:
                    
                if(fire_mode_tab[FIRE_MODE_SPELL].spell)
                {
                        /* we use wiz spells as default */
                    sprite_blt(Bitmaps[BITMAP_RANGE_WIZARD],x+3, y+2, NULL, NULL);
                    if(fire_mode_tab[FIRE_MODE_SPELL].spell->flag != -1)
                    {
                        sprite_blt(fire_mode_tab[FIRE_MODE_SPELL].spell->icon ,x+43, y+2, NULL, NULL);
                        StringBlt(ScreenSurface, &SystemFont,fire_mode_tab[FIRE_MODE_SPELL].spell->name , x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                    }
                    else
                        fire_mode_tab[FIRE_MODE_SPELL].spell=NULL;
                }
                else
                {
                    sprite_blt(Bitmaps[BITMAP_RANGE_WIZARD_NO],x+3, y+2, NULL, NULL);
                    sprintf(buf,"no spell selected");
                    StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                }
                sprintf(buf,"cast spell");
                StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+38, COLOR_WHITE, &rec_range, NULL);                
                
                break;
            case FIRE_MODE_THROW:
                if(!locate_item_from_item(cpl.ob, fire_mode_tab[FIRE_MODE_THROW].item))
                    fire_mode_tab[FIRE_MODE_THROW].item = FIRE_ITEM_NO;
                if(fire_mode_tab[FIRE_MODE_THROW].item != FIRE_ITEM_NO)
                {

                    sprite_blt(Bitmaps[BITMAP_RANGE_THROW],x+3, y+2, NULL, NULL);
                    blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_THROW].item,x+43,y+2);
                    sprintf(buf,"%s", 
                        get_range_item_name(fire_mode_tab[FIRE_MODE_THROW].item));
                    StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                }
                else
                {
                    sprite_blt(Bitmaps[BITMAP_RANGE_THROW_NO],x+3, y+2, NULL, NULL);
                    sprintf(buf,"no item ready");
                    StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+49, COLOR_WHITE, &rec_item, NULL);                
                }
                sprintf(buf,"throw item");
                StringBlt(ScreenSurface, &SystemFont,buf, x+3, y+38, COLOR_WHITE, &rec_range, NULL);                
                                
                break;
        };
}

static char *get_range_item_name(int tag)
{   
    item *tmp;
    
    if(tag != FIRE_ITEM_NO)
    {
        tmp = locate_item (tag);
        if(tmp)
            return tmp->s_name;
    }  
    return("Nothing");
}

void blt_inventory_face_from_tag(int tag, int x, int y)
{
    item *tmp;
    
    /* check item is in inventory and faces are loaded, etc */ 
    tmp = locate_item (tag);
    if(!tmp)
        return;
    blt_inv_item_centered(tmp, x, y);
}

void show_menu(void)
{
        if(!cpl.menustatus)
                return;
        if(cpl.menustatus == MENU_KEYBIND)
                show_keybind();
        else if(cpl.menustatus == MENU_STATUS)
                show_status();
        else if(cpl.menustatus == MENU_SLIST)
            show_spelllist();
        else if(cpl.menustatus == MENU_SKILL)
            show_skilllist();
}

void show_media(int x, int y)
{
        _Sprite *bmap;
        int xtemp;

        if(media_show!=MEDIA_SHOW_NO)
        {
                /* we show a png*/
                if(media_file[media_show].type == MEDIA_TYPE_PNG)
                {
                        bmap = (_Sprite*) media_file[media_show].data;
                        if(bmap)
                        {
                            xtemp = x-bmap->bitmap->w;
                            sprite_blt(bmap ,xtemp, y, NULL, NULL);
                        }
                }
        }
}


void show_keybind(void)
{
        int y, x,i;
		SDL_Rect rec_in;
		SDL_Rect rec_key;
		SDL_Rect rec_macro;
        static int keybind_startoff=0;

		rec_key.w   = 125;
		rec_macro.w = 160;
		rec_in.w    = 225;
        x= SCREEN_XLEN/2-Bitmaps[BITMAP_KEYBIND]->bitmap->w/2;
        y= SCREEN_YLEN/2-Bitmaps[BITMAP_KEYBIND]->bitmap->h/2;
        sprite_blt(Bitmaps[BITMAP_KEYBIND],x, y, NULL, NULL);

        if(keybind_status == KEYBIND_STATUS_NEW ||keybind_status == KEYBIND_STATUS_NEWKEY)
            sprite_blt(Bitmaps[BITMAP_KEYBIND_NEW],x+17, y+330, NULL, NULL);
        else if(keybind_status == KEYBIND_STATUS_EDIT ||keybind_status == KEYBIND_STATUS_EDITKEY)
            sprite_blt(Bitmaps[BITMAP_KEYBIND_EDIT],x+91, y+330, NULL, NULL);
        
        if(!keybind_repeat)
            sprite_blt(Bitmaps[BITMAP_KEYBIND_REPEAT],x+260, y+75, NULL, NULL);

        if(keybind_status == KEYBIND_STATUS_NEW ||keybind_status == KEYBIND_STATUS_EDIT)
        {
            sprite_blt(Bitmaps[BITMAP_KEYBIND_INPUT],x+15, y+55, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont,
            show_input_string(InputString,&SystemFont,Bitmaps[BITMAP_KEYBIND_INPUT]->bitmap->w-22)
                            ,x+17, y+57, COLOR_WHITE, NULL, NULL);
        }

        /* at last - show the new macro string and the PRESS KEY msg -
         *  when pressed in event, it will be done there
         */
        if(keybind_status == KEYBIND_STATUS_EDITKEY || keybind_status == KEYBIND_STATUS_NEWKEY)
        {
            StringBlt(ScreenSurface, &SystemFont,keybind_key.macro,x+17, y+57, COLOR_WHITE, NULL, NULL);
            sprite_blt(Bitmaps[BITMAP_KEYPRESS],x+261, y+56, NULL, NULL);
        }

        /* first, be sure our entry is inside the legal area */
        if(keybind_entry < 0)
            keybind_entry = 0;        
        if(keybind_entry >= keymap_count && keybind_entry != 0)
            keybind_entry = keymap_count-1;

        /* adjust startoff for scrolling */
        if(keybind_startoff+17 <keybind_entry)
            keybind_startoff = keybind_entry-17;
        if(keybind_startoff>keybind_entry)
            keybind_startoff = keybind_entry;
        
        for(i=0;i+keybind_startoff<keymap_count && i<18;i++)
        {
            if(keybind_entry == keybind_startoff+i)
                sprite_blt(Bitmaps[BITMAP_KEYBINDSLIDER],x+15, y+91+i*13, NULL, NULL);
            StringBlt(ScreenSurface, &SystemFont, keymap[i+keybind_startoff].keyname,
                                       x+20, y+91+i*13,COLOR_WHITE, &rec_key, NULL);
            StringBlt(ScreenSurface, &SystemFont, keymap[i+keybind_startoff].text,
                                          x+150, y+91+i*13,COLOR_WHITE, &rec_macro, NULL);
        }        
	 	blt_window_slider(Bitmaps[BITMAP_KEYBIND_SCROLL], keymap_count,18,keybind_startoff, x+316, y+103);
}

void show_status(void)
{
        int y, x;

        x= SCREEN_XLEN/2-Bitmaps[BITMAP_STATUS]->bitmap->w/2;
        y= SCREEN_YLEN/2-Bitmaps[BITMAP_STATUS]->bitmap->h/2;
        sprite_blt(Bitmaps[BITMAP_STATUS],x, y, NULL, NULL);
}



int init_media_tag(char *tag)
{
        char *p1, *p2, buf[256];
        int temp;
		int ret=0;

        if(tag == NULL)
        {
                LOG(LOG_MSG, "MediaTagError: Tag == NULL\n");
                return ret;
        }
        p1 = strchr(tag, '|');
        p2 = strrchr(tag, '|');
        if(p1 == NULL || p2==NULL)
        {
                LOG(LOG_MSG, "MediaTagError: Parameter == NULL (%x %x)\n", p1,
                        p2);
                return ret;
        }
        *p1++=0;
        *p2++=0;

        if(strstr(tag+1,".ogg"))
        {
            sound_play_music(tag+1, options.music_volume,2000,atoi(p2),atoi(p1),MUSIC_MODE_NORMAL);
			ret = 1; /* because we have called sound_play_music, we don't must fade out extern */
        }
        else if(strstr(tag+1,".png"))
        {
            media_show_update = 2;
            /* because we chain this to map_scroll, but map_scroll can
            * come behind the draw_info cmd... sigh*/

            /* first, we look in our media buffers.. perhaps this is still buffered
            * is so, just update the paramter and fire it up */
            if(!strcmp(media_file[media_count].name, tag+1))
            {
                media_show = media_count;
                media_file[media_count].p1=atoi(p1);
                media_file[media_count].p2=atoi(p2);
                return ret;
            }
            /* if not loaded, we overwrite our oldest buffered media file */

            media_show = MEDIA_SHOW_NO;
            temp = (media_count+1)%MEDIA_MAX;
            if(media_file[temp].data) /* if some here, kick it*/
            {
                media_file[temp].type = MEDIA_TYPE_NO;
                FreeMemory(&media_file[temp].data);
            }
            sprintf(buf,"%s%s",GetMediaDirectory(), tag+1);
            if(!(media_file[temp].data = sprite_load_file(buf,0)))
                return ret;

            media_file[temp].type = MEDIA_TYPE_PNG;
            strcpy(media_file[temp].name, tag+1);
            media_count = temp;
            media_show = media_count;
        }
		return ret;
}


void blt_window_slider(_Sprite *slider, int maxlen, int winlen, int startoff, int x, int y)
{
    SDL_Rect box;
    double temp;
    int startpos;

    if(maxlen < winlen)
        maxlen=winlen;
    if(startoff+winlen >maxlen)
        maxlen=startoff+winlen;

    box.x=0;
    box.y=0;
    box.w=slider->bitmap->w;

    /* now we have 100% = 1.0 to 0% = 0.0 of the length */
    temp = (double)winlen/(double)maxlen; /* between 0.0 <-> 1.0 */
    startpos = (int)((double)startoff *((double)slider->bitmap->h/(double )maxlen)); /* startpixel */
    temp = (double)slider->bitmap->h*temp; 
    box.h = (Uint16) temp;

    if(startoff+winlen >=maxlen && startpos+box.h<slider->bitmap->h)
		startpos ++;

   sprite_blt(slider,x,y+startpos, &box, NULL);
}


void show_spelllist(void)
{
    int y, x,i;
    
    x= SCREEN_XLEN/2-Bitmaps[BITMAP_SPELLLIST]->bitmap->w/2;
    y= SCREEN_YLEN/2-Bitmaps[BITMAP_SPELLLIST]->bitmap->h/2;
    sprite_blt(Bitmaps[BITMAP_SPELLLIST],x, y, NULL, NULL);

    sprite_blt(Bitmaps[BITMAP_SPELLLIST_BUTTON],x+108+33*spell_list_set.group_nr, y+42, NULL, NULL);
    
    if(spell_list_set.class_nr)
        sprite_blt(Bitmaps[BITMAP_SPELLLIST_SLIDERG],x+270, y+69+13*spell_list_set.entry_nr, NULL, NULL);
    else
        sprite_blt(Bitmaps[BITMAP_SPELLLIST_SLIDER],x+19, y+69+13*spell_list_set.entry_nr, NULL, NULL);

    for(i=0;i<SPELL_LIST_ENTRY;i++)
    {
        if(spell_list[spell_list_set.group_nr].entry[0][i].flag==LIST_ENTRY_KNOWN)
            StringBlt(ScreenSurface, &SystemFont,
            spell_list[spell_list_set.group_nr].entry[0][i].name
                                        ,x+24, y+69+i*13, COLOR_WHITE, NULL, NULL);

        if(spell_list[spell_list_set.group_nr].entry[1][i].flag==LIST_ENTRY_KNOWN)
            StringBlt(ScreenSurface, &SystemFont,
            spell_list[spell_list_set.group_nr].entry[1][i].name
                                       ,x+275, y+69+i*13, COLOR_WHITE, NULL, NULL);
    }

    if(spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].flag==LIST_ENTRY_KNOWN)
    {
        sprite_blt(spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].icon,x+14,y+421, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[0][0]
            ,x+55, y+418+0*13, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[1][0]
            ,x+55, y+418+1*13, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][spell_list_set.entry_nr].desc[2][0]
            ,x+55, y+418+2*13, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &spell_list[spell_list_set.group_nr].entry[spell_list_set.class_nr][skill_list_set.entry_nr].desc[3][0]
            ,x+55, y+418+3*13, COLOR_BLACK, NULL, NULL);
        
    }
}

void show_skilllist(void)
{
    int y, x, i;
    char buf[64];
    
    x= SCREEN_XLEN/2-Bitmaps[BITMAP_SKILL_LIST]->bitmap->w/2;
    y= SCREEN_YLEN/2-Bitmaps[BITMAP_SKILL_LIST]->bitmap->h/2;
    sprite_blt(Bitmaps[BITMAP_SKILL_LIST],x, y, NULL, NULL);

    sprite_blt(Bitmaps[BITMAP_SKILL_LIST_BUTTON],x+21+60*skill_list_set.group_nr, y+46, NULL, NULL);

    sprite_blt(Bitmaps[BITMAP_SKILL_LIST_SLIDER],x+19, y+69+13*skill_list_set.entry_nr, NULL, NULL);
    
    for(i=0;i<SKILL_LIST_ENTRY;i++)
    {
        if(skill_list[skill_list_set.group_nr].entry[i].flag==LIST_ENTRY_KNOWN)
        {
            StringBlt(ScreenSurface, &SystemFont,
                    skill_list[skill_list_set.group_nr].entry[i].name
                                    ,x+24, y+69+i*13, COLOR_WHITE, NULL, NULL);
            if(skill_list[skill_list_set.group_nr].entry[i].exp != -1)
                sprintf(buf,"%d",skill_list[skill_list_set.group_nr].entry[i].exp_level);
            else
                strcpy(buf,"--");
            StringBlt(ScreenSurface, &SystemFont,buf,x+320, y+69+i*13, COLOR_WHITE, NULL, NULL);
            if(skill_list[skill_list_set.group_nr].entry[i].exp != -1)
                sprintf(buf,"%d",skill_list[skill_list_set.group_nr].entry[i].exp);
            StringBlt(ScreenSurface, &SystemFont,buf,x+350, y+69+i*13, COLOR_WHITE, NULL, NULL);
        }
    }


    if(skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].flag>=LIST_ENTRY_KNOWN)
    {
        sprite_blt(skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].icon,x+14,y+421, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[0][0]
            ,x+55, y+418+0*13, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[1][0]
            ,x+55, y+418+1*13, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[2][0]
            ,x+55, y+418+2*13, COLOR_BLACK, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont,
            &skill_list[skill_list_set.group_nr].entry[skill_list_set.entry_nr].desc[3][0]
            ,x+55, y+418+3*13, COLOR_BLACK, NULL, NULL);
        
    }
}

void read_spells(void)
{
    int i,panel;
    char type, nchar, *tmp, *tmp2;
    FILE *stream;
    char line[255], name[255], d1[255], d2[255], d3[255], d4[255], icon[128];
    
    if( (stream = fopen("./spells.dat", "r" )) != NULL )
    {
        for(i=0;;i++)
        {
            if( fgets( line, 255, stream ) == NULL)
                    break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(name, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            sscanf(line,"%c %c %d %s", &type, &nchar, &panel, icon);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d1, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d2, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d3, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d4, tmp+1);
            panel--;
            spell_list[panel].entry[type=='w'?0:1][nchar-'a'].flag = LIST_ENTRY_USED;   
            strcpy(spell_list[panel].entry[type=='w'?0:1][nchar-'a'].icon_name, icon);
            sprintf(line,"%s%s", GetIconDirectory(),icon);
            spell_list[panel].entry[type=='w'?0:1][nchar-'a'].icon = sprite_load_file(line,0);
            
            strcpy(spell_list[panel].entry[type=='w'?0:1][nchar-'a'].name, name);             
            strcpy(spell_list[panel].entry[type=='w'?0:1][nchar-'a'].desc[0], d1);             
            strcpy(spell_list[panel].entry[type=='w'?0:1][nchar-'a'].desc[1], d2);             
            strcpy(spell_list[panel].entry[type=='w'?0:1][nchar-'a'].desc[2], d3);             
            strcpy(spell_list[panel].entry[type=='w'?0:1][nchar-'a'].desc[3], d4);             
        }
        fclose( stream );
    }
}

void read_skills(void)
{
    int i,panel;
    char nchar, *tmp, *tmp2;
    FILE *stream;
    char line[255], name[255], d1[255], d2[255], d3[255], d4[255], icon[128];
    
    if( (stream = fopen("./skills.dat", "r" )) != NULL )
    {
        for(i=0;;i++)
        {
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(name, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            sscanf(line,"%d %c %s", &panel, &nchar, icon);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d1, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d2, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d3, tmp+1);
            if( fgets( line, 255, stream ) == NULL)
                break;
            line[250]=0;
            tmp = strchr(line,'"' );            
            tmp2 = strchr(tmp+1,'"' );
            *tmp2=0;
            strcpy(d4, tmp+1);

            skill_list[panel].entry[nchar-'a'].flag = LIST_ENTRY_USED;   
            skill_list[panel].entry[nchar-'a'].exp=0;   
            skill_list[panel].entry[nchar-'a'].exp_level=0;   

            strcpy(skill_list[panel].entry[nchar-'a'].icon_name, icon);
            sprintf(line,"%s%s", GetIconDirectory(),icon);
            skill_list[panel].entry[nchar-'a'].icon = sprite_load_file(line,0);
            
            strcpy(skill_list[panel].entry[nchar-'a'].name, name);             
            strcpy(skill_list[panel].entry[nchar-'a'].desc[0], d1);             
            strcpy(skill_list[panel].entry[nchar-'a'].desc[1], d2);             
            strcpy(skill_list[panel].entry[nchar-'a'].desc[2], d3);             
            strcpy(skill_list[panel].entry[nchar-'a'].desc[3], d4);             
        }
        fclose( stream );
    }
}


void show_quickslots(int x, int y)
{
	int i;

	update_quickslots(-1);
    sprite_blt(Bitmaps[BITMAP_QUICKSLOTS],x, y, NULL, NULL);

	for(i=0;i<MAX_QUICK_SLOTS;i++)
	{
		if(quick_slots[i] != -1)
			blt_inventory_face_from_tag(quick_slots[i],x+quickslots_pos[i][0],y+quickslots_pos[i][1]);
		
	}
}

void update_quickslots(int del_item)
{
	int i, tmp_slots[BITMAP_QUICKSLOTS];

	memset(tmp_slots,-1,sizeof(tmp_slots));

	for(i=0;i<MAX_QUICK_SLOTS;i++)
	{
		if(quick_slots[i] == del_item)
			quick_slots[i]=-1;
		if(quick_slots[i] == -1)
			continue;
		if(!locate_item_from_item(cpl.ob , quick_slots[i])) 
			quick_slots[i]=-1;
	}


}

void show_group(int x, int y)
{
	int s;
	
	for(s=0;s<MAX_GROUP_MEMBER;s++)
	{
		sprite_blt(Bitmaps[BITMAP_GROUP],x+group_pos[s][0]+1, y+group_pos[s][1]+1, NULL, NULL);
	}
	/*
    StringBlt(ScreenSurface, &SystemFont,cpl.name,52, 525,COLOR_DEFAULT, NULL, NULL);
    sprintf(buf, "L %03d",cpl.stats.level);
    StringBlt(ScreenSurface, &Font6x3Out,buf,24, 522,COLOR_DEFAULT, NULL, NULL);
	*/
}


void show_target(int x, int y)
{
	char *ptr=NULL;

	sprite_blt(Bitmaps[cpl.target_mode?BITMAP_TARGET_ATTACK:BITMAP_TARGET_NORMAL],x, y, NULL, NULL);
	if(cpl.target_code==0)
	{
		if(cpl.target_mode)
			ptr = "target self (hold attack)";
		else
			ptr = "target self";
	}
	else if(cpl.target_code==1)
	{
		if(cpl.target_mode)
			ptr = "target and attack enemy";
		else
			ptr = "target enemy";
	}
	else if(cpl.target_code==2)
	{
		if(cpl.target_mode)
			ptr = "target friend (hold attack)";
		else
			ptr = "target friend";
	}
	if(ptr)
	{
		StringBlt(ScreenSurface, &SystemFont,cpl.target_name ,x+30, y, COLOR_WHITE, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont,ptr ,x+30, y+11, COLOR_WHITE, NULL, NULL);
	}
}
