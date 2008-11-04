" Vim syntax file
" Language:	Daimonin archetype files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Nov 2

if exists("b:current_syntax")
  finish
endif

runtime! syntax/anim.vim
unlet b:current_syntax

syntax sync minlines=200
syntax case match


" Objects
" -------
syntax region archObject contains=animComment,animIdentifier,archAttribute,archMsgBlock,archArchBlock,animAnimBlock
     \ matchgroup=animStructure start="^Object\>"
     \ matchgroup=animStructure end="^end$"

syntax match animStructure
     \ "^More$"

" MsgBlocks
" ---------
syntax region archMsgBlock contained
     \ matchgroup=animStructure start="^msg$"
     \ matchgroup=animStructure end="^endmsg$"

" ArchBlocks
" ----------
syntax region archArchBlock contained contains=animComment,animIdentifier,archAttribute,archMsgBlock
     \ matchgroup=animStructure start="^arch\>"
     \ matchgroup=animStructure end="^end$"

" Attributes
" ----------

" These attributes take an arbitrary string value. The string may contain any
" character but cannot cross a line boundary. Leading spaces are not included
" in the string.
syntax match archAttribute contained nextgroup=archString
     \ "^\%(amask\|animation\|editor_folder\|inv_animation\|item_race\|name\|other_arch\|race\|slaying\|title\)\s\+"
syntax match archString contained
     \ "\S.*$"

" These attributes take a face value. A face is a string which is the filename
" of an image minus the extension (eg, .png). Within this string, flag (.u and
" .d) and XYZ substrings are highlighted specially.
syntax match archAttribute contained nextgroup=archFace
     \ "^\%(face\|inv_face\)\s\+"
syntax match archFace contained contains=animFlag,animXYZ
     \ "\S\+$"

" This attribute takes a treasurelist value. A treasurelist is a special type
" of string.
syntax match archAttribute contained nextgroup=archTLlist
     \ "^randomitems\s\+"
syntax match archTLlist contained contains=archTLdelimiter,archTLparameter
     \ "\S.*$"
syntax match archTLdelimiter contained
     \ "[&;,]"
syntax match archTLparameter contained
     \ "[acdimqrsxBCDIMQ]\d*[;,]"me=e-1
syntax match archTLparameter contained
     \ "[acdimqrsxBCDIMQ]\d*$"

" This attribute takes a path string value.
syntax match archAttribute contained nextgroup=archPath
     \ "^editor_folder\s\+"
syntax match archPath contained contains=archDirSeparator
     \ "\S\+$"
syntax match archDirSeparator contained
     \ "/"

" These attributes take integer values. The value can be positive (unsigned)
" or negative (signed) or zero.
syntax match archAttribute contained nextgroup=archNumber
     \ "^\%(ac\|ac_add\|anim_speed\|attack_acid\|attack_aether\|attack_cancellation\|attack_chaos\|attack_cleave\|attack_cold\|attack_confusion\|attack_corruption\|attack_countermagic\|attack_death\|attack_depletion\|attack_drain\|attack_electricity\|attack_fear\|attack_fire\|attack_godpower\|attack_impact\|attack_internal\|attack_lifesteal\|attack_light\|attack_magic\|attack_nether\|attack_paralyze\|attack_pierce\|attack_poison\|attack_psionic\|attack_shadow\|attack_slash\|attack_slow\|attack_snare\|attack_sonic\|attack_weaponmagic\|block_movement\|carrying\|cha\|con\|connected\|container\|dam\|dam_add\|dex\|direction\|exp\|food\|glow_radius\|grace\|hp\|int\|item_condition\|item_level\|item_level_art\|item_quality\|item_skill\|last_eat\|last_grace\|last_heal\|last_hp\|last_sp\|layer\|level\|magic\|material\|material_real\|maxgrace\|maxhp\|maxsp\|mpart_id\|mpart_nr\|nrof\|object_int1\|object_int2\|object_int3\|path_attuned\|path_denied\|path_repelled\|pow\|resist_acid\|resist_aether\|resist_cancellation\|resist_chaos\|resist_cleave\|resist_cold\|resist_confusion\|resist_corruption\|resist_countermagic\|resist_death\|resist_depletion\|resist_drain\|resist_electricity\|resist_fear\|resist_fire\|resist_godpower\|resist_impact\|resist_internal\|resist_lifesteal\|resist_light\|resist_magic\|resist_nether\|resist_paralyze\|resist_pierce\|resist_poison\|resist_psionic\|resist_shadow\|resist_slash\|resist_slow\|resist_snare\|resist_sonic\|resist_weaponmagic\|run_away\|sp\|state\|str\|sub_type\|terrain_flag\|terrain_type\|thac0\|thacm\|type\|value\|weight\|wis\|wc\|wc_add\|x\|y\|z\)\s\+"
syntax match archNumber contained
     \ "\-\?\d\+$"

" These attributes take boolean values. The value can either be 0 or 1.
syntax match archAttribute contained nextgroup=archBoolean
     \ "^\%(alive\|applied\|auto_apply\|been_applied\|berserk\|blocksview\|can_open_door\|can_pass_thru\|can_reflect_missile\|can_reflect_spell\|can_roll\|can_stack\|can_use_armour\|can_see_in_dark\|can_use_ring\|can_use_weapon\|changing\|confused\|corpse\|corpse_forced\|cursed\|cursed_perm\|damned\|damned_perm\|door_closed\|ego\|egobound\|egoclan\|egolock\|feared\|fly_off\|fly_on\|flying\|friendly\|generator\|has_ready_bow\|has_ready_spell\|has_ready_weapon\|hidden\|hitback\|identified\|inv_locked\|invulnerable\|is_aged\|is_animated\|is_assassin\|is_blind\|is_cauldron\|is_dust\|is_ethereal\|is_evil\|is_female\|is_good\|is_hilly\|is_invisible\|is_magical\|is_male\|is_missile\|is_named\|is_neutral\|is_player\|is_thrown\|is_turnable\|is_traped\|is_used_up\|is_wooded\|known_cursed\|known_magical\|levitate\|lifesave\|make_ethereal\|make_invisible\|monster\|no_attack\|no_cleric\|no_drop\|no_fix_player\|no_inv\|no_magic\|no_pass\|no_pick\|no_save\|no_skill_ident\|no_steal\|no_teleport\|one_drop\|one_hit\|only_attack\|paralyzed\|pass_ethereal\|pass_thru\|player_only\|proof_ele\|proof_mag\|proof_phy\|proof_sph\|quest_item\|random_move\|reflect_missile\|reflect_spell\|reflecting\|reg_f\|rooted\|scared\|see_anywhere\|see_invisible\|sleep\|slowed\|splitting\|stand_still\|startequip\|stealth\|surrendered\|sys_object\|tear_down\|treasure\|unaggressive\|undead\|unpaid\|use_dmg_info\|walk_off\|walk_on\|was_reflected\|wiz\|xrays\)\s\+"
syntax match archBoolean contained
     \ "[01]$"

" These attributes take a floating point or integer value. The value can be
" positive (unsigned) or negative (signed) or zero.
syntax match archAttribute contained nextgroup=archFloat
     \ "^\%(speed\|speed_left\|weapon_speed\)\s\+"
syntax match archFloat contained
     \ "\-\?\d*\.\?\d\+$"


highlight def link archObject Error

highlight def link archMsgBlock String

highlight def link archArchBlock Error

highlight def link archAttribute Keyword
highlight def link archString String
highlight def link archFace String
highlight def link archFlag SpecialChar
highlight def link archXYZ SpecialChar
highlight def link archTLlist String
highlight def link archTLdelimiter Delimiter
highlight def link archTLparameter Function
highlight def link archPath String
highlight def link archDirSeparator Delimiter
highlight def link archNumber Number
highlight def link archBoolean Number
highlight def link archFloat Float

let b:current_syntax="arch"
