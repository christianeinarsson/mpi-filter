#include "pixel.h"

/*
  File: thresfilter.h

  Declaration of pixel structure and thresfilter function.

 */
#ifndef _THRESFILTER_H_
#define _THRESFILTER_H_

int threshold(const int size, pixel* src);
void thresfilter(const int size, const int threshold_value, pixel * src);



#endif
