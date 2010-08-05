dllname='idlmiafilter.so'
function_c = 'echo'
filter = 'test'
in = 10
out = 20
tests_good = 0
tests = 2
settest='set_test'
OK = CALL_EXTERNAL(dllname, 'mia2', settest)
OK = CALL_EXTERNAL(dllname, 'mia2', 'set_debug_level', 'debug')
OK = CALL_EXTERNAL(dllname, 'mia2', function_c, filter, in, out)
IF ( OK NE 0 ) THEN PRINT, 'ECHO failed', OK ELSE BEGIN
	IF ( in EQ out ) THEN tests_good = tests_good + 1 ELSE PRINT, 'ECHO failed'
ENDELSE

xsize = 3S
ysize = 3S

str_file_dll = get_dllname() 
function_c = 'filter2d'
filter = 'gauss:w=1'
in_type = 'float'
out_type = 'float'

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

imag_flt = fltarr(xsize,ysize)

;---------------------------------------------------------

OK = CALL_EXTERNAL(dllname,'mia2', function_c, filter, xsize, ysize, in_type, imag_in, out_type, imag_flt)

;---------------------------------------------------------

filter2d_failed = 0

FOR i = 0, 2  DO BEGIN
        FOR j = 0, 2 DO BEGIN
                IF ( imag_out[i,j] NE imag_flt[i,j] ) THEN BEGIN
			PRINT, 'Test failed:', i, j, imag_in[i,j], imag_out[i,j], imag_flt[i,j]
			filter2d_failed = 1
		ENDIF
        ENDFOR
ENDFOR

IF (filter2d_failed EQ 0) THEN tests_good = tests_good + 1

PRINT, tests_good, ' OF ', tests, 'PASSED'
END
