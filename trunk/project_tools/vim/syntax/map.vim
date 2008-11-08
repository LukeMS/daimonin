" Vim syntax file
" Language:	Daimonin map files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 31

if exists("b:current_syntax")
  finish
endif

runtime! syntax/arch.vim
unlet b:current_syntax

syntax sync minlines=200
syntax case match


" ArchBlocks
" ----------
syntax region archArchBlock contains=animComment,animIdentifier,archAttribute,archMsgBlock,archArchBlock
     \ matchgroup=animStructure start="^arch\>"
     \ matchgroup=animStructure end="^end$"

syntax region mapArchBlock contains=animComment,animIdentifier,mapAttribute,archMsgBlock
     \ matchgroup=animStructure start="^arch map$"
     \ matchgroup=animStructure end="^end$"

" Attributes
" ----------

" These attributes take an arbitrary string value. The string may contain any
" character but cannot cross a line boundary. Leading spaces are not included
" in the string.
syntax match mapAttribute contained nextgroup=mapString
     \ "^\%(name\|orig_path\|orig_tile_path_1\|orig_tile_path_2\|orig_tile_path_3\|orig_tile_path_4\|orig_tile_path_5\|orig_tile_path_6\|orig_tile_path_7\|orig_tile_path_8\|reference\|tile_path_1\|tile_path_2\|tile_path_3\|tile_path_4\|tile_path_5\|tile_path_6\|tile_path_7\|tile_path_8\)\s\+"
syntax match mapString contained
     \ "\S.*$"

" These attributes take integer values. The value can be positive (unsigned)
" or negative (signed) or zero.
syntax match mapAttribute contained nextgroup=mapNumber
     \ "^\%(darkness\|difficulty\|enter_x\|enter_y\|fixed_login\|fixed_resettime\|height\|instance\|light\|map_tag\|multi\|no_harm\|no_magic\|no_priest\|no_save\|no_summon\|outdoor\|pvp\|perm_death\|reset_timeout\|swap_time\|tileset_id\|tileset_x\|tileset_y\|ultimate_death\|ultra_death\|unique\|width\)\s\+"
syntax match mapNumber contained
     \ "\-\?\d\+$"


highlight def link mapArchBlock Error

highlight def link mapAttribute Keyword
highlight def link mapString String
highlight def link mapNumber Number

let b:current_syntax="map"
