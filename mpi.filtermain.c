#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include "ppmio.h"
#include "blurfilter.h"
#include "gaussw.h"
#include "thresfilter.h"
#include "partition.h"
#include "pixel.h"
#include <VT.h>
#include "debug.h"

int msglevel = 100;

int main (int argc, char * argv[]) {
	/*
	 * Init itac tracing
	 * */
	VT_initialize(&argc, &argv);
	int func_statehandle;
	VT_funcdef("Main", VT_NOCLASS, &func_statehandle);
	VT_enter(func_statehandle, VT_NOSCL);

	/*
	* Init MPI
	**/
	int np, me;
	int ierr = MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);

	/*
	* MPI commiting type  pixel
	* */
	pixel item; // Element of the type
	MPI_Datatype MPI_PIXEL; // MPI type to commit
	int block_lengths[] = { 1, 1, 1 }; // Set lengths of type elements
	MPI_Datatype block_types[] = { MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR }; // Set types
	MPI_Aint start, displ[3];
	MPI_Address(&item, &start);
	MPI_Address(&item.r, &displ[0]);
	MPI_Address(&item.g, &displ[1]);
	MPI_Address(&item.b, &displ[2]);
	 // Displacements relative to address of start
	displ[0] -= start;
	displ[1] -= start;
	displ[2] -= start;
	MPI_Type_struct(3, block_lengths, displ, block_types, &MPI_PIXEL);
	MPI_Type_commit(&MPI_PIXEL);

	/*
	* Program start
	* */
	int radius = MAX_RAD + 1;
	int xsize, ysize;

	int * blockdata = malloc((BLOCKDATA_SIZE) * sizeof(int));
	pixel *src;
	double weight[radius];

	pmesg(50, "Process %d of %d\n",me,np);

	/*
	* Take care of the arguments
	* */
	if ((me == 0) && (argc != 4)) {
		fprintf(stderr, "Usage: %s radius infile outfile (radius = 0 -> threshold otherwise blur)\n", argv[0]);
		MPI_Finalize();
		exit(1);
	}

	if(argc == 4){
		radius = atoi(argv[1]);
	}

	if(me == 1){
		if((radius > MAX_RAD) || (radius < 0)) {
			fprintf(stderr, "Radius (%d) must be greater than or equal to zero and less then %d\n", radius, MAX_RAD);
			MPI_Finalize();
			exit(1);
		}
		if(radius == 0){
			pmesg(50, " *** Doing threshold filtering *** \n");
		}else{
			pmesg(50, " *** Doing blur filtering *** \n");
		}
	}

	/*
	 * If we are going to do blur filteringen let process 1 calculate the weightfunction
	 * */
	if((me == 1) && (radius > 0)){
		pmesg(50, "Process %d: Calculating weight function.\n", me);
		get_gauss_weights(radius, weight);
		pmesg(50, "Process %d: Calculating weight function. Done!\n", me);
	}

	/*
	 * Let process zero alocate memmory and read the picture from disk
	 * */
	if(me == 0){

		src = (pixel *) malloc(MAX_PIXELS * sizeof(pixel));

		int colmax = 0;

		if(read_ppm (argv[2], &xsize, &ysize, &colmax, (char *) src) != 0)
			exit(1);

		if (colmax > 255) {
			fprintf(stderr, "Too large maximum color-component value\n");
			MPI_Finalize();
			exit(1);
		}

		pmesg(50, "Process %d: Has read the image, starting partitioning...\n", me);
		pmesg(50, "Process %d: Picture size(x,y): %d, %d.\n", me, xsize, ysize);

		partition(np,radius,	xsize, ysize , blockdata);

		pmesg(50, "Process %d: Partitioning on %d processes done.\n", me, np);

	}

	// Basic timing
	double starttime, endtime;
	starttime = MPI_Wtime();

	/*
	 * Do a broadcast of the partitioning data.
	 * */
	if(me==0)pmesg(50, "blockdata.xsize=%i\n",blockdata[X_SIZE]);
	if(me==0)pmesg(50, "will bcast blockdata\n");

	MPI_Bcast(&blockdata[0], BLOCKDATA_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

	if(me==0)pmesg(50, "will bcast blockdata\n");

	pixel *local_src;
	/*
	 * Do blur filtering1
	 * */
	if(radius > 0 ){
		/*
		 * setup tracing of blur filter
		 * */
		int blur_symdef;
		VT_funcdef("Filter:Blur", VT_NOCLASS, &blur_symdef);
		VT_enter(blur_symdef, VT_NOSCL);

		/*
		 * Let everyone alocate enough memmory for there task.
		 * */
		if(me==0)pmesg(50, "will allocate local_src\n");
		local_src = (pixel *) malloc(blockdata[SCATTER_SIZE + me]*sizeof(pixel));

		/*
		 * Broadcast the weighted array.
		 * */
		if(me==0)pmesg(50, "will bcast weight\n");
		MPI_Bcast(weight, radius, MPI_DOUBLE, 1, MPI_COMM_WORLD);
		if(me==0)pmesg(50, "done bcast weight\n");

		/*
		 * setup tracing of the scatter operation
		 * */
		int scatterv_blur_symdef;
		VT_funcdef("Filter:Blur:ScatterV", VT_NOCLASS, &scatterv_blur_symdef);
		VT_enter(scatterv_blur_symdef, VT_NOSCL);
		/*
		 * Scatter the image to each process local_src
		 * */
		if(me==0)pmesg(50, "will scatterv picture\n");
		MPI_Scatterv(src, &blockdata[SCATTER_SIZE], &blockdata[SCATTER_OFFSET], MPI_PIXEL,
						local_src, blockdata[SCATTER_SIZE + me], MPI_PIXEL,
						0, MPI_COMM_WORLD);
		VT_leave(VT_NOSCL);
		if(me==0)pmesg(50, "done scatterv picture\n");

		/*
		 * Read vital data for the blur funtion from the partition data in blockdata
		 * */
		int local_y_offset =  blockdata[GATHER_LOCAL_OFFSET + me ] / blockdata[X_SIZE];
		int local_y_size = blockdata[SCATTER_SIZE + me] / blockdata[X_SIZE];
		int local_y_stop = (blockdata[GATHER_LOCAL_OFFSET + me] + blockdata[GATHER_SIZE + me]) / blockdata[X_SIZE];

		/*
		 * Each process blurs it's part of the image
		 * */
		pmesg(50, "Process %d: Calling blurfilter.\n",me);
		blurfilter(local_y_offset, blockdata[X_SIZE], local_y_size, local_src, radius, weight, local_y_stop);
		pmesg(50, "Process %d: Filtering done!\n", me);

		VT_leave(VT_NOSCL);
	}else{
		/*
		 * Do threshold filtering
		 * */
		/*
		 * Setup tracing of the threshold filter
		 * */
		int threshold_symdef;
		VT_funcdef("Filter:Threshold", VT_NOCLASS, &threshold_symdef);
		VT_enter(threshold_symdef, VT_NOSCL);

		/*
		 * Alocate memmory for local picture buffer
		 * */
		local_src = (pixel *) malloc(blockdata[GATHER_SIZE + me]*sizeof(pixel));

		/*
		 * Setup tracing of the scatterv operation
		 * */
		int scatterv_threshold_symdef;
		VT_funcdef("Filter:Threshold:ScatterV", VT_NOCLASS, &scatterv_threshold_symdef);
		VT_enter(scatterv_threshold_symdef, VT_NOSCL);

		/*
		 * Do the scatterv operation
		 * */
		MPI_Scatterv(src, &blockdata[GATHER_SIZE], &blockdata[GATHER_GLOBAL_OFFSET], MPI_PIXEL,
			local_src, blockdata[GATHER_SIZE + me], MPI_PIXEL,
			0, MPI_COMM_WORLD);
		VT_leave(VT_NOSCL);

		/*
		 * Calculate  local threshold value
		 * */
		int my_threshold = threshold(blockdata[GATHER_SIZE + me], local_src);
		pmesg(50, "Process %d: my_threshold=%d\n",me, my_threshold);


		/*
		 * Collect the local theshold values with MPI_Allreduce sum.
		 * */
		int sum_threshold;
		MPI_Allreduce( &my_threshold, &sum_threshold, 1, MPI_INT,MPI_SUM,MPI_COMM_WORLD);

		/*
		 * Calculate the global threshold value (done by everyone in parallel, thanks to mpi_ALLreduce :-))
		 * */
		sum_threshold /= np;

		/*
		 * Apply threshold filter
		 * */
		thresfilter(blockdata[GATHER_SIZE + me],sum_threshold,local_src);
		VT_leave(VT_NOSCL);
	}

	/*
	 * When the filtering is done gather all results in process ones src buffer
	 * */
	MPI_Gatherv(&local_src[blockdata[GATHER_LOCAL_OFFSET +  me]], blockdata[GATHER_SIZE + me], MPI_PIXEL,
		src, &blockdata[GATHER_SIZE], &blockdata[GATHER_GLOBAL_OFFSET], MPI_PIXEL,
		0, MPI_COMM_WORLD);

	endtime = MPI_Wtime();
	double timing = endtime - starttime;
	if(me==0) printf("Filtering took %f seconds.\n", timing);

	/*
	 * Let process one write the result to disk.
	 * */
	if((me==0)&&(write_ppm (argv[3], xsize, ysize, (char *)src) != 0)){
		MPI_Finalize();
		exit(1);
	}

	/*
	 * Close down MPI and tracing.
	 * */
	MPI_Finalize();

	VT_leave(VT_NOSCL);
	VT_finalize();

	return(0);
}
