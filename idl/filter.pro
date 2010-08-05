FUNCTION filter

xsize = 3S
ysize = 3S

filter = 'gauss:w=1'

imag_in = fltarr(xsize,ysize)

imag_in[0,0] = 0.0
imag_in[1,0] = 0.0
imag_in[2,0] = 0.0
imag_in[0,1] = 0.0
imag_in[1,1] = 1.0
imag_in[2,1] = 0.0
imag_in[0,2] = 0.0
imag_in[1,2] = 0.0
imag_in[2,2] = 0.0

imag_out = fltarr(xsize,ysize)

imag_out[0,0] = 0.0625
imag_out[1,0] = 0.125
imag_out[2,0] = 0.0625
imag_out[0,1] = 0.125
imag_out[1,1] = 0.25
imag_out[2,1] = 0.125
imag_out[0,2] = 0.0625
imag_out[1,2] = 0.125
imag_out[2,2] = 0.0625

;---------------------------------------------------------

 imag_flt = MIA_FILTER2D(filter, imag_in)

;---------------------------------------------------------


FOR i = 0, 2  DO BEGIN
    FOR j = 0, 2 DO BEGIN
       IF ( imag_out[i,j] NE imag_flt[i,j] ) THEN RETURN, 0
    ENDFOR
ENDFOR

RETURN, 1

END

