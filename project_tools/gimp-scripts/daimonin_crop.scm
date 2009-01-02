; Updated to comply with minor script-fu changes to comply with GIMP v2.6

(define (script-fu-daimonin-crop img tdrawable)

	(let*
		(

			; Get the starting width / height of the image
			(theImageWidth (car (gimp-image-width img)))
			(theImageHeight (car (gimp-image-height img)))

			; Define variables to store the layer size and offsets after autocrop
			(theCroppedWidth 0)
			(theCroppedHeight 0)
			(theCroppedOffsets 0)
			(theCroppedX 0)
			(theCroppedY 0)

		)
	
		; Autocrop the current layer
		(plug-in-autocrop-layer RUN-NONINTERACTIVE img tdrawable)

		; Get the width / height of the cropped layer
		(set! theCroppedWidth (car (gimp-drawable-width tdrawable)))
		(set! theCroppedHeight (car (gimp-drawable-height tdrawable)))

		; Get the x,y offsets of the cropped layer
		(set! theCroppedOffsets (gimp-drawable-offsets tdrawable))
		(set! theCroppedX (car theCroppedOffsets))
		(set! theCroppedY (cadr theCroppedOffsets))	

		; Now restore the cropped layer back to full size
		(gimp-layer-resize-to-image-size tdrawable)

		; Now crop the top and right sides off the whole image
		(gimp-image-crop img (+ theCroppedWidth theCroppedX) (- theImageHeight theCroppedY) 0 theCroppedY)

		; Update the display
		(gimp-displays-flush)
	)

)

(script-fu-register
	"script-fu-daimonin-crop"			;func name
	"_Crop"						;menu label
	"Crop empty space from top and right of image"  ;description
	"Torchwood"                                     ;author
	"Copyright 2007, Jim White"			;copyright notice
	"December 11, 2007"				;date created
	""						;image type that the script works on
	SF-IMAGE       "Input image"    0
	SF-DRAWABLE    "Input drawable" 0
)
(script-fu-menu-register "script-fu-daimonin-crop" "<Image>/_Daimonin")
