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

//vectorized - float4
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
}

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