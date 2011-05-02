Welcome to the Daimonin SDL client
----------------------------------

This client comes in different packages for different OS.
So called "binary packages" come usually with an installer -
if you have run an installer before you read this you will
see an executable file in the same folder as you find this
README.txt file. Your system should also be configured to run
the client without problems.

Now some tips for the packages and OS.

Playing with Laptops
--------------------
The keys.dat file in the settings directory defines the
default keybindings. It assumes a num-pad and binds movement
to the num-pad keys.

For laptops where this is not convenient, an alternative
set of bindings is provided in keys-laptop.dat. To use,
simply rename keys.dat as keys-full.dat and keys-laptop.dat
as keys.dat.


MS Windows (98, XP, ...)
------------------------
Your package should have arrived as a binary with installer.
Just start the file DAIMONIN.EXE. Your client should run
out of the box.

Source packages should ONLY be needed for developers. If
you've got a simple .zip file and no installer and you see no
DAIMONIN.EXE and there is a folder called /make
in your /client folder - then you have got a source package
instead of the binary package. Just go back to the download
site and get the binary package!

Linux (and other *nix)
----------------------
You usually need to download the source package ...

SOURCE PACKAGES
---------------
Downloading the source package requires compiling (of course).
You will see in the folder /client a sub-folder called /make.
Go into this folder and read the README.txt there. You will be
guided to your OS and what you need to do to compile the client.
Note that for some OS you may need to install other libraries
first.
