FUNCTION test_echo

in = 10
out = 20

;---------------------------------------------------------

out = MIA_ECHO(in)

;---------------------------------------------------------

IF ( in NE out ) THEN RETURN, 0

RETURN, 1

END

