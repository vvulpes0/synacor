/*-
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2021 Dakotah Lambert
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the right to use, copy, modifiy, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "svm.h"

#include <stdlib.h>

static _Bool reallocate_ram(struct SVM_State *, size_t);
static uint16_t * reg(struct SVM_State *, uint16_t);
static uint16_t reglit(struct SVM_State *, uint16_t);

int
svm_step(struct SVM_State * s)
{
	struct SVM_Stack * q = NULL;
	uint16_t * t = NULL;
	uint16_t i = 0;
	uint16_t a = 0;
	uint16_t b = 0;
	uint16_t c = 0;
	if (!s) { return SVM_NULL; }
	reallocate_ram(s, s->pc);
	s->rpc = s->pc;
	i = s->ram[(s->pc)++];
	switch (i)
	{
	case SVM_HALT:
		s->halted = 1;
		break;
	case SVM_SET:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, reglit(s, b));
		break;
	case SVM_PUSH:
		a = s->ram[(s->pc)++];
		q = svm_push(s->sp, reglit(s, a));
		if (!q) { return SVM_NULL; }
		s->sp = q;
		break;
	case SVM_POP:
		a = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		if (!svm_pop(&(s->sp), t)) { return SVM_EMPTY; }
		break;
	case SVM_EQ:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, reglit(s, b) == reglit(s, c) ? 1 : 0);
		break;
	case SVM_GT:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, reglit(s, b) > reglit(s, c) ? 1 : 0);
		break;
	case SVM_JMP:
		a = s->ram[(s->pc)++];
		s->pc = reglit(s, a);
		break;
	case SVM_JT:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		if (reglit(s, a)) { s->pc = reglit(s, b); }
		break;
	case SVM_JF:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		if (!reglit(s, a)) { s->pc = reglit(s, b); }
		break;
	case SVM_ADD:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, (reglit(s, b) + reglit(s, c)) % 32768);
		break;
	case SVM_MULT:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, (reglit(s, b) * reglit(s, c)) % 32768);
		break;
	case SVM_MOD:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, reglit(s, b) % reglit(s, c));
		break;
	case SVM_AND:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, reglit(s, b) & reglit(s, c));
		break;
	case SVM_OR:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, reglit(s, b) | reglit(s, c));
		break;
	case SVM_NOT:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_set(t, (~reglit(s, b)) & 0x7FFF);
		break;
	case SVM_RMEM:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		c = reglit(s, b);
		reallocate_ram(s, c);
		svm_set(t, (s->ram)[c]);
		break;
	case SVM_WMEM:
		a = s->ram[(s->pc)++];
		b = s->ram[(s->pc)++];
		c = reglit(s, a);
		reallocate_ram(s, c);
		svm_set(&((s->ram)[c]), reglit(s, b));
		break;
	case SVM_CALL:
		a = s->ram[(s->pc)++];
		q = svm_push(s->sp, (uint16_t)(s->pc));
		if (!q) { return SVM_NULL; }
		s->sp = q;
		s->pc = reglit(s, a);
		break;
	case SVM_RET:
		if (!svm_pop(&(s->sp), &a)) { return SVM_EMPTY; }
		s->pc = a;
		break;
	case SVM_OUT:
		a = s->ram[(s->pc)++];
		svm_out(reglit(s, a));
		break;
	case SVM_IN:
		a = s->ram[(s->pc)++];
		t = reg(s, a);
		if (!t) { return SVM_NONREG; }
		svm_in(t);
		break;
	case SVM_NOOP:
		break;
	default:
		return SVM_UNIMPLEMENTED;
	}
	return 0;
}

_Bool
reallocate_ram(struct SVM_State * s, size_t i)
{
	uint16_t * t;
	size_t x;
	if (!s) { return 0; }
	if (i < s->ram_size) { return 1; }
	if (i && !(i & (i - 1)))
	{
		t = realloc(s->ram, i * 2 * sizeof(*(s->ram)));
		if (!t) { return 0; }
		s->ram = t;
		s->ram_size = i * 2;
		return 1;
	}
	x = 1;
	while (x < i) { x <<= 1; }
	t = realloc(s->ram, x * sizeof(*(s->ram)));
	if (!t) { return 0; }
	s->ram = t;
	s->ram_size = x;
	return 1;
}

uint16_t *
reg(struct SVM_State * s, uint16_t i)
{
	if (i < 32768) { return NULL; }
	if (!s) { return NULL; }
	switch (i & 32767)
	{
	case 0: return &(s->regs.r0);
	case 1: return &(s->regs.r1);
	case 2: return &(s->regs.r2);
	case 3: return &(s->regs.r3);
	case 4: return &(s->regs.r4);
	case 5: return &(s->regs.r5);
	case 6: return &(s->regs.r6);
	case 7: return &(s->regs.r7);
	}
	return NULL;
}

uint16_t
reglit(struct SVM_State * s, uint16_t i)
{
	uint16_t * x;
	if (i < 32768) { return i; }
	if (!s) { return 0; }
	x = reg(s, i);
	if (!x) { return 0; }
	return *x;
}
