" Vim syntax file
" Language:	Daimonin anim files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 30

if exists("b:current_syntax")
  finish
endif

syntax sync minlines=200
syntax case ignore

" Errors
syntax match animError contains=animComment
     \ ".\+"

highlight def link animError Error

" Comments
syntax keyword animTodo contained
     \ FIXME TODO XXX NOTE
syntax match animComment contains=animTodo
     \ "#.*$"

highlight def link animTodo Todo
highlight def link animComment Comment

" AnimBlocks
syntax region animAnimBlock contains=animComment,animIdentifier,animAttribute,animFace
     \ matchgroup=animStructure start="^anim\>"
     \ matchgroup=animStructure end="^mina$"
syntax match animIdentifier contained
     \ "\s\+\S\+$"hs=s-1
syntax match animFace contained contains=animFlag,animXYZ
     \ "^\S\+$"
syntax match animFlag contained
     \ "\.\%(u\|d\)"hs=s+1
syntax match animXYZ contained
     \ "\.\%(\a\|\d\)\{3}$"hs=s+1

highlight def link animAnimBlock Error
highlight def link animStructure Structure
highlight def link animIdentifier Identifier
highlight def link animFace String
highlight def link animFlag SpecialChar
highlight def link animXYZ SpecialChar

" Attributes
syntax match animAttribute contained nextgroup=animNumber
     \ "^facings\s\+"
syntax match animNumber contained
     \ "\-\?\d\+$"

highlight def link animAttribute Keyword
highlight def link animNumber Number

let b:current_syntax="anim"
