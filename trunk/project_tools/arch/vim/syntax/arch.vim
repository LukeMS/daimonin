" Vim syntax file
" Language:	Daimonin archetype files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 30
" Remarks:      Includes syntax/anim.vim.

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syntax clear
syntax sync minlines=200
syntax case match

" Errors
syntax match archError contains=archComment
     \ ".\+"

highlight def link archError Error

" Comments
syntax keyword archTodo contained
     \ FIXME TODO XXX NOTE
syntax match archComment contains=archTodo
     \ "#.*$"

highlight def link archTodo Todo
highlight def link archComment Comment

" Objects
syntax include syntax/anim.vim
syntax region archObject contains=archComment,archIdentifier,archAttribute,archMsgBlock,animAnimBlock
     \ matchgroup=archStructure start="^Object\>"
     \ matchgroup=archStructure end="^end$"
syntax match archIdentifier contained
     \ "\s\+\S\+$"
syntax match archStructure
     \ "^More$"

highlight def link archObject Error
highlight def link archStructure Structure
highlight def link archIdentifier Identifier

" MsgBlocks
syntax region archMsgBlock contained
     \ matchgroup=archStructure start="^msg$"
     \ matchgroup=archStructure end="^endmsg$"

highlight def link archMsgBlock String

" Attributes
"   ac alive amask anim_speed animation applied attack_acid attack_aether
"   attack_chaos attack_cleave attack_cold attack_confusion
"   attack_corruption attack_death attack_depletion attack_drain
"   attack_electricity attack_fear attack_fire attack_godpower attack_impact
"   attack_lifesteal attack_light attack_magic attack_nether attack_paralyze
"   attack_pierce attack_poison attack_psionic attack_shadow attack_slash
"   attack_slow attack_snare attack_sonic attack_weaponmagic auto_apply
"   been_applied berserk block_movement blocksview
"   can_open_door can_pass_thru can_reflect_missile can_reflect_spell
"   can_roll can_stack can_use_armour can_see_in_dark can_use_ring
"   can_use_weapon carrying cha changing con confused connected container
"   corpse corpse_forced cursed cursed_perm
"   dam damned damned_perm dex direction door_closed
"   editor_folder ego egobound egoclan egolock exp
"   face feared fly_off fly_on flying food friendly
"   generator glow_radius grace
"   has_ready_bow has_ready_spell has_ready_weapon hidden hitback hp
"   identified int inv_animation inv_face inv_locked invulnerable is_aged
"   is_animated is_assassin is_blind is_cauldron is_dust is_ethereal is_evil
"   is_female is_good is_hilly is_invisible is_magical is_male is_missile
"   is_named is_neutral is_player is_thrown is_turnable is_traped is_used_up
"   is_wooded item_condition item_level item_quality item_race item_skill
"   known_cursed known_magical
"   last_eat last_grace last_heal last_hp last_sp layer level levitate
"   lifesave
"   magic make_ethereal make_invisible material material_real maxgrace maxhp
"   maxsp monster mpart_id mpart_nr
"   name no_attack no_cleric no_drop no_fix_player no_inv no_magic no_pass
"   no_pick no_save no_skill_ident no_steal no_teleport nrof
"   object_int1 object_int2 object_int3 one_drop one_hit only_attack
"   other_arch
"   paralyzed pass_ethereal pass_thru path_attuned path_denied path_repelled
"   player_only pow proof_ele proof_mag proof_phy proof_sph
"   quest_item
"   race random_move randomitems reflect_missile reflect_spell reflecting
"   reg_f resist_acid resist_aether resist_chaos resist_cleave resist_cold
"   resist_confusion resist_corruption resist_death resist_depletion
"   resist_drain resist_electricity resist_fear resist_fire resist_godpower
"   resist_impact resist_lifesteal resist_light resist_magic resist_nether
"   resist_paralyze resist_pierce resist_poison resist_psionic resist_shadow
"   resist_slash resist_slow resist_snare resist_sonic resist_weaponmagic
"   rooted run_away
"   scared see_anywhere see_invisible slaying sleep slowed sp speed
"   speed_left splitting stand_still startequip state stealth str sub_type
"   surrendered sys_object
"   tear_down terrain_flag terrain_type thac0 thacm title treasure type
"   unaggressive undead unpaid use_dmg_info
"   value
"   walk_off walk_on was_reflected weapon_speed weight weight_limit wis wiz
"   wc
"   x xrays
"   y
"   z
syntax match archAttribute contained nextgroup=archString
     \ "^\%(amask\|animation\|editor_folder\|inv_animation\|item_race\|name\|other_arch\|race\|randomitems\|slaying\|title\)\s\+"
syntax match archString contained
     \ "\S.\+$"
syntax match archAttribute contained nextgroup=archFace
     \ "^\%(face\|inv_face\)\s\+"
syntax match archFace contained contains=archFlag,archXYZ
     \ "\S\+$"
syntax match archFlag contained
     \ "\.\%(u\|d\)"hs=s+1
syntax match archXYZ contained
     \ "\.\%(\a\|\d\)\{3}$"hs=s+1
syntax match archAttribute contained nextgroup=archNumber
     \ "^\%(ac\|alive\|anim_speed\|applied\|attack_acid\|attack_aether\|attack_chaos\|attack_cleave\|attack_cold\|attack_confusion\|attack_corruption\|attack_death\|attack_depletion\|attack_drain\|attack_electricity\|attack_fear\|attack_fire\|attack_godpower\|attack_impact\|attack_lifesteal\|attack_light\|attack_magic\|attack_nether\|attack_paralyze\|attack_pierce\|attack_poison\|attack_psionic\|attack_shadow\|attack_slash\|attack_slow\|attack_snare\|attack_sonic\|attack_weaponmagic\|auto_apply\|been_applied\|berserk\|block_movement\|blocksview\|can_open_door\|can_pass_thru\|can_reflect_missile\|can_reflect_spell\|can_roll\|can_stack\|can_use_armour\|can_see_in_dark\|can_use_ring\|can_use_weapon\|carrying\|cha\|changing\|con\|confused\|connected\|container\|corpse\|corpse_forced\|cursed\|cursed_perm\|dam\|damned\|damned_perm\|dex\|direction\|door_closed\|ego\|egobound\|egoclan\|egolock\|exp\|feared\|fly_off\|fly_on\|flying\|food\|friendly\|generator\|glow_radius\|grace\|has_ready_bow\|has_ready_spell\|has_ready_weapon\|hidden\|hitback\|hp\|identified\|int\|inv_locked\|invulnerable\|is_aged\|is_animated\|is_assassin\|is_blind\|is_cauldron\|is_dust\|is_ethereal\|is_evil\|is_female\|is_good\|is_hilly\|is_invisible\|is_magical\|is_male\|is_missile\|is_named\|is_neutral\|is_player\|is_thrown\|is_turnable\|is_traped\|is_used_up\|is_wooded\|item_condition\|item_level\|item_quality\|item_skill\|known_cursed\|known_magical\|last_eat\|last_grace\|last_heal\|last_hp\|last_sp\|layer\|level\|levitate\|lifesave\|magic\|make_ethereal\|make_invisible\|material\|material_real\|maxgrace\|maxhp\|maxsp\|monster\|mpart_id\|mpart_nr\|no_attack\|no_cleric\|no_drop\|no_fix_player\|no_inv\|no_magic\|no_pass\|no_pick\|no_save\|no_skill_ident\|no_steal\|no_teleport\|nrof\|object_int1\|object_int2\|object_int3\|one_drop\|one_hit\|only_attack\|paralyzed\|pass_ethereal\|pass_thru\|path_attuned\|path_denied\|path_repelled\|player_only\|pow\|proof_ele\|proof_mag\|proof_phy\|proof_sph\|quest_item\|random_move\|reflect_missile\|reflect_spell\|reflecting\|reg_f\|resist_acid\|resist_aether\|resist_chaos\|resist_cleave\|resist_cold\|resist_confusion\|resist_corruption\|resist_death\|resist_depletion\|resist_drain\|resist_electricity\|resist_fear\|resist_fire\|resist_godpower\|resist_impact\|resist_lifesteal\|resist_light\|resist_magic\|resist_nether\|resist_paralyze\|resist_pierce\|resist_poison\|resist_psionic\|resist_shadow\|resist_slash\|resist_slow\|resist_snare\|resist_sonic\|resist_weaponmagic\|rooted\|run_away\|scared\|see_anywhere\|see_invisible\|sleep\|slowed\|sp\|splitting\|stand_still\|startequip\|state\|stealth\|str\|sub_type\|surrendered\|sys_object\|tear_down\|terrain_flag\|terrain_type\|thac0\|thacm\|treasure\|type\|unaggressive\|undead\|unpaid\|use_dmg_info\|value\|walk_off\|walk_on\|was_reflected\|weight\|weight_limit\|wis\|wiz\|wc\|x\|xrays\|y\|z\)\s\+"
syntax match archNumber contained
     \ "\-\?\d\+$"
syntax match archAttribute contained nextgroup=archFloat
     \ "^\%(speed\|speed_left\|weapon_speed\)\s\+"
syntax match archFloat contained
     \ "\-\?\d*\.\?\d\+$"

highlight def link archAttribute Keyword
highlight def link archString String
highlight def link archFace String
highlight def link archFlag Special
highlight def link archXYZ Special
highlight def link archNumber Number
highlight def link archFloat Float

let b:current_syntax="arch"
