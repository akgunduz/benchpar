# Standard Makefile

SOURCE_PATH = src
EXEC_PATH = Debug
EXECUTABLE = benchtool
GCC_VERSION = 4.9
TOOLCHAIN_TYPE = toolchain_arm

CFLAGS = -g -std=c++11 -fopenmp -Isrc -Isrc/power -Isrc/app -Isrc/app/matrix -Isrc/app/scan -Isrc/app/conv
LDFLAGS = -fopenmp

OBJECTS_PATH = Objects

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    TOOLCHAIN_PATH = /home/akgunduz/Toolchains
    TOOLCHAIN_ARM_PREFIX = arm-linux-gnueabihf
else
	TOOLCHAIN_PATH = /Volumes/Toolchains
	TOOLCHAIN_ARM_PREFIX = arm-linux-gnueabihf
endif

FILES = \
		gpu.cpp \
		timer.cpp \
		power/power.cpp \
		power/power_smart_calc.cpp \
		power/power_smart_real.cpp \
		cpu.cpp \
		app/app.cpp \
		app/matrix/matrix.cpp \
		app/matrix/matrixfuncs.cpp \
		app/matrix/matrixapp.cpp \
		app/scan/scanapp.cpp \
		app/scan/scan.cpp \
		app/scan/scanfuncs.cpp \
		app/conv/convapp.cpp \
		app/conv/conv.cpp \
		app/conv/convfuncs.cpp \
		main.cpp

CMD_FILES = $(FILES) \
		power/hid_linux.cpp \
		power/power_msr.cpp \
		power/power_perf.cpp 

OSX_FILES = $(FILES) \
		power/hid_apple.cpp 

C15_FILES = $(FILES) \
		power/hid_libusb.cpp \
		power/power_ina.cpp



C15_CC = $(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/bin/$(TOOLCHAIN_ARM_PREFIX)-g++

C15_OBJECTS_PATH = Objects/c15

C15_CFLAGS = $(CFLAGS) -O3 -DC15 -march=armv7ve -mcpu=cortex-a15 -mfloat-abi=hard -mfpu=neon \
	-mvectorize-with-neon-quad -I$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/include \
	-I$(TOOLCHAIN_PATH)/libs/c15/$(GCC_VERSION)/include

C15_LDFLAGS = -L$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/lib \
	-L$(TOOLCHAIN_PATH)/libs/c15/$(GCC_VERSION)/lib $(LDFLAGS) -lOpenCL -lusb-1.0

C15_FILES_DIR = $(addprefix $(C15_OBJECTS_PATH)/, $(sort $(dir $(C15_FILES))))

C15_EXECUTABLE = $(EXECUTABLE)_c15

C15_SOURCES = $(addprefix $(SOURCE_PATH)/, $(C15_FILES))

C15_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(C15_OBJECTS_PATH)/%.o, $(C15_SOURCES))

C15_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(C15_OBJECTS_PATH)/%.d, $(C15_SOURCES))



CMD_CC = g++

CMD_OBJECTS_PATH = Objects/cmd

CMD_CFLAGS = $(CFLAGS) -O0 -mavx

CMD_LDFLAGS = -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu $(LDFLAGS) -lOpenCL -ludev

CMD_FILES_DIR = $(addprefix $(CMD_OBJECTS_PATH)/, $(sort $(dir $(CMD_FILES))))

CMD_EXECUTABLE = $(EXECUTABLE)_cmd

CMD_SOURCES = $(addprefix $(SOURCE_PATH)/, $(CMD_FILES))

CMD_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(CMD_OBJECTS_PATH)/%.o, $(CMD_SOURCES))

CMD_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(CMD_OBJECTS_PATH)/%.d, $(CMD_SOURCES))



OSX_CC = clang-omp++

OSX_OBJECTS_PATH = Objects/osx

OSX_CFLAGS = $(CFLAGS) -O3 -mavx

OSX_LDFLAGS = -L/usr/local/lib $(LDFLAGS) -framework OpenCL -framework IOKit -framework CoreFoundation

OSX_FILES_DIR = $(addprefix $(OSX_OBJECTS_PATH)/, $(sort $(dir $(OSX_FILES))))

OSX_EXECUTABLE = $(EXECUTABLE)_osx

OSX_SOURCES = $(addprefix $(SOURCE_PATH)/, $(OSX_FILES))

OSX_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(OSX_OBJECTS_PATH)/%.o, $(OSX_SOURCES))

OSX_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(OSX_OBJECTS_PATH)/%.d, $(OSX_SOURCES))



all: cmd


$(EXEC_PATH):
	@mkdir -p $@



$(OSX_OBJECTS_PATH):
	@mkdir -p $(OSX_FILES_DIR)

$(OSX_EXECUTABLE): $(OSX_OBJECTS)
	$(OSX_CC) -o $(EXEC_PATH)/$@ $^ $(OSX_LDFLAGS) 

$(OSX_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(OSX_CC) $(OSX_CFLAGS) -MM -MT $(OSX_OBJECTS_PATH)/$*.o $< >> $@

$(OSX_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(OSX_CC) $(OSX_CFLAGS) -c -o $@ $<

osx: $(OSX_OBJECTS_PATH) $(EXEC_PATH) $(OSX_EXECUTABLE) $(OSX_DEPENDENCIES)



$(CMD_OBJECTS_PATH):
	@mkdir -p $(CMD_FILES_DIR)

$(CMD_EXECUTABLE): $(CMD_OBJECTS)
	$(CMD_CC) -o $(EXEC_PATH)/$@ $^ $(CMD_LDFLAGS) 

$(CMD_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(CMD_CC) $(CMD_CFLAGS) -MM -MT $(CMD_OBJECTS_PATH)/$*.o $< >> $@

$(CMD_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(CMD_CC) $(CMD_CFLAGS) -c -o $@ $<

cmd: $(CMD_OBJECTS_PATH) $(EXEC_PATH) $(CMD_EXECUTABLE) $(CMD_DEPENDENCIES)



$(C15_OBJECTS_PATH):
	@mkdir -p $(C15_FILES_DIR)

$(C15_EXECUTABLE): $(C15_OBJECTS)
	$(C15_CC) -o $(EXEC_PATH)/$@ $^ $(C15_LDFLAGS) 

$(C15_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(C15_CC) $(C15_CFLAGS) -MM -MT $(C15_OBJECTS_PATH)/$*.o $< >> $@

$(C15_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(C15_CC) $(C15_CFLAGS) -c -o $@ $<

c15: $(C15_OBJECTS_PATH) $(EXEC_PATH) $(C15_EXECUTABLE) $(C15_DEPENDENCIES)


install:

clean:
	-rm -f -R $(OBJECTS_PATH)
