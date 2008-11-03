" Vim syntax file
" Language:	Daimonin artifact files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 31

if exists("b:current_syntax")
  finish
endif

runtime! syntax/arch.vim
unlet b:current_syntax

syntax sync minlines=200
syntax case match


" Objects
" -------
syntax region artObject keepend contains=animComment,animIdentifier,artAttribute,archObject
     \ matchgroup=animStructure start="^Allowed\>"
     \ matchgroup=animStructure end="^end$"

" Attributes
" ----------

" These attributes take an arbitrary string value. The string may contain any
" character but cannot cross a line boundary. Leading spaces are not included
" in the string.
syntax match artAttribute contained nextgroup=artString
     \ "^\%(artifact\|def_arch\|editor\|name\)\s\+"
syntax match artString contained
     \ "\S.*$"

" These attributes take integer values. The value can be positive (unsigned)
" or negative (signed) or zero.
syntax match artAttribute contained nextgroup=artNumber
     \ "^\%(chance\|difficulty\|t_style\)\s\+"
syntax match artNumber contained
     \ "\-\?\d\+$"


highlight def link artObject Error

highlight def link artAttribute Keyword

highlight def link artString String
highlight def link artNumber Number

let b:current_syntax="art"
