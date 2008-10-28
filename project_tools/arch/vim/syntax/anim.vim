" Vim syntax file
" Language:	Daimonin anim files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 28

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syntax clear
syntax sync minlines=30
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
syntax region animAnimBlock contains=animComment,animIdentifier,animAttribute
     \ matchgroup=animStructure start="^anim\>"
     \ matchgroup=animStructure end="^mina$"
syntax match animIdentifier contained
     \ "\s\+\S\+$"hs=s-1

highlight def link animAnimBlock String
highlight def link animStructure Structure
highlight def link animIdentifier Identifier

" Attributes
syntax match animAttribute contained nextgroup=animNumber
     \ "^facings\s\+"

highlight def link animAttribute Keyword

" Values
syntax match animNumber contained
     \ "\-\?\d\+$"

highlight def link animNumber Number

let b:current_syntax="anim"
