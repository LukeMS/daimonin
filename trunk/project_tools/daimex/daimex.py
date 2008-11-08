  ##########################################################################################
 # DAIMEX - script for Blender, for rendering and formating for use in Daimonin 2d client #
##########################################################################################
  # See settings starting on line 42! #
 # To run the script press ALT+P     #
#####################################
 # version 0.4 #
###############

# Use CatRoom rendering filter for sharpest results
# I recommend playing with renderer setting sky/prem/key and the sky colour 
# - on the edges of transparency of images you can see a lot of differences

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

import Blender, os, math, copy

class Daimex:
    """daimex script settings"""
    
    scene = Blender.Scene.GetCurrent()
    
      ############
     # SETTINGS ###################################################################
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
    outputdir = os.path.dirname(Blender.Get("filename"))
    
    # number of directions to render
    # starting position of camera is on y axis (under median when looking from top view) 
    # use only natural numbers (1;2;3...)
    directions = 8
    
    # number, by which is default render size multiplied; 
    # so for default image of 48px x 70px = 1x1 daimonin tiles (1,5 high - height of full walls) leave 1 here, for 2x2x3 daim tiles use 2
    rsmulti = 1
    
    #use the scripts own lightning (one rotating sun) and turn off all other lights during rendering
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
    framestep = -1
    
    # ADVANCED SETTINGS - don't configure unless you know what you are doing
    rsx = 48    # standard image width
    rsy = 70    # standard image height
    orframes = False    # if to override default time-line/renderer start/end frames
    framestart = 1
    frameend = 10
    camscale = 8.1    # camera lens scale
    camdofdist = 0.0    # camera lens distance of focus
    camcstart = 0.1    # camera clipping start (FOV)
    camcend = 100.0    # camera clipping start (FOV)
    camlocx = 0.0
    camlocy = -21.0
    camlocz = 14.2
    camrotx = 62.0
    camroty = 0.0
    camrotz = 0.0
    sunenergy = 2.0
    sundist = 20.0
    sunlocx = -9.7 #-0.7
    sunlocy = -15.85 
    sunlocz = 19.0
    sunrotx = 0.0
    sunroty = 40.0
    sunrotz = -120.0
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
            render = self.scene.getRenderingContext()
            brsx = render.imageSizeX()
            render.imageSizeX(self.rsx*self.rsmulti)
            brsy = render.imageSizeY()
            render.imageSizeY(self.rsy*self.rsmulti)
            render.setRenderPath("")
            render.enableRGBAColor()
            render.setImageType(Blender.Scene.Render.PNG)
        
            if (self.framestep == -1):
                self.framestep = render.eFrame - render.sFrame + 1
            
            # create camera
            cam = Blender.Object.New("Camera" ,"daimex_cam")
            daimexcam = Blender.Camera.New("ortho")
            daimexcam.clipEnd = self.camcend
            daimexcam.clipStart = self.camcstart
            daimexcam.dofDist = self.camdofdist
            daimexcam.scale = self.camscale
            cam.link(daimexcam)
            cam.setLocation(self.camlocx, self.camlocy, self.camlocz)
            cam.setEuler([((self.camrotx/180.0)*math.pi), ((self.camroty/180.0)*math.pi), ((self.camrotz/180.0)*math.pi)])
            self.scene.objects.link(cam)
            
            if self.usescriptlamp == 1:
                remlamps = []
                for ob in self.scene.objects:
                    if ob.type == 'Lamp':
                        remlamps.append(ob.name)
                        self.scene.objects.unlink(ob)
                # create sun
                suny = Blender.Object.New("Lamp" ,"daimex_sun")
                daimexsun = Blender.Lamp.New("Sun")
                daimexsun.setEnergy(self.sunenergy)
                daimexsun.setDist(self.sundist)
                suny.link(daimexsun)
                suny.setLocation(self.sunlocx, self.sunlocy, self.sunlocz)
                suny.setEuler([((self.sunrotx/180.0)*math.pi), ((self.sunroty/180.0)*math.pi), ((self.sunrotz/180.0)*math.pi)])
                self.scene.objects.link(suny)
                
            Blender.Redraw()
            
            #shooting
            rotsf = copy.copy(self.directions)
            rots = 1
            self.scene.objects.camera = cam
            while(rotsf >= rots):
                #rotations
                oldcampos = cam.getLocation("worldspace")
                cam.setLocation(oldcampos[0]+self.modelx, oldcampos[1]+self.modely, oldcampos[2]+self.modelz)
                if self.usescriptlamp == 1:
                    oldsunpos = suny.getLocation("worldspace")
                    suny.setLocation(oldsunpos[0]+self.modelx, oldsunpos[1]+self.modely, oldsunpos[2]+self.modelz)
                npic = 1
                x = rots+self.modeldir
                while x > 8:
                    x = x-8
                if(self.orframes == False):
                    for j in range(render.startFrame(), 1 + render.endFrame(), self.framestep):
                        render.currentFrame(j)
                        render.render()
                        render.saveRenderedImage(os.path.join(self.outputdir, self.filename+str(x)+str(npic)))
                        npic = npic+1
                else:
                    for j in range(self.framestart, 1 + self.frameend, self.framestep):
                        render.currentFrame(j)
                        render.render()
                        render.saveRenderedImage(os.path.join(self.outputdir, self.filename+str(x)+str(npic)))
                        npic = npic+1
                cam.setLocation((oldcampos[0]), (oldcampos[1]), (oldcampos[2]))
                rotateobj(cam.getName())
                if self.usescriptlamp == 1:
                    suny.setLocation((oldsunpos[0]), (oldsunpos[1]), (oldsunpos[2]))
                    rotateobj(suny.getName())
                rots = rots + 1
        finally:
            # clean up
            self.scene = Blender.Scene.GetCurrent()
            render.imageSizeX(brsx)
            render.imageSizeY(brsy)
            self.scene.objects.unlink(cam)
            if self.usescriptlamp == 1:
                self.scene.objects.unlink(suny)
                for obn in remlamps:
                    ob = Blender.Object.Get(obn)
                    self.scene.objects.link(ob)
            print "finished; output =",self.outputdir
        
    

def rotateobj(name):
    kostka = Blender.Object.Get(name)
    if Daimex.directions == 1:
        uhel = 0
    else:
        uhel = Daimex.uhx
    #print "\nangle =",uhel,"radians"
    vectorCA = Blender.Mathutils.Vector(list(kostka.getLocation("worldspace")))
    matrix = Blender.Mathutils.Matrix([math.cos(uhel), -math.sin(uhel), 0], [math.sin(uhel), math.cos(uhel), 0], [0, 0, 1])
    target = (matrix * vectorCA)
    #print "starting coords =",kostka.getLocation("worldspace")
    #print "target coords =",target
    kostka.setLocation(target.x, target.y, target.z)
    kostka.RotZ = kostka.RotZ + Daimex.uhx
    if (kostka.RotZ >= Daimex.pix2):
        kostka.RotZ = kostka.RotZ - Daimex.pix2
    Blender.Redraw()
    
###############################################################################

  #################
 # CONFIGURATION #
#################

# This is where you are encouraged to make changes

# create an instance of Daimex class with:
instance1 = Daimex()

#then change some settings if you like, for example:
instance1.filename = "type_description_a.1"

#and run the script with:
instance1.run()

