;1) Adds Alpha Channel (if not already there) 
;2) Selects all pure white (or color of my choice would be nice) 
;3) Clears selection 


(define (script-fu-daimonin-add-alpha img tdrawable color)

	(let*
		(
			; Define local variables ...

			; Check to see if the image has an alpha layer already
			(hasAlpha (car (gimp-drawable-has-alpha tdrawable)))

			; Other variables
			(isSelection FALSE)
		)

		(if (= hasAlpha FALSE)
			(gimp-layer-add-alpha tdrawable)
		)
	
		; The parameters in this line can be adjusted, in particular the parameter directly
		; after 'color'.  Range is 0-255.  See the GIMP Procedure Browser for more details.
		(gimp-by-color-select tdrawable color 100 0 TRUE FALSE 0 FALSE)

		; Did we select anything?
		(set! isSelection (car (gimp-selection-bounds img)))

		(if (= isSelection TRUE)
			(gimp-edit-cut tdrawable)
		)

		; Update the display
		(gimp-displays-flush)
	)
)

(script-fu-register
	"script-fu-daimonin-add-alpha"                  ;func name
	"Add Alpha"                                     ;menu label
	"Daimonin - Add alpha layer"                    ;description
	"Torchwood"					;author
	"Copyright 2007, Jim White"			;copyright notice
	"December 27, 2007"				;date created
	""						;image type that the script works on
	SF-IMAGE       "Input image"           0
	SF-DRAWABLE    "Input drawable"        0
	SF-COLOR       "Color to erase"        '(255 255 255)
)

(script-fu-menu-register "script-fu-daimonin-add-alpha" "<Image>/Daimonin")
