_main:
	set r0, 6
	call _factorial
	call _pnum
	set r0, 0
	out 10
	halt
