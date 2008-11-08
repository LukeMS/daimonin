if did_filetype()
  finish
endif

if getline(1)=~"^arch map$"
  set filetype=map
endif
