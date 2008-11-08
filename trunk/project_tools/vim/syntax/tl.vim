" Vim syntax file
" Language:	Daimonin treasure list files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Nov 8

if exists("b:current_syntax")
  finish
endif

syntax sync minlines=200
syntax case ignore


" Errors
" ------

" This will catch anything at the top level which is neither a comment nor a
" block and highlight it as an error.
syntax match tlError contains=tlComment
     \ ".\+"

" Comments
" --------
syntax match tlComment contains=tlTodo
     \ "#.*$"
syntax keyword tlTodo contained
     \ FIXME TODO XXX NOTE

" Blocks
" ------
syntax region tlBlock contains=tlComment,tlIdentifier,tlAttribute,tlMore
     \ matchgroup=tlStructure start="^\s*\%(treasure\|treasureone\)\>"
     \ matchgroup=tlStructure end="^\s*end$"

syntax match tlIdentifier contained
     \ "\s\+\S\+$"

syntax match tlMore contained
     \ "^\s*more$"

" Attributes
" ----------
syntax match tlAttribute contained nextgroup=tlString
     \ "^\s*\%(face\|name\|race\|slaying\|title\)\s\+"
syntax match tlString contained
     \ "\S.*$"

syntax match tlAttribute contained nextgroup=tlIdString
     \ "^\s*\%(anim\|arch\|list\)\s\+"
syntax match tlIdString contained
     \ "\S\+$"

syntax match tlAttribute contained nextgroup=tlNumber
     \ "^\s*\(artifact_chance\|chance\|chance_fix\|difficulty\|item_race\|magic\|magic_chance\|material\|material_quality\|material_range\|nrof\|quality\|quality_range\|t_style\)\s\+"
syntax match tlNumber contained
     \ "\d\+$"

syntax match tlAttribute contained
     \ "^\s*\%(no\|yes\)$"

highlight def link tlError Error

highlight def link tlComment Comment
highlight def link tlTodo Todo

highlight def link tlBlock Error
highlight def link tlStructure Structure
highlight def link tlIdentifier Identifier

highlight def link tlMore Structure

highlight def link tlAttribute Keyword
highlight def link tlString String
highlight def link tlIdString Identifier
highlight def link tlNumber Number

let b:current_syntax="tl"
