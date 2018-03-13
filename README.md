# BenchPar - A benchmark utility with parallelization capabilities

BenchPar is a commandline utility that make benchmarks on three different algorithms; matrix, scan and convolution, using both cpu and gpu parallelization capabilities of the host system. Beside it can also benchmark power consumption if necessary modules exists in the system.


### Requirements

Basically binutils is the only dependency that the utility needs, however based on the configuration and working mode, additional modules, devices and cpu capabilities should be supported by the system.

### Compilation

The compile process can be divided into two parts; 

- **Native** : Basically first it is needed to precompile for makefile generation with cmake tool. Then standard Make compile can be processed.

```
~/bankor $ mkdir build
~/bankor $ cd build
~/bankor/build $ cmake ..
~/bankor/build $ make
```

- **CrossCompile** : If the compilation aims for nodes running on Linux based embedded computers then external toolchain (currently only arm supported) and GNU make based compile platform is used. Toolchain path should be updated in Makefile with **TOOLCHAIN_PATH** definition. It is only needed to define the target platform through parameter

```
~/bankor $ make pi 			#Raspberry pi v1
~/bankor $ make c15			#Arm Cortex A15 optimized compilation
~/bankor $ make c9			#Arm Cortex A9 optimized compilation
~/bankor $ make c8			#Arm Cortex A8 optimized compilation
~/bankor $ make c7			#Arm Cortex A7 optimized compilation
~/bankor $ make arm			#Generic Arm compilation
~/bankor $ make cmd			#Console based compilation for x86_64
```

### Execution

Utility can be executed from any linux shell. It has three main working modes;

- **Benchmark** : Runs the predefined algorithms with several options.
- **Query** 	: Checks the GPU capability. Triggered with **-q** parameter.
- **Help**      : Prints the parameter usage. Triggered with **-h** parameter.
- **Create**    : Creates sample files to be tested with algorithms. Triggered with **-c** parameter. 


#### Benchmark mode parameters

- **Algorithm** : Benchmark can be run using three different algorithms. Algorithm can be selected with **-a** parameter together with the desired algorithm parameter; ***m*** for matrix, ***s*** for scan and ***c*** for convolution 

- **Print** : Algorithm outputs can be saved to a file if enabled. Print state can be selected with **-p** parameter together with the ***on*** or ***off*** parameter. Disabled by default.

- **Timestamp** : Normally only the minimum, maximum and total times spend on the whole benchmark is logged, with this parameter timestamps in every repeat state can also be logged. Timestamp state can be selected with **-t** parameter together with the ***on*** or ***off*** parameter. Disabled by default.

- **Repeat** : Specifies the repeat count of the selected algorithm. Limited to max 2000000. Default count is 1.

- **Power** : Enables the power consumption measurements of the running becnhmark. Five different power measurement types are supported. Default state is ***off***
	- ***msr***: Initiates the power consumption measurement through MSR registers of the CPU.
	- ***perf***: Initiates the power consumption measurement with PERF events in the RAPL Domains.
	- ***ina***: Initiates the power consumption measurement with Texas Instruments INA231 chipset. (Used HardKernels XU4 board)
	- ***spc***: Initiates the power consumption measurement using HardKernels Smart Power with calculation method using the data gathered in every 250ms.
	- ***spr***: Initiates the power consumption measurement using HardKernels Smart Power with using devices' own calculation unit.
	- ***off***: Disables the power consumption measurement.

- **Run Mode**: One of the predefined algorithm modes can be selected with this parameter. Values are defined between ***0-6***
	- ***matrix***: 
		- CPU based calculations -> 
			- 0 : standard, 
			- 1 : tile based (tile size: 32), 
			- 2 : other tile based, (tile size: 32), 
			- 3 : tile based with openmp enabled. (tile size: 32), 
		- GPU based calculations -> 
			- 4 : standard, 
			- 5 : vectorized with float size 4, 
			- 6 : and vectorized with float size 8.
	- ***scan***: 
		- CPU based calculations -> 
			- 0 : standard, 
			- 1 : using intels AVX registers, 
			- 2 : using intels SSE registers, 
			- 3 : using intels SSE registers with openmp enabled. 
		- GPU based calculations -> 
			- 4 : standard
	- ***convolution***: 
		- CPU based calculations -> 
			- 0 : standard,
			- 1 : openmp enabled. 
		- GPU based calculations -> 
			- 2 : standard using same worksizes for rows and columns, 
			- 3 : vectorized with float size 4, 
			- 4 : using separate worksizes for rows and columns.

- **Sequencial Mode**: Run multiple modes sequencially with parameters;
 	- ***a*** : run all modes
 	- ***c*** : run only cpu specific modes
	- ***g*** : run only gpu specific modes

- **Sanity Mode**: Runs other mode different from the main running mode and compares the results if they are same. Used mostly for debug purposes to test the algorithm working correctly. If main running mode selected as **a** or **c** or **g** then sanity mode is forced to disabled. Default mode is disabled.

#### Create mode parameters

Each algorithm has its own creation process.

- ***matrix***: Needs print ID, row and column sizes as parameter.
- ***scan***: Needs print ID and size as parameter.
- ***convolution***: Needs print ID, row and column sizes as parameter.

#### Algorithm specific parameters
Currently only convolution algorithm can get external parameter. To use different filters other than the default one, **-f** parameter can be used to specify the filter file.

### Samples
```
$ benchpar -a m -m 3 -o ina -r 10 1 
/* Utility runs in matrix mode with tile based and openmp enabled, 
 * power measurement is done with ina231, 
 * repeats 10 times and uses matrix/MatrixInput_1 as input.
 */

$ benchpar -a s -m 1 -o msr -r 1000 2 
/* Utility runs in scan mode with using intels AVX registers,
 * power measurement is done with msr registers, 
 * repeats 1000 times and uses scan/ScanInput_2 as input.
 */

 $ benchpar -a c -m 3 4 
/* Utility runs in convolution mode with using gpu vectorized with float size 4,
 * power measurement is disabled, 
 * repeats 1 time and uses conv/ConvInput_4 as input.
 */

 $ benchpar -a m -m a -o spr -r 5 
/* Utility runs in matrix mode with running all modes sequentially, 
 * power measurement is done with smart power using devices' own calculation unit.
 * repeats 5 times and benchmark all files in matrix directory.
 */

  $ benchpar -a s -m c -o perf -p on
/* Utility runs in scan mode with running all cpu only modes sequentially, 
 * power measurement is done with PERF events in the RAPL Domains,
 * print outputs to debug file enabled
 * repeats 1 time and benchmark all files in scan directory.
 */

  $ benchpar -a c -m g -f 1 2 
/* Utility runs in convolution mode with running all gpu only modes sequentially, 
 * power measurement is disabled.
 * repeats 1 time and uses conv/FilterInput_1 as filter and conv/ConvInput_2 as input.
 */

  $ benchpar -a s -m 3 -t on -r 5 1 2 
/* Utility runs in scan mode with using intels SSE registers with openmp enabled. 
 * power measurement is disabled and timestamp in repeats enabled.
 * repeats 5 times and uses scan/ScanInput_1 and scan/ScanInput_2 as inputs.
 */

  $ benchpar -a c -m 3 -s 0 5 
/* Utility runs in convolution mode with using gpu vectorized with float size 4,
 * power measurement is disabled, 
 * also runs cpu standard mode and compares the results with the gpu vectorized mode.
 * repeats 1 time and uses conv/ConvInput_5 as input.
 */

 $ benchpar -a c -c 3 20 80 
/* Utility creates input file for convolution algorithm,
 * with dimensions row : 20, column : 80
 * saves the file as conv/ConvInput_3.
 */

 $ benchpar -a s -c 5 30 
/* Utility creates input file for scan algorithm,
 * with size : 30,
 * saves the file as scan/ScanInput_5.
 */
```

### Contributing

You are welcome to contribute to this project in all manner, involving in the development, making suggestions, notifying of wrong or missing design related parts or testing.

### License

Project is licensed under [GNU Affero General Public License v3.0](LICENSE)