#include <stdio.h>

#include "pixel.h"
#include "ppmio.h"

pixel* pix(pixel* image, const int xx, const int yy, const int xsize)
{
	register int off = xsize*yy + xx;

	if(off >= MAX_PIXELS) {
		fprintf(stderr, "\n Terribly wrong: %d %d %d\n",xx,yy,xsize);
	}
	return (image + off);
}
