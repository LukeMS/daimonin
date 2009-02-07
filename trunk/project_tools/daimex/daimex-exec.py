# This is an example file that works with Daimex.py core script
# This file would be placed within the Blend file, and Daimex.py
# must be copied into the .scripts folder for Blender


# Import the Daimex class
from daimex import Daimex

#####

# Create an instance of Daimex class with:
instance1 = Daimex()
instance2 = Daimex()
instance3 = Daimex()

#####

mymodeldir = 3

instance1.modeldir = mymodeldir
instance2.modeldir = mymodeldir
instance3.modeldir = mymodeldir

#####

filename_base = "imp_air"

instance1.filename = filename_base + ".1"
instance2.filename = filename_base + ".2"
instance3.filename = filename_base + ".3"

#####

myrsmulti = 1

instance1.rsmulti = myrsmulti
instance2.rsmulti = myrsmulti
instance3.rsmulti = myrsmulti

#####

myoutputdir = "X:\\Output\\gargoyle\\air\\v10s"

instance1.outputdir = myoutputdir
instance2.outputdir = myoutputdir
instance3.outputdir = myoutputdir

#####

# Use framestart/end figures
instance1.orframes = True
instance2.orframes = True
instance3.orframes = True
instance1.framestep = 1
instance2.framestep = 1
instance3.framestep = 1

#####

# Adjust shadow intensity
myspotdist = 10

instance1.spotdist = myspotdist
instance2.spotdist = myspotdist
instance3.spotdist = myspotdist

#####

# Set up frame start/end points
instance1.framestart = 2   #2-5, 8-11, 14-17, 20-23
instance1.frameend = 5

instance2.framestart = 8   #2-5, 8-11, 14-17, 20-23
instance2.frameend = 11

instance3.framestart = 14   #2-5, 8-11, 14-17, 20-23
instance3.frameend = 17

#####

# apply scale of 0.47 to keep same size
instance1.wideimage = True
instance2.wideimage = True
instance3.wideimage = True

#####

# Run the script with:
instance1.run()
instance2.run()
instance3.run()