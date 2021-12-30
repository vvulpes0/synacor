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

#undef _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L

#include "svm.h"
#include "svmreader.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

static void debug(int);
static uint16_t * initialize(struct SVM_State *);
static struct SVM_State * prog = NULL;
static void printreg(struct SVM_State *, int);
static void setreg(struct SVM_State *, int, int);
static uint16_t * reg(struct SVM_State *, int);

/*
 * This program instantiates a virtual machine and runs a supplied
 * Synacor program.
 */
int
main(int argc, char** argv)
{
	FILE * f = NULL;
	uint16_t * t = NULL;
	char buf[256];
	struct SVM_State s;
	int i;
	if (argc != 2)
	{
		fputs("usage: svm program\n", stderr);
		return 1;
	}
	f = fopen(argv[1], "rb");
	if (!f) {
		strerror_r(errno, buf, 256);
		fprintf(stderr, "%s: failed to open file \"%s\". %s\n",
		        argv[0], argv[1], buf);
		return 1;
	}
	if (!initialize(&s))
	{
		fputs(argv[0], stderr);
		fputs(": failed to allocate VM ram\n", stderr);
		return 1;
	}
	i = 0;
	while (!feof(f))
	{
		s.ram[i++] = svm_readn(f);
		if (i < s.ram_size) { continue; }
		t = realloc(s.ram, (s.ram_size << 1) * sizeof(*(s.ram)));
		if (!t)
		{
			free(s.ram);
			fputs(argv[0], stderr);
			fputs(": failed to allocate VM ram\n", stderr);
			return 1;
		}
		memset(&(t[s.ram_size]), 0,
		       s.ram_size * sizeof(*(s.ram)));
		s.ram = t;
		s.ram_size <<= 1;
	}
	fclose(f);
	prog = &s;
	if (signal(SIGUSR1, debug) == SIG_ERR)
	{
		fputs(argv[0], stderr);
		fputs(": failed to attach debug handler\n", stderr);
		fputs("continuing without debugger\n", stderr);
	}
	if (signal(SIGINT, debug) == SIG_ERR)
	{
		fputs(argv[0], stderr);
		fputs(": failed to attach debug handler\n", stderr);
		fputs("continuing without debugger\n", stderr);
	}
	i = svm_run(&s);
	/* cleanup */
	s.ram_size = 0;
	free(s.ram);
	s.ram = NULL;
	while (svm_pop(&(s.sp), &(s.regs.r0))) {;}
	return i;
}

uint16_t *
initialize(struct SVM_State * s)
{
	if (!s) { return NULL; }
	s->sp = NULL;
	s->ram_size = 256;
	s->ram = calloc(s->ram_size, sizeof(*(s->ram)));
	s->pc = s->rpc = 0;
	s->regs.r0 = s->regs.r1 = s->regs.r2 = s->regs.r3 = 0;
	s->regs.r4 = s->regs.r5 = s->regs.r6 = s->regs.r7 = 0;
	s->halted = 0;
	return s->ram;
}

void
debug(int n)
{
	char const * fmt = "%d";
	char * start = NULL;
	char * end = NULL;
	char buf[256];
	int i;
	int j;
	int x;
	int y;
	if (!prog) { return; }
	printf("\033[31m=== DEBUGGER ===\033[0m\n");
	while (!feof(stdin))
	{
		if (!fgets(buf, 256, stdin)) { continue; }
		if (feof(stdin)) { continue; }
		if (buf[0] == 'c') { break; }
		if (buf[0] == 'h') { prog->halted = 1; break; }
		switch (buf[0])
		{
		case 'd':
			start = buf + 1;
			x = strtol(start, &end, 0);
			if (end == start)
			{
				svm_da(prog, prog->rpc, 1);
				fputs("\033[1m", stdout); /* bold */
				y = svm_da(prog, prog->pc, 1);
				fputs("\033[0m", stdout); /* unbold */
				svm_da(prog, prog->pc + y, 8);
				break;
			}
			start = end;
			y = strtol(start, &end, 0);
			if (end == start) { y = 10; }
			svm_da(prog, x, y);
			break;
		case 'm':
			start = buf + 1;
			x = strtol(start, &end, 0);
			if (end == start)
			{
				printf("which address?\n");
				break;
			}
			if (x >= prog->ram_size)
			{
				printf("not in address space\n");
				break;
			}
			start = end;
			y = strtol(start, &end, 0);
			if (end == start)
			{
				printf("%04x:\t$%04x\n",
				       x, prog->ram[x]);
				break;
			}
			printf("%04x:\t$%04x -> $%04x\n",
			       x, prog->ram[x], y);
			prog->ram[x] = y;
			break;
		case 'r':
			start = buf + 1;
			x = strtol(start, &end, 0);
			if (end == start)
			{
				for (y = 0; y < 8; ++y)
				{
					printreg(prog, y);
				}
				break;
			}
			start = end;
			y = strtol(start, &end, 0);
			if (end == start)
			{
				printreg(prog, x);
				break;
			}
			setreg(prog, x, y);
			break;
		case 's':
			start = buf + 1;
			y = strtol(start, &end, 0);
			y = (y < 0) ? 0 : y;
			for (x = 0; x < y; ++x)
			{
				svm_step(prog);
			}
			break;
		default:
			break;
		}
	}
	printf("\033[31m=== CONTINUE ===\033[0m\n");
	if (feof(stdin))
	{
		prog->halted = 1;
	}
}

void
setreg(struct SVM_State * s, int i, int a)
{
	uint16_t * x = reg(s, i);
	if (!x) { return; }
	printf("r%d:\t%5d -> %5d\n", i, *x, a);
	*x = a;
	return;
}

void
printreg(struct SVM_State * s, int i)
{
	uint16_t * x = reg(s, i);
	if (!x) { return; }
	printf("r%d:\t%5d\n", i, *x);
	return;
}

uint16_t *
reg(struct SVM_State * s, int i)
{
	if (!s) { return NULL; }
	switch (i)
	{
	case 0: return &(s->regs.r0); break;
	case 1: return &(s->regs.r1); break;
	case 2: return &(s->regs.r2); break;
	case 3: return &(s->regs.r3); break;
	case 4: return &(s->regs.r4); break;
	case 5: return &(s->regs.r5); break;
	case 6: return &(s->regs.r6); break;
	case 7: return &(s->regs.r7); break;
	}
	return NULL;
}
