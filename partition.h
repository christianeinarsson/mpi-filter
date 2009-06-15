#ifndef _PARTITION_H_
#define _PARTITION_H_


/*
 * Define offsets for datastoring in the blockdata array with partition data.
 * process specific data should be accessed the following way
 * bd[DATANAME + processid]. For example bd[GATHER_SIZE + me]
 * The advantagde from this aproach is
 * 	Everything the partitions need to know about themself and others are
 * 		stored in one array easiely brodcasted to everyone
 * 	Arrays of offsets and otherthings are needed when doing scatterv and
 * 		gatherv
 * Every size and offset is in pixels
 * */

/*
 * SCATTER_OFFSET - 			The offset of the first pixel for each process
 * 								(including the radius)
 * */
#define SCATTER_OFFSET			0
/*
 * SCATTER_SIZE - 			The number of pixel each process are to receive
 * */
#define SCATTER_SIZE				1*np
/*
 * GATHER_GLOBAL_OFFSET -	The offset where the result from each process should
 * 								start in the finished picture
 * */
#define GATHER_GLOBAL_OFFSET	2*np
/*
 * GATHER_LOCAL_OFFSET - 	The offset in each local partition to the actual part
 * 								alloted to it (excluding radius).
 * */
#define GATHER_LOCAL_OFFSET	3*np
/*
 * GATHER_SIZE	- 				The size to gather from each process, also the size
 * 								the process should filter.
 * */
#define GATHER_SIZE				4*np
/*
 * X_SIZE - 					The width of the picture
 * */
#define X_SIZE						5*np
/*
 * BLOCKDATA_SIZE				The size of the blockdata array.
 * */
#define BLOCKDATA_SIZE			5*np+1

/*
 * Partitions the image for all processes.
 * Takes np - number of processes
 * 		radius - the blur radius
 * 		xsize,ysize - size of picture
 * 		blockdata - array to put result in
 * Returns all data needed to split and filter the picture on multiple processes,
 * the data is writen to the array using the offsets defined above.
 * */

void partition(const int np, const int radius, const int xsize, const int ysize, int * blockdata);

#endif
