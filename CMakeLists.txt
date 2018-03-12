cmake_minimum_required(VERSION 3.3)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(APPLE)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain-apple.cmake")
    elseif(UNIX)
        if (DEFINED ARM)
            set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain-linux-arm.cmake")
        else()
            set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain-linux-x86.cmake")
        endif()
    elseif(WIN32)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain-windows-x86.cmake")
    else()
        message(FATAL_ERROR "Your platform does not supported!!!")
    endif()
endif()


project(BenchPar)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -mavx -fopenmp -D__OPENCL__")

set(COMMON_FILES
    src/app/conv/conv.cpp
    src/app/conv/convapp.cpp
    src/app/conv/convfuncs.cpp
    src/app/matrix/matrix.cpp
    src/app/matrix/matrixapp.cpp
    src/app/matrix/matrixfuncs.cpp
    src/app/scan/scan.cpp
    src/app/scan/scanapp.cpp
    src/app/scan/scanfuncs.cpp
    src/app/app.cpp
    src/app/app.h
    src/app/function.cpp
    src/power/power.cpp
    src/power/power_ina.cpp
    src/power/power_msr.cpp
    src/power/power_perf.cpp
    src/power/power_smart_calc.cpp
    src/power/power_smart_real.cpp
    src/cpu.cpp
    src/gpu.cpp
    src/main.cpp
    src/timer.cpp
        )

set(APPLE_FILES
        ${COMMON_FILES}
        src/power/hid_apple.cpp
        )

include_directories(src)
include_directories(src/app)
include_directories(src/app/conv)
include_directories(src/app/matrix)
include_directories(src/app/scan)
include_directories(src/power)

add_executable(benchpar ${COMMON_FILES})

target_link_libraries(benchpar OpenCL hidapi-libusb)
