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

/* For loading, use SKIN_FONT_xx in the other modules. */
static char *FontName[SKIN_FONT_NROF] =
{
    "tiny.png",
    "small.png",
    "medium.png",
    "large.png",
    "huge.png",
    "booknormal.png",
    "booktitle.png",
    "npcicon.png",
    "npcnormal.png",
    "npctitle.png",
    "heading.png",
};

/* For loading, use SKIN_SPRITE_xx in the other modules. */
static char *BitmapName[SKIN_SPRITE_NROF] =
{
    "intro.png",
    "progress.png",
    "progress_back.png",
    "black_tile.png",
    "grid.png",
    "login_inp.png",
    "hp.png",
    "sp.png",
    "grace.png",
    "food.png",
    "hp_back.png",
    "sp_back.png",
    "grace_back.png",
    "food_back.png",
    "apply.png",
    "food2.png",
    "unpaid.png",
    "cursed.png",
    "damned.png",
    "lock.png",
    "magic.png",
    "unidentified.png",
    "range.png",
    "range_marker.png",
    "range_ctrl.png",
    "range_ctrl_no.png",
    "range_skill.png",
    "range_skill_no.png",
    "range_throw.png",
    "range_throw_no.png",
    "range_tool.png",
    "range_tool_no.png",
    "range_wizard.png",
    "range_wizard_no.png",
    "range_priest.png",
    "range_priest_no.png",
    "cmark_start.png",
    "cmark_end.png",
    "cmark_middle.png",
    "inv_scroll.png",
    "below_scroll.png",
    "number.png",
    "death.png",
    "confused.png",
    "paralyzed.png",
    "scared.png",
    "blind.png",
    "exclusive_effect.png",
    "enemy1.png",
    "enemy2.png",
    "probe.png",
    "quickslots.png",
    "quickslotsv.png",
    "inventory.png",
    "group.png",
    "exp_border.png",
    "exp_line.png",
    "exp_bubble.png",
    "exp_bubble2.png",
    "below.png",
    "frame_line.png",
    "target_attack.png",
    "target_talk.png",
    "target_normal.png",
    "loading.png",
    "warn_hp.png",
    "main_stats.png",
    "warn_weight.png",
    "logo270.png",
    "dialog_bg.png",
    "dialog_title_options.png",
    "dialog_title_keybind.png",
    "dialog_title_skill.png",
    "dialog_title_spell.png",
    "dialog_title_creation.png",
    "dialog_title_login.png",
    "dialog_icon_bg_active.png",
    "dialog_icon_bg_inactive.png",
    "dialog_icon_bg_negative.png",
    "dialog_icon_bg_positive.png",
    "dialog_icon_fg_active.png",
    "dialog_icon_fg_inactive.png",
    "dialog_icon_fg_selected.png",
    "dialog_button_selected.png",
    "dialog_button_up_prefix.png",
    "dialog_button_down_prefix.png",
    "dialog_button_up.png",
    "dialog_button_down.png",
    "dialog_tab_start.png",
    "dialog_tab.png",
    "dialog_tab_stop.png",
    "dialog_tab_sel.png",
    "dialog_checker.png",
    "dialog_range_off.png",
    "dialog_range_l.png",
    "dialog_range_r.png",
    "target_hp.png",
    "target_hp_b.png",
    "alpha.png",
    "slider/hbarge.png",
    "slider/hcanal.png",
    "slider/left.png",
    "slider/right.png",
    "slider/hl_hbarge.png",
    "slider/hl_hcanal.png",
    "slider/hl_left.png",
    "slider/hl_right.png",
    "slider/vbarge.png",
    "slider/vcanal.png",
    "slider/down.png",
    "slider/up.png",
    "slider/hl_vbarge.png",
    "slider/hl_vcanal.png",
    "slider/hl_down.png",
    "slider/hl_up.png",
    "group_clear.png",
    "exp_skill_border.png",
    "exp_skill_line.png",
    "exp_skill_bubble.png",
    "options_head.png",
    "options_keys.png",
    "options_settings.png",
    "options_logout.png",
    "options_back.png",
    "options_mark_left.png",
    "options_mark_right.png",
    "options_alpha.png",
    "pentagram.png",
    "quad_button_up.png",
    "quad_button_down.png",
    "traped.png",
    "pray.png",
    "wand.png",
    "invite.png",
    "dialog_button_black_up.png",
    "dialog_button_black_down.png",
    "button_small_up.png",
    "button_small_down.png",
    "group_mana.png",
    "group_grace.png",
    "group_hp.png",
    "npc_interface_top.png",
    "npc_interface_middle.png",
    "npc_interface_bottom.png",
    "npc_interface_panel.png",
    "npc_int_slider.png",
    "journal.png",
    "mouse_cursor_move.png",
    "resist_bg.png",
    "main_level_bg.png",
    "skill_exp_bg.png",
    "regen_bg.png",
    "skill_lvl_bg.png",
    "menu_buttons.png",
    "group_bg2.png",
    "group_bg2_bottom.png",
    "player_doll_bg.png",
    "player_info_bg.png",
    "target_bg.png",
    "inventory_bg.png",
    "textinput.png",
    "stimer.png",
    "closeb.png",
    "locator/map.png",
    "locator/client.png",
    "locator/player_that.png",
    "locator/player_this.png",
    "locator/server_that.png",
    "locator/server_this.png",
    "vim.png",
    "tooltip.png",
};

_Sprite      *skin_fonts[SKIN_FONT_NROF],
             *skin_sprites[SKIN_SPRITE_NROF];
skin_prefs_t  skin_prefs;

void skin_deinit(void)
{
    uint16 i;

    for (i = 0; i < SKIN_FONT_NROF; i++)
    {
        sprite_free_sprite(skin_fonts[i]);
    }

    for (i = 0; i < SKIN_SPRITE_NROF; i++)
    {
        sprite_free_sprite(skin_sprites[i]);
    }

    FREE(skin_prefs.effect_eating);
    FREE(skin_prefs.effect_sleeping);
}

void skin_load_fonts (void)
{
    skin_font_id_t i;

    /* add later better error handling here*/
    for (i = 0; i < SKIN_FONT_NROF; i++)
    {
        char buf[SMALL_BUF];

        sprintf(buf, "%s/%s", DIR_FONTS, FontName[i]);
        skin_fonts[i] = sprite_load(buf, NULL);

        if (!skin_fonts[i] ||
            !skin_fonts[i]->bitmap)
        {
            LOG(LOG_FATAL, "Couldn't load font '%s'!\n", buf);
        }

        if (!skin_fonts[i]->bitmap->format->palette)
        {
            LOG(LOG_FATAL, "Font '%s' has no palette!\n", buf);
        }

        if (skin_fonts[i]->bitmap->format->palette->ncolors < 3)
        {
            LOG(LOG_FATAL, "Font '%s' has too few colours (must be >= 3) %d!\n",
                buf);
        }
    }
}

void skin_load_bitmaps(skin_sprite_id_t nrof)
{
    skin_sprite_id_t i;

    /* add later better error handling here*/
    for (i = 0; i < nrof; i++)
    {
        char buf[SMALL_BUF];

        sprintf(buf, "%s/%s", DIR_BITMAPS, BitmapName[i]);
        skin_sprites[i] = sprite_load(buf, NULL);

        if (!skin_sprites[i] ||
            !skin_sprites[i]->bitmap)
        {
            LOG(LOG_FATAL, "Couldn't load bitmap '%s'!\n", buf);
        }
    }
}

void skin_reload(void)
{
    skin_deinit();
    skin_load_fonts();
    skin_load_bitmaps(SKIN_SPRITE_NROF);
    font_init();
    skin_default_prefs();
    skin_load_prefs(FILE_SKINDEF);
    skin_load_prefs(FILE_MASTER_SKINDEF);
}

void skin_default_prefs(void)
{
    skin_prefs.chat_gmaster = NDI_COLR_RED;
    skin_prefs.chat_buddy = NDI_COLR_SILVER;
    skin_prefs.chat_channel = NDI_COLR_MAROON;
    skin_prefs.chat_eavesdrop = NDI_COLR_FUSCHIA;
    skin_prefs.chat_emote = NDI_COLR_TEAL;
    skin_prefs.chat_gsay = NDI_COLR_YELLOW;
    skin_prefs.chat_say = NDI_COLR_WHITE;
    skin_prefs.chat_shout = NDI_COLR_ORANGE;
    skin_prefs.chat_tell = NDI_COLR_AQUA;
    skin_prefs.dialog_rows0 = NDI_COLR_OLIVE;
    skin_prefs.dialog_rows1 = NDI_COLR_MAROON;
    skin_prefs.dialog_rowsS = NDI_COLR_BLUE;
    skin_prefs.ecc_emphasis = NDI_COLR_LIME;
    skin_prefs.ecc_strong = NDI_COLR_YELLOW;
    skin_prefs.ecc_intertitle = NDI_COLR_ORANGE;
    skin_prefs.magic_prayer = NDI_COLR_GREEN;
    skin_prefs.magic_spell = NDI_COLR_BLUE;
    skin_prefs.pname_gmaster = NDI_COLR_RED;
    skin_prefs.pname_leader = NDI_COLR_LIME;
    skin_prefs.pname_member = NDI_COLR_YELLOW;
    skin_prefs.pname_other = NDI_COLR_WHITE;
    skin_prefs.pname_self = NDI_COLR_SILVER;
    skin_prefs.target_grey = NDI_COLR_GREY;
    skin_prefs.target_green = NDI_COLR_LIME;
    skin_prefs.target_blue = NDI_COLR_BLUE;
    skin_prefs.target_yellow = NDI_COLR_YELLOW;
    skin_prefs.target_orange = NDI_COLR_ORANGE;
    skin_prefs.target_red = NDI_COLR_RED;
    skin_prefs.target_purple = NDI_COLR_PURPLE;
    skin_prefs.widget_info = NDI_COLR_ORANGE;
    skin_prefs.widget_key = NDI_COLR_AQUA;
    skin_prefs.widget_title = NDI_COLR_SILVER;
    skin_prefs.widget_valueEq = NDI_COLR_WHITE;
    skin_prefs.widget_valueHi = NDI_COLR_LIME;
    skin_prefs.widget_valueLo = NDI_COLR_RED;
    skin_prefs.input_string = NDI_COLR_WHITE;
    skin_prefs.input_caret = NDI_COLR_RED;
    skin_prefs.scale_fire = SPRITE_COLRSCALE_GREY;
    skin_prefs.mask_fire = NDI_COLR_ORANGE;
    skin_prefs.scale_cold = SPRITE_COLRSCALE_GREY;
    skin_prefs.mask_cold = NDI_COLR_AQUA;
    skin_prefs.scale_electricity = SPRITE_COLRSCALE_NEGATIVE;
    skin_prefs.mask_electricity = NDI_COLR_BLACK;
    skin_prefs.scale_light = SPRITE_COLRSCALE_INTENSITY;
    skin_prefs.mask_light = NDI_COLR_SILVER;
    skin_prefs.scale_shadow = SPRITE_COLRSCALE_INTENSITY;
    skin_prefs.mask_shadow = 0x303030; // No suitable NDI_COLR
    skin_prefs.scale_fogofwar = SPRITE_COLRSCALE_GREY;
    skin_prefs.mask_fogofwar = NDI_COLR_BLUE;
    skin_prefs.scale_infravision = SPRITE_COLRSCALE_GREY;
    skin_prefs.mask_infravision = NDI_COLR_MAROON;
    skin_prefs.scale_xrayvision = SPRITE_COLRSCALE_GREY;
    skin_prefs.mask_xrayvision = NDI_COLR_BLACK;
    skin_prefs.effect_width = 9;
    skin_prefs.effect_height = 16;
    MALLOC_STRING(skin_prefs.effect_eating, "Nyom! ");
    MALLOC_STRING(skin_prefs.effect_sleeping, "Zzz! ");
    skin_prefs.item_size = 32;
    skin_prefs.icon_size = 8;
}

void skin_load_prefs(const char *filename)
{
    PHYSFS_File *handle;
    char         buf[SMALL_BUF];

    /* No file? Nothing to do. */
    if (!PHYSFS_exists(filename))
    {
        return;
    }

    /* Log what we're doing. */
    LOG(LOG_MSG, "Loading '%s'... ", filename);

    /* Open the file for reading. */
    if (!(handle = PHYSFS_openRead(filename)))
    {
        LOG(LOG_ERROR, "FAILED (%s)!\n", PHYSFS_getLastError());

        return;
    }

    /* Read line by line. */
    while (PHYSFS_readString(handle, buf, sizeof(buf)) >= 0)
    {
        char   *key,
               *val;

        /* Skip comments and blank lines. */
        if (buf[0]=='#' ||
            buf[0]=='\0')
        {
            continue;
        }

        if (!(val = strchr(buf, ':')))
        {
            LOG(LOG_ERROR, "Ignoring malformed entry: '%s'\n", buf);

            continue;
        }

        key = buf;
        *val = '\0';
        val += 2;

        if (!strcmp(key, "chat_gmaster"))
        {
            skin_prefs.chat_gmaster = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_buddy"))
        {
            skin_prefs.chat_buddy = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_channel"))
        {
            skin_prefs.chat_channel = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_eavesdrop"))
        {
            skin_prefs.chat_eavesdrop = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_emote"))
        {
            skin_prefs.chat_emote = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_gsay"))
        {
            skin_prefs.chat_gsay = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_say"))
        {
            skin_prefs.chat_say = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_shout"))
        {
            skin_prefs.chat_shout = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "chat_tell"))
        {
            skin_prefs.chat_tell = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "dialog_rows0"))
        {
            skin_prefs.dialog_rows0 = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "dialog_rows1"))
        {
            skin_prefs.dialog_rows1 = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "dialog_rowsS"))
        {
            skin_prefs.dialog_rowsS = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "ecc_emphasis"))
        {
            skin_prefs.ecc_emphasis = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "ecc_strong"))
        {
            skin_prefs.ecc_strong = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "ecc_intertitle"))
        {
            skin_prefs.ecc_intertitle = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "ecc_hypertext"))
        {
            skin_prefs.ecc_hypertext = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "magic_prayer"))
        {
            skin_prefs.magic_prayer = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "magic_spell"))
        {
            skin_prefs.magic_spell = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "pname_gmaster"))
        {
            skin_prefs.pname_gmaster = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "pname_leader"))
        {
            skin_prefs.pname_leader = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "pname_member"))
        {
            skin_prefs.pname_member = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "pname_other"))
        {
            skin_prefs.pname_other = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "pname_self"))
        {
            skin_prefs.pname_self = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "target_grey"))
        {
            skin_prefs.target_grey = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "target_green"))
        {
            skin_prefs.target_green = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "target_blue"))
        {
            skin_prefs.target_blue = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "target_yellow"))
        {
            skin_prefs.target_yellow = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "target_orange"))
        {
            skin_prefs.target_orange = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "target_red"))
        {
            skin_prefs.target_red = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "target_purple"))
        {
            skin_prefs.target_purple = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "widget_info"))
        {
            skin_prefs.widget_info = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "widget_key"))
        {
            skin_prefs.widget_key = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "widget_title"))
        {
            skin_prefs.widget_title = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "widget_valueEq"))
        {
            skin_prefs.widget_valueEq = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "widget_valueHi"))
        {
            skin_prefs.widget_valueHi = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "widgetog_valueLo"))
        {
            skin_prefs.widget_valueLo = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "input_string"))
        {
            skin_prefs.input_string = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "input_caret"))
        {
            skin_prefs.input_caret = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_fire"))
        {
            skin_prefs.scale_fire = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_fire"))
        {
            skin_prefs.mask_fire = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_cold"))
        {
            skin_prefs.scale_cold = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_cold"))
        {
            skin_prefs.mask_cold = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_electricity"))
        {
            skin_prefs.scale_electricity = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_electricity"))
        {
            skin_prefs.mask_electricity = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_light"))
        {
            skin_prefs.scale_light = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_light"))
        {
            skin_prefs.mask_light = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_shadow"))
        {
            skin_prefs.scale_shadow = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_shadow"))
        {
            skin_prefs.mask_shadow = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_fogofwar"))
        {
            skin_prefs.scale_fogofwar = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_fogofwar"))
        {
            skin_prefs.mask_fogofwar = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_infravision"))
        {
            skin_prefs.scale_infravision = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_infravision"))
        {
            skin_prefs.mask_infravision = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "scale_xrayvision"))
        {
            skin_prefs.scale_xrayvision = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "mask_xrayvision"))
        {
            skin_prefs.mask_xrayvision = (uint32)strtoul(val, NULL, 16);
        }
        else if (!strcmp(key, "effect_width"))
        {
            skin_prefs.effect_width = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "effect_height"))
        {
            skin_prefs.effect_height = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "effect_eating"))
        {
            FREE(skin_prefs.effect_eating);
            MALLOC_STRING(skin_prefs.effect_eating, val);
        }
        else if (!strcmp(key, "effect_sleeping"))
        {
            FREE(skin_prefs.effect_sleeping);
            MALLOC_STRING(skin_prefs.effect_sleeping, val);
        }
        else if (!strcmp(key, "item_size"))
        {
            skin_prefs.item_size = (uint8)strtoul(val, NULL, 10);
        }
        else if (!strcmp(key, "icon_size"))
        {
            skin_prefs.icon_size = (uint8)strtoul(val, NULL, 10);
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}
