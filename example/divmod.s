# _divmod(dividend, divisor) -> (quotient, remainder)
_divmod:
	push r3
	push r4
	push r5
	jf r0 end
	set r2, 0
	set r3, 15
	set r5, 0
loop:	jf r3, end
	gt  r4, r0, $3FFF
	add r2, r2, r2
	add r2, r2, r4
	add r0, r0, r0
	add r0, r0, r5
	not r4, r1
	add r4, r4, 1
	add r4, r2, r4
	gt  r5, $4000, r4
	jf  r5, endl
	set r2, r4
endl:	add r3, r3, -1
	jmp loop
end:	gt r4, r0, $3FFF
	add r1, r2, r4
	add r0, r0, r0
	add r0, r0, r5
	pop r5
	pop r4
	pop r3
	ret
