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

#undef _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

static void output_baserel(FILE *, struct VStream *, struct VStream *);
static void output_exported(FILE *, struct VStream *);
static void output_unknown(FILE *, struct VStream *, struct VStream *);
static void output_insns(FILE *, struct VStream *);
static void write16(FILE *, unsigned int);

int
main(int argc, char** argv)
{
	struct TokenList * t = NULL;
	void * p = NULL;
	struct VStream * i = NULL;
	struct VStream * a = NULL;
	FILE * fi = stdin;
	FILE * fo = stdout;
	char buf[256];
	char * outfile = NULL;
	char * name = argv[0];
	int c;
	while ((c = getopt(argc, argv, "o:")) != -1)
	{
		switch (c)
		{
		case 'o':
			outfile = optarg;
			break;
		default:
			fputs("usage: ",stderr);
			fputs("synas [-o outfile] [infile]\n", stderr);
			fputs("outfile defaults to a.out\n", stderr);
			fputs("infile defaults to stdin\n", stderr);
			if (fo != stdout) { fclose(fo); }
			return 1;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 1)
	{
		fputs("usage: ",stderr);
		fputs("synas [-o outfile] [infile]\n", stderr);
		fputs("outfile defaults to a.out\n", stderr);
		fputs("infile defaults to stdin\n", stderr);
		return 1;
	}
	if (argc == 1)
	{
		fi = fopen(argv[0], "r");
		if (!fi)
		{
			fputs(name, stderr);
			fprintf(stderr, ": failed to open file ");
			fprintf(stderr, "%s. ", optarg);
			strerror_r(errno, buf, 256);
			fputs(buf, stderr);
			fputs(".\n", stderr);
			return 1;
		}
	}
	t = tokenize(fi);
	if (parse(t, &i, &a) < 0)
	{
		fputs("syntax error\n", stderr);
		return 1;
	}
	fo = fopen(outfile ? outfile : "a.out", "wb");
	if (!fo)
	{
		fputs(name, stderr);
		fprintf(stderr, ": failed to open file ");
		fprintf(stderr, "%s. ", outfile ? outfile : "a.out");
		strerror_r(errno, buf, 256);
		fputs(buf, stderr);
		fputs(".\n", stderr);
		if (fi != stdin) { fclose(fi); }
		return 1;
	}
	while (t)
	{
		p = t->tail;
		free(t->str);
		t->str = NULL;
		free(t);
		t = (struct TokenList *)p;
	}
	fputs("=SYN10>\n",fo);
	output_baserel(fo, i, a);
	output_exported(fo, a);
	output_unknown(fo, i, a);
	while (a)
	{
		free(a->str);
		a->str = NULL;
		p = a->tail;
		free(a);
		a = (struct VStream *)p;
	}
	output_insns(fo, i);
	if (fo != stdout) { fclose(fo); }
	while (i)
	{
		free(i->str);
		i->str = NULL;
		p = i->tail;
		free(i);
		i = (struct VStream *)p;
	}
	return 0;
}

void
write16(FILE * f, unsigned int n)
{
	fputc(n & 0xFF, f);
	fputc((n>>8) & 0xFF, f);
}

void
output_baserel(FILE * f, struct VStream * i, struct VStream * a)
{
	struct VStream * p = i;
	struct VStream * q;
	int n = 0;
	while (p)
	{
		if (!p->str) { p = p->tail; continue; }
		q = a;
		while (q)
		{
			if (!strncmp(p->str, q->str, 64))
			{
				++n;
				break;
			}
			q = q->tail;
		}
		p = p->tail;
	}
	write16(f, n);
	while (i)
	{
		if (!i->str) { i = i->tail; continue; }
		q = a;
		while (q)
		{
			if (!strncmp(i->str, q->str, 64))
			{
				write16(f, i->value);
				i->value = q->value;
				break;
			}
			q = q->tail;
		}
		i = i->tail;
	}
}

void
output_exported(FILE * f, struct VStream * a)
{
	struct VStream * p = a;
	int n = 0;
	while (p)
	{
		if (!p->str) { p = p->tail; continue; }
		if ((p->str)[0] == '_') { ++n; }
		p = p->tail;
	}
	write16(f, n);
	while (a)
	{
		if (!a->str) { a = a->tail; continue; }
		if ((a->str)[0] == '_')
		{
			write16(f, a->value);
			fprintf(f, "%.64s", a->str);
			fputc(0, f);
		}
		a = a->tail;
	}
}

void
output_unknown(FILE * f, struct VStream * i, struct VStream * a)
{
	struct VStream * p = i;
	struct VStream * q;
	int n = 0;
	while (p)
	{
		if (!p->str) { p = p->tail; continue; }
		q = a;
		while (q)
		{
			if (!strncmp(p->str, q->str, 64))
			{
				break;
			}
			q = q->tail;
		}
		if (!q) { ++n; }
		p = p->tail;
	}
	write16(f, n);
	while (i)
	{
		if (!i->str) { i = i->tail; continue; }
		q = a;
		while (q)
		{
			if (!strncmp(i->str, q->str, 64))
			{
				break;
			}
			q = q->tail;
		}
		if (!q)
		{
			write16(f, i->value);
			fprintf(f, "%.64s", i->str);
			fputc(0, f);
			i->value = -1;
		}
		i = i->tail;
	}
}

void
output_insns(FILE * f, struct VStream * i)
{
	while (i)
	{
		write16(f, i->value);
		i = i->tail;
	}
}
