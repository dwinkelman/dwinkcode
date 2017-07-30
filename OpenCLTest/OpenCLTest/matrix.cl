/******************************************************************************
 * Matrix multiplication kernel with loop unrolling optimization
 * 
 * This algorithm performs better than the naive implementation with matrices
 * that are approximately [96 x 96]
**/
__kernel void matrix_multiply(
		__global float * A,
		__global float * B,
		__global float * C,
		int a_cols, int b_cols
){
#define BLOCK_SIZE 16

	int block_x = get_group_id(0);
	int block_y = get_group_id(1);

	int thread_x = get_local_id(0);
	int thread_y = get_local_id(1);

	int global_x = get_global_id(0);
	int global_y = get_global_id(1);

	int a_begin = a_cols * BLOCK_SIZE * block_y;
	int a_end = a_begin + a_cols - 1;
	int a_step = BLOCK_SIZE;

	int b_begin = BLOCK_SIZE * block_x;
	int b_step = BLOCK_SIZE * b_cols;

	float c_sub = 0;

	for(int a = a_begin, b = b_begin;
			a < a_end;
			a += a_step, b += b_step)
	{
		__local float a_sub[BLOCK_SIZE][BLOCK_SIZE];
		__local float b_sub[BLOCK_SIZE][BLOCK_SIZE];

		a_sub[thread_y][thread_x] = A[a + a_cols * thread_y + thread_x];
		b_sub[thread_y][thread_x] = B[b + b_cols * thread_y + thread_x];

		barrier(CLK_LOCAL_MEM_FENCE);

		#pragma unroll
		for(int k = 0; k < BLOCK_SIZE; ++k)
			c_sub += a_sub[thread_y][k] * b_sub[k][thread_x];

		barrier(CLK_LOCAL_MEM_FENCE);
	}

	C[global_y * b_cols + global_x] = c_sub;
}