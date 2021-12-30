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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define RL(x) ((x) > 32767 ? "r%d" : "$%04x")
#define RLS(x) ((x) > 32767 ? ", r%d" : ", $%04x")
#define PRI(x) printf(RL(x), (x) % 32768)
#define PRIS(x) printf(RLS(x), (x) % 32768)

int
svm_da(struct SVM_State * s, size_t i, int n)
{
	size_t j = i = (i < 0) ? 0 : i;
	int m = 0;
	uint16_t a;
	if (!s) { return 0; }
	while ((m < n || n < 0) && (i < s->ram_size))
	{
		++m;
		printf("%04x\t", (uint16_t)(i));
		switch (s->ram[i++])
		{
		case SVM_HALT:
			fputs("halt", stdout);
			break;
		case SVM_SET:
			fputs("set\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_PUSH:
			fputs("push\t", stdout);
			a = s->ram[i++]; PRI(a);
			break;
		case SVM_POP:
			fputs("pop\t", stdout);
			a = s->ram[i++]; PRI(a);
			break;
		case SVM_EQ:
			fputs("eq\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_GT:
			fputs("gt\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_JMP:
			fputs("jmp\t", stdout);
			a = s->ram[i++]; PRI(a);
			break;
		case SVM_JT:
			fputs("jt\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_JF:
			fputs("jf\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_ADD:
			fputs("add\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_MULT:
			fputs("mult\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_MOD:
			fputs("mod\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_AND:
			fputs("and\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_OR:
			fputs("or\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_NOT:
			fputs("not\t", stdout);
			a = s->ram[i++]; PRI(a);
			a = s->ram[i++]; PRIS(a);
			break;
		case SVM_RMEM:
			fputs("rmem\t", stdout);
			a = s->ram[i++]; PRI(a);
			fputs(", [", stdout);
			a = s->ram[i++]; PRI(a);
			fputs("]", stdout);
			break;
		case SVM_WMEM:
			fputs("wmem\t[", stdout);
			a = s->ram[i++]; PRI(a);
			fputs("], ", stdout);
			a = s->ram[i++]; PRI(a);
			break;
		case SVM_CALL:
			fputs("call\t", stdout);
			a = s->ram[i++]; PRI(a);
			break;
		case SVM_RET:
			fputs("ret", stdout);
			break;
		case SVM_OUT:
			fputs("out\t", stdout);
			a = s->ram[i++]; PRI(a);
			break;
		case SVM_IN:
			fputs("in\t", stdout);
			a = s->ram[i++]; PRI(a);
			break;
		case SVM_NOOP:
			fputs("noop", stdout);
			break;
		default:
			fprintf(stdout, "illegal\t%02x, %02x",
			        s->ram[i-1] % 256,
			        (s->ram[i-1] / 256) % 256);
			break;
		}
		fputs("\n", stdout);
	}
	return (int)(i - j);
}
