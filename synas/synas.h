/*-
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2021 Dakotah Lambert
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
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

#include <stdint.h>
#include <stdio.h>

enum ParseState
{
	P_BEGIN,
	P_DEC,
	P_HEX,
	P_WORD,
	P_COMMENT,
	P_ERROR,
};

enum TokenType
{
	T_ID,
	T_LABEL,
	T_NUM,
};

struct TokenList
{
	struct TokenList * tail;
	char * str;
	size_t size;
	enum TokenType type;
};

struct VStream
{
	struct VStream * tail;
	char * str;
	uint16_t value;
};

struct TokenList * tokenize(FILE *);
int parse(struct TokenList *, struct VStream **, struct VStream **);
