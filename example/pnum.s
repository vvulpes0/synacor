_pnum:
	set r2, 0
L5:	gt  r1, r0,   9999
	jf  r1, L5p
	add r0, r0, -10000
	add r2, r2, 1
	jmp L5
L5p:	add r2, r2, 48
	out r2
	set r2, 0
L4:	gt  r1, r0,   999
	jf  r1, L4p
	add r0, r0, -1000
	add r2, r2, 1
	jmp L4
L4p:	add r2, r2, 48
	out r2
	set r2, 0
L3:	gt  r1, r0,   99
	jf  r1, L3p
	add r0, r0, -100
	add r2, r2, 1
	jmp L3
L3p:	add r2, r2, 48
	out r2
	set r2, 0
L2:	gt  r1, r0,   9
	jf  r1, L2p
	add r0, r0, -10
	add r2, r2, 1
	jmp L2
L2p:	add r2, r2, 48
	out r2
	set r2, 0
L1:	gt  r1, r0,  0
	jf  r1, L1p
	add r0, r0, -1
	add r2, r2, 1
	jmp L1
L1p:	add r2, r2, 48
	out r2
	ret
