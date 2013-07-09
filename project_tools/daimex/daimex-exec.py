# This is an example file that works with Daimex.py core script
# This file would be placed within the Blend file, and Daimex.py
# must be copied into the .scripts folder for Blender

#
# Stolen shamelessly from the human_male blend configuration.
#


# Import the Daimex class
from daimex import Daimex

# create an instance of Daimex class with:
instance0 = Daimex()
instance1 = Daimex()
instance2 = Daimex()
instance3 = Daimex()

#then change some settings if you like, for example:
instance0.filename = "human_male.0"
instance1.filename = "human_male.1"
instance2.filename = "human_male.2"
instance3.filename = "human_male.3"

instance0.framestart = 20   #2-5, 8-11, 14-17, 20-23
instance0.frameend = 23
instance0.framestep = 1

instance1.framestart = 2   #2-5, 8-11, 14-17, 20-23
instance1.frameend = 5
instance1.framestep = 1

instance2.framestart = 8   #2-5, 8-11, 14-17, 20-23
instance2.frameend = 11
instance2.framestep = 1

instance3.framestart = 14   #2-5, 8-11, 14-17, 20-23
instance3.frameend = 17
instance3.framestep = 1

mysunengergy = 1.4

instance0.sunenergy = mysunengergy
instance1.sunenergy = mysunengergy
instance2.sunenergy = mysunengergy
instance3.sunenergy = mysunengergy

myoutputdir = "C:\\Output\\human\\male\\v7"

instance0.outputdir = myoutputdir
instance1.outputdir = myoutputdir
instance2.outputdir = myoutputdir
instance3.outputdir = myoutputdir


# and run the script with:
instance0.run()
instance1.run()
instance2.run()
instance3.run()
