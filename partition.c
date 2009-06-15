#include "partition.h"
#include <math.h>
#include <stdio.h>
#include <VT.h>

void partition(const int np, const int radius, const int xsize, const int ysize, int * blockdata){

int func_statehandle;
	VT_funcdef("Partition", VT_NOCLASS, &func_statehandle);
	VT_enter(func_statehandle, VT_NOSCL);

	blockdata[X_SIZE] = xsize;

	int block_height = floor(ysize/np);
	int odd_rows = ysize - block_height * np;

	int end_of_local_block, i;

	for(i = 0; i < np; i++){

		if(odd_rows > 0){
			blockdata[GATHER_SIZE + i] = xsize * (block_height + 1);
			odd_rows--;
		}else{
			blockdata[GATHER_SIZE + i] = xsize * block_height;
		}

		if(i == 0){
			blockdata[GATHER_GLOBAL_OFFSET + i] = 0;
		}else{
			blockdata[GATHER_GLOBAL_OFFSET + i] = blockdata[GATHER_GLOBAL_OFFSET + i - 1] + blockdata[GATHER_SIZE + i - 1];
		}

		if(blockdata[GATHER_GLOBAL_OFFSET + i]/xsize > radius){
			blockdata[GATHER_LOCAL_OFFSET + i] = radius * xsize;
		}else{
			blockdata[GATHER_LOCAL_OFFSET + i] = blockdata[GATHER_GLOBAL_OFFSET + i];
		}

		blockdata[SCATTER_OFFSET + i] = blockdata[GATHER_GLOBAL_OFFSET + i] - blockdata[GATHER_LOCAL_OFFSET + i];

		end_of_local_block = (blockdata[GATHER_GLOBAL_OFFSET + i] + blockdata[GATHER_SIZE + i])/xsize;

		if(end_of_local_block + radius < ysize){
			blockdata[SCATTER_SIZE + i] = blockdata[GATHER_LOCAL_OFFSET + i] + blockdata[GATHER_SIZE + i] + radius * xsize;
		}else{
			blockdata[SCATTER_SIZE + i] = blockdata[GATHER_LOCAL_OFFSET + i] + blockdata[GATHER_SIZE + i] + (ysize - end_of_local_block)*xsize;
		}

		/*printf("Block %i partition data:\n" , i);
		printf("\t%10i : %4i - scatter offset\n", 		blockdata[SCATTER_OFFSET + i],		(int)blockdata[SCATTER_OFFSET + i]/xsize);
		printf("\t%10i : %4i - scatter size\n"  ,			blockdata[SCATTER_SIZE + i],			(int)blockdata[SCATTER_SIZE + i]/xsize);
		printf("\t%10i : %4i - gather global offset\n",	blockdata[GATHER_GLOBAL_OFFSET + i],(int)blockdata[GATHER_GLOBAL_OFFSET + i]/xsize);
		printf("\t%10i : %4i - gather local offset\n",	blockdata[GATHER_LOCAL_OFFSET + i],	(int)blockdata[GATHER_LOCAL_OFFSET + i]/xsize);
		printf("\t%10i : %4i - gather local size\n",		blockdata[GATHER_SIZE + i],			(int)blockdata[GATHER_SIZE + i]/xsize);

	* */
	}

VT_leave(VT_NOSCL);
}
