; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

(define (sg-clone-text-attributes image drawable
                                        criteria
                                        text?
                                        font?
                                        font-size?
                                        hinting?
                                        antialias?
                                        color?
                                        justification?
                                        indent?
                                        line-spacing?
                                        letter-spacing?
                                        kerning?
                                        language?
                                        base-direction?
                                        )
  (when (= (car (gimp-drawable-is-text-layer drawable)) TRUE)
    (gimp-image-undo-group-start image)
    (let* (
        (all-layers (gimp-image-get-layers image))
        (i (car all-layers))
        (layer's nil)
        (layer 0)
        )
      (set! all-layers (cadr all-layers))
      (while (> i 0)
        (set! i (- i 1))
        (set! layer (vector-ref all-layers i))
        (when (= (car (gimp-drawable-is-text-layer layer)) TRUE)
          (if (= criteria 0)
            (unless (= (car (gimp-drawable-get-visible layer)) FALSE)
              (set! layer's (cons layer layer's))
              )
            (if (= criteria 2)
              (unless (= (car (gimp-drawable-get-linked layer)) FALSE)
                (set! layer's (cons layer layer's))
                )
              (set! layer's (cons layer layer's))
              )
            )
          )
        )
      (map (lambda (layer)
           (when (= text? TRUE)
             (let* ((name (car (gimp-drawable-get-name layer))))
               (gimp-text-layer-set-text layer (car (gimp-text-layer-get-text drawable)))
               (gimp-drawable-set-name layer name)))
           (when (= font? TRUE)
             (gimp-text-layer-set-font layer (car (gimp-text-layer-get-font drawable))))
           (when (= font-size? TRUE)
             (let* ((font-size (gimp-text-layer-get-font-size drawable)))
               (gimp-text-layer-set-font-size layer (car font-size) (cadr font-size))))
           (when (= hinting? TRUE)
             (let* ((hinting (gimp-text-layer-get-hinting drawable)))
               (gimp-text-layer-set-hinting layer (car hinting) (cadr hinting))))
           (when (= antialias? TRUE)
             (gimp-text-layer-set-antialias layer (car (gimp-text-layer-get-antialias drawable))))
           (when (= color? TRUE)
             (gimp-text-layer-set-color layer (car (gimp-text-layer-get-color drawable))))
           (when (= justification? TRUE)
             (gimp-text-layer-get-justification layer (car (gimp-text-layer-set-justification drawable))))
           (when (= indent? TRUE)
             (gimp-text-layer-set-indent layer (car (gimp-text-layer-get-indent drawable))))
           (when (= line-spacing? TRUE)
             (gimp-text-layer-set-line-spacing layer (car (gimp-text-layer-get-line-spacing drawable))))
           (when (= letter-spacing? TRUE)
             (gimp-text-layer-set-letter-spacing layer (car (gimp-text-layer-get-letter-spacing drawable))))
           (when (= kerning? TRUE)
             (gimp-text-layer-set-kerning layer (car (gimp-text-layer-get-kerning drawable))))
           (when (= language? TRUE)
             (gimp-text-layer-set-language layer (car (gimp-text-layer-get-language drawable))))
           (when (= base-direction? TRUE)
             (gimp-text-layer-set-base-direction layer (car (gimp-text-layer-get-base-direction drawable))))
           )
        layer's
        )
      )
    (gimp-displays-flush)
    (gimp-image-undo-group-end image)
    )
  )

(script-fu-register "sg-clone-text-attributes"
 "<Layers>/Clone text properties..."
 "Make text attributes match active text layer"
 "Saul Goode"
 "Saul Goode"
 "2/23/2008"
 "*"
  SF-IMAGE    "Image"    0
  SF-DRAWABLE "Drawable" 0
  SF-OPTION "Criteria" '("Visible layers" "All layers" "Linked layers")
  SF-TOGGLE "Clone text?" FALSE
  SF-TOGGLE "Clone font?" FALSE
  SF-TOGGLE "Clone font-size?" FALSE
  SF-TOGGLE "Clone hinting?" FALSE
  SF-TOGGLE "Clone antialias?" FALSE
  SF-TOGGLE "Clone color?" FALSE
  SF-TOGGLE "Clone justification?" FALSE
  SF-TOGGLE "Clone indent?" FALSE
  SF-TOGGLE "Clone line-spacing?" FALSE
  SF-TOGGLE "Clone letter-spacing?" FALSE
  SF-TOGGLE "Clone kerning?" FALSE
  SF-TOGGLE "Clone language?" FALSE
  SF-TOGGLE "Clone base-direction?" FALSE
  )
