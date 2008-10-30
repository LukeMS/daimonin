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
" ------

" This will catch anything at the top level which is neither a comment nor an
" animblock and highlight it as an error.
syntax match animError contains=animComment
     \ ".\+"

" Comments
" --------
syntax match animComment contains=animTodo
     \ "#.*$"
syntax keyword animTodo contained
     \ FIXME TODO XXX NOTE

" AnimBlocks
" ----------
syntax region animAnimBlock contains=animComment,animIdentifier,animAttribute,animFace
     \ matchgroup=animStructure start="^anim\>"
     \ matchgroup=animStructure end="^mina$"

syntax match animIdentifier contained
     \ "\s\+\S\+$"hs=s-1

" This is a face value. A face is a string which is the filename of an image
" minus the extension (eg, .png). Within this string, flag (.u and .d) and XYZ
" substrings are highlighted specially.
syntax match animFace contained contains=animFlag,animXYZ
     \ "^\S\+$"
syntax match animFlag contained
     \ "\.\%(u\|d\)"hs=s+1
syntax match animXYZ contained
     \ "\.\%(\a\|\d\)\{3}$"hs=s+1

" Attributes
" ----------

" This attribute takes an integer value. The value must be positive (unsigned)
" and non-zero.
syntax match animAttribute contained nextgroup=animNumber
     \ "^facings\s\+"
syntax match animNumber contained
     \ "[1-9]\d*$"


highlight def link animError Error

highlight def link animComment Comment
highlight def link animTodo Todo

highlight def link animAnimBlock Error
highlight def link animStructure Structure
highlight def link animIdentifier Identifier
highlight def link animFace String
highlight def link animFlag SpecialChar
highlight def link animXYZ SpecialChar

highlight def link animAttribute Keyword
highlight def link animNumber Number

let b:current_syntax="anim"
