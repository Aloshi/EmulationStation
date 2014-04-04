/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The polygon rasterization is heavily based on stb_truetype rasterizer
 * by Sean Barrett - http://nothings.org/
 *
 */

#ifndef NANOSVGRAST_H
#define NANOSVGRAST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Example Usage:
	// Load SVG
	struct SNVGImage* image = nsvgParseFromFile("test.svg.");

	// Create rasterizer (can be used to render multiple images).
	struct NSVGrasterizer* rast = nsvgCreateRasterizer();
	// Allocate memory for image
	unsigned char* img = malloc(w*h*4);
	// Rasterize
	nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);
*/

// Allocated rasterizer context.
struct NSVGrasterizer* nsvgCreateRasterizer();

// Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
//   r - pointer to rasterizer context
//   image - pointer to image to rasterize
//   tx,ty - image offset (applied after scaling)
//   scale - image scale
//   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
//   w - width of the image to render
//   h - height of the image to render
//   stride - number of bytes per scaleline in the destination buffer
void nsvgRasterize(struct NSVGrasterizer* r,
				   struct NSVGimage* image, float tx, float ty, float scale,
				   unsigned char* dst, int w, int h, int stride);

// Deletes rasterizer context.
void nsvgDeleteRasterizer(struct NSVGrasterizer*);


#ifdef __cplusplus
};
#endif

#endif // NANOSVGRAST_H

#ifdef NANOSVGRAST_IMPLEMENTATION

#include <math.h>

#define NSVG__SUBSAMPLES	5
#define NSVG__FIXSHIFT		10
#define NSVG__FIX			(1 << NSVG__FIXSHIFT)
#define NSVG__FIXMASK		(NSVG__FIX-1)
#define NSVG__MEMPAGE_SIZE	1024

struct NSVGedge {
   float x0,y0, x1,y1;
   int dir;
   struct NSVGedge* next;
};

struct NSVGactiveEdge {
	int x,dx;
	float ey;
	int dir;
	struct NSVGactiveEdge *next;
};

struct NSVGmemPage {
	unsigned char mem[NSVG__MEMPAGE_SIZE];
	int size;
	struct NSVGmemPage* next;
};

struct NSVGcachedPaint {
	char type;
	char spread;
	float xform[6];
	unsigned int colors[256];
};

struct NSVGrasterizer
{
	float px, py;

	struct NSVGedge* edges;
	int nedges;
	int cedges;

	struct NSVGactiveEdge* freelist;
	struct NSVGmemPage* pages;
	struct NSVGmemPage* curpage;

	unsigned char* scanline;
	int cscanline;

	unsigned char* bitmap;
	int width, height, stride;
};

struct NSVGrasterizer* nsvgCreateRasterizer()
{
	struct NSVGrasterizer* r = (struct NSVGrasterizer*)malloc(sizeof(struct NSVGrasterizer));
	if (r == NULL) goto error;
	memset(r, 0, sizeof(struct NSVGrasterizer));

	return r;

error:
	nsvgDeleteRasterizer(r);
	return NULL;
}

void nsvgDeleteRasterizer(struct NSVGrasterizer* r)
{
	struct NSVGmemPage* p;

	if (r == NULL) return;

	p = r->pages;
	while (p != NULL) {
		struct NSVGmemPage* next = p->next;
		free(p);
		p = next;
	}

	if (r->edges) free(r->edges);
	if (r->scanline) free(r->scanline);

	free(r);
}

static struct NSVGmemPage* nsvg__nextPage(struct NSVGrasterizer* r, struct NSVGmemPage* cur)
{
	struct NSVGmemPage *newp;

	// If using existing chain, return the next page in chain
	if (cur != NULL && cur->next != NULL) {
		return cur->next;
	}
	
	// Alloc new page
	newp = (struct NSVGmemPage*)malloc(sizeof(struct NSVGmemPage));
	if (newp == NULL) return NULL;
	memset(newp, 0, sizeof(struct NSVGmemPage));
	
	// Add to linked list
	if (cur != NULL)
		cur->next = newp;
	else
		r->pages = newp;

	return newp;
}

static void nsvg__resetPool(struct NSVGrasterizer* r)
{
	struct NSVGmemPage* p = r->pages;
	while (p != NULL) {
		p->size = 0;
		p = p->next;
	}
	r->curpage = r->pages;
}

static unsigned char* nsvg__alloc(struct NSVGrasterizer* r, int size)
{
	unsigned char* buf;
	if (size > NSVG__MEMPAGE_SIZE) return NULL;
	if (r->curpage == NULL || r->curpage->size+size > NSVG__MEMPAGE_SIZE) {
		r->curpage = nsvg__nextPage(r, r->curpage);
	}
	buf = &r->curpage->mem[r->curpage->size];
	r->curpage->size += size;
	return buf;
}

static void nsvg__addEdge(struct NSVGrasterizer* r, float x0, float y0, float x1, float y1)
{
	struct NSVGedge* e;

	// Skip horizontal edges
	if (y0 == y1)
		return;

	if (r->nedges+1 > r->cedges) {
		r->cedges = r->cedges > 0 ? r->cedges * 2 : 64;
		r->edges = (struct NSVGedge*)realloc(r->edges, sizeof(struct NSVGedge) * r->cedges);
		if (r->edges == NULL) return;
	}

	e = &r->edges[r->nedges];
	r->nedges++;

	if (y0 < y1) {
		e->x0 = x0;
		e->y0 = y0;
		e->x1 = x1;
		e->y1 = y1;
		e->dir = 1;
	} else {
		e->x0 = x1;
		e->y0 = y1;
		e->x1 = x0;
		e->y1 = y0;
		e->dir = -1;
	}
}

static float nsvg__absf(float x) { return x < 0 ? -x : x; }

static void nsvg__flattenCubicBez(struct NSVGrasterizer* r, 
								  float x1, float y1, float x2, float y2,
								  float x3, float y3, float x4, float y4,
								  float tol, int level)
{
	float x12,y12,x23,y23,x34,y34,x123,y123,x234,y234,x1234,y1234;
	
	if (level > 10) return;

	if (nsvg__absf(x1+x3-x2-x2) + nsvg__absf(y1+y3-y2-y2) + nsvg__absf(x2+x4-x3-x3) + nsvg__absf(y2+y4-y3-y3) < tol) {
		nsvg__addEdge(r, r->px, r->py, x4, y4);
		r->px = x4;
		r->py = y4;
		return;
	}

	x12 = (x1+x2)*0.5f;
	y12 = (y1+y2)*0.5f;
	x23 = (x2+x3)*0.5f;
	y23 = (y2+y3)*0.5f;
	x34 = (x3+x4)*0.5f;
	y34 = (y3+y4)*0.5f;
	x123 = (x12+x23)*0.5f;
	y123 = (y12+y23)*0.5f;
	x234 = (x23+x34)*0.5f;
	y234 = (y23+y34)*0.5f;
	x1234 = (x123+x234)*0.5f;
	y1234 = (y123+y234)*0.5f;

	nsvg__flattenCubicBez(r, x1,y1, x12,y12, x123,y123, x1234,y1234, tol, level+1); 
	nsvg__flattenCubicBez(r, x1234,y1234, x234,y234, x34,y34, x4,y4, tol, level+1); 
}

static void nsvg__flattenShape(struct NSVGrasterizer* r, struct NSVGshape* shape, float scale)
{
	struct NSVGpath* path;
	float tol = 0.25f * 4.0f / scale;
	int i;

	for (path = shape->paths; path != NULL; path = path->next) {
		// Flatten path
		r->px = path->pts[0];
		r->py = path->pts[1];
		for (i = 0; i < path->npts-1; i += 3) {
			float* p = &path->pts[i*2];
			nsvg__flattenCubicBez(r, p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7], tol, 0);
		}
		// Close path
		nsvg__addEdge(r, r->px,r->py, path->pts[0],path->pts[1]);
	}
}

static int nsvg__cmpEdge(const void *p, const void *q)
{
	struct NSVGedge* a = (struct NSVGedge*)p;
	struct NSVGedge* b = (struct NSVGedge*)q;

	if (a->y0 < b->y0) return -1;
	if (a->y0 > b->y0) return  1;
	return 0;
}


static struct NSVGactiveEdge* nsvg__addActive(struct NSVGrasterizer* r, struct NSVGedge* e, float startPoint)
{
	struct NSVGactiveEdge* z;

	if (r->freelist != NULL) {
		// Restore from freelist.
		z = r->freelist;
		r->freelist = z->next;
	} else {
		// Alloc new edge.
		z = (struct NSVGactiveEdge*)nsvg__alloc(r, sizeof(struct NSVGactiveEdge));
		if (z == NULL) return NULL;
	}

	float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
//	STBTT_assert(e->y0 <= start_point);
	// round dx down to avoid going too far
	if (dxdy < 0)
		z->dx = -floorf(NSVG__FIX * -dxdy);
	else
		z->dx = floorf(NSVG__FIX * dxdy);
	z->x = floorf(NSVG__FIX * (e->x0 + dxdy * (startPoint - e->y0)));
//	z->x -= off_x * FIX;
	z->ey = e->y1;
	z->next = 0;
	z->dir = e->dir;

	return z;
}

static void nsvg__freeActive(struct NSVGrasterizer* r, struct NSVGactiveEdge* z)
{
	z->next = r->freelist;
	r->freelist = z;
}

// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap
static void nsvg__fillActiveEdges(unsigned char* scanline, int len, struct NSVGactiveEdge* e, int maxWeight, int* xmin, int* xmax)
{
	// non-zero winding fill
	int x0 = 0, w = 0;

	while (e != NULL) {
		if (w == 0) {
			// if we're currently at zero, we need to record the edge start point
			x0 = e->x; w += e->dir;
		} else {
			int x1 = e->x; w += e->dir;
			// if we went to zero, we need to draw
			if (w == 0) {
				int i = x0 >> NSVG__FIXSHIFT;
				int j = x1 >> NSVG__FIXSHIFT;
				if (i < *xmin) *xmin = i;
				if (j > *xmax) *xmax = j;
				if (i < len && j >= 0) {
					if (i == j) {
						// x0,x1 are the same pixel, so compute combined coverage
						scanline[i] += (unsigned char)((x1 - x0) * maxWeight >> NSVG__FIXSHIFT);
					} else {
						if (i >= 0) // add antialiasing for x0
							scanline[i] += (unsigned char)(((NSVG__FIX - (x0 & NSVG__FIXMASK)) * maxWeight) >> NSVG__FIXSHIFT);
						else
							i = -1; // clip

						if (j < len) // add antialiasing for x1
							scanline[j] += (unsigned char)(((x1 & NSVG__FIXMASK) * maxWeight) >> NSVG__FIXSHIFT);
						else
							j = len; // clip

						for (++i; i < j; ++i) // fill pixels between x0 and x1
							scanline[i] += (unsigned char)maxWeight;
					}
				}
			}
		}
		e = e->next;
	}
}

static float nsvg__clampf(float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }

static unsigned int nsvg__RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return (r) | (g << 8) | (b << 16) | (a << 24);
}

static unsigned int nsvg__lerpRGBA(unsigned int c0, unsigned int c1, float u)
{
	int iu = (float)(nsvg__clampf(u, 0.0f, 1.0f) * 256.0f);
	int r = (((c0) & 0xff)*(256-iu) + (((c1) & 0xff)*iu)) >> 8;
	int g = (((c0>>8) & 0xff)*(256-iu) + (((c1>>8) & 0xff)*iu)) >> 8;
	int b = (((c0>>16) & 0xff)*(256-iu) + (((c1>>16) & 0xff)*iu)) >> 8;
	int a = (((c0>>24) & 0xff)*(256-iu) + (((c1>>24) & 0xff)*iu)) >> 8;
	return nsvg__RGBA(r,g,b,a);
}

static unsigned int nsvg__applyOpacity(unsigned int c, float u)
{
	int iu = (float)(nsvg__clampf(u, 0.0f, 1.0f) * 256.0f);
	int r = (c) & 0xff;
	int g = (c>>8) & 0xff;
	int b = (c>>16) & 0xff;
	int a = (((c>>24) & 0xff)*iu) >> 8;
	return nsvg__RGBA(r,g,b,a);
}

static void nsvg__scanlineSolid(unsigned char* dst, int count, unsigned char* cover, int x, int y,
								float tx, float ty, float scale, struct NSVGcachedPaint* cache)
{

	if (cache->type == NSVG_PAINT_COLOR) {
		int i, cr, cg, cb, ca;
		cr = cache->colors[0] & 0xff;
		cg = (cache->colors[0] >> 8) & 0xff;
		cb = (cache->colors[0] >> 16) & 0xff;
		ca = (cache->colors[0] >> 24) & 0xff;

		for (i = 0; i < count; i++) {
			int r,g,b;
			int a = ((int)cover[0] * ca) >> 8;
			int ia = 255 - a;
			// Premultiply
			r = (cr * a) >> 8;
			g = (cg * a) >> 8;
			b = (cb * a) >> 8;

			// Blend over
			r += ((ia * (int)dst[0]) >> 8);
			g += ((ia * (int)dst[1]) >> 8);
			b += ((ia * (int)dst[2]) >> 8);
			a += ((ia * (int)dst[3]) >> 8);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
		}
	} else if (cache->type == NSVG_PAINT_LINEAR_GRADIENT) {
		// TODO: spread modes.
		// TODO: plenty of opportunities to optimize.
		float fx, fy, dx, gy;
		float* t = cache->xform;
		int i, cr, cg, cb, ca;
		unsigned int c;

		fx = (x - tx) / scale;
		fy = (y - ty) / scale;
		dx = 1.0f / scale;

		for (i = 0; i < count; i++) {
			int r,g,b,a,ia;
			gy = fx*t[1] + fy*t[3] + t[5];
			c = cache->colors[(int)nsvg__clampf(gy*255.0f, 0, 255.0f)];
			cr = (c) & 0xff;
			cg = (c >> 8) & 0xff;
			cb = (c >> 16) & 0xff;
			ca = (c >> 24) & 0xff;

			a = ((int)cover[0] * ca) >> 8;
			ia = 255 - a;

			// Premultiply
			r = (cr * a) >> 8;
			g = (cg * a) >> 8;
			b = (cb * a) >> 8;

			// Blend over
			r += ((ia * (int)dst[0]) >> 8);
			g += ((ia * (int)dst[1]) >> 8);
			b += ((ia * (int)dst[2]) >> 8);
			a += ((ia * (int)dst[3]) >> 8);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
			fx += dx;
		}
	} else if (cache->type == NSVG_PAINT_RADIAL_GRADIENT) {
		// TODO: spread modes.
		// TODO: plenty of opportunities to optimize.
		// TODO: focus (fx,fy)
		float fx, fy, dx, gx, gy, gd;
		float* t = cache->xform;
		int i, cr, cg, cb, ca;
		unsigned int c;

		fx = (x - tx) / scale;
		fy = (y - ty) / scale;
		dx = 1.0f / scale;

		for (i = 0; i < count; i++) {
			int r,g,b,a,ia;
			gx = fx*t[0] + fy*t[2] + t[4];
			gy = fx*t[1] + fy*t[3] + t[5];
			gd = sqrtf(gx*gx + gy*gy);
			c = cache->colors[(int)nsvg__clampf(gd*255.0f, 0, 255.0f)];
			cr = (c) & 0xff;
			cg = (c >> 8) & 0xff;
			cb = (c >> 16) & 0xff;
			ca = (c >> 24) & 0xff;

			a = ((int)cover[0] * ca) >> 8;
			ia = 255 - a;

			// Premultiply
			r = (cr * a) >> 8;
			g = (cg * a) >> 8;
			b = (cb * a) >> 8;

			// Blend over
			r += ((ia * (int)dst[0]) >> 8);
			g += ((ia * (int)dst[1]) >> 8);
			b += ((ia * (int)dst[2]) >> 8);
			a += ((ia * (int)dst[3]) >> 8);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
			fx += dx;
		}
	}
}

static void nsvg__rasterizeSortedEdges(struct NSVGrasterizer *r, float tx, float ty, float scale, struct NSVGcachedPaint* cache)
{
	struct NSVGactiveEdge *active = NULL;
	int y, s;
	int e = 0;
	int maxWeight = (255 / NSVG__SUBSAMPLES);  // weight per vertical scanline
	int xmin, xmax;

	for (y = 0; y < r->height; y++) {
		memset(r->scanline, 0, r->width);
		xmin = r->width;
		xmax = 0;
		for (s = 0; s < NSVG__SUBSAMPLES; ++s) {
			// find center of pixel for this scanline
			float scany = y*NSVG__SUBSAMPLES + s + 0.5f;
			struct NSVGactiveEdge **step = &active;

			// update all active edges;
			// remove all active edges that terminate before the center of this scanline
			while (*step) {
				struct NSVGactiveEdge *z = *step;
				if (z->ey <= scany) {
					*step = z->next; // delete from list
//					NSVG__assert(z->valid);
					nsvg__freeActive(r, z);
				} else {
					z->x += z->dx; // advance to position for current scanline
					step = &((*step)->next); // advance through list
				}
			}

			// resort the list if needed
			for (;;) {
				int changed = 0;
				step = &active;
				while (*step && (*step)->next) {
					if ((*step)->x > (*step)->next->x) {
						struct NSVGactiveEdge* t = *step;
						struct NSVGactiveEdge* q = t->next;
						t->next = q->next;
						q->next = t;
						*step = q;
						changed = 1;
					}
					step = &(*step)->next;
				}
				if (!changed) break;
			}

			// insert all edges that start before the center of this scanline -- omit ones that also end on this scanline
			while (e < r->nedges && r->edges[e].y0 <= scany) {
				if (r->edges[e].y1 > scany) {
					struct NSVGactiveEdge* z = nsvg__addActive(r, &r->edges[e], scany);
					if (z == NULL) break;
					// find insertion point
					if (active == NULL) {
						active = z;
					} else if (z->x < active->x) {
						// insert at front
						z->next = active;
						active = z;
					} else {
						// find thing to insert AFTER
						struct NSVGactiveEdge* p = active;
						while (p->next && p->next->x < z->x)
							p = p->next;
						// at this point, p->next->x is NOT < z->x
						z->next = p->next;
						p->next = z;
					}
				}
				e++;
			}

			// now process all active edges in non-zero fashion
			if (active != NULL)
				nsvg__fillActiveEdges(r->scanline, r->width, active, maxWeight, &xmin, &xmax);
		}
		// Blit
		if (xmin < 0) xmin = 0;
		if (xmax > r->width-1) xmax = r->width-1;
		if (xmin <= xmax) {
			nsvg__scanlineSolid(&r->bitmap[y * r->stride] + xmin*4, xmax-xmin+1, &r->scanline[xmin], xmin, y, tx,ty,scale,cache);
		}
	}

}

static void nsvg__unpremultiplyAlpha(unsigned char* image, int w, int h, int stride)
{
	int x,y;

	// Unpremultiply
	for (y = 0; y < h; y++) {
		unsigned char *row = &image[y*stride];
		for (x = 0; x < w; x++) {
			int r = row[0], g = row[1], b = row[2], a = row[3];
			if (a != 0) {
				row[0] = (int)(r*255/a);
				row[1] = (int)(g*255/a);
				row[2] = (int)(b*255/a);
			}
			row += 4;
		}
	}

	// Defringe
	for (y = 0; y < h; y++) {
		unsigned char *row = &image[y*stride];
		for (x = 0; x < w; x++) {
			int r = 0, g = 0, b = 0, a = row[3], n = 0;
			if (a == 0) {
				if (x-1 > 0 && row[-1] != 0) {
					r += row[-4];
					g += row[-3];
					b += row[-2];
					n++;
				}
				if (x+1 < w && row[7] != 0) {
					r += row[4];
					g += row[5];
					b += row[6];
					n++;
				}
				if (y-1 > 0 && row[-stride+3] != 0) {
					r += row[-stride];
					g += row[-stride+1];
					b += row[-stride+2];
					n++;
				}
				if (y+1 < h && row[stride+3] != 0) {
					r += row[stride];
					g += row[stride+1];
					b += row[stride+2];
					n++;
				}
				if (n > 0) {
					row[0] = r/n;
					row[1] = g/n;
					row[2] = b/n;
				}
			}
			row += 4;
		}
	}
}


static void nsvg__initPaint(struct NSVGcachedPaint* cache, struct NSVGpaint* paint, float opacity)
{
	int i, j;
	struct NSVGgradient* grad;

	cache->type = paint->type;

	if (paint->type == NSVG_PAINT_COLOR) {
		cache->colors[0] = nsvg__applyOpacity(paint->color, opacity);
		return;
	}

	grad = paint->gradient;

	cache->spread = grad->spread;
	memcpy(cache->xform, grad->xform, sizeof(float)*6);

	if (grad->nstops == 0) {
		for (i = 0; i < 256; i++)
			cache->colors[i] = 0;
	} if (grad->nstops == 1) {
		for (i = 0; i < 256; i++)
			cache->colors[i] = nsvg__applyOpacity(grad->stops[i].color, opacity);
	} else {
		unsigned int ca, cb;
		float ua, ub, du, u;
		int ia, ib, count;

		ca = nsvg__applyOpacity(grad->stops[0].color, opacity);
		ua = nsvg__clampf(grad->stops[0].offset, 0, 1);
		ub = nsvg__clampf(grad->stops[grad->nstops-1].offset, ua, 1);
		ia = ua * 255.0f;
		ib = ub * 255.0f;
		for (i = 0; i < ia; i++) {
			cache->colors[i] = ca;
		}

		for (i = 0; i < grad->nstops-1; i++) {
			ca = nsvg__applyOpacity(grad->stops[i].color, opacity);
			cb = nsvg__applyOpacity(grad->stops[i+1].color, opacity);
			ua = nsvg__clampf(grad->stops[i].offset, 0, 1);
			ub = nsvg__clampf(grad->stops[i+1].offset, 0, 1);
			ia = ua * 255.0f;
			ib = ub * 255.0f;
			count = ib - ia;
			if (count <= 0) continue;
			u = 0;
			du = 1.0f / (float)count;
			for (j = 0; j < count; j++) {
				cache->colors[ia+j] = nsvg__lerpRGBA(ca,cb,u);
				u += du;
			}
		}

		for (i = ib; i < 256; i++)
			cache->colors[i] = cb;
	}

}

void nsvgRasterize(struct NSVGrasterizer* r,
				   struct NSVGimage* image, float tx, float ty, float scale,
				   unsigned char* dst, int w, int h, int stride)
{
	struct NSVGshape *shape = NULL;
	struct NSVGedge *e = NULL;
	struct NSVGcachedPaint cache;
	int i;
	
	r->bitmap = dst;
	r->width = w;
	r->height = h;
	r->stride = stride;

	if (w > r->cscanline) {
		r->cscanline = w;
		r->scanline = (unsigned char*)realloc(r->scanline, w);
		if (r->scanline == NULL) return;
	}

	for (i = 0; i < h; i++)
		memset(&dst[i*stride], 0, w*4);

	for (shape = image->shapes; shape != NULL; shape = shape->next) {

		if (shape->fill.type == NSVG_PAINT_NONE)
			continue;

		nsvg__resetPool(r);
		r->freelist = NULL;
		r->nedges = 0;

		nsvg__flattenShape(r, shape, scale);

		// Scale and translate edges
		for (i = 0; i < r->nedges; i++) {
			e = &r->edges[i];
			e->x0 = tx + e->x0 * scale;
			e->y0 = (ty + e->y0 * scale) * NSVG__SUBSAMPLES;
			e->x1 = tx + e->x1 * scale;
			e->y1 = (ty + e->y1 * scale) * NSVG__SUBSAMPLES;
		}

		// Rasterize edges
		qsort(r->edges, r->nedges, sizeof(struct NSVGedge), nsvg__cmpEdge);

		// now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
		nsvg__initPaint(&cache, &shape->fill, shape->opacity);
	
		nsvg__rasterizeSortedEdges(r, tx,ty,scale, &cache);
	}

	nsvg__unpremultiplyAlpha(dst, w, h, stride);

	r->bitmap = NULL;
	r->width = 0;
	r->height = 0;
	r->stride = 0;
}

#endif
