/*
  File: blurfilter.c

  Implementation of blurfilter function.

 */
#include <stdio.h>
#include "blurfilter.h"
#include "ppmio.h"
#include "pixel.h"



/*
 * Changed to first y-wise then x-wise to eliminate black lines.
 * When doing x-wise first the radius dont get copied to dst, if we do
 * y-wise first then radius is avalible when needed and there is no need
 * to copy it to dst.
 * We also introduced the input variable ystop to hinder the filter to
 * continue filtering down through the radius belowe the alotted partition.
 * */

void blurfilter(const int y_start, const int xsize, const int ysize, pixel* src, const int radius, const double *w, const int ystop){
	int x,y,x2,y2, wi;
	double r,g,b,n, wc;
	//pixel dst[MAX_PIXELS];
	pixel dst[xsize*ysize];


	for (y=y_start; y<ystop; y++) {
		for (x=0; x<xsize; x++) {
			r = w[0] * pix(src, x, y, xsize)->r;
			g = w[0] * pix(src, x, y, xsize)->g;
			b = w[0] * pix(src, x, y, xsize)->b;
			n = w[0];
			for ( wi=1; wi <= radius; wi++) {
				wc = w[wi];
				y2 = y - wi;
				if(y2 >= 0) {
					r += wc * pix(src, x, y2, xsize)->r;
					g += wc * pix(src, x, y2, xsize)->g;
					b += wc * pix(src, x, y2, xsize)->b;
					n += wc;
				}
				y2 = y + wi;
				if(y2 < ysize) {
					r += wc * pix(src, x, y2, xsize)->r;
					g += wc * pix(src, x, y2, xsize)->g;
					b += wc * pix(src, x, y2, xsize)->b;
					n += wc;
				}
			}
			pix(dst,x,y, xsize)->r = r/n;
			pix(dst,x,y, xsize)->g = g/n;
			pix(dst,x,y, xsize)->b = b/n;
		}
	}

	for (y=y_start; y<ystop; y++) {
		for (x=0; x<xsize; x++) {
			r = w[0] * pix(dst, x, y, xsize)->r;
			g = w[0] * pix(dst, x, y, xsize)->g;
			b = w[0] * pix(dst, x, y, xsize)->b;
			n = w[0];
			for ( wi=1; wi <= radius; wi++) {
				wc = w[wi];
				x2 = x - wi;
				if(x2 >= 0) {
					r += wc * pix(dst, x2, y, xsize)->r;
					g += wc * pix(dst, x2, y, xsize)->g;
					b += wc * pix(dst, x2, y, xsize)->b;
					n += wc;
				}
				x2 = x + wi;
				if(x2 < xsize) {
					r += wc * pix(dst, x2, y, xsize)->r;
					g += wc * pix(dst, x2, y, xsize)->g;
					b += wc * pix(dst, x2, y, xsize)->b;
					n += wc;
				}
			}
			pix(src,x,y, xsize)->r = r/n;
			pix(src,x,y, xsize)->g = g/n;
			pix(src,x,y, xsize)->b = b/n;
		}
	}


}



