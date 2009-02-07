; Updated to comply with minor script-fu changes to comply with GIMP v2.6
; Added option to allow for centre-width cropping used with images wider than 48xmobsize

(define (script-fu-daimonin-crop img tdrawable rsmulti)

	(let*
		(

			; Get the starting width / height of the image
			(theImageWidth (car (gimp-image-width img)))
			(theImageHeight (car (gimp-image-height img)))

			; Define variables to store the layer size and offsets after autocrop
			(theCroppedWidth 0)
			(theCroppedHeight 0)
			(theCroppedOffsets 0)
			(theCroppedXLeft 0)
			(theCroppedXRight 0)
			(theCroppedY 0)

			; Variable to store how much we will actually crop of sides when doing a centred crop
			(theCropX 0)

			; Variable to store new width and height of image
			(theNewWidth 0)
			(theNewHeight 0)

		)
	
		; Autocrop the current layer
		(plug-in-autocrop-layer RUN-NONINTERACTIVE img tdrawable)

		; Get the width / height of the cropped layer
		(set! theCroppedWidth (car (gimp-drawable-width tdrawable)))
		(set! theCroppedHeight (car (gimp-drawable-height tdrawable)))

		; Get the x,y offsets of the cropped layer
		(set! theCroppedOffsets (gimp-drawable-offsets tdrawable))
		(set! theCroppedY (cadr theCroppedOffsets))
		(set! theCroppedXLeft (car theCroppedOffsets))
		(set! theCroppedXRight (- (- theImageWidth theCroppedWidth) theCroppedXLeft))

		; Now restore the cropped layer back to full size
		(gimp-layer-resize-to-image-size tdrawable)


		; Check which side of the image had the least cropped off
		(if (< theCroppedXLeft theCroppedXRight)
			(set! theCropX theCroppedXLeft)
			; else
			(set! theCropX theCroppedXRight)
		)

		; Calculate new image width/height
		(set! theNewWidth (- (- theImageWidth theCropX) theCropX))
		(set! theNewHeight (- theImageHeight theCroppedY))


		; Check if width is less than 48 * rsmulti
		(if (< theNewWidth (* 48 rsmulti))
			(begin
				; In this case, we can fully crop the right side of the image
				(set! theCropX (/ (- theImageWidth (* 48 rsmulti)) 2))
				(set! theNewWidth (- (- theImageWidth theCropX) theCroppedXRight))
			)
		)


		; Now crop the top and sides off the layer
		(gimp-image-crop img theNewWidth theNewHeight theCropX theCroppedY)


		; Update the display
		(gimp-displays-flush)
	)

)

(script-fu-register
	"script-fu-daimonin-crop"			;func name
	"_Crop"						;menu label
	"Crop empty space from top and right, or top and equal both sides of image"  ;description
	"Torchwood"                                     ;author
	"Copyright 2009, Jim White"			;copyright notice
	"February 1, 2009"				;date created
	""						;image type that the script works on
	SF-IMAGE       "Input image"    0
	SF-DRAWABLE    "Input drawable" 0
	SF-VALUE       "Tile size factor" "1"
)
(script-fu-menu-register "script-fu-daimonin-crop" "<Image>/_Daimonin")
