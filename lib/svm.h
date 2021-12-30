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

#ifndef SVM_H
#define SVM_H

#include <stddef.h>
#include <stdint.h>

enum SVM_Error
{
	SVM_UNIMPLEMENTED = 3,
	SVM_NULL = 4,
	SVM_NONREG = 5,
	SVM_EMPTY = 6,
};

enum SVM_INSNS
{
	SVM_HALT =  0,
	SVM_SET  =  1,
	SVM_PUSH =  2,
	SVM_POP  =  3,
	SVM_EQ   =  4,
	SVM_GT   =  5,
	SVM_JMP  =  6,
	SVM_JT   =  7,
	SVM_JF   =  8,
	SVM_ADD  =  9,
	SVM_MULT = 10,
	SVM_MOD  = 11,
	SVM_AND  = 12,
	SVM_OR   = 13,
	SVM_NOT  = 14,
	SVM_RMEM = 15,
	SVM_WMEM = 16,
	SVM_CALL = 17,
	SVM_RET  = 18,
	SVM_OUT  = 19,
	SVM_IN   = 20,
	SVM_NOOP = 21,
};

struct SVM_RegBank
{
	uint16_t r0;
	uint16_t r1;
	uint16_t r2;
	uint16_t r3;
	uint16_t r4;
	uint16_t r5;
	uint16_t r6;
	uint16_t r7;
};

struct SVM_Stack
{
	struct SVM_Stack * tail;
	uint16_t head;
};

struct SVM_State
{
	struct SVM_Stack * sp;
	uint16_t * ram;
	size_t ram_size;
	size_t pc;
	size_t rpc;
	struct SVM_RegBank regs;
	_Bool halted;
};

int svm_da(struct SVM_State *, size_t, int);
int svm_run(struct SVM_State *);
int svm_step(struct SVM_State *);
/*  0 */ /* HALT */
/*  1 */ void svm_set(uint16_t * const, uint16_t);
/*  2 */ struct SVM_Stack * svm_push(struct SVM_Stack *, uint16_t);
/*  3 */ _Bool svm_pop(struct SVM_Stack **, uint16_t *);
/* 19 */ void svm_out(uint16_t);
/* 20 */ void svm_in(uint16_t *);
/* 21 */ /* NOOP */

#endif
