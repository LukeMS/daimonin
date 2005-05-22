Linux SDL parachute
-------------------
Under some systems the client will die at startup without any
log message but a sld error.
There can be 3 reasons: Buggy SDL installation, root/account problem
or a sound card problem (linux soundcard drivers have sometimes problem
to report their status in the right way to SDL).

If you get this problem, try this:

a.) type "killall artsd" and try again to run daimonin
b.) start the client typing "SDL_AUDIODRIVER=null ./daimonin"
c.) login as root and try to run it
d.) type "sdl-config" and check your sdl installation. Install it again.
e.) check the access rights of your devices


SOURCE PACKAGES
---------------
Downloading the source package requieres compiling (of course).
You will see in the folder /client a sup folder called /make.
Go in this folder and read the README.txt there. You will be
guided to your OS and what you need to do to compile the client.
Note, that you need for some OS other libaries you must perhaps
first install.
