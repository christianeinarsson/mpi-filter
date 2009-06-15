#ifndef _PARTITION_H_
#define _PARTITION_H_


#define SCATTER_OFFSET			0
#define SCATTER_SIZE				1*np
#define GATHER_GLOBAL_OFFSET	2*np
#define GATHER_LOCAL_OFFSET	3*np
#define GATHER_SIZE				4*np
#define X_SIZE						5*np
#define BLOCKDATA_SIZE			5*np+1




void partition(const int np, const int radius, const int xsize, const int ysize, int * blockdata);

#endif
