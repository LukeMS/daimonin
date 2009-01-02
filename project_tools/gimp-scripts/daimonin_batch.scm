; Updated to comply with minor script-fu changes to comply with GIMP v2.6

(define (script-fu-daimonin-batch pattern doAlpha cutColour doUnsharp inRadius inAmount inThreshold doShadow shadowZOffset doIndexed doCrop)
	(let*
		(
			(isIndexed 0)
			(shadowLayer 0)
			(layerList 0)
			(shadowXCrop 0)

			; Get list of files using 'pattern' supplied
			(numfiles (car (file-glob pattern 1))) 
			(filelist (cadr (file-glob pattern 1)))
		)


		; On Linux, filelist will be a vector, so convert to a simple list
		(if (vector? filelist) (set! filelist (vector->list filelist)))


		; Display number of files on the error window 
		(gimp-message-set-handler ERROR-CONSOLE) 
		(gimp-message (string-append "Number of files found: " (number->string numfiles))) 

		(while 
			(not (null? filelist))

			(let*
				(
					(filename (car filelist))
					(image (car (gimp-file-load RUN-NONINTERACTIVE filename filename)))
					(drawable (car (gimp-image-get-active-layer image)))
				)

				(gimp-message (string-append "File found: " filename)) 

				(if (= doAlpha TRUE)
					(script-fu-daimonin-add-alpha image drawable cutColour)
				)


				(if (= doUnsharp TRUE)
					(begin

						; Check to see if the image is indexed
						(set! isIndexed (car (gimp-drawable-is-indexed drawable)))


						; Now run unsharp plug-in
						(if (= isIndexed FALSE)
							(plug-in-unsharp-mask RUN-NONINTERACTIVE image drawable inRadius inAmount inThreshold)
						)
					)
				)


				(if (= doShadow TRUE)
					(begin
						(script-fu-perspective-shadow image drawable 45 3 0.3 3 '(0 0 0) 33 1 TRUE)

						; Find the shadow layer
						(set! layerList (cadr (gimp-image-get-layers image)))

						; For simple, single layer, files this will work
						(set! shadowLayer (aref layerList 1))

						; Adding shadow can grow image on the left - so chop this back off else images will
						; be misaligned in game
						(set! shadowXCrop (car (gimp-drawable-offsets drawable)))
						(if (> shadowXCrop 0)
							(gimp-image-crop image (- (car (gimp-image-width image)) shadowXCrop) (car (gimp-image-height image)) shadowXCrop 0)
						)

						(gimp-layer-resize-to-image-size shadowLayer)
						(gimp-layer-set-offsets shadowLayer 0 shadowZOffset)

						(set! drawable (car (gimp-image-merge-down image drawable 0)))

						; Force the image to crop back to original size (might be a better way to do this?)
						(gimp-image-crop image (car (gimp-image-width image)) (car (gimp-image-height image)) 0 0)
					)
				)


				(if (= doIndexed TRUE)
					(begin

						; Check to see if the image is indexed
						(set! isIndexed (car (gimp-drawable-is-indexed drawable)))


						; Now convert to indexed mode
						(if (= isIndexed FALSE)
							(gimp-image-convert-indexed image FS-DITHER MAKE-PALETTE 255 FALSE TRUE "")
						)
					)
				)


				(if (= doCrop TRUE)
					(script-fu-daimonin-crop image drawable)
				)


				(gimp-file-save RUN-NONINTERACTIVE image drawable filename filename)
				(gimp-image-delete image)
			)

			(set! filelist (cdr filelist))
		)

		; Display number of files on the error window 
		(gimp-message-set-handler MESSAGE-BOX) 
		(gimp-message (string-append "Processed " (number->string numfiles) " files"))

	)
)


(script-fu-register
	"script-fu-daimonin-batch"                      ;func name
	"_Batch Process"					;menu label
	"Daimonin - Batch process a set of Images"      ;description
	"Torchwood"					;author
	"Copyright 2007, Jim White"			;copyright notice
	"December 11, 2007"				;date created
	""						;image type that the script works on
	SF-STRING	"Path/Pattern to process" 	""
	SF-TOGGLE	"Add Alpha"				FALSE
	SF-COLOR	"Colour to change to Alpha"	'(255 255 255)
	SF-TOGGLE	"Unsharp Mask"			FALSE
	SF-ADJUSTMENT	"Radius"			'(5 0.1 120 0.1 1 1 0)
	SF-ADJUSTMENT	"Amount"			'(0.5 0 10 0.01 0.1 1 0)
	SF-ADJUSTMENT	"Threshold"			'(0 0 255 1 10 0 0)
	SF-TOGGLE	"Add Perspective Shadow"        TRUE
	SF-ADJUSTMENT	"Shadow Z Offset"		'(0 0 50 1 5 0 0)
	SF-TOGGLE	"Convert to Indexed"		FALSE
	SF-TOGGLE	"Crop"				TRUE)
(script-fu-menu-register "script-fu-daimonin-batch" "<Image>/_Daimonin")
