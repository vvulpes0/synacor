_factorial:
	set r1, r0
LF:	add r1, r1, -1
	jf  r1, LFe
	mult r0, r1, r0
	jmp LF
LFe:	eq  r1, r0, 0
	add r0, r0, r1
	ret
