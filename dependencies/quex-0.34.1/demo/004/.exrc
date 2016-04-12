if &cp | set nocp | endif
map [3~ x
map [G i
let s:cpo_save=&cpo
set cpo&vim
nmap gx <Plug>NetrwBrowseX
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetBrowseX(expand("<cWORD>"),0)
map! [3~ <Del>
map! [G <Insert>
let &cpo=s:cpo_save
unlet s:cpo_save
set background=dark
set backspace=indent,eol,start
set cindent
set cinoptions=g0,(0,:0
set expandtab
set fileencodings=ucs-bom,utf-8,default,latin1
set helplang=en
set hlsearch
set modelines=1
set ruler
set shiftwidth=4
set showmatch
set tabstop=4
" vim: set ft=vim :
