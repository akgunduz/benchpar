INCLUDE(CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

#set(TOOLCHAIN_DIR  /opt/toolchains)
#set(TOOLCHAIN_TYPE x86_64-siemens-linux-gnu)
#set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/${TOOLCHAIN_TYPE}/bin/${TOOLCHAIN_TYPE}-gcc)
#set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/${TOOLCHAIN_TYPE}/bin/${TOOLCHAIN_TYPE}-gcc)
#set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/${TOOLCHAIN_TYPE}/)
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

set(CMAKE_CXX_COMPILER /usr/local/bin/clang-omp++)

set(CMAKE_EXE_LINKER_FLAGS "-framework OpenCL -framework IOKit -framework CoreFoundation")

add_definitions(-D__OPENCL__ -D__APPLE__ -D__IDE__)

include_directories(/usr/local/include/libiomp)

## Link libraries
find_package(Threads REQUIRED)
find_library(OPENMP iomp5)
find_library(IOKit_LIBRARY IOKit)

mark_as_advanced (IOKit_LIBRARY)
mark_as_advanced (Foundation_LIBRARY)