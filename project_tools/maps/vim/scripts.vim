if did_filetype()
  finish
endif

let lnum = 1
while lnum <= line("$")
  let fbuf = getline(lnum)
  if fbuf =~ "^$" || fbuf =~ "^#.*$"
    let lnum += 1
  elseif fbuf =~# "^arch map$"
    set filetype=map
    finish
  else
    finish
  endif
endwhile
