#include "partition.h"
#include <math.h>
#include <stdio.h>
#include <VT.h>

void partition(const int np, const int radius, const int xsize, const int ysize, int * blockdata){

int func_statehandle;
	/*
	 * setup tracing of the partitioning operation
	 * */
	VT_funcdef("Partition", VT_NOCLASS, &func_statehandle);
	VT_enter(func_statehandle, VT_NOSCL);

	/*
	 * Store picture width in the return array
	 * */
	blockdata[X_SIZE] = xsize;

	/*
	 * Calulate initial partition size
	 * */
	int block_height = floor(ysize/np);

	/*
	 * Calculate the remaining rows
	 * */
	int odd_rows = ysize - block_height * np;

	int end_of_local_block, i;

	/*
	 * Calculate the partition for each process in turn.
	 * */
	for(i = 0; i < np; i++){

		/*
		 * GATHER_SIZE - store the actual partition size (excl. radius) for this process
		 * If there are any remaining rows left add one, this makes sure that the
		 * differense in size betwen processes is maxumum one row.
		 * */
		if(odd_rows > 0){
			blockdata[GATHER_SIZE + i] = xsize * (block_height + 1);
			odd_rows--;
		}else{
			blockdata[GATHER_SIZE + i] = xsize * block_height;
		}

		/*
		 * GATHER_GLOBAL_OFFSET - store the offset of the partition in the big picture
		 * this is dependant on the offset and size of the last partition
		 * */
		if(i == 0){
			blockdata[GATHER_GLOBAL_OFFSET + i] = 0;
		}else{
			blockdata[GATHER_GLOBAL_OFFSET + i] = blockdata[GATHER_GLOBAL_OFFSET + i - 1] + blockdata[GATHER_SIZE + i - 1];
		}

		/*
		 * GATHER_LOCAL_OFFSET - I.e. how much radius data should there be befor
		 * the actual partition. Be careful, the radius part cant start before
		 * pixel zero.
		 * */
		if(blockdata[GATHER_GLOBAL_OFFSET + i]/xsize > radius){
			blockdata[GATHER_LOCAL_OFFSET + i] = radius * xsize;
		}else{
			blockdata[GATHER_LOCAL_OFFSET + i] = blockdata[GATHER_GLOBAL_OFFSET + i];
		}

		/*
		 * SCATTER_OFFSET - Caculate where in the big picture to start sending
		 * from (i.e. where each partition incl radius begins)
		 * */
		blockdata[SCATTER_OFFSET + i] = blockdata[GATHER_GLOBAL_OFFSET + i] - blockdata[GATHER_LOCAL_OFFSET + i];


		/*
		 * SCATTER_SIZE - Calulate the size to send to each process (incl radius)
		 * becareful not to send anything beyond the end of the picture
		 * */
		end_of_local_block = (blockdata[GATHER_GLOBAL_OFFSET + i] + blockdata[GATHER_SIZE + i])/xsize;
		if(end_of_local_block + radius < ysize){
			blockdata[SCATTER_SIZE + i] = blockdata[GATHER_LOCAL_OFFSET + i] + blockdata[GATHER_SIZE + i] + radius * xsize;
		}else{
			blockdata[SCATTER_SIZE + i] = blockdata[GATHER_LOCAL_OFFSET + i] + blockdata[GATHER_SIZE + i] + (ysize - end_of_local_block)*xsize;
		}

	}

VT_leave(VT_NOSCL);
}
