//Passed down by clBuildProgram
/*
#define KERNEL_RADIUS 8

#define      ROWS_BLOCKDIM_X 16
#define      ROWS_BLOCKDIM_Y 4
#define   COLUMNS_BLOCKDIM_X 16
#define   COLUMNS_BLOCKDIM_Y 8

#define    ROWS_RESULT_STEPS 4
#define      ROWS_HALO_STEPS 1
#define COLUMNS_RESULT_STEPS 4
#define   COLUMNS_HALO_STEPS 1
*/
#define KERNEL_LENGTH (2 * KERNEL_RADIUS + 1)

////////////////////////////////////////////////////////////////////////////////
// Row convolution filter
////////////////////////////////////////////////////////////////////////////////
__kernel __attribute__((reqd_work_group_size(ROWS_BLOCKDIM_X, ROWS_BLOCKDIM_Y, 1)))
void convolutionRows(
    __global float *d_Dst,
    __global float *d_Src,
    __constant float *c_Kernel,
    int imageW,
    int imageH,
    int pitch)
{
    __local float l_Data[ROWS_BLOCKDIM_Y][(ROWS_RESULT_STEPS + 2 * ROWS_HALO_STEPS) * ROWS_BLOCKDIM_X];

    //Offset to the left halo edge
    const int baseX = (get_group_id(0) * ROWS_RESULT_STEPS - ROWS_HALO_STEPS) * ROWS_BLOCKDIM_X + get_local_id(0);
    const int baseY = get_group_id(1) * ROWS_BLOCKDIM_Y + get_local_id(1);

    d_Src += baseY * pitch + baseX;
    d_Dst += baseY * pitch + baseX;

    //Load main data
	#pragma unroll
    for(int i = ROWS_HALO_STEPS; i < ROWS_HALO_STEPS + ROWS_RESULT_STEPS; i++)
        l_Data[get_local_id(1)][get_local_id(0) + i * ROWS_BLOCKDIM_X] = d_Src[i * ROWS_BLOCKDIM_X];

    //Load left halo
	#pragma unroll
    for(int i = 0; i < ROWS_HALO_STEPS; i++)
        l_Data[get_local_id(1)][get_local_id(0) + i * ROWS_BLOCKDIM_X]  = (baseX + i * ROWS_BLOCKDIM_X >= 0) ? d_Src[i * ROWS_BLOCKDIM_X] : 0;

    //Load right halo
	#pragma unroll
    for(int i = ROWS_HALO_STEPS + ROWS_RESULT_STEPS; i < ROWS_HALO_STEPS + ROWS_RESULT_STEPS + ROWS_HALO_STEPS; i++)
        l_Data[get_local_id(1)][get_local_id(0) + i * ROWS_BLOCKDIM_X]  = (baseX + i * ROWS_BLOCKDIM_X < imageW) ? d_Src[i * ROWS_BLOCKDIM_X] : 0;

    //Compute and store results
    barrier(CLK_LOCAL_MEM_FENCE);
	#pragma unroll
    for(int i = ROWS_HALO_STEPS; i < ROWS_HALO_STEPS + ROWS_RESULT_STEPS; i++){
        float sum = 0;
		#pragma unroll
        for(int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; j++)
            sum += c_Kernel[KERNEL_RADIUS - j] * l_Data[get_local_id(1)][get_local_id(0) + i * ROWS_BLOCKDIM_X + j];

        d_Dst[i * ROWS_BLOCKDIM_X] = sum;
    }
}



////////////////////////////////////////////////////////////////////////////////
// Column convolution filter
////////////////////////////////////////////////////////////////////////////////
__kernel __attribute__((reqd_work_group_size(COLUMNS_BLOCKDIM_X, COLUMNS_BLOCKDIM_Y, 1)))
void convolutionColumns(
    __global float *d_Dst,
    __global float *d_Src,
    __constant float *c_Kernel,
    int imageW,
    int imageH,
    int pitch)
{
    __local float l_Data[COLUMNS_BLOCKDIM_X][(COLUMNS_RESULT_STEPS + 2 * COLUMNS_HALO_STEPS) * COLUMNS_BLOCKDIM_Y + 1];

    //Offset to the upper halo edge
    const int baseX = get_group_id(0) * COLUMNS_BLOCKDIM_X + get_local_id(0);
    const int baseY = (get_group_id(1) * COLUMNS_RESULT_STEPS - COLUMNS_HALO_STEPS) * COLUMNS_BLOCKDIM_Y + get_local_id(1);
    d_Src += baseY * pitch + baseX;
    d_Dst += baseY * pitch + baseX;

    //Load main data
	#pragma unroll
    for(int i = COLUMNS_HALO_STEPS; i < COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS; i++)
        l_Data[get_local_id(0)][get_local_id(1) + i * COLUMNS_BLOCKDIM_Y] = d_Src[i * COLUMNS_BLOCKDIM_Y * pitch];

    //Load upper halo
	#pragma unroll
    for(int i = 0; i < COLUMNS_HALO_STEPS; i++)
        l_Data[get_local_id(0)][get_local_id(1) + i * COLUMNS_BLOCKDIM_Y] = (baseY + i * COLUMNS_BLOCKDIM_Y >= 0) ? d_Src[i * COLUMNS_BLOCKDIM_Y * pitch] : 0;

    //Load lower halo
	#pragma unroll
    for(int i = COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS; i < COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS + COLUMNS_HALO_STEPS; i++)
        l_Data[get_local_id(0)][get_local_id(1) + i * COLUMNS_BLOCKDIM_Y]  = (baseY + i * COLUMNS_BLOCKDIM_Y < imageH) ? d_Src[i * COLUMNS_BLOCKDIM_Y * pitch] : 0;

    //Compute and store results
    barrier(CLK_LOCAL_MEM_FENCE);
	#pragma unroll
    for(int i = COLUMNS_HALO_STEPS; i < COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS; i++){
        float sum = 0;
		#pragma unroll
        for(int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; j++)
            sum += c_Kernel[KERNEL_RADIUS - j] * l_Data[get_local_id(0)][get_local_id(1) + i * COLUMNS_BLOCKDIM_Y + j];

        d_Dst[i * COLUMNS_BLOCKDIM_Y * pitch] = sum;
    }
}

kernel void convRows5_float(__global float* restrict out, 
							__global const float* restrict in,
							__constant float* filter,
							const int width,
							const int height,
							const int pitch)
{
	float in1, in2, in3, in4, in5;
	if( (get_global_id(0) > 1 ) && (get_global_id(0) < (width-2)) )
	{
		const int offset = get_global_id(1) * width + get_global_id(0);

		in1 = *( in + (offset - 2));
		in2 = *( in + (offset - 1));
		in3 = *( in + (offset    ));
		in4 = *( in + (offset + 1));
		in5 = *( in + (offset + 2));

		*(out + offset) = (in1*filter[0]
		                 + in2*filter[1]
						 + in3*filter[2]
						 + in4*filter[3]
						 + in5*filter[4]);
	}
}

kernel void convCols5_float(__global float * restrict out, 
							__global const float * restrict in,
							__constant float * filter,
							const int width,
							const int height,
							const int pitch)
{
	float in1, in2, in3, in4, in5;
	if( (get_global_id(1) > 1 ) && (get_global_id(1) < (height-2)) )
	{
		const int offset = get_global_id(1) * width + get_global_id(0);

		in1 = *( in + (offset - 2*width));
		in2 = *( in + (offset -   width));
		in3 = *( in + (offset          ));
		in4 = *( in + (offset +   width));
		in5 = *( in + (offset + 2*width));

		*(out + offset) = (in1*filter[0]
		                 + in2*filter[1]
						 + in3*filter[2]
						 + in4*filter[3]
						 + in5*filter[4]);
	}
}

kernel void convRows5Vec4_float(__global float4* restrict out,
								__global const float4* restrict in,
								__constant float* filter,
								const int width,
								const int height,
								const int pitch)
{
	float4 in1, in2, in3, temp;
	const int width4 = width/4;

	if( (get_global_id(0) > 0 ) && (get_global_id(0) < (width4-1)) )
	{
		const int offset = get_global_id(1) * width4 + get_global_id(0);
		
		in1 = *( in + (offset - 1));
		in2 = *( in + (offset    ));
		in3 = *( in + (offset + 1));	

		temp.x = in1.z * filter[0]
		       + in1.w * filter[1]
			   + in2.x * filter[2]
			   + in2.y * filter[3]
			   + in2.z * filter[4];

		temp.y = in1.w * filter[0]
		       + in2.x * filter[1]
			   + in2.y * filter[2]
			   + in2.z * filter[3]
			   + in2.w * filter[4];

		temp.z = in2.x * filter[0]
		       + in2.y * filter[1]
			   + in2.z * filter[2]
			   + in2.w * filter[3]
			   + in3.x * filter[4];

		temp.w = in2.y * filter[0]
		       + in2.z * filter[1]
			   + in2.w * filter[2]
			   + in3.x * filter[3]
			   + in3.y * filter[4];

		*(out + offset) = temp;
	}
}

kernel void convCols5Vec4_float(__global float4 * restrict out,
								__global const float4 * restrict in,
								__constant float * filter,
								const int width,
								const int height,
								const int pitch)
{
	float4 in1, in2, in3, in4, in5, temp;
	const int width4 = width/4;
	if( (get_global_id(1) > 1) && (get_global_id(1) < (height-2)) )
	{
		const int offset = get_global_id(1) * width4 + get_global_id(0);

		in1 = *( in + (offset - 2*width4));
		in2 = *( in + (offset -   width4));
		in3 = *( in + (offset           ));
		in4 = *( in + (offset +   width4));
		in5 = *( in + (offset + 2*width4));
		
		temp.x = in1.x * filter[0]
		       + in2.x * filter[1]
			   + in3.x * filter[2]
			   + in4.x * filter[3]
			   + in5.x * filter[4];

		temp.y = in1.y * filter[0]
		       + in2.y * filter[1]
			   + in3.y * filter[2]
			   + in4.y * filter[3]
			   + in5.y * filter[4];

		temp.z = in1.z * filter[0]
		       + in2.z * filter[1]
			   + in3.z * filter[2]
			   + in4.z * filter[3]
			   + in5.z * filter[4];
			   
		temp.w = in1.w * filter[0]
		       + in2.w * filter[1]
			   + in3.w * filter[2]
			   + in4.w * filter[3]
			   + in5.w * filter[4];

		*(out + offset) = temp;
	}
}

kernel void convRows5Vec4VI_float(__global float* restrict out,
								  __global const float* restrict in,
								  __constant float* filter,
								  const int width,
								  const int height,
								  const int pitch)
{
	float4 in1, in2, in3, temp;
	if( (get_global_id(0) > 0 ) && (get_global_id(0) < ((width/4)-1)) )
	{
		const int offset = get_global_id(1) * width + 4*get_global_id(0);
	
		in1 = vload4(0, in + (offset - 4));
		in2 = vload4(0, in + (offset    ));
		in3 = vload4(0, in + (offset + 4));

		temp.x = in1.z * filter[0]
		       + in1.w * filter[1]
			   + in2.x * filter[2]
			   + in2.y * filter[3]
			   + in2.z * filter[4];

		temp.y = in1.w * filter[0]
		       + in2.x * filter[1]
			   + in2.y * filter[2]
			   + in2.z * filter[3]
			   + in2.w * filter[4];

		temp.z = in2.x * filter[0]
		       + in2.y * filter[1]
			   + in2.z * filter[2]
			   + in2.w * filter[3]
			   + in3.x * filter[4];

		temp.w = in2.y * filter[0]
		       + in2.z * filter[1]
			   + in2.w * filter[2]
			   + in3.x * filter[3]
			   + in3.y * filter[4];

	    vstore4(temp, 0, (out + offset));
	}
}

kernel void convCols5Vec4VI_float(__global float * restrict out,
								  __global const float * restrict in,
								  __constant float * filter,
								  const int width,
								  const int height,
								  const int pitch)
{
	float4 in1, in2, in3, in4, in5, temp;
	if( (get_global_id(1) > 1) && (get_global_id(1) < (height-2)) )
	{
		const int offset = get_global_id(1) * width + 4*get_global_id(0);

		in1 = vload4(0, in + (offset - 2*width));
		in2 = vload4(0, in + (offset -   width));
		in3 = vload4(0, in + (offset          ));
		in4 = vload4(0, in + (offset +   width));
		in5 = vload4(0, in + (offset + 2*width));
		
		temp.x = in1.x * filter[0]
		       + in2.x * filter[1]
			   + in3.x * filter[2]
			   + in4.x * filter[3]
			   + in5.x * filter[4];

		temp.y = in1.y * filter[0]
		       + in2.y * filter[1]
			   + in3.y * filter[2]
			   + in4.y * filter[3]
			   + in5.y * filter[4];

		temp.z = in1.z * filter[0]
		       + in2.z * filter[1]
			   + in3.z * filter[2]
			   + in4.z * filter[3]
			   + in5.z * filter[4];
			   
		temp.w = in1.w * filter[0]
		       + in2.w * filter[1]
			   + in3.w * filter[2]
			   + in4.w * filter[3]
			   + in5.w * filter[4];

		vstore4(temp, 0, out + offset);
	}
}

kernel void convCombined5Vec4_float(__global float4* restrict out,
									__global const float4* restrict in,
									__constant float* filter,
									const int width,
									const int height,
									const int pitch)
{
	float4 in_c1, in_c2, in_c3, in_c4, in_c5;
	float4 in_t1, in_t2, in_t3, in_t4, in_t5;
	float4 temp1, temp2, temp3, temp4, temp5;
	//float4 temp_out;

	const int width4 = width/4;
	
	if( (get_global_id(0) > 0 ) && (get_global_id(0) < (width4-1)) &&
		(get_global_id(1) > 1 ) && (get_global_id(1) < (height-2)) )
	{
		const int offset = get_global_id(1) * width4 + get_global_id(0);
		
		in_c1 = *(in + (offset - 2*width4));
		in_c2 = *(in + (offset -   width4));
		in_c3 = *(in + (offset           ));
		in_c4 = *(in + (offset +   width4));
		in_c5 = *(in + (offset + 2*width4));

		in_t1 = *(in + (offset - 2*width4 - 1));
		in_t2 = *(in + (offset -   width4 - 1));
		in_t3 = *(in + (offset            - 1));
		in_t4 = *(in + (offset +   width4 - 1));
		in_t5 = *(in + (offset + 2*width4 - 1));

		//
		temp1.x = in_t1.z * filter[0]
		        + in_t1.w * filter[1]
			    + in_c1.x * filter[2]
			    + in_c1.y * filter[3]
			    + in_c1.z * filter[4];
		
		temp2.x = in_t2.z * filter[0]
		        + in_t2.w * filter[1]
			    + in_c2.x * filter[2]
			    + in_c2.y * filter[3]
			    + in_c2.z * filter[4];

		temp3.x = in_t3.z * filter[0]
		        + in_t3.w * filter[1]
			    + in_c3.x * filter[2]
			    + in_c3.y * filter[3]
			    + in_c3.z * filter[4];

		temp4.x = in_t4.z * filter[0]
		        + in_t4.w * filter[1]
			    + in_c4.x * filter[2]
			    + in_c4.y * filter[3]
			    + in_c4.z * filter[4];

		temp5.x = in_t5.z * filter[0]
		        + in_t5.w * filter[1]
			    + in_c5.x * filter[2]
			    + in_c5.y * filter[3]
			    + in_c5.z * filter[4];
		//
		temp1.y = in_t1.w * filter[0]
		        + in_c1.x * filter[1]
			    + in_c1.y * filter[2]
			    + in_c1.z * filter[3]
			    + in_c1.w * filter[4];

		temp2.y = in_t2.w * filter[0]
		        + in_c2.x * filter[1]
			    + in_c2.y * filter[2]
			    + in_c2.z * filter[3]
			    + in_c2.w * filter[4];

		temp3.y = in_t3.w * filter[0]
		        + in_c3.x * filter[1]
			    + in_c3.y * filter[2]
			    + in_c3.z * filter[3]
			    + in_c3.w * filter[4];

		temp4.y = in_t4.w * filter[0]
		        + in_c4.x * filter[1]
			    + in_c4.y * filter[2]
			    + in_c4.z * filter[3]
			    + in_c4.w * filter[4];

		temp5.y = in_t5.w * filter[0]
		        + in_c5.x * filter[1]
			    + in_c5.y * filter[2]
			    + in_c5.z * filter[3]
			    + in_c5.w * filter[4];
				
		barrier(CLK_GLOBAL_MEM_FENCE);
		
		in_t1 = *(in + (offset - 2*width4 + 1));
		in_t2 = *(in + (offset -   width4 + 1));
		in_t3 = *(in + (offset            + 1));
		in_t4 = *(in + (offset +   width4 + 1));
		in_t5 = *(in + (offset + 2*width4 + 1));

		//
		temp1.z = in_c1.x * filter[0]
		        + in_c1.y * filter[1]
			    + in_c1.z * filter[2]
			    + in_c1.w * filter[3]
			    + in_t1.x * filter[4];

		temp2.z = in_c2.x * filter[0]
		        + in_c2.y * filter[1]
			    + in_c2.z * filter[2]
			    + in_c2.w * filter[3]
			    + in_t2.x * filter[4];

		temp3.z = in_c3.x * filter[0]
		        + in_c3.y * filter[1]
			    + in_c3.z * filter[2]
			    + in_c3.w * filter[3]
			    + in_t3.x * filter[4];
			
		temp4.z = in_c4.x * filter[0]
		        + in_c4.y * filter[1]
			    + in_c4.z * filter[2]
			    + in_c4.w * filter[3]
			    + in_t4.x * filter[4];

		temp5.z = in_c5.x * filter[0]
		        + in_c5.y * filter[1]
			    + in_c5.z * filter[2]
			    + in_c5.w * filter[3]
			    + in_t5.x * filter[4];

		//
		temp1.w = in_c1.y * filter[0]
		        + in_c1.z * filter[1]
			    + in_c1.w * filter[2]
			    + in_t1.x * filter[3]
			    + in_t1.y * filter[4];

		temp2.w = in_c2.y * filter[0]
		        + in_c2.z * filter[1]
			    + in_c2.w * filter[2]
			    + in_t2.x * filter[3]
			    + in_t2.y * filter[4];

		temp3.w = in_c3.y * filter[0]
		        + in_c3.z * filter[1]
			    + in_c3.w * filter[2]
			    + in_t3.x * filter[3]
			    + in_t3.y * filter[4];

		temp4.w = in_c4.y * filter[0]
		        + in_c4.z * filter[1]
			    + in_c4.w * filter[2]
			    + in_t4.x * filter[3]
			    + in_t4.y * filter[4];

		temp5.w = in_c5.y * filter[0]
		        + in_c5.z * filter[1]
			    + in_c5.w * filter[2]
			    + in_t5.x * filter[3]
			    + in_t5.y * filter[4];

		temp1.x = temp1.x * filter[0]
			       + temp2.x * filter[1]
				   + temp3.x * filter[2]
				   + temp4.x * filter[3]
				   + temp5.x * filter[4];
		
		temp1.y = temp1.y * filter[0]
			       + temp2.y * filter[1]
				   + temp3.y * filter[2]
				   + temp4.y * filter[3]
				   + temp5.y * filter[4];

		temp1.z = temp1.z * filter[0]
			       + temp2.z * filter[1]
				   + temp3.z * filter[2]
				   + temp4.z * filter[3]
				   + temp5.z * filter[4];

		temp1.w = temp1.w * filter[0]
			       + temp2.w * filter[1]
				   + temp3.w * filter[2]
				   + temp4.w * filter[3]
				   + temp5.w * filter[4];

		*(out + offset) = temp1;
	}
}
