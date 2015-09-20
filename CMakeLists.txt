cmake_minimum_required(VERSION 3.3)
project(BenchTool_Clion)

set(CMAKE_CXX_COMPILER /usr/local/bin/clang-omp++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -mavx -fopenmp")

set(CMAKE_EXE_LINKER_FLAGS "-framework OpenCL -framework IOKit -framework CoreFoundation")

add_definitions(-D__OPENCL__ -D__APPLE__ -D__IDE__)

set(SOURCE_FILES
    src/app/conv/conv.cpp
    src/app/conv/conv.h
    src/app/conv/convapp.cpp
    src/app/conv/convapp.h
    src/app/conv/convfuncs.cpp
    src/app/matrix/matrix.cpp
    src/app/matrix/matrix.h
    src/app/matrix/matrixapp.cpp
    src/app/matrix/matrixapp.h
    src/app/matrix/matrixfuncs.cpp
    src/app/scan/scan.cpp
    src/app/scan/scan.h
    src/app/scan/scanapp.cpp
    src/app/scan/scanapp.h
    src/app/scan/scanfuncs.cpp
    src/app/app.cpp
    src/app/app.h
    src/app/function.cpp
    src/app/function.h
    src/power/hid_apple.cpp
    src/power/hidapi.h
    src/power/power.cpp
    src/power/power.h
    src/power/power_ina.cpp
    src/power/power_ina.h
    src/power/power_msr.cpp
    src/power/power_msr.h
    src/power/power_smart_calc.cpp
    src/power/power_smart_calc.h
    src/power/power_smart_real.cpp
    src/power/power_smart_real.h
    src/common.h
    src/cpu.cpp
    src/cpu.h
    src/gpu.cpp
    src/gpu.h
    src/main.cpp
    src/timer.cpp)

include_directories(src)
include_directories(src/app)
include_directories(src/app/conv)
include_directories(src/app/matrix)
include_directories(src/app/scan)
include_directories(src/power)
include_directories(/usr/local/include/libiomp)

add_executable(benchtool_osx ${SOURCE_FILES})

## Link libraries
find_package(Threads REQUIRED)
target_link_libraries(benchtool_osx ${CMAKE_THREAD_LIBS_INIT})

find_library(OPENMP iomp5)
find_library(IOKit_LIBRARY IOKit)

mark_as_advanced (IOKit_LIBRARY)
mark_as_advanced (Foundation_LIBRARY)