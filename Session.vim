let SessionLoad = 1
let s:so_save = &g:so | let s:siso_save = &g:siso | setg so=0 siso=0 | setl so=-1 siso=-1
let v:this_session=expand("<sfile>:p")
doautoall SessionLoadPre
silent only
silent tabonly
cd ~/BigDrive/Programming/SkeletOSS/ccc
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
let s:shortmess_save = &shortmess
set shortmess+=aoO
badd +171 tests/doubly_linked_list/test_doubly_linked_list_insert.c
badd +203 ~/BigDrive/Programming/SkeletOSS/ccc/ccc/doubly_linked_list.h
badd +233 ./source/doubly_linked_list.c
badd +144 ./tests/doubly_linked_list/test_doubly_linked_list_erase.c
argglobal
%argdel
$argadd tests/doubly_linked_list/test_doubly_linked_list_insert.c
edit tests/doubly_linked_list/test_doubly_linked_list_insert.c
let s:save_splitbelow = &splitbelow
let s:save_splitright = &splitright
set splitbelow splitright
wincmd _ | wincmd |
vsplit
wincmd _ | wincmd |
vsplit
2wincmd h
wincmd w
wincmd w
wincmd _ | wincmd |
split
1wincmd k
wincmd w
let &splitbelow = s:save_splitbelow
let &splitright = s:save_splitright
wincmd t
let s:save_winminheight = &winminheight
let s:save_winminwidth = &winminwidth
set winminheight=0
set winheight=1
set winminwidth=0
set winwidth=1
exe 'vert 1resize ' . ((&columns * 106 + 159) / 319)
exe 'vert 2resize ' . ((&columns * 103 + 159) / 319)
exe '3resize ' . ((&lines * 29 + 30) / 61)
exe 'vert 3resize ' . ((&columns * 108 + 159) / 319)
exe '4resize ' . ((&lines * 29 + 30) / 61)
exe 'vert 4resize ' . ((&columns * 108 + 159) / 319)
argglobal
balt ./tests/doubly_linked_list/test_doubly_linked_list_erase.c
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
silent! normal! zE
let &fdl = &fdl
let s:l = 59 - ((22 * winheight(0) + 29) / 59)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 59
normal! 0
wincmd w
argglobal
if bufexists(fnamemodify("./tests/doubly_linked_list/test_doubly_linked_list_erase.c", ":p")) | buffer ./tests/doubly_linked_list/test_doubly_linked_list_erase.c | else | edit ./tests/doubly_linked_list/test_doubly_linked_list_erase.c | endif
if &buftype ==# 'terminal'
  silent file ./tests/doubly_linked_list/test_doubly_linked_list_erase.c
endif
balt tests/doubly_linked_list/test_doubly_linked_list_insert.c
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
silent! normal! zE
let &fdl = &fdl
let s:l = 131 - ((16 * winheight(0) + 29) / 59)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 131
normal! 018|
wincmd w
argglobal
enew | setl bt=help
help CCC_doubly_linked_list_insert
balt ./source/doubly_linked_list.c
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal nofoldenable
silent! normal! zE
let &fdl = &fdl
let s:l = 84 - ((14 * winheight(0) + 14) / 29)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 84
normal! 024|
wincmd w
argglobal
if bufexists(fnamemodify("./source/doubly_linked_list.c", ":p")) | buffer ./source/doubly_linked_list.c | else | edit ./source/doubly_linked_list.c | endif
if &buftype ==# 'terminal'
  silent file ./source/doubly_linked_list.c
endif
balt ~/BigDrive/Programming/SkeletOSS/ccc/ccc/doubly_linked_list.h
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
silent! normal! zE
let &fdl = &fdl
let s:l = 235 - ((1 * winheight(0) + 14) / 29)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 235
normal! 07|
wincmd w
3wincmd w
exe 'vert 1resize ' . ((&columns * 106 + 159) / 319)
exe 'vert 2resize ' . ((&columns * 103 + 159) / 319)
exe '3resize ' . ((&lines * 29 + 30) / 61)
exe 'vert 3resize ' . ((&columns * 108 + 159) / 319)
exe '4resize ' . ((&lines * 29 + 30) / 61)
exe 'vert 4resize ' . ((&columns * 108 + 159) / 319)
tabnext 1
if exists('s:wipebuf') && len(win_findbuf(s:wipebuf)) == 0 && getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20
let &shortmess = s:shortmess_save
let &winminheight = s:save_winminheight
let &winminwidth = s:save_winminwidth
let s:sx = expand("<sfile>:p:r")."x.vim"
if filereadable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &g:so = s:so_save | let &g:siso = s:siso_save
set hlsearch
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
