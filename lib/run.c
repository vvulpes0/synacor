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

int
svm_run(struct SVM_State * s)
{
	int r = 0;
	if (!s) { return SVM_NULL; }
	while (!s->halted && !r) { r = svm_step(s); }
	return r;
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
