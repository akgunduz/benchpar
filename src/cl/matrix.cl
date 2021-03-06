#pragma OPENCL EXTENSION cl_arm_printf : enable

#define BLOCK_SIZE 16

//  KEY:
//
//  o-------> dimension 0, x, col, width
//  |
//  |
//  |
//  v
//  dimension 1, y, rows, height

// OpenCL Kernel - naive
__kernel void matrixMul(__global float* A, 
                        __global float* B,
						__global float* C,
						int wA,
						int wB)
{
   const int tx = get_global_id(0); //col
   const int ty = get_global_id(1); //row
   float sum = 0.0f;

   for (int k = 0; k < wA; ++k)
   {
      float elementA = A[ty * wA + k];
      float elementB = B[k * wB + tx];
      sum += elementA * elementB;
   }
 
   C[ty * wB + tx] = sum;
}

/*//vectorized - float4
__kernel void matrixMulVec4(__global float* A, 
                            __global float* B,
						    __global float* C,
						    int wA,
						    int wB)
{
    const int i = get_global_id(1); //row
    const int j = get_global_id(0); //col
    float4 sum = (float4)0.0f;
    float4 Bcol;

    A += i * wA;
    uint bOffset = j;

    for (int k = 0; k < wA; k+=4)
    {
        Bcol.x = B[bOffset];
        bOffset += wB;

        Bcol.y = B[bOffset];
        bOffset += wB;

        Bcol.z = B[bOffset];
        bOffset += wB;

        Bcol.w = B[bOffset];
        bOffset += wB;

        sum += vload4(0, A) * Bcol;
        A += 4;
    }

    C[i * wB + j] = sum.x + sum.y + sum.z + sum.w;
    //C[i * wB + j] = dot(sum, (float4)(1.0f, 1.0f, 1.0f, 1.0f)); //must be tested in detail
}*/

/*//vectorized - float4
__kernel void matrixMulVec4(__global float* A, 
                            __global float* B,
						    __global float* C,
						    int wA,
						    int wB)
{
    uint j = get_global_id(0) ;
    uint i = get_global_id(1) ;

    float4 accum = (float4) 0.0f ;
    for(uint k = 0; k < wA; k+=4)
    {
    	uint AOffSet = i * wA + k;
   		float4 a  = vload4(AOffSet, A):
   		A+=4;

   		uint BOffset = (k + 0) * wB + j;
    	float4 b0 = vload4(BOffSet, B);

    	BOffset += wB;
    	float4 b1 = vload4(BOffSet, B);
    	
    	BOffset += wB;
    	float4 b2 = vload4(BOffSet, B);
    	
    	BOffset += wB;
    	float4 b3 = vload4(BOffSet, B);

    	accum += a.s0 * b0 + a.s1 * b1 + a.s2 * b2 + a.s3 * b3;
    }

    vstore4(accum, i * wB + j, C);
}*/

__kernel void matrixMulVec4(__global float4 const *A, 
                            __global float4 const *B,
                            __global float4 *C,
                            int wA,
                            int wB)
{
    __global float4* A4 = (__global float4*)A;
    __global float4* B4 = (__global float4*)B;
    __global float4* C4 = (__global float4*)C;
    uint nv4 = wA / 4;
    
    uint j = get_global_id(0);
    uint i = get_global_id(1);
    float4 accum = (float4) 0.0f;
    for(uint k = 0; k < nv4; ++k)
    {
        float4 a  = A4 [     i * nv4 + k];
        float4 b0 = B4 [ (4*k+0) * nv4 + j];
        float4 b1 = B4 [ (4*k+1) * nv4 + j];
        float4 b2 = B4 [ (4*k+2) * nv4 + j];
        float4 b3 = B4 [ (4*k+3) * nv4 + j];
        accum += a.s0*b0 + a.s1*b1 + a.s2*b2 + a.s3*b3;
    }
    C4[i*nv4 + j] = accum;
}

/*2048 dahil olmak uzere artan boylarda calismiyor....*/

//vectorized - float8
__kernel void matrixMulVec8(__global float* A, 
                            __global float* B,
						    __global float* C,
						    int wA,
						    int wB)
{
    const int i = get_global_id(1);
    const int j = get_global_id(0);
    float8 sum = (float8)0.0f;
    float8 Bcol;

    A += i * wA;
    uint bOffset = j;

    for (int k = 0; k < wA; k+=8)
    {
        Bcol.s0 = B[bOffset];
        bOffset += wB;

        Bcol.s1 = B[bOffset];
        bOffset += wB;

        Bcol.s2 = B[bOffset];
        bOffset += wB;

        Bcol.s3 = B[bOffset];
        bOffset += wB;

        Bcol.s4 = B[bOffset];
        bOffset += wB;

        Bcol.s5 = B[bOffset];
        bOffset += wB;

        Bcol.s6 = B[bOffset];
        bOffset += wB;

        Bcol.s7 = B[bOffset];
        bOffset += wB;

        sum += vload8(0, A) * Bcol;
        A += 8;
    }

    C[i * wB + j] = sum.s0 + sum.s1 + sum.s2 + sum.s3 + sum.s4 + sum.s5 + sum.s6 + sum.s7;
    //C[i * wB + j] = dot(sum, (float8)(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f)); //compiler could find float8 version...
}

/*
__kernel void matrixMulVec8(__global float8 const *A, 
                            __global float8 const *B,
                            __global float8 *C,
                            int wA,
                            int wB)
{
    __global float8* A8 = (__global float8*)A;
    __global float8* B8 = (__global float8*)B;
    __global float8* C8 = (__global float8*)C;
    uint nv8 = wA / 8;
    
    uint j = get_global_id(0);
    uint i = get_global_id(1);
    float8 accum = (float8) 0.0f;
    for(uint k = 0; k < nv8; ++k)
    {
        float8 a  = A8 [     i * nv8 + k];
        float8 b0 = B8 [ (8*k+0) * nv8 + j];
        float8 b1 = B8 [ (8*k+1) * nv8 + j];
        float8 b2 = B8 [ (8*k+2) * nv8 + j];
        float8 b3 = B8 [ (8*k+3) * nv8 + j];
        float8 b4 = B8 [ (8*k+4) * nv8 + j];
        float8 b5 = B8 [ (8*k+5) * nv8 + j];
        float8 b6 = B8 [ (8*k+6) * nv8 + j];
        float8 b7 = B8 [ (8*k+7) * nv8 + j];
        accum += a.s0*b0 + a.s1*b1 + a.s2*b2 + a.s3*b3 + a.s4*b4 + a.s5*b5 + a.s6*b6 + a.s7*b7;
    }
    C8[i*nv8 + j] = accum;
}
*/
__kernel void
matrixMulDiscrete(__global float* A, 
          __global float* B, 
          __global float* C, int wA, int wB)
{
    // Block index
    int bx = get_group_id(0);
    int by = get_group_id(1);
 
    // Thread index
    int tx = get_local_id(0);
    int ty = get_local_id(1);
 
    // Index of the first sub-matrix of A processed 
    // by the block
    int aBegin = wA * BLOCK_SIZE * by;
 
    // Index of the last sub-matrix of A processed 
    // by the block
    int aEnd   = aBegin + wA - 1;
 
    // Step size used to iterate through the 
    // sub-matrices of A
    int aStep  = BLOCK_SIZE;
 
    // Index of the first sub-matrix of B processed 
    // by the block
    int bBegin = BLOCK_SIZE * bx;
 
    // Step size used to iterate through the 
    // sub-matrices of B
    int bStep  = BLOCK_SIZE * wB;

    float Csub = 0.0f;
 
    // Loop over all the sub-matrices of A and B
    // required to compute the block sub-matrix
    for (int a = aBegin, b = bBegin;
             a <= aEnd;
             a += aStep, b += bStep) 
    {

        // Declaration of the local memory array As 
        // used to store the sub-matrix of A
        __local float As[BLOCK_SIZE][BLOCK_SIZE];
 
        // Declaration of the local memory array Bs 
        // used to store the sub-matrix of B
        __local float Bs[BLOCK_SIZE][BLOCK_SIZE];
 
        // Load the matrices from global memory
        // to local memory; each thread loads
        // one element of each matrix
        As[ty][tx] = A[a + wA * ty + tx];
        Bs[ty][tx] = B[b + wB * ty + tx];
 
        // Synchronize to make sure the matrices 
        // are loaded
        barrier(CLK_LOCAL_MEM_FENCE);
 
        // Multiply the two matrices together;
        // each thread computes one element
        // of the block sub-matrix
        for (int k = 0; k < BLOCK_SIZE; ++k)
            Csub += As[ty][k] * Bs[k][tx];
 
        // Synchronize to make sure that the preceding
        // computation is done before loading two new
        // sub-matrices of A and B in the next iteration
        barrier(CLK_LOCAL_MEM_FENCE);
 
    }
 
    // Write the block sub-matrix to device memory;
    // each thread writes one element
    int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;
    C[c + wB * ty + tx] = Csub;

}