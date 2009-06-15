#include "thresfilter.h"
#include "pixel.h"

#define uint unsigned int

int threshold(const int size, pixel* src){

  uint sum, i;

  for(i = 0, sum = 0; i < size; i++){
    sum += (uint)src[i].r + (uint)src[i].g + (uint)src[i].b;
  }

  return (int)sum/size;
}

void thresfilter(const int size, const int threshold_value, pixel * src){
  int i, psum;
  for(i=0;i<size;i++){
    psum = (int)src[i].r + (int)src[i].g + (int)src[i].b;
    if(threshold_value > psum){
      src[i].r = src[i].g = src[i].b = 0;
    }else{
      src[i].r = src[i].g = src[i].b = 255;
    }
  }
}

