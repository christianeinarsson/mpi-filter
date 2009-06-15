 /*
  File: blurfilter.h

  Declaration of blurfilter function.

 */
#include "pixel.h"

#ifndef _BLURFILTER_H_
#define _BLURFILTER_H_

#define MAX_RAD 1000

void blurfilter(const int x_start, const int xsize, const int ysize, pixel* src, const int radius, const double *w, const int ystop);

#endif
