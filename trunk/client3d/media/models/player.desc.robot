##################################################################################
# This is a client3d description file for an ogre3d model.
# This file is part of Daimonin (http://daimonin.sourceforge.net)
##################################################################################
# IMPORTANT:
# Every model MUST have an animation-state IDLE1 !!!!!
# If you don't have IDLE1 ready yet -> fill it with something that exist.
#
# In future there will be fixed names for animation states.
# Therefore the animation-state should ALWAYS be named like the function.
# e.g. "Walk1" for Walk1. NOT "Walk" like in the original ogre3d robot-model.
#
# Post all comments to the daimonin devel-list.
# <polyveg>
##################################################################################

MeshName : "robot.mesh"

# Size of the model
MeshSize : "0.5"

# The ground-pos of the model in the world.
SeaLevel : "0"

# The facing direction in degrees.
# If your model walks sideways you can fix this here.
Facing   : "0"

# The animation groups (xyz1 - xyz3) are toggled by F1.
Idle1    : "Idle"
Idle2    : ""
Idle3    : ""

# Triggered by cursor keys.
Walk1    : "Walk"
Walk2    : ""
Walk3    : ""

# Not implemented yet.
# Triggered by shift + cursor keys.
Run1     : ""
Run2     : ""
Run3     : ""

# Triggered by 'a'key. 
Attack1  : "Shoot"
Attack2  : ""
Attack3  : ""

# Triggered by 'b'key.
Block1   : ""
Block2   : ""
Block3   : ""

# Triggered by 's' key.
Slump1   : "Slump"
Slump2   : ""
Slump3   : ""

# Triggered by 'd' key.
Death1   : "Die"
Death2   : ""
Death3   : ""

# Triggered by 'h' key.
Hit1     : ""
Hit2     : ""
Hit3     : ""
