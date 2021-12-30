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

#include "synas.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

struct TokenList *
tokenize(FILE * f)
{
	struct TokenList * t = NULL;
	struct TokenList ** a = NULL;
	struct TokenList ** r = &t;
	enum ParseState p = P_BEGIN;
	int c;
	do
	{
		c = fgetc(f);
		if (c == EOF) { break; }
		switch (p)
		{
		case P_BEGIN:
			if (isspace(c) || c == '[') { continue; }
			if (isdigit(c) || c == '$' || c == '-')
			{
				p = (c == '$') ? P_HEX : P_DEC;
				(*r) = calloc(1, sizeof(**r));
				(*r)->str = calloc(64, sizeof(char));
				((*r)->str)[((*r)->size)++] = c;
				(*r)->type = T_NUM;
				continue;
			}
			if (isalpha(c) || c == '_')
			{
				p = P_WORD;
				(*r) = calloc(1, sizeof(**r));
				(*r)->str = calloc(64, sizeof(char));
				((*r)->str)[((*r)->size)++] = c;
				(*r)->type = T_ID;
				continue;
			}
			if (c == ':' && a && (*a))
			{
				(*a)->type = T_LABEL;
				continue;
			}
			if (c == '#') { p = P_COMMENT; continue; }
			p = P_ERROR;
			break;
		case P_HEX:
			if (('a' <= c && c <= 'f')
			    || ('A' <= c && c <= 'F'))
			{
				((*r)->str)[((*r)->size)++] = c;
				continue;
			}
			/* FALLTHROUGH */
		case P_DEC:
			if (isdigit(c))
			{
				((*r)->str)[((*r)->size)++] = c;
				continue;
			}
			if (isspace(c) || c == ',' || c == ']')
			{
				a = r;
				r = &((*r)->tail);
				p = P_BEGIN;
				continue;
			}
			if (c == '#')
			{
				a = r;
				r = &((*r)->tail);
				p = P_COMMENT;
				continue;
			}
			p = P_ERROR;
			break;
		case P_WORD:
			if (isspace(c) || c == ',' || c == ']')
			{
				a = r;
				r = &((*r)->tail);
				p = P_BEGIN;
				continue;
			}
			if (c == ':')
			{
				(*r)->type = T_LABEL;
				a = r;
				r = &((*r)->tail);
				p = P_BEGIN;
				continue;
			}
			if (isalnum(c) || c == '_')
			{
				((*r)->str)[((*r)->size)++] = c;
				continue;
			}
			if (c == '#')
			{
				a = r;
				r = &((*r)->tail);
				p = P_COMMENT;
				continue;
			}
			p = P_ERROR;
			break;
		case P_COMMENT:
			if (c == '\n') { p = P_BEGIN; }
			break;
		default:
			break;
		}
	} while (c != EOF && p != P_ERROR);
	return t;
}
