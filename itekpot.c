/* $Id: tekpot.c,v 1.3 2010/01/31 08:30:19 demon Exp $ */
/*
 * Copyright (c) 2009 Dimitri Sokolyuk <demon@dim13.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef	struct point Point;
typedef	struct patch Patch;

struct	point {
	float	x, y, z;
} *point, *ppoint, max;

struct	patch {
	int	p[16];
} *patch, *ppatch;

int	tekheight = 3072;
int	tekwidth = 4096;
int	npoints = 100;

void
loadpatch(char *filename, int *patches, int *verticles)
{
	int	i, k;
	float	x, y, z;
	int	a, b, c, d;
	FILE	*fd;

	fd = fopen(filename, "r");
	if (!fd)
		err(1, "can't open %s", filename);

	fscanf(fd, "%i\n", patches);

	patch = calloc(*patches, sizeof(Patch));
	if (!patch)
		err(1, "can't allocate memory");
	ppatch = patch;

	for (i = 0; i < *patches; i++) {
		k = 0;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[k++] = --a;
		ppatch->p[k++] = --b;
		ppatch->p[k++] = --c;
		ppatch->p[k++] = --d;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[k++] = --a;
		ppatch->p[k++] = --b;
		ppatch->p[k++] = --c;
		ppatch->p[k++] = --d;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[k++] = --a;
		ppatch->p[k++] = --b;
		ppatch->p[k++] = --c;
		ppatch->p[k++] = --d;
		fscanf(fd, "%i, %i, %i, %i\n", &a, &b, &c, &d);
		ppatch->p[k++] = --a;
		ppatch->p[k++] = --b;
		ppatch->p[k++] = --c;
		ppatch->p[k++] = --d;
		++ppatch;
	}

	fscanf(fd, "%i\n", verticles);

	point = calloc(*verticles, sizeof(Point));
	if (!point)
		err(1, "can't allocate memory");
	ppoint = point;

	max.x = 0;
	max.y = 0;
	max.z = 0;

	for (i = 0; i < *verticles; i++) {
		fscanf(fd, "%f, %f, %f\n", &x, &y, &z);
		ppoint->x = x;
		if (abs(x) > max.x)
			max.x = abs(x);
		ppoint->y = y;
		if (abs(y) > max.y)
			max.y = abs(y);
		ppoint->z = z;
		if (abs(z) > max.z)
			max.z = abs(z);
		++ppoint;
	}

	fclose(fd);
}

int
rnd(float f)
{
	if (f > 0)
		f += 0.5;
	else
		f -= 0.5;

	return (int)f;
}

void
rotx(Point *p)
{
	p->y = 0.5 * p->y + 0.866 * p->z;
	p->z = -0.866 * p->y + 0.5 * p->z;
}

void
project(Point *p)
{
	float	d = 100 * max.z;
	float	zoom = 1000;

	rotx(p);

	p->x *= d/(2*d - p->z);
	p->y *= d/(2*d - p->z);

	p->x *= zoom;
	p->y *= zoom;

	p->x += tekwidth/2;
	p->y += tekheight/3;
}

void
vec(Point *a, Point *b, float lambda)
{
	a->x += lambda*(b->x - a->x);
	a->y += lambda*(b->y - a->y);
	a->z += lambda*(b->z - a->z);
}

#if 0
void
bernstein(int steps)
{
	int uinc = 1.0/steps;
	int u = uinc;

	for (i = 1; i < steps; i++, u += uinc) {
		u_sqr = u * u;			/* u^2 */
		tmp = 1.0 - u;			/* (1-u) */
		tmp_sqr = tmp * tmp;		/* (1-u)^2 */
		b[0][i] = tmp * tmp_sqr;	/* (1-u)^3 */
		b[1][i] = 3 * u * tmp_sqr;	/* 3u(1-u)^2 */
		b[2][i] = 3 * u_sqr * tmp;	/* 3u^2(1-u) */
		b[2][i] = u * u_sqr;		/* u^3 */
	}
}
#endif

void
bezier(Patch *pp, int steps)
{
	Point	p[16];
	int	i, j, k, step;
	float	s;

	for (step = 0; step < steps; step++) {
		for (i = 0; i < 16; i++) {
			k = pp->p[i];
			p[i].x = point[k].x;
			p[i].y = point[k].y;
			p[i].z = point[k].z;
		}

		s = (float)step/(float)steps;
		for (i = 15; i > 0; i--)
			for (j = 0; j < i; j++)
				vec(&p[j], &p[j + 1], s);	

		project(p);
		iplot(!!step, rnd(p->x), rnd(p->y));
	}
}

void
usage()
{
	extern	char *__progname;

	fprintf(stderr, "usage: %s datafile\n", __progname);

	exit(1);
}

int
main(int argc, char **argv)
{
	int	patches, verticles;
	int	i, j;

	if (argc != 2)
		usage();

	loadpatch(*++argv, &patches, &verticles);

	inittek();
	page();
	for (i = 0; i < patches; i++)
		bezier(&patch[i], npoints);
	endtek();

	return 0;
}
