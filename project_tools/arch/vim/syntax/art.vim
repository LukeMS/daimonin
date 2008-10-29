" Vim syntax file
" Language:	Daimonin artifact files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 29
" Remarks:      Includes syntax/arch.vim.

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syntax clear
syntax sync minlines=30
syntax case match

" Errors
syntax match artError contains=artComment
     \ ".\+"

highlight def link artError Error

" Comments
syntax keyword artTodo contained
     \ FIXME TODO XXX NOTE
syntax match artComment contains=artTodo
     \ "#.*$"

highlight def link artTodo Todo
highlight def link artComment Comment

" Objects
syntax include syntax/arch.vim
syntax region artObject keepend contains=artComment,artIdentifier,artAttribute,archObject
     \ matchgroup=artStructure start="^Allowed\>"
     \ matchgroup=artStructure end="^end$"
syntax match artIdentifier contained
     \ "\s\+\S\+$"

highlight def link artObject Error
highlight def link artStructure Structure
highlight def link artIdentifier Identifier

" Attributes
syntax match artAttribute contained nextgroup=artString
     \ "^\%(artifact\|def_arch\|editor\|name\)\s\+"
syntax match artAttribute contained nextgroup=artNumber
     \ "^\%(chance\|difficulty\|t_style\)\s\+"
syntax match archAttribute contained nextgroup=archNumber
     \ "^\%(ac_add\|dam_add\|item_level_art\|wc_add\)\s\+"

highlight def link artAttribute Keyword

" Values
syntax match artString contained
     \ ".\+$"
syntax match artNumber contained
     \ "\-\?\d\+$"

highlight def link artString String
highlight def link artNumber Number

let b:current_syntax="art"
