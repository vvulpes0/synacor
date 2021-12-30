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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * I could in theory use a trie here,
 * but I'm lazy and this is easier.
 */
struct SymbolTable
{
	struct SymbolTable * tail;
	char * symbol;
	uint16_t value;
};

struct IStream
{
	struct IStream * tail;
	uint16_t value;
};

static void clean(struct SymbolTable *, struct SymbolTable *,
                  struct IStream *);
static uint16_t read16(FILE *);
static void write16(FILE *, uint16_t);
static int lookup(struct SymbolTable *, char *);
static int parse(FILE *, struct SymbolTable **, struct SymbolTable **,
                 struct IStream **, uint16_t);

int
main(int argc, char** argv)
{
	struct SymbolTable * g = NULL;
	struct SymbolTable * u = NULL;;
	struct IStream * o = NULL;
	struct SymbolTable * ut = NULL;
	struct IStream * ot = NULL;
	char * name = "a.out";
	FILE * fi = NULL;
	FILE * fo = NULL;
	int c = 0;
	int i = 0;
	int x = 0;
	while ((c = getopt(argc, argv, "o:")) != -1)
	{
		switch (c)
		{
		case 'o':
			name = optarg;
			break;
		default:
			printf("usage: synld [-o out_file] file...\n");
			return 1;
		}
	}
	argc -= optind;
	argv += optind;
	u = malloc(sizeof(*u));
	if (!u) { return 1; }
	o = calloc(1, sizeof(*o));
	if (!o) { free(u); return 1; }
	u->tail = NULL;
	u->symbol = calloc(6, sizeof(char));
	strncpy(u->symbol, "_main", 6);
	u->value = 1;
	o->tail = calloc(1, sizeof(*(o->tail)));
	o->value = 6;
	o->tail->value = -1;
	ut = u;
	ot = o;
	c = 2;
	for (i = 0; i < argc; ++i)
	{
		fi = fopen(argv[i], "rb");
		if (!fi) { clean(g,u,o); return 1; }
		while (ut->tail) { ut = ut->tail; }
		while (ot->tail) { ot = ot->tail; }
		x = parse(fi, &g, &(ut->tail), &(ot->tail), c);
		if (x < 0) { clean(g,u,o); return 1; }
		c += x;
		fclose(fi);
	}

	i = 0;
	ut = u;
	ot = o;
	while (ot)
	{
		c = ot->value;
		if (ut && i == ut->value)
		{
			c = lookup(g, ut->symbol);
			if (c < 0)
			{
				fprintf(stderr, "unknown symbol: %s\n",
				        ut->symbol);
				return 1;
			}
			ut = ut->tail;
		}
		ot->value = c;
		ot = ot->tail;
		++i;
	}
	fo = fopen(name, "wb");
	if (!fo)
	{
		/* cleanup and get out */
		fprintf(stderr, "failed to open file %s.", name);
		clean(g,u,o);
		return 1;
	}
	ot = o;
	while (ot)
	{
		write16(fo, ot->value);
		ot = ot->tail;
	}
	fclose(fo);
	clean(g,u,o);
	return 0;
}

int
parse(FILE * f, struct SymbolTable ** s, struct SymbolTable ** u,
      struct IStream ** o, uint16_t b)
{
	struct SymbolTable * p;
	char buf[9];
	uint16_t * brels;
	int r;
	int c;
	int i;
	int j;
	int x;
	if (!f || !s || !o) { return -1; }

	/* file magic */
	fgets(buf, 9, f);
	if (strncmp(buf, "=SYN10>\n", 8)) { return -1; }
	if (feof(f)) { return -1; }
	*o = calloc(1, sizeof(**o));

	/* base relocation list */
	r = read16(f);
	if (feof(f)) { return -1; }
	brels = calloc(r, sizeof(*brels));
	for (i = 0; i < r; ++i)
	{
		brels[i] = read16(f);
		if (feof(f)) { free(brels); return -1; }
	}

	/* exported symbols */
	x = read16(f);
	for (i = 0; i < x; ++i)
	{
		p = malloc(sizeof(*p));
		if (!p) { return -1; }
		p->tail = (*s);
		p->value = read16(f) + b;
		p->symbol = calloc(64,sizeof(char));
		*s = p;
		if (!p->symbol) { return -1; }
		c = -1;
		for (j = 0; j < 63 && c != 0 && !feof(f); ++j)
		{
			c = fgetc(f);
			(p->symbol)[j] = c;
		}
		if (feof(f)) { return -1; }
		if (lookup(p->tail, p->symbol) >= 0)
		{
			fprintf(stderr, "duplicate symbol: %s\n",
			        p->symbol);
			return -1;
		}
	}

	/* unknown symbols */
	x = read16(f);
	for (i = 0; i < x; ++i)
	{
		*u = malloc(sizeof(**u));
		if (!*u) { return -1; }
		(*u)->value = read16(f) + b;
		(*u)->symbol = calloc(64,sizeof(char));
		if (!(*u)->symbol) { return -1; }
		c = -1;
		for (j = 0; j < 63 && c != 0 && !feof(f); ++j)
		{
			c = fgetc(f);
			((*u)->symbol)[j] = c;
		}
		if (feof(f)) { return -1; }
		u = &((*u)->tail);
	}

	/* instruction stream, account for brels */
	i = j = 0;
	while (!feof(f))
	{
		x = read16(f);
		if (feof(f)) { break; }
		if (j < r && i == brels[j])
		{
			++j;
			x += b;
		}
		*o = calloc(1, sizeof(**o));
		if (!*o) { return -1; }
		(*o)->value = x;
		++i;
		o = &((*o)->tail);
	}

	free(brels);
	return i;
}

int
lookup(struct SymbolTable * s, char * x)
{
	while (s)
	{
		if (!strncmp(s->symbol, x, 64)) { return s->value; }
		s = s->tail;
	}
	return -1;
}

uint16_t
read16(FILE * f)
{
	int x = fgetc(f);
	x |= fgetc(f) << 8;
	return (uint16_t)(x);
}

void
write16(FILE * f, uint16_t x)
{
	fputc(x & 0xFF, f);
	fputc((x>>8) & 0xFF, f);
}

void
clean(struct SymbolTable * g, struct SymbolTable * u, struct IStream * o)
{
	void * t;
	while (g)
	{
		t = g->tail;
		free(g->symbol);
		g->symbol = NULL;
		free(g);
		g = (struct SymbolTable *) t;
	}
	while (u)
	{
		t = u->tail;
		free(u->symbol);
		u->symbol = NULL;
		free(u);
		u = (struct SymbolTable *) t;
	}
	while (o)
	{
		t = o->tail;
		free(o);
		o = (struct IStream *) t;
	}
}
