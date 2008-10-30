" Vim syntax file
" Language:	Daimonin archetype files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 30
" Remarks:      Includes syntax/anim.vim.

if exists("b:current_syntax")
  finish
endif

syntax sync minlines=200
syntax case match


" Errors
" ------

" This will catch anything at the top level which is neither a comment nor an
" object and highlight it as an error.
syntax match archError contains=archComment
     \ ".\+"

" Comments
" --------
syntax match archComment contains=archTodo
     \ "#.*$"
syntax keyword archTodo contained
     \ FIXME TODO XXX NOTE

" Objects
" -------
syntax include syntax/anim.vim

syntax region archObject contains=archComment,archIdentifier,archAttribute,archMsgBlock,animAnimBlock
     \ matchgroup=archStructure start="^Object\>"
     \ matchgroup=archStructure end="^end$"

syntax match archIdentifier contained
     \ "\s\+\S\+$"

syntax match archStructure
     \ "^More$"

" MsgBlocks
" ---------
syntax region archMsgBlock contained
     \ matchgroup=archStructure start="^msg$"
     \ matchgroup=archStructure end="^endmsg$"

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
syntax match archFace contained contains=archFlag,archXYZ
     \ "\S\+$"
syntax match archFlag contained
     \ "\.\%(u\|d\)"hs=s+1
syntax match archXYZ contained
     \ "\.\%(\a\|\d\)\{3}$"hs=s+1

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

" These attributes take integer values. The value can be positive (unsigned)
" or negative (signed) or zero.
syntax match archAttribute contained nextgroup=archNumber
     \ "^\%(ac\|alive\|anim_speed\|applied\|attack_acid\|attack_aether\|attack_chaos\|attack_cleave\|attack_cold\|attack_confusion\|attack_corruption\|attack_death\|attack_depletion\|attack_drain\|attack_electricity\|attack_fear\|attack_fire\|attack_godpower\|attack_impact\|attack_lifesteal\|attack_light\|attack_magic\|attack_nether\|attack_paralyze\|attack_pierce\|attack_poison\|attack_psionic\|attack_shadow\|attack_slash\|attack_slow\|attack_snare\|attack_sonic\|attack_weaponmagic\|auto_apply\|been_applied\|berserk\|block_movement\|blocksview\|can_open_door\|can_pass_thru\|can_reflect_missile\|can_reflect_spell\|can_roll\|can_stack\|can_use_armour\|can_see_in_dark\|can_use_ring\|can_use_weapon\|carrying\|cha\|changing\|con\|confused\|connected\|container\|corpse\|corpse_forced\|cursed\|cursed_perm\|dam\|damned\|damned_perm\|dex\|direction\|door_closed\|ego\|egobound\|egoclan\|egolock\|exp\|feared\|fly_off\|fly_on\|flying\|food\|friendly\|generator\|glow_radius\|grace\|has_ready_bow\|has_ready_spell\|has_ready_weapon\|hidden\|hitback\|hp\|identified\|int\|inv_locked\|invulnerable\|is_aged\|is_animated\|is_assassin\|is_blind\|is_cauldron\|is_dust\|is_ethereal\|is_evil\|is_female\|is_good\|is_hilly\|is_invisible\|is_magical\|is_male\|is_missile\|is_named\|is_neutral\|is_player\|is_thrown\|is_turnable\|is_traped\|is_used_up\|is_wooded\|item_condition\|item_level\|item_quality\|item_skill\|known_cursed\|known_magical\|last_eat\|last_grace\|last_heal\|last_hp\|last_sp\|layer\|level\|levitate\|lifesave\|magic\|make_ethereal\|make_invisible\|material\|material_real\|maxgrace\|maxhp\|maxsp\|monster\|mpart_id\|mpart_nr\|no_attack\|no_cleric\|no_drop\|no_fix_player\|no_inv\|no_magic\|no_pass\|no_pick\|no_save\|no_skill_ident\|no_steal\|no_teleport\|nrof\|object_int1\|object_int2\|object_int3\|one_drop\|one_hit\|only_attack\|paralyzed\|pass_ethereal\|pass_thru\|path_attuned\|path_denied\|path_repelled\|player_only\|pow\|proof_ele\|proof_mag\|proof_phy\|proof_sph\|quest_item\|random_move\|reflect_missile\|reflect_spell\|reflecting\|reg_f\|resist_acid\|resist_aether\|resist_chaos\|resist_cleave\|resist_cold\|resist_confusion\|resist_corruption\|resist_death\|resist_depletion\|resist_drain\|resist_electricity\|resist_fear\|resist_fire\|resist_godpower\|resist_impact\|resist_lifesteal\|resist_light\|resist_magic\|resist_nether\|resist_paralyze\|resist_pierce\|resist_poison\|resist_psionic\|resist_shadow\|resist_slash\|resist_slow\|resist_snare\|resist_sonic\|resist_weaponmagic\|rooted\|run_away\|scared\|see_anywhere\|see_invisible\|sleep\|slowed\|sp\|splitting\|stand_still\|startequip\|state\|stealth\|str\|sub_type\|surrendered\|sys_object\|tear_down\|terrain_flag\|terrain_type\|thac0\|thacm\|treasure\|type\|unaggressive\|undead\|unpaid\|use_dmg_info\|value\|walk_off\|walk_on\|was_reflected\|weight\|weight_limit\|wis\|wiz\|wc\|x\|xrays\|y\|z\)\s\+"
syntax match archNumber contained
     \ "\-\?\d\+$"

" These attributes take a floating point or integer value. The value can be
" positive (unsigned) or negative (signed) or zero.
syntax match archAttribute contained nextgroup=archFloat
     \ "^\%(speed\|speed_left\|weapon_speed\)\s\+"
syntax match archFloat contained
     \ "\-\?\d*\.\?\d\+$"


highlight def link archError Error

highlight def link archComment Comment
highlight def link archTodo Todo

highlight def link archObject Error
highlight def link archStructure Structure
highlight def link archIdentifier Identifier

highlight def link archMsgBlock String

highlight def link archAttribute Keyword
highlight def link archString String
highlight def link archFace String
highlight def link archFlag SpecialChar
highlight def link archXYZ SpecialChar
highlight def link archTLlist String
highlight def link archTLdelimiter Delimiter
highlight def link archTLparameter Function
highlight def link archNumber Number
highlight def link archFloat Float

let b:current_syntax="arch"
