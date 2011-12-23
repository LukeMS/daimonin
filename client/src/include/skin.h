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

#ifndef __SKIN_H
#define __SKIN_H

typedef enum skin_sprite_id_t
{
    SKIN_SPRITE_FONTSMALL,
    SKIN_SPRITE_FONTTINYOUT,
    SKIN_SPRITE_FONTBIGOUT,
    SKIN_SPRITE_FONTSMALLOUT,
    SKIN_SPRITE_FONTMEDIUM,
    SKIN_SPRITE_FONTMEDIUMOUT,
    SKIN_SPRITE_INTRO,
    SKIN_SPRITE_PROGRESS,
    SKIN_SPRITE_PROGRESS_BACK,
    SKIN_SPRITE_BLACKTILE,
    SKIN_SPRITE_GRID,
    SKIN_SPRITE_LOGIN_INP,
    SKIN_SPRITE_HP,
    SKIN_SPRITE_SP,
    SKIN_SPRITE_GRACE,
    SKIN_SPRITE_FOOD,
    SKIN_SPRITE_HP_BACK,
    SKIN_SPRITE_SP_BACK,
    SKIN_SPRITE_GRACE_BACK,
    SKIN_SPRITE_FOOD_BACK,
    SKIN_SPRITE_APPLY,
    SKIN_SPRITE_FOOD2,
    SKIN_SPRITE_UNPAID,
    SKIN_SPRITE_CURSED,
    SKIN_SPRITE_DAMNED,
    SKIN_SPRITE_LOCK,
    SKIN_SPRITE_MAGIC,
    SKIN_SPRITE_UNIDENTIFIED,
    SKIN_SPRITE_RANGE,
    SKIN_SPRITE_RANGE_MARKER,
    SKIN_SPRITE_RANGE_CTRL,
    SKIN_SPRITE_RANGE_CTRL_NO,
    SKIN_SPRITE_RANGE_SKILL,
    SKIN_SPRITE_RANGE_SKILL_NO,
    SKIN_SPRITE_RANGE_THROW,
    SKIN_SPRITE_RANGE_THROW_NO,
    SKIN_SPRITE_RANGE_TOOL,
    SKIN_SPRITE_RANGE_TOOL_NO,
    SKIN_SPRITE_RANGE_WIZARD,
    SKIN_SPRITE_RANGE_WIZARD_NO,
    SKIN_SPRITE_RANGE_PRIEST,
    SKIN_SPRITE_RANGE_PRIEST_NO,
    SKIN_SPRITE_CMARK_START,
    SKIN_SPRITE_CMARK_END,
    SKIN_SPRITE_CMARK_MIDDLE,
    SKIN_SPRITE_TWIN_SCROLL,
    SKIN_SPRITE_INV_SCROLL,
    SKIN_SPRITE_BELOW_SCROLL,
    SKIN_SPRITE_NUMBER,
    SKIN_SPRITE_DEATH,
    SKIN_SPRITE_CONFUSE,
    SKIN_SPRITE_PARALYZE,
    SKIN_SPRITE_SCARED,
    SKIN_SPRITE_BLIND,
    SKIN_SPRITE_EXCLUSIVE_EFFECT,
    SKIN_SPRITE_ENEMY1,
    SKIN_SPRITE_ENEMY2,
    SKIN_SPRITE_PROBE,
    SKIN_SPRITE_QUICKSLOTS,
    SKIN_SPRITE_QUICKSLOTSV,
    SKIN_SPRITE_INVENTORY,
    SKIN_SPRITE_GROUP,
    SKIN_SPRITE_EXP_BORDER,
    SKIN_SPRITE_EXP_SLIDER,
    SKIN_SPRITE_EXP_BUBBLE1,
    SKIN_SPRITE_EXP_BUBBLE2,
    SKIN_SPRITE_BELOW,
    SKIN_SPRITE_FLINE,
    SKIN_SPRITE_TARGET_ATTACK,
    SKIN_SPRITE_TARGET_TALK,
    SKIN_SPRITE_TARGET_NORMAL,
    SKIN_SPRITE_LOADING,
    SKIN_SPRITE_WARN_HP,
    SKIN_SPRITE_STATS_BG,
    SKIN_SPRITE_WARN_WEIGHT,
    SKIN_SPRITE_LOGO270,
    SKIN_SPRITE_DIALOG_BG,
    SKIN_SPRITE_DIALOG_TITLE_OPTIONS,
    SKIN_SPRITE_DIALOG_TITLE_KEYBIND,
    SKIN_SPRITE_DIALOG_TITLE_SKILL,
    SKIN_SPRITE_DIALOG_TITLE_SPELL,
    SKIN_SPRITE_DIALOG_TITLE_CREATION,
    SKIN_SPRITE_DIALOG_TITLE_LOGIN,
    SKIN_SPRITE_DIALOG_ICON_BG_ACTIVE,
    SKIN_SPRITE_DIALOG_ICON_BG_INACTIVE,
    SKIN_SPRITE_DIALOG_ICON_BG_NEGATIVE,
    SKIN_SPRITE_DIALOG_ICON_BG_POSITIVE,
    SKIN_SPRITE_DIALOG_ICON_FG_ACTIVE,
    SKIN_SPRITE_DIALOG_ICON_FG_INACTIVE,
    SKIN_SPRITE_DIALOG_ICON_FG_SELECTED,
    SKIN_SPRITE_DIALOG_BUTTON_SELECTED,
    SKIN_SPRITE_DIALOG_BUTTON_UP_PREFIX,
    SKIN_SPRITE_DIALOG_BUTTON_DOWN_PREFIX,
    SKIN_SPRITE_DIALOG_BUTTON_UP,
    SKIN_SPRITE_DIALOG_BUTTON_DOWN,
    SKIN_SPRITE_DIALOG_TAB_START,
    SKIN_SPRITE_DIALOG_TAB,
    SKIN_SPRITE_DIALOG_TAB_STOP,
    SKIN_SPRITE_DIALOG_TAB_SEL,
    SKIN_SPRITE_DIALOG_CHECKER,
    SKIN_SPRITE_DIALOG_RANGE_OFF,
    SKIN_SPRITE_DIALOG_RANGE_L,
    SKIN_SPRITE_DIALOG_RANGE_R,
    SKIN_SPRITE_TARGET_HP,
    SKIN_SPRITE_TARGET_HP_B,
    SKIN_SPRITE_ALPHA,
    SKIN_SPRITE_SLIDER_UP,
    SKIN_SPRITE_SLIDER_DOWN,
    SKIN_SPRITE_SLIDER,
    SKIN_SPRITE_GROUP_CLEAR,
    SKIN_SPRITE_EXP_SKILL_BORDER,
    SKIN_SPRITE_EXP_SKILL_LINE,
    SKIN_SPRITE_EXP_SKILL_BUBBLE,
    SKIN_SPRITE_OPTIONS_HEAD,
    SKIN_SPRITE_OPTIONS_KEYS,
    SKIN_SPRITE_OPTIONS_SETTINGS,
    SKIN_SPRITE_OPTIONS_LOGOUT,
    SKIN_SPRITE_OPTIONS_BACK,
    SKIN_SPRITE_OPTIONS_MARK_LEFT,
    SKIN_SPRITE_OPTIONS_MARK_RIGHT,
    SKIN_SPRITE_OPTIONS_ALPHA,
    SKIN_SPRITE_PENTAGRAM,
    SKIN_SPRITE_BUTTONQ_UP,
    SKIN_SPRITE_BUTTONQ_DOWN,
    SKIN_SPRITE_TRAPED,
    SKIN_SPRITE_PRAY,
    SKIN_SPRITE_WAND,
    SKIN_SPRITE_GROUP_INVITE,
    SKIN_SPRITE_BUTTON_BLACK_UP,
    SKIN_SPRITE_BUTTON_BLACK_DOWN,
    SKIN_SPRITE_SMALL_UP,
    SKIN_SPRITE_SMALL_DOWN,
    SKIN_SPRITE_GROUP_MANA,
    SKIN_SPRITE_GROUP_GRACE,
    SKIN_SPRITE_GROUP_HP,
    SKIN_SPRITE_GUI_NPC_TOP,
    SKIN_SPRITE_GUI_NPC_MIDDLE,
    SKIN_SPRITE_GUI_NPC_BOTTOM,
    SKIN_SPRITE_GUI_NPC_PANEL,
    SKIN_SPRITE_NPC_INT_SLIDER,
    SKIN_SPRITE_JOURNAL,
    SKIN_SPRITE_MSCURSOR_MOVE,
    SKIN_SPRITE_RESIST_BG,
    SKIN_SPRITE_MAIN_LVL_BG,
    SKIN_SPRITE_SKILL_EXP_BG,
    SKIN_SPRITE_REGEN_BG,
    SKIN_SPRITE_SKILL_LVL_BG,
    SKIN_SPRITE_MENU_BUTTONS,
    SKIN_SPRITE_GROUP_BG,
    SKIN_SPRITE_GROUP_BG_BOTTOM,
    SKIN_SPRITE_DOLL_BG,
    SKIN_SPRITE_PLAYER_INFO,
    SKIN_SPRITE_TARGET_BG,
    SKIN_SPRITE_INV_BG,
    SKIN_SPRITE_TEXTINPUT,
    SKIN_SPRITE_STIMER,
    SKIN_SPRITE_CLOSEBUTTON,
    SKIN_SPRITE_LOCATOR_MAP,
    SKIN_SPRITE_LOCATOR_CLIENT,
    SKIN_SPRITE_LOCATOR_PLAYER_THAT,
    SKIN_SPRITE_LOCATOR_PLAYER_THIS,
    SKIN_SPRITE_LOCATOR_SERVER_THAT,
    SKIN_SPRITE_LOCATOR_SERVER_THIS,

    SKIN_SPRITE_NROF // must be last element
}
skin_sprite_id_t;

typedef struct skin_prefs_t
{
    uint32  chat_admin;
    uint32  chat_buddy;
    uint32  chat_channel;
    uint32  chat_eavesdrop;
    uint32  chat_emote;
    uint32  chat_gsay;
    uint32  chat_say;
    uint32  chat_shout;
    uint32  chat_tell;
    uint32  dialog_rows0;
    uint32  dialog_rows1;
    uint32  dialog_rowsS;
    uint32  ecc_emphasis;
    uint32  ecc_strong;
    uint32  ecc_intertitle;
    uint32  ecc_hypertext;
    uint32  pname_admin;
    uint32  pname_leader;
    uint32  pname_member;
    uint32  pname_other;
    uint32  pname_self;
    uint32  target_grey;
    uint32  target_green;
    uint32  target_blue;
    uint32  target_yellow;
    uint32  target_orange;
    uint32  target_red;
    uint32  target_purple;
    uint32  widget_info;
    uint32  widget_key;
    uint32  widget_title;
    uint32  widget_valueEq;
    uint32  widget_valueHi;
    uint32  widget_valueLo;
    uint32  input_string;
    uint32  input_caret;
    uint8   effect_width;
    uint8   effect_height;
    char   *effect_eating;
    char   *effect_sleeping;
    uint8   item_size;
    uint8   icon_size;
}
skin_prefs_t;

extern _Sprite      *skin_sprites[SKIN_SPRITE_NROF];
extern skin_prefs_t  skin_prefs;

extern void skin_load_bitmaps(skin_sprite_id_t nrof);
extern void skin_free_bitmaps(void);
extern void skin_reload(void);
extern void skin_load_prefs(void);

#endif /* ifndef __SKIN_H */
