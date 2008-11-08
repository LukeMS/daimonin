You need to get pngout first.

Linux: http://www.jonof.id.au/index.php?p=kenutils
Win: http://advsys.net/ken/utils.htm

This is a quick and dirty facepack_repacker. With only very minimal error checking, 
it don't uses threads, and so is very slow.

You need to put the executable, (under win as well the pngout.exe) in the same dir as the daimonin.0 file
resides. The daimonin.0 file itself is untouched, the packer creates a packed daimonin.p0 file.

You should not simply commit packed facepacks. NEVER mix packed and non packed. 
If we switch to packed format we need to always use packed format, and of course provide a update
through the autoupdater for the client. So only use it for testing until we officially switch to packed format.

Maybe its better (maybe even faster) to repack the original pngs, but then all the additional information is lost.

Instructions how to batch run, can be found here: http://www.daimonin.net/index.php?name=PNphpBB2&file=viewtopic&p=65186#65186

