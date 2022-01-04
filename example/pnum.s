# _pnum(n) -> print n as a decimal value
_pnum:	jt r0, pnumx
	out $30
	ret
pnumx:	jf r0, endp
	set r1, 10
	call _divmod
	push r1
	call pnumx
	pop r1
	add r1, r1, $30
	out r1
endp:	ret
