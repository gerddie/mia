FUNCTION filter3d

filter = 'gauss:w=1'
xsize = 3
ysize = 3
zsize = 3

imag_in = fltarr(xsize,ysize,zsize)

imag_in[0,0,0] = 0.0
imag_in[1,0,0] = 0.0
imag_in[2,0,0] = 0.0
imag_in[0,1,0] = 0.0
imag_in[1,1,0] = 0.0
imag_in[2,1,0] = 0.0
imag_in[0,2,0] = 0.0
imag_in[1,2,0] = 0.0
imag_in[2,2,0] = 0.0

imag_in[0,0,1] = 0.0
imag_in[1,0,1] = 0.0
imag_in[2,0,1] = 0.0
imag_in[0,1,1] = 0.0
imag_in[1,1,1] = 64.0
imag_in[2,1,1] = 0.0
imag_in[0,2,1] = 0.0
imag_in[1,2,1] = 0.0
imag_in[2,2,1] = 0.0

imag_in[0,0,2] = 0.0
imag_in[1,0,2] = 0.0
imag_in[2,0,2] = 0.0
imag_in[0,1,2] = 0.0
imag_in[1,1,2] = 0.0
imag_in[2,1,2] = 0.0
imag_in[0,2,2] = 0.0
imag_in[1,2,2] = 0.0
imag_in[2,2,2] = 0.0

imag_out = fltarr(xsize,ysize,zsize)

imag_out[0,0,0] = 1
imag_out[1,0,0] = 2
imag_out[2,0,0] = 1
imag_out[0,1,0] = 2
imag_out[1,1,0] = 4
imag_out[2,1,0] = 2
imag_out[0,2,0] = 1
imag_out[1,2,0] = 2
imag_out[2,2,0] = 1

imag_out[0,0,1] = 2
imag_out[1,0,1] = 4
imag_out[2,0,1] = 2
imag_out[0,1,1] = 4
imag_out[1,1,1] = 8
imag_out[2,1,1] = 4
imag_out[0,2,1] = 2
imag_out[1,2,1] = 4
imag_out[2,2,1] = 2

imag_out[0,0,2] = 1
imag_out[1,0,2] = 2
imag_out[2,0,2] = 1
imag_out[0,1,2] = 2
imag_out[1,1,2] = 4
imag_out[2,1,2] = 2
imag_out[0,2,2] = 1
imag_out[1,2,2] = 2
imag_out[2,2,2] = 1


;---------------------------------------------------------

imag_flt = MIA_FILTER3D(filter, imag_in)

;---------------------------------------------------------

FOR i = 0, 2  DO BEGIN
    FOR j = 0, 2 DO BEGIN
        FOR k= 0, 2 DO BEGIN
       IF ( imag_out[i,j,k] NE imag_flt[i,j,k] ) THEN RETURN, 0
        ENDFOR
    ENDFOR
ENDFOR

RETURN, 1

END
