; Updated to comply with minor script-fu changes to comply with GIMP v2.6
; Updated to improve GIF animation, by merging each layer with a white background

(define (script-fu-daimonin-tile-frame pattern size inDirections endFrame onlyLastFrame start end inBorderWidth borderColour saveGIMP keepImagesOpen)


	; Define a local procedure that handles a single frame
	(define (script-fu-daimonin-tile-frame-local pass pattern size actualImgWidth inDirections frame start end inBorderWidth borderColour saveGIMP keepImagesOpen)
		; Local function to update a 4 point array
		(define (update-segment! s x0 y0 x1 y1)
			(aset s 0 x0)
			(aset s 1 y0)
			(aset s 2 x1)
			(aset s 3 y1)
		)

		(let*
			(
				(baseWidth 48)
				(baseHeight 70)
	
				(tileWidth 0)
				(tileHeight 0)

				(aTileWidth 0)
				(TileOffsetX 0)
				(maxImgWidth 0)

				(imgWidth 1)
				(imgHeight 1)

				(img 0)
				(gridLayer 0)

				(curWidth 0)
				(curHeight 0)

				(segment (cons-array 4 'double))
	
				(numFiles 0)
				(fileName 0)
				(tileLayer 0)

				(numDirections 0)
				(colOffset 0)
				(fileSuffix 0)
	
				(curTileColumn 0)
				(curTileRow 0)
				(curAction 0)

                        (numDirections 0)

				(nominalOffset 0)

				; Convert inBorderWidth to actual border width
				(borderWidth (+ (* 2 inBorderWidth) 1))

			) ; end of variable declaration

			(gimp-context-push)
			(gimp-message-set-handler ERROR-CONSOLE) 


			; Check inDirections and set parameters
			(cond
				((= inDirections 0)
						; Convert inDirections from zero based 'choice' (0, 1, 2) to actual number of directions: 1-8, 1-4, 5-8
						(set! numDirections 8)

						; Work out the tile column offset - used to put image 5 in column 1 when only doing directions 5-8
						(set! colOffset 0)

						(set! fileSuffix "")
				)

				((= inDirections 1)
						(set! numDirections 4)
						(set! colOffset 0)
						(set! fileSuffix "a")
				)

				(else
						(set! numDirections 4)
						(set! colOffset 4)
						(set! fileSuffix "b")
				)
			)


			; Image size and grip preparation - only need to do on pass 2
			(if (= pass 2)
				(begin
					; Set size of a single "tile"
					(set! tileWidth actualImgWidth)
					(set! tileHeight (* baseHeight size))


					; Calculate nominal offset for non-wide images
					(set! nominalOffset (/ (- actualImgWidth (* baseWidth size)) 2))


					; Calculate overall image size
					; 8 (or 4) colums (for 8 (or 4) directions) + borders
					; rows depend on start and end actions (would normally be 1, 3, but could be 4 for resting images)
					(set! imgWidth (+ (* tileWidth numDirections) (* borderWidth (+ numDirections 1))))
					(set! imgHeight (+ (* tileHeight (+ end (- start) 1)) (* borderWidth (+ end (- start) 2))))
				)
			)


			; Create the new image, only used as temp on pass 1
			(set! img (car (gimp-image-new imgWidth imgHeight RGB)))


			; Only need to do stuff with the image on pass 2
			(if (= pass 2)
				(begin

					; Add Create and add the base grid layer
					(set! gridLayer (car (gimp-layer-new img imgWidth imgHeight RGB-IMAGE "Grid" 100 NORMAL-MODE)))
					(gimp-image-add-layer img gridLayer 0)


					; Set grid layer to be transparent and fill
					(gimp-layer-add-alpha gridLayer)
					(gimp-drawable-fill gridLayer TRANSPARENT-FILL)


					; Set brush/colour to draw grid
					(gimp-context-set-foreground borderColour)
					(gimp-context-set-brush (string-append "Circle (0" (number->string borderWidth) ")"))

					; Need to set the starting position for grid lines (to the centre of the border)
					(set! curWidth (/ (- borderWidth 1) 2))
					(set! curHeight curWidth)


					; Draw grid vertical lines
					(while (<= curWidth imgWidth)
						(update-segment! segment curWidth 0 curWidth (- imgHeight 1))
						(gimp-pencil gridLayer 4 segment)
						(set! curWidth (+ curWidth tileWidth borderWidth))
					)


					; Draw grid horizontal lines
					(while (<= curHeight imgHeight)
						(update-segment! segment 0 curHeight (- imgWidth 1) curHeight)
						(gimp-pencil gridLayer 4 segment)
						(set! curHeight (+ curHeight tileHeight borderWidth))
					)
				)
			)

			; Now add the images
			(set! curAction start)
			(set! curTileRow 1)

			(while (<= curAction end)

				(set! curTileColumn 1)

				(while (<= curTileColumn numDirections)

					; Build the file name
					(set! fileName (string-append pattern "." (number->string curAction) (number->string (+ curTileColumn colOffset)) (number->string frame) ".png"))


					; Check to see if the appropriate file exists
					(set! numFiles (car (file-glob fileName 1)))
					(if (= numFiles 1)

						(begin
	
							;(gimp-message (string-append "Found file: " fileName))


							; Load this file as a new layer in our image
							(set! tileLayer (car (gimp-file-load-layer RUN-NONINTERACTIVE img fileName)))


							; Get width of this loaded image
							(set! aTileWidth (car (gimp-drawable-width tileLayer)))


							(if (= pass 2)
								(begin
									; Add new loaded layer to image
									(gimp-image-add-layer img tileLayer -1)


									; Calculate the X offset within the tile
									(set! TileOffsetX (/ (- actualImgWidth aTileWidth) 2))
									; If offset is small, then we have a non-centred image, so use fixed offset = 1/2 * (tileWidth - rsmultiWidth)
									(if (> TileOffsetX nominalOffset)
										(set! TileOffsetX nominalOffset)
									)


									; Position the new layer correctly in the grid
									(gimp-layer-set-offsets tileLayer (+ (- (* (+ tileWidth borderWidth) curTileColumn) tileWidth) TileOffsetX)
									                                  (- (* (+ tileHeight borderWidth) curTileRow) (car (gimp-drawable-height tileLayer))))

								)

								; else

								(begin
									(if (< maxImgWidth aTileWidth)
										(set! maxImgWidth aTileWidth)
									)
								)
							)						
						)
					)

					(set! curTileColumn (+ curTileColumn 1))
				)


				(set! curAction (+ curAction 1))
				(set! curTileRow (+ curTileRow 1))
			)

			(if (= pass 2)
				(begin
					; Force the image to crop back to the calculated size
					(gimp-image-crop img imgWidth imgHeight 0 0)

					; Create a file name for the new image
					(set! fileName (string-append pattern "." (number->string frame) fileSuffix))


					(if (= saveGIMP TRUE)
						(begin

							(set! fileName (string-append fileName ".xcf"))

						)

						; else ...
						(begin

							; Merge all layers into 1
							(gimp-image-merge-visible-layers img EXPAND-AS-NECESSARY)
							(set! gridLayer (car (gimp-image-get-active-drawable img)))

							(set! fileName (string-append fileName ".png"))
						)
					)


					; Save the file
					(gimp-image-set-filename img fileName)
					(gimp-file-save RUN-NONINTERACTIVE img gridLayer fileName fileName)
					(gimp-image-clean-all img)


					(if (= keepImagesOpen TRUE)
						(begin
							(gimp-displays-flush)
							(gimp-display-new img)
						)
					)
				)

				; else
				(gimp-image-clean-all img)

			)


			; Some standard GIMP stuff just to finish off ...
			(gimp-context-pop)


			; Return filename or maxTileWidth to the calling function
			(if (= pass 2)
				(begin
					fileName
				)
				(begin
					maxImgWidth
				)
			)
		) ; end of (let* statement
	) ; end of function defn
	

	
	; Start of the actual code for the main function
	(let*
		(
			(firstFrame 1)
			(curFrame 0)
			(animImg 0)
			(newLayer 0)
			(whiteLayer 0)
			(animFileName 0)
			(frameFileName 0)
			(curImgWidth 0)
			(maxImgWidth 0)
		)


		(gimp-message-set-handler ERROR-CONSOLE) 

		; Check actions are OK - i.e. end => start
		(if (< end start)

			(gimp-message "'END' action must not be less than 'START' action")
	
			; else ...

			(begin

				(if (= onlyLastFrame TRUE)
					(set! firstFrame endFrame)
				)


				(set! curFrame firstFrame)


				; Create a new image to hold the animation
				(if (and (= saveGIMP FALSE) (> endFrame firstFrame))
					(set! animImg (car (gimp-image-new 1 1 RGB)))
				)


				; 1st pass - get max tile width
				(gimp-message "Starting pass 1 ...")
				(while (<= curFrame endFrame)
					(gimp-message (string-append "Scanning animation frame: " (number->string curFrame)))

					(set! curImgWidth (script-fu-daimonin-tile-frame-local 1 pattern size 0 inDirections curFrame start end inBorderWidth borderColour saveGIMP keepImagesOpen))
				
					(if (> curImgWidth maxImgWidth)
						(set! maxImgWidth curImgWidth)
					)
				
					(set! curFrame (+ curFrame 1))
				)


				(set! curFrame firstFrame)


				; 2nd pass - do actual work!
				(gimp-message "Starting pass 2 ...")
				(while (<= curFrame endFrame)
					(gimp-message (string-append "Building animation frame: " (number->string curFrame)))

					(set! frameFileName (script-fu-daimonin-tile-frame-local 2 pattern size maxImgWidth inDirections curFrame start end inBorderWidth borderColour saveGIMP keepImagesOpen))

					(if (and (= saveGIMP FALSE) (> endFrame firstFrame))
						(begin
							; Add a new layer - fill later with white
							(set! whiteLayer (car (gimp-layer-new animImg 1 1 RGB "white" 100 0)))
							(gimp-image-add-layer animImg whiteLayer -1)

							; Import the actual animation frame, add to image and resize image to match new layer size
							(set! newLayer (car (gimp-file-load-layer RUN-NONINTERACTIVE animImg frameFileName)))
							(gimp-image-add-layer animImg newLayer -1)
							(gimp-image-resize-to-layers animImg)

							; Resize the white layer, fill with white and merge animation frame down into it
							(gimp-layer-resize-to-image-size whiteLayer)
							(gimp-drawable-fill whiteLayer WHITE-FILL)
							(gimp-image-merge-down animImg newLayer CLIP-TO-IMAGE)
						)
					)

					(set! curFrame (+ curFrame 1))

				)


				(if (and (= saveGIMP FALSE) (> endFrame firstFrame))

					(begin
						(gimp-message "Creating GIF image ...")

						(gimp-image-resize-to-layers animImg)

						; newLayer gets modified when we do the merge-down command, so re-get the active layer
						(set! newLayer (car (gimp-image-get-active-layer animImg))) 

						(gimp-image-convert-indexed animImg FS-DITHER MAKE-PALETTE 255 FALSE TRUE "")

						(set! animFileName (string-append pattern "." (number->string inDirections) ".gif"))

						(gimp-image-set-filename animImg animFileName)
						(file-gif-save RUN-NONINTERACTIVE animImg newLayer animFileName animFileName 0 1 500 2)
						(gimp-image-clean-all animImg)


						(if (= keepImagesOpen TRUE)
							(begin
								(gimp-displays-flush)
								(gimp-display-new animImg)
							)
						)
					)

				)


				(gimp-message-set-handler MESSAGE-BOX) 
				(gimp-message "Finished!")
			)
		)
	)
)


; Register function with GIMP
(script-fu-register 
   "script-fu-daimonin-tile-frame"         ;func name 
   "_Tile"               ;menu label 
   "Daimonin - Tile a set of Images"      ;description 
   "Torchwood"               ;author 
   "Copyright 2007, Jim White"         ;copyright notice 
   "January 15, 2007"            ;date created 
   ""                  ;image type that the script works on 
   SF-STRING   "File base name (+ path) to tile"    ""
   SF-VALUE    "Tile size factor" "1"
   SF-OPTION   "Directions" '("1 - 8" "1 - 4" "5 - 8")
   SF-VALUE    "Last animation frame number" "4"
   SF-TOGGLE   "Tile only last frame" FALSE
   SF-OPTION   "Start Action" '("0 - Resting" "1 - Standing" "2 - Running" "3 - Attacking")
   SF-OPTION   "End Action"   '("0 - Resting" "1 - Standing" "2 - Running" "3 - Attacking")
   SF-OPTION   "Grid Line Width"   '("1" "3" "5" "7")
   SF-COLOR    "Grid Line Colour"  '(0 0 0)
   SF-TOGGLE   "Save XCF GIMP format file only (no png file, no animated gif created)"  FALSE
   SF-TOGGLE   "Keep images open" FALSE
) 
(script-fu-menu-register "script-fu-daimonin-tile-frame" "<Image>/_Daimonin")
