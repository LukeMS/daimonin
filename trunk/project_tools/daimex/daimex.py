##########################################################################################
# DAIMEX - script for Blender, for rendering and formating for use in Daimonin 2d client #
##########################################################################################
# To run the script press ALT+P     #
#####################################
# version 0.6 #
###############

# Use CatRoom rendering filter for sharpest results
# I recommend playing with renderer setting sky/prem/key and the sky colour 
# - on the edges of transparency of images you can see a lot of differences

# How to make semi-transparent shadows by Torchwood: You add a new mesh object to your scene - a flat plane. 
# Align this to the 'ground' level (i.e. just under the feet of the actual object). 
# Add a single material and under the material/shaders tab, set the "only shadow" option. 
# This has the effect of making the plane invisible (so it doesn't appear in the rendered image), 
# but making the plane receive shadows - which then DO appear in the final rendered image.

"""
    Copyright (c) 2007, Jiri Prochazka

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to Pro.J@seznam.cz
"""

import bpy, os, math, copy
import mathutils
from math import radians

class Daimex:
    """daimex script settings"""
    
    scene = bpy.context.scene
    
    ############
    # SETTINGS #
    ############
    
    ##########################################################
    # DO NOT MAKE ANY CHANGES HERE                           #
    # This is default setting - a template                   #
    # Changes are to be made on END OF THE SCRIPT (line 230) #
    ##########################################################
    
    # mask for the names of the images output; read naming conventions: 
    # http://www.daimonin.net/index.php?name=PNphpBB2&file=viewtopic&t=3879&start=0
    filename = "type_description_a.1"
    
    # directory to which save the rendered images; default is directory of the current blend file;
    # to set the outputdir relative to current blend file use: os.path.join(os.path.dirname(Blender.Get("filename")), "subdir")
    outputdir = "C:\\output"
    
    # number of directions to render
    # starting position of camera is on y axis (under median when looking from top view) 
    # use only natural numbers (1;2;3...)
    directions = 8
    
    # number, by which is default render size multiplied; 
    # so for default image of 48px x 70px = 1x1 daimonin tiles (1,5 high - height of full walls) leave 1 here, for 2x2x3 daim tiles use 2
    rsmulti = 1
    
    #use the scripts own lightning (two rotating suns and one rotating spotlight for shadows) and turn off all other lights during rendering
    #lights with name beginning "daimex_" are NOT removed from the scene
    usescriptlamp = 1

    # direction which your model is facing
    # 6 7 0   7 = Y axis, 1 = X axis
    # 5 M 1   M - model
    # 4 3 2
    modeldir = 7
    
    # coordinates of your models centre on ground level, so you don't have to move it to the origin
    modelx = 0.0
    modely = 0.0
    modelz = 0.0

    # ( framestep - 1 ) == number of frames to skip in animation
    # If framestep is -1, than there is no animation and just first frame is rendered
    # NOTE: This MUST be overridden in each instance of Daimex that has animations.
    framestep = -1
    
    # Which animation frame to start and end this render at.
    framestart = 0
    frameend = 0
    
    # ADVANCED SETTINGS - don't configure unless you know what you are doing
    rsx = 48    # standard image width
    rsxwide = 150 # wide image width
    rsy = 70    # standard image height
    orframes = False    # if to override default time-line/renderer start/end frames
    framestart = 1
    frameend = 10

    camscale = 8.1  # camera lens scale
    camdofdist = 0.0    # camera lens distance of focus
    camcstart = 0.1 # camera clipping start (FOV)
    camcend = 100.0 # camera clipping start (FOV)
    camlocx = 0.0
    camlocy = -21.0

    camlocz = 14.4 #14.2
    camloczWide = 12.7 # camera z position for wide images
    # Note:  Scale objects to approx 0.47 to keep same size in wide images vs. normal images

    camrotx = 62.0
    camroty = 0.0
    camrotz = 0.0

    sunenergy = 0.8
    sundist = 20.0

    spotenergy = 0.8
    spotdist = 20.0

    #sunlocx = -9.7 #-0.7
    #sunlocy = -15.85 

    sunlocx1 = -9.7 #-0.7
    sunlocy1 = -15.85 
    sunlocx2 = +9.7 #-0.7
    sunlocy2 = -15.85 

    sunlocz = 19.0
    sunrotx = 0.0
    sunroty = 40.0

    sunrotz = -120.0
    sunrotz2 = -60.0
    
    spotlocx = -5
    spotlocy = -5
    spotlocz = 25
    
    spotrotx = 0
    spotroty = 10
    spotrotz = -150

    spotshadowsoftness = .12
    spotshadowsamples = 6

    wideimage = False

    ################
    pix2 = (math.pi * 2)
    uhx = pix2 / directions
    
    #######
    # RUN #
    #######
    def run(self):
        """main run function"""
        try:
            #set up renderer
            render = self.scene.render
            
            if (self.wideimage == False):
                render.resolution_x *= self.rsmulti
            else:
                render.resolution_x = self.rsxwide * self.rsmulti
                
            brsx = render.resolution_x
            brsy = render.resolution_y
            render.resolution_y = self.rsy * self.rsmulti
            render.alpha_mode = "TRANSPARENT"
            render.image_settings.file_format = "PNG"
        
            if (self.framestep == -1):
                self.framestep = bpy.context.scene.frame_end - bpy.context.scene.frame_start + 1
            
            cam = self.setup_camera()           
            
            if self.usescriptlamp == 1:
                remlamps = []
                for ob in self.scene.objects:
                    if ob.type == 'LAMP':

                         # Lamps with name beginning "daimex_" will NOT be removed from the scene
                        if ob.name.find("daimex_") == -1:
                            remlamps.append(ob)
                            self.scene.objects.unlink(ob)

                bpy.ops.object.lamp_add(type="SUN", location=(self.sunlocx1, self.sunlocy1, self.sunlocz), 
                        rotation = (radians(self.sunrotx), radians(self.sunroty), radians(self.sunrotz)))
                        
                suny = bpy.context.active_object
                        
                suny.data.energy = self.sunenergy
                suny.data.distance = self.sundist
                
                bpy.ops.object.lamp_add(type="SUN", location=(self.sunlocx2, self.sunlocy2, self.sunlocz), 
                        rotation = (radians(self.sunrotx), radians(self.sunroty), radians(self.sunrotz)))
                        
                suny2 = bpy.context.active_object
                        
                suny2.data.energy = self.sunenergy
                suny2.data.distance = self.sundist

                
                bpy.ops.object.lamp_add(type="SPOT", location=(self.spotlocx, self.spotlocy, self.spotlocz), 
                        rotation = (radians(self.spotrotx), radians(self.spotroty), radians(self.spotrotz)))
                        
                spot = bpy.context.active_object
                        
                spot.data.energy = self.spotenergy
                spot.data.distance = self.spotdist
                spot.data.spot_blend = self.spotshadowsoftness
                spot.data.shadow_buffer_samples = self.spotshadowsamples        
            
            #shooting
            rotsf = copy.copy(self.directions)
            rots = 1
            
            while(rotsf >= rots):
                #rotations
                oldcampos = cam.location
                cam.location.x = oldcampos[0]+self.modelx
                cam.location.y = oldcampos[1]+self.modely
                cam.location.z = oldcampos[2]+self.modelz

                if self.usescriptlamp == 1:
                    oldsunpos = suny.location
                    suny.location.x += self.modelx
                    suny.location.y += self.modely
                    suny.location.z += self.modelz
 
                    oldsunpos2 = suny2.location
                    suny2.location.x += self.modelx
                    suny2.location.y += self.modely
                    suny2.location.z += self.modelz
 
                    oldspotpos = spot.location
                    spot.location.x += self.modelx
                    spot.location.y += self.modely
                    spot.location.z += self.modelz

                npic = 1
                x = rots+self.modeldir
                while x > 8:
                    x = x-8
                if(self.orframes == False):
                    for j in range(self.framestart, 1 + self.frameend, self.framestep):
                        self.scene.frame_set(j)
                        render.filepath = os.path.join(self.outputdir, self.filename+str(x)+str(npic))
                        bpy.ops.render.render(write_still=True)
                        npic = npic+1
                else:
                    for j in range(self.framestart, 1 + self.frameend, self.framestep):
                        self.scene.frame_set(j)
                        render.filepath = os.path.join(self.outputdir, self.filename+str(x)+str(npic))
                        bpy.ops.render.render(write_still=True)
                        npic = npic+1
                        
                cam.location.x = oldcampos[0]
                cam.location.y = oldcampos[1]
                cam.location.z = oldcampos[2]
                self.rotateobj(cam.name)

                if self.usescriptlamp == 1:
                    suny.location.x = oldsunpos.x
                    suny.location.y = oldsunpos.y
                    suny.location.z = oldsunpos.z
                    self.rotateobj(suny.name)

                    suny2.location.x = oldsunpos2.x
                    suny2.location.y = oldsunpos2.y
                    suny2.location.z = oldsunpos2.z
                    self.rotateobj(suny2.name)

                    spot.location.x = oldspotpos.x
                    spot.location.y = oldspotpos.y
                    spot.location.z = oldspotpos.z
                    self.rotateobj(spot.name)

                rots = rots + 1
        finally:
            # clean up
            self.scene = bpy.context.scene
            render.resolution_x = brsx
            render.resolution_y = brsy
            self.scene.objects.unlink(cam)
            if self.usescriptlamp == 1:
                self.scene.objects.unlink(suny)
                self.scene.objects.unlink(suny2)
                self.scene.objects.unlink(spot)
                for obn in remlamps:
                    self.scene.objects.link(obn)
            print("finished; output =",self.outputdir)
        
    def setup_camera(self):
        bpy.ops.object.camera_add()
        cam = bpy.context.active_object
        cam.name = "daimex_cam"
        cam.data.name = "daimex_cam_cam" 
        self.scene.camera = cam
        cam.data.type = "ORTHO"
        
        cam.data.ortho_scale = self.camscale
        cam.data.clip_end = self.camcend
        cam.data.clip_start = self.camcstart
        cam.data.dof_distance = self.camdofdist
        
        cam.location.x = self.camlocx
        cam.location.y = self.camlocy
        cam.location.z = self.camlocz if self.wideimage == False else self.camloczWide
        
        cam.rotation_mode = 'XYZ'
        cam.rotation_euler = (radians(self.camrotx), radians(self.camroty), radians(self.camrotz))
        
        return cam    

    def rotateobj(self, name):
        ob = bpy.context.scene.objects[name]
        loc = ob.location

        if self.directions == 1:
            uhel = 0
        else:
            uhel = self.uhx
        
        matrix = mathutils.Matrix()
        matrix[0][0], matrix[0][1], matrix[0][2] = math.cos(uhel), -math.sin(uhel), 0
        matrix[1][0], matrix[1][1], matrix[1][2] = math.sin(uhel), math.cos(uhel), 0
        matrix[2][0], matrix[2][1], matrix[2][2] = 0, 0, 1
        
        target = matrix * loc
        ob.location.x = target.x
        ob.location.y = target.y
        ob.location.z = target.z
        
        ob.rotation_euler.z += self.uhx
        if (ob.rotation_euler.z >= self.pix2):
            ob.rotation_euler.z -= self.pix2
        
###############################################################################

#################
# CONFIGURATION #
#################

# This is where you are encouraged to make changes

# You can also make another python script from which you can import this one with 
# "import daimex" (prefix the objects with daimex. - for example daimex.Daimex())
# or "from daimex import *" (import to current namespace - no need for prefix)
# then you can use just the new script - but the daimex script needs to be in current directory or in python path
# (I am unsure if this will work, because of how Blender uses python)

# create an instance of Daimex class with:
#instance1 = Daimex()

#then change some settings if you like, for example:
#instance1.filename = "type_description_a.1"

#and run the script with:
#instance1.run()

