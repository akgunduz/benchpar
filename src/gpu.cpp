//
//  gpu.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "gpu.h"

GPU::GPU() {

	enabled = platformInit();

	if (enabled) {
		getInfo();
	} else {
		printf("No GPU devices found!!!\n");
	}
}


GPU::~GPU() {

	if (enabled) {
		platformDeInit();
	}

}

bool GPU::platformInit() {

	cl_int errCode;

	errCode = clGetPlatformIDs(0, NULL, &numPlatforms);
	checkErr("clGetPlatformIDs", errCode);
	if (errCode != CL_SUCCESS) {
		return false;
	}

	platforms = new cl_platform_id[numPlatforms];
	errCode = clGetPlatformIDs(numPlatforms, platforms, NULL);
	checkErr("clGetPlatformIDs", errCode);
	if (errCode != CL_SUCCESS) {
		delete[] platforms;
		return false;
	}

	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties) platforms[0], 0};

	clGPUContext = clCreateContextFromType(properties,
			CL_DEVICE_TYPE_GPU,
			NULL, NULL, &errCode);

	checkErr("clCreateContextFromType", errCode);
	if (errCode != CL_SUCCESS) {
		delete[] platforms;
		return false;
	}

	errCode = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	checkErr("clGetDeviceIDs", errCode);
	if (numDevices == 0 || errCode != CL_SUCCESS) {
		printf("No GPU Devices found, GPU tests will not be run!!!");
		delete[] platforms;
		clReleaseContext(clGPUContext);
		return false;
	}

	clDevices = new cl_device_id[numDevices];

	errCode = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices, clDevices, NULL);
	checkErr("clGetDeviceIDs", errCode);
	if (errCode != CL_SUCCESS) {
		delete[] platforms;
		delete[] clDevices;
		clReleaseContext(clGPUContext);
		return false;
	}

	return true;
}

bool GPU::platformDeInit() {

	delete[] platforms;
	delete[] clDevices;

	clReleaseContext(clGPUContext);

	return false;
}

//OCL program build - from file
bool GPU::createBuildProgramFromFile(int deviceIndex, const char* buildOptions,	const char* fileName) {

	cl_int errNum;

	//create programs
	FILE  *fProgramHandle;
	size_t sProgramSize;
	char *cProgramBuffer;

	//read program file and learn the length of it
	fProgramHandle = fopen(fileName, "rb");
	if(fProgramHandle <= 0)
	{
		printf("cannot read file \n");
		return false;
	}
	fseek(fProgramHandle, 0, SEEK_END);
	sProgramSize = ftell(fProgramHandle);
	rewind(fProgramHandle);
	cProgramBuffer = (char*)malloc(sProgramSize + 1);

	int res = fread(cProgramBuffer, sizeof(char), sProgramSize, fProgramHandle);

	fclose(fProgramHandle);

	if (res == 0) {
		printf("cannot read kernel data \n");
		return false;
	}

	cProgramBuffer[sProgramSize] = '\0';

	//create program using source
	clProgram = clCreateProgramWithSource(clGPUContext,
			1, (const char **)&cProgramBuffer,
			&sProgramSize, &errNum);

	free(cProgramBuffer);

	if(errNum != CL_SUCCESS) {
		printf("Failed to create CL program from source.\n");
		checkErr("", errNum);
		return false;
	}

	//build program
	errNum = clBuildProgram(clProgram, 1, &clDevices[deviceIndex], buildOptions, NULL,	NULL);

	char cBuildLog[16384];

	if(errNum != CL_SUCCESS) {

		printf("cannot build program %d !\n", errNum);

/*		clGetProgramBuildInfo(clProgram,
				clDevices[deviceIndex],
				CL_PROGRAM_BUILD_LOG,
				0,
				NULL,
				&sLogSize);

		cBuildLog = (char*) malloc(sLogSize + 1);
*/
		errNum = clGetProgramBuildInfo(clProgram,
				clDevices[deviceIndex],
				CL_PROGRAM_BUILD_LOG,
				sizeof(cBuildLog),
				cBuildLog,
				NULL);

		printf("%s \n",cBuildLog);

		clReleaseProgram(clProgram);

		return false;
	}

	return true;
}

//OCL program build - from const char array
bool GPU::createBuildProgram(int deviceIndex,
		const char* buildOptions, const char* source)
{
	cl_int errNum;

	//create program using source
	clProgram = clCreateProgramWithSource(clGPUContext,
			1, &source,	NULL, &errNum);

	if(errNum != CL_SUCCESS) {
		printf("Failed to create CL program from source.\n");
		checkErr("", errNum);
		return false;
	}

	//build program
	errNum = clBuildProgram(clProgram,
			1, &clDevices[deviceIndex],
			buildOptions, NULL,	NULL);

	char *cBuildLog;
	size_t sLogSize;
	if(errNum != CL_SUCCESS)
	{
		printf("cannot build program %d !\n", errNum);

		clGetProgramBuildInfo(clProgram,
				clDevices[deviceIndex],
				CL_PROGRAM_BUILD_LOG,
				0,
				NULL,
				&sLogSize);

		cBuildLog = (char*) malloc(sLogSize + 1);

		clGetProgramBuildInfo(clProgram,
				clDevices[deviceIndex],
				CL_PROGRAM_BUILD_LOG,
				sLogSize+1,
				cBuildLog,
				NULL);

		printf("%s \n",cBuildLog);

		free(cBuildLog);

		clReleaseProgram(clProgram);

		return false;
	}

	return true;
}

bool GPU::createCommandQueue(int deviceIndex, cl_command_queue_properties properties) {

	cl_int errCode;

	clCommandQue = clCreateCommandQueue(clGPUContext,
			clDevices[deviceIndex], properties, &errCode);

	checkErr("clCreateCommandQueue", errCode);

	return errCode == CL_SUCCESS;
}

void GPU::getInfo() {

	char buffer[256];
	clGetDeviceInfo(clDevices[0], CL_DEVICE_VENDOR, 256, buffer, NULL);
	printf("GPU %s is found with %d devices : ", buffer, numDevices);
	for (int i = 0; i < numDevices; i++) {
		clGetDeviceInfo(clDevices[i], CL_DEVICE_NAME, 256, buffer, NULL);
		printf("%s, ", buffer);
	}
	printf("\n");
}

void GPU::platformQuery() {

	cl_int errCode;

	size_t size;
	for(unsigned int i=0; i<numPlatforms; i++)
	{
		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &size);
		char* name = (char*)malloc(sizeof(char) * size);
		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, size, name, NULL);

		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 0, NULL, &size);
		char* vname = (char*)malloc(sizeof(char) * size);
		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, size, vname, NULL);

		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, 0, NULL, &size);
		char* prof = (char*)malloc(sizeof(char) * size);
		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, size, prof, NULL);

		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, 0, NULL, &size);
		char* vers = (char*)malloc(sizeof(char) * size);
		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, size, vers, NULL);

		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 0, NULL, &size);
		char* exts = (char*)malloc(sizeof(char) * size);
		errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, size, exts, NULL);

		printf("Platform properties for platform #%d: \n", (i+1));
		printf("------------------------------------\n");
		printf("Platform name: %s \n", name);
		printf("Vendor name: %s \n", vname);
		printf("Profile: %s \n", prof);
		printf("Version: %s \n", vers);
		printf("Extensions: %s \n\n\n", exts);

		free(name);
		free(vname);
		free(prof);
		free(vers);
		free(exts);
	}

	cl_uint attribute_ui;
	size_t attribute_st;
	cl_ulong mem_size;
	cl_bool available;

	for(unsigned int i=0; i<numDevices; i++)
	{
		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_IMAGE_SUPPORT, 0, NULL, &size);
		bool* imageSupport = new bool[size];
		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_IMAGE_SUPPORT, size, imageSupport, NULL);
		printf("Image support: ");
		for(unsigned int j=0; j<size; j++) {
			printf("%d ", ((imageSupport[j]) ? 1 : 0));
		}
		printf("\n\n\n");
		delete[] imageSupport;

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_NAME, 0, NULL, &size);
		char *info = (char*)malloc(size);
		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_NAME, size, info, NULL);
		printf("CL_DEVICE_NAME is %s\n",info);
		free(info);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_VENDOR, 0, NULL, &size);
		info = (char*)malloc(size);
		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_VENDOR, size, info, NULL);
		printf("CL_DEVICE_VENDOR is %s\n",info);
		free(info);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_EXTENSIONS, 0, NULL, &size);
		info = (char*)malloc(size);
		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_EXTENSIONS, size, info, NULL);
		printf("CL_DEVICE_EXTENSIONS is \n%s\n",info);
		free(info);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &attribute_ui, NULL);
		printf("CL_DEVICE_MAX_COMPUTE_UNITS is %u\n",attribute_ui);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &attribute_ui, NULL);
		unsigned int work_dims = attribute_ui;
		printf("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS is %u\n",attribute_ui);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &attribute_st, NULL);
		printf("CL_DEVICE_MAX_WORK_GROUP_SIZE is %ld\n",attribute_st);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &size);
		size_t *item_info = (size_t *)malloc(size);
		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, size, item_info, NULL);
		for(int p=0; p < (int)work_dims; p++)
			printf("CL_DEVICE_MAX_WORK_ITEM_SIZE in dim %d is %ld\n",p,item_info[p]);
		free(item_info);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem_size, NULL);
		printf("CL_DEVICE_GLOBAL_MEM_SIZE is %lu MBs\n", (unsigned long)mem_size/1024/1024);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &attribute_ui, NULL);
		printf("CL_DEVICE_ADDRESS_BITS is %u\n",attribute_ui);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_AVAILABLE, sizeof(cl_uint), &available, NULL);
		printf("CL_DEVICE_AVAILABLE is %d\n",available?1:0);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_COMPILER_AVAILABLE, sizeof(cl_uint), &available, NULL);
		printf("CL_DEVICE_COMPILER_AVAILABLE is %d\n",available?1:0);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_uint), &available, NULL);
		printf("CL_DEVICE_ENDIAN_LITTLE is %d\n",available?1:0);

		errCode = clGetDeviceInfo(clDevices[i], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &mem_size, NULL);
		printf("CL_DEVICE_LOCAL_MEM_SIZE is %lu KBs\n", (unsigned long)mem_size/1024);

	}

}

bool GPU::getEnabled() {

	return enabled;
}

void GPU::checkErr(const char *id, cl_int err)
{
	switch(err)
	{
		case CL_SUCCESS:                                    break;
		case CL_DEVICE_NOT_FOUND:                           printf("%s -> CL_DEVICE_NOT_FOUND\n", id); break;
		case CL_DEVICE_NOT_AVAILABLE:                       printf("%s -> CL_DEVICE_NOT_AVAILABLE\n", id); break;
		case CL_COMPILER_NOT_AVAILABLE:                     printf("%s -> CL_COMPILER_NOT_AVAILABLE\n", id); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:              printf("%s -> CL_MEM_OBJECT_ALLOCATION_FAILURE\n", id); break;
		case CL_OUT_OF_RESOURCES:                           printf("%s -> CL_OUT_OF_RESOURCES\n", id); break;
		case CL_OUT_OF_HOST_MEMORY:                         printf("%s -> CL_OUT_OF_HOST_MEMORY\n", id); break;
		case CL_PROFILING_INFO_NOT_AVAILABLE:               printf("%s -> CL_PROFILING_INFO_NOT_AVAILABLE\n", id); break;
		case CL_MEM_COPY_OVERLAP:                           printf("%s -> CL_MEM_COPY_OVERLAP\n", id); break;
		case CL_IMAGE_FORMAT_MISMATCH:                      printf("%s -> CL_IMAGE_FORMAT_MISMATCH\n", id); break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:                 printf("%s -> CL_IMAGE_FORMAT_NOT_SUPPORTED\n", id); break;
		case CL_BUILD_PROGRAM_FAILURE:                      printf("%s -> CL_BUILD_PROGRAM_FAILURE\n", id); break;
		case CL_MAP_FAILURE:                                printf("%s -> CL_MAP_FAILURE\n", id); break;
		case CL_INVALID_VALUE:                              printf("%s -> CL_INVALID_VALUE\n", id); break;
		case CL_INVALID_DEVICE_TYPE:                        printf("%s -> CL_INVALID_DEVICE_TYPE\n", id); break;
		case CL_INVALID_PLATFORM:                           printf("%s -> CL_INVALID_PLATFORM\n", id); break;
		case CL_INVALID_DEVICE:                             printf("%s -> CL_INVALID_DEVICE\n", id); break;
		case CL_INVALID_CONTEXT:                            printf("%s -> CL_INVALID_CONTEXT\n", id); break;
		case CL_INVALID_QUEUE_PROPERTIES:                   printf("%s -> CL_INVALID_QUEUE_PROPERTIES\n", id); break;
		case CL_INVALID_COMMAND_QUEUE:                      printf("%s -> CL_INVALID_COMMAND_QUEUE\n", id); break;
		case CL_INVALID_HOST_PTR:                           printf("%s -> CL_INVALID_HOST_PTR\n", id); break;
		case CL_INVALID_MEM_OBJECT:                         printf("%s -> CL_INVALID_MEM_OBJECT\n", id); break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:            printf("%s -> CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n", id); break;
		case CL_INVALID_IMAGE_SIZE:                         printf("%s -> CL_INVALID_IMAGE_SIZE\n", id); break;
		case CL_INVALID_SAMPLER:                            printf("%s -> CL_INVALID_SAMPLER\n", id); break;
		case CL_INVALID_BINARY:                             printf("%s -> CL_INVALID_BINARY\n", id); break;
		case CL_INVALID_BUILD_OPTIONS:                      printf("%s -> CL_INVALID_BUILD_OPTIONS\n", id); break;
		case CL_INVALID_PROGRAM:                            printf("%s -> CL_INVALID_PROGRAM\n", id); break;
		case CL_INVALID_PROGRAM_EXECUTABLE:                 printf("%s -> CL_INVALID_PROGRAM_EXECUTABLE\n", id); break;
		case CL_INVALID_KERNEL_NAME:                        printf("%s -> CL_INVALID_KERNEL_NAME\n", id); break;
		case CL_INVALID_KERNEL_DEFINITION:                  printf("%s -> CL_INVALID_KERNEL_DEFINITION\n", id); break;
		case CL_INVALID_KERNEL:                             printf("%s -> CL_INVALID_KERNEL\n", id); break;
		case CL_INVALID_ARG_INDEX:                          printf("%s -> CL_INVALID_ARG_INDEX\n", id); break;
		case CL_INVALID_ARG_VALUE:                          printf("%s -> CL_INVALID_ARG_VALUE\n", id); break;
		case CL_INVALID_ARG_SIZE:                           printf("%s -> CL_INVALID_ARG_SIZE\n", id); break;
		case CL_INVALID_KERNEL_ARGS:                        printf("%s -> CL_INVALID_KERNEL_ARGS\n", id); break;
		case CL_INVALID_WORK_DIMENSION:                     printf("%s -> CL_INVALID_WORK_DIMENSION\n", id); break;
		case CL_INVALID_WORK_GROUP_SIZE:                    printf("%s -> CL_INVALID_WORK_GROUP_SIZE\n", id); break;
		case CL_INVALID_WORK_ITEM_SIZE:                     printf("%s -> CL_INVALID_WORK_ITEM_SIZE\n", id); break;
		case CL_INVALID_GLOBAL_OFFSET:                      printf("%s -> CL_INVALID_GLOBAL_OFFSET\n", id); break;
		case CL_INVALID_EVENT_WAIT_LIST:                    printf("%s -> CL_INVALID_EVENT_WAIT_LIST\n", id); break;
		case CL_INVALID_EVENT:                              printf("%s -> CL_INVALID_EVENT\n", id); break;
		case CL_INVALID_OPERATION:                          printf("%s -> CL_INVALID_OPERATION\n", id); break;
		case CL_INVALID_GL_OBJECT:                          printf("%s -> CL_INVALID_GL_OBJECT\n", id); break;
		case CL_INVALID_BUFFER_SIZE:                        printf("%s -> CL_INVALID_BUFFER_SIZE\n", id); break;
		case CL_INVALID_MIP_LEVEL:                          printf("%s -> CL_INVALID_MIP_LEVEL\n", id); break;
		case CL_INVALID_GLOBAL_WORK_SIZE:                   printf("%s -> CL_INVALID_GLOBAL_WORK_SIZE\n", id); break;
		default:                                            printf("%s -> Unknown\n", id);
	}
}



