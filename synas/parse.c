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

#include "synas.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int
parse(struct TokenList * t, struct VStream ** i, struct VStream ** a)
{
	int c = 0;
	uint16_t s = 0;
	_Bool om = 0;
	if (!i || !a) { return -1; }
	while (t)
	{
		c = 0;
		if (t->type == T_NUM)
		{
			fputs("unexpected T_NUM\n", stderr);
			return -1;
		}
		if (t->type == T_LABEL)
		{
			(*a) = calloc(1, sizeof(**a));
			(*a)->tail = NULL;
			(*a)->str = calloc(64, sizeof(char));
			strncpy((*a)->str, t->str, 63);
			(*a)->value = s;
			a = &((*a)->tail);
			t = t->tail;
			continue;
		}

		/* the instruction itself */
		(*i) = calloc(1, sizeof(**i));
		(*i)->tail = NULL;
		(*i)->str = NULL;
		(*i)->value = 0;
		if (!strncmp(t->str, "halt", 64))
		{
			(*i)->value = 0;
			c = 0;
		}
		else if (!strncmp(t->str, "set", 64))
		{
			(*i)->value = 1;
			c = 2;
		}
		else if (!strncmp(t->str, "push", 64))
		{
			(*i)->value = 2;
			c = 1;
		}
		else if (!strncmp(t->str, "pop", 64))
		{
			(*i)->value = 3;
			c = 1;
		}
		else if (!strncmp(t->str, "eq", 64))
		{
			(*i)->value = 4;
			c = 3;
		}
		else if (!strncmp(t->str, "gt", 64))
		{
			(*i)->value = 5;
			c = 3;
		}
		else if (!strncmp(t->str, "jmp", 64))
		{
			om = 1;
			(*i)->value = 6;
			c = 1;
		}
		else if (!strncmp(t->str, "jt", 64))
		{
			(*i)->value = 7;
			c = 2;
		}
		else if (!strncmp(t->str, "jf", 64))
		{
			(*i)->value = 8;
			c = 2;
		}
		else if (!strncmp(t->str, "add", 64))
		{
			(*i)->value = 9;
			c = 3;
		}
		else if (!strncmp(t->str, "mult", 64))
		{
			(*i)->value = 10;
			c = 3;
		}
		else if (!strncmp(t->str, "mod", 64))
		{
			(*i)->value = 11;
			c = 3;
		}
		else if (!strncmp(t->str, "and", 64))
		{
			(*i)->value = 12;
			c = 3;
		}
		else if (!strncmp(t->str, "or", 64))
		{
			(*i)->value = 13;
			c = 3;
		}
		else if (!strncmp(t->str, "not", 64))
		{
			(*i)->value = 14;
			c = 2;
		}
		else if (!strncmp(t->str, "rmem", 64))
		{
			(*i)->value = 15;
			c = 2;
		}
		else if (!strncmp(t->str, "wmem", 64))
		{
			om = 1;
			(*i)->value = 16;
			c = 2;
		}
		else if (!strncmp(t->str, "call", 64))
		{
			om = 1;
			(*i)->value = 17;
			c = 1;
		}
		else if (!strncmp(t->str, "ret", 64))
		{
			(*i)->value = 18;
			c = 0;
		}
		else if (!strncmp(t->str, "out", 64))
		{
			(*i)->value = 19;
			c = 1;
		}
		else if (!strncmp(t->str, "in", 64))
		{
			(*i)->value = 20;
			c = 1;
		}
		else if (!strncmp(t->str, "noop", 64))
		{
			(*i)->value = 21;
			c = 0;
		}
		else if (!strncmp(t->str, "dw", 64))
		{
			c = 0;
			t = t->tail;
			++s;
			if (!t) { return -1; }
			if (t->type == T_LABEL) { return -1; }
			if (t->type == T_NUM && t->str[0] != '$')
			{
				(*i)->value = strtol(t->str, NULL, 0);
			}
			else if (t->type == T_NUM)
			{
				(*i)->value = strtol(t->str+1, NULL, 16);
			}
			else if (t->type == T_ID)
			{
				(*i)->str = calloc(64,sizeof(char));
				strncpy((*i)->str, t->str, 63);
				(*i)->value = s;
			}
		}
		i = &((*i)->tail);
		t = t->tail;
		++s;
		if (!c) { continue; }
		if (!t) { return -1; }

		/* first argument */
		(*i) = calloc(1, sizeof(**i));
		(*i)->tail = NULL;
		(*i)->str = NULL;
		(*i)->value = strtol(t->str, NULL, 0);
		if (t->type == T_NUM && (t->str)[0] == '$')
		{
			(*i)->value = strtol((t->str)+1, NULL, 16);
		}
		(*i)->value &= 0x7FFF;
		if (t->type == T_LABEL) { return -1; }
		if (t->type == T_ID)
		{
			if (t->size == 2 && (t->str)[0] == 'r'
			    && isdigit((t->str)[1]))
			{
				(*i)->value = strtol((t->str)+1, NULL, 0);
				(*i)->value |= 32768;
			}
			else if (om)
			{
				(*i)->str = calloc(64,sizeof(char));
				strncpy((*i)->str, t->str, 63);
				(*i)->value = s;
			}
			else
			{
				return -1;
			}
		}
		i = &((*i)->tail);
		t = t->tail;
		++s;
		--c;
		if (!c) { continue; }
		if (!t) { return -1; }

		/* second argument */
		(*i) = calloc(1, sizeof(**i));
		(*i)->tail = NULL;
		(*i)->str = NULL;
		(*i)->value = strtol(t->str, NULL, 0);
		if (t->type == T_NUM && (t->str)[0] == '$')
		{
			(*i)->value = strtol((t->str)+1, NULL, 16);
		}
		(*i)->value &= 0x7FFF;
		if (t->type == T_LABEL) { return -1; }
		if (t->type == T_ID)
		{
			if (t->size == 2 && (t->str)[0] == 'r'
			    && isdigit((t->str)[1]))
			{
				(*i)->value = strtol((t->str)+1, NULL, 0);
				(*i)->value |= 32768;
			}
			else
			{
				(*i)->str = calloc(64,sizeof(char));
				strncpy((*i)->str, t->str, 63);
				(*i)->value = s;
			}
		}
		i = &((*i)->tail);
		t = t->tail;
		++s;
		--c;
		if (!c) { continue; }
		if (!t) { return -1; }

		/* third argument */
		(*i) = calloc(1, sizeof(**i));
		(*i)->tail = NULL;
		(*i)->str = NULL;
		(*i)->value = strtol(t->str, NULL, 0);
		if (t->type == T_NUM && (t->str)[0] == '$')
		{
			(*i)->value = strtol((t->str)+1, NULL, 16);
		}
		(*i)->value &= 0x7FFF;
		if (t->type == T_LABEL) { return -1; }
		if (t->type == T_ID)
		{
			if (t->size == 2 && (t->str)[0] == 'r'
			    && isdigit((t->str)[1]))
			{
				(*i)->value = strtol((t->str)+1, NULL, 0);
				(*i)->value |= 32768;
			}
			else
			{
				(*i)->str = calloc(64,sizeof(char));
				strncpy((*i)->str, t->str, 63);
				(*i)->value = s;
			}
		}
		i = &((*i)->tail);
		t = t->tail;
		++s;
	}
	return s;
}
