" Vim syntax file
" Language:	Daimonin artifact files
" Maintainer:	Smacky <smacky@smackysguides.net>
" Last Change:	2008 Oct 30
" Remarks:      Includes syntax/arch.vim.

if exists("b:current_syntax")
  finish
endif

syntax sync minlines=200
syntax case match


" Errors
" ------

" This will catch anything at the top level which is neither a comment nor an
" object and highlight it as an error.
syntax match artError contains=artComment
     \ ".\+"

" Comments
" --------
syntax match artComment contains=artTodo
     \ "#.*$"
syntax keyword artTodo contained
     \ FIXME TODO XXX NOTE

" Objects
" -------
syntax include syntax/arch.vim

syntax region artObject keepend contains=artComment,artIdentifier,artAttribute,archObject
     \ matchgroup=artStructure start="^Allowed\>"
     \ matchgroup=artStructure end="^end$"

syntax match artIdentifier contained
     \ "\s\+\S\+$"

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
syntax match archAttribute contained nextgroup=archNumber
     \ "^\%(ac_add\|dam_add\|item_level_art\|wc_add\)\s\+"
syntax match artNumber contained
     \ "\-\?\d\+$"


highlight def link artError Error

highlight def link artComment Comment
highlight def link artTodo Todo

highlight def link artObject Error
highlight def link artStructure Structure
highlight def link artIdentifier Identifier

highlight def link artAttribute Keyword

highlight def link artString String
highlight def link artNumber Number

let b:current_syntax="art"
