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
	TOOLCHAIN_PATH = /Users/akgunduz/Toolchains
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
		app/function.cpp \
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

ARM_FILES = $(FILES) \
		power/hid_libusb.cpp \
		power/power_ina.cpp

ARM_CC = $(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/bin/$(TOOLCHAIN_ARM_PREFIX)-g++

ARM_OBJECTS_PATH = Objects/arm

ARM_CFLAGS = $(CFLAGS) -O3 -D__ARM__ -mfloat-abi=hard -mfpu=neon \
	-mvectorize-with-neon-quad -I$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/include \
	-I$(TOOLCHAIN_PATH)/libs/arm/$(GCC_VERSION)/include

ARM_LDFLAGS = -L$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/lib \
	-L$(TOOLCHAIN_PATH)/libs/arm/$(GCC_VERSION)/lib $(LDFLAGS) -lusb-1.0

ARM_FILES_DIR = $(addprefix $(ARM_OBJECTS_PATH)/, $(sort $(dir $(ARM_FILES))))

ARM_EXECUTABLE = $(EXECUTABLE)_arm

ARM_SOURCES = $(addprefix $(SOURCE_PATH)/, $(ARM_FILES))

ARM_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(ARM_OBJECTS_PATH)/%.o, $(ARM_SOURCES))

ARM_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(ARM_OBJECTS_PATH)/%.d, $(ARM_SOURCES))


PI_CC = $(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION).pi/bin/$(TOOLCHAIN_ARM_PREFIX)-g++

PI_OBJECTS_PATH = Objects/pi

PI_CFLAGS = $(CFLAGS) -O2 -D__ARM__ -march=armv6zk -mtune=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp -marm \
	-I$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION).pi/$(TOOLCHAIN_ARM_PREFIX)/include \
	-I$(TOOLCHAIN_PATH)/libs/pi/$(GCC_VERSION)/include

PI_LDFLAGS = -L$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION).pi/$(TOOLCHAIN_ARM_PREFIX)/lib \
	-L$(TOOLCHAIN_PATH)/libs/pi/$(GCC_VERSION)/lib $(LDFLAGS) -lusb-1.0 -lrt

PI_FILES_DIR = $(addprefix $(PI_OBJECTS_PATH)/, $(sort $(dir $(ARM_FILES))))

PI_EXECUTABLE = $(EXECUTABLE)_pi

PI_SOURCES = $(addprefix $(SOURCE_PATH)/, $(ARM_FILES))

PI_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(PI_OBJECTS_PATH)/%.o, $(PI_SOURCES))

PI_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(PI_OBJECTS_PATH)/%.d, $(PI_SOURCES))


C7_CC = $(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/bin/$(TOOLCHAIN_ARM_PREFIX)-g++

C7_OBJECTS_PATH = Objects/c7

C7_CFLAGS = $(CFLAGS) -O3 -D__ARM__ -march=armv7-a -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 \
	-mvectorize-with-neon-quad -I$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/include \
	-I$(TOOLCHAIN_PATH)/libs/c7/$(GCC_VERSION)/include

C7_LDFLAGS = -L$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/lib \
	-L$(TOOLCHAIN_PATH)/libs/c7/$(GCC_VERSION)/lib $(LDFLAGS) -lusb-1.0

C7_FILES_DIR = $(addprefix $(C7_OBJECTS_PATH)/, $(sort $(dir $(ARM_FILES))))

C7_EXECUTABLE = $(EXECUTABLE)_c7

C7_SOURCES = $(addprefix $(SOURCE_PATH)/, $(ARM_FILES))

C7_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(C7_OBJECTS_PATH)/%.o, $(C7_SOURCES))

C7_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(C7_OBJECTS_PATH)/%.d, $(C7_SOURCES))


C8_CC = $(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/bin/$(TOOLCHAIN_ARM_PREFIX)-g++

C8_OBJECTS_PATH = Objects/c8

C8_CFLAGS = $(CFLAGS) -O3 -D__ARM__ -march=armv7-a -mcpu=cortex-a8 -mfloat-abi=hard -mfpu=neon \
	-mvectorize-with-neon-quad -I$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/include \
	-I$(TOOLCHAIN_PATH)/libs/c8/$(GCC_VERSION)/include

C8_LDFLAGS = -L$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/lib \
	-L$(TOOLCHAIN_PATH)/libs/c8/$(GCC_VERSION)/lib $(LDFLAGS) -lusb-1.0

C8_FILES_DIR = $(addprefix $(C8_OBJECTS_PATH)/, $(sort $(dir $(ARM_FILES))))

C8_EXECUTABLE = $(EXECUTABLE)_c8

C8_SOURCES = $(addprefix $(SOURCE_PATH)/, $(ARM_FILES))

C8_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(C8_OBJECTS_PATH)/%.o, $(C8_SOURCES))

C8_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(C8_OBJECTS_PATH)/%.d, $(C8_SOURCES))


C9_CC = $(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/bin/$(TOOLCHAIN_ARM_PREFIX)-g++

C9_OBJECTS_PATH = Objects/c9

C9_CFLAGS = $(CFLAGS) -O3 -D__ARM__ -march=armv7-a -mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon \
	-mvectorize-with-neon-quad -I$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/include \
	-I$(TOOLCHAIN_PATH)/libs/c9/$(GCC_VERSION)/include

C9_LDFLAGS = -L$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/lib \
	-L$(TOOLCHAIN_PATH)/libs/c9/$(GCC_VERSION)/lib $(LDFLAGS) -lusb-1.0

C9_FILES_DIR = $(addprefix $(C9_OBJECTS_PATH)/, $(sort $(dir $(ARM_FILES))))

C9_EXECUTABLE = $(EXECUTABLE)_c9

C9_SOURCES = $(addprefix $(SOURCE_PATH)/, $(ARM_FILES))

C9_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(C9_OBJECTS_PATH)/%.o, $(C9_SOURCES))

C9_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(C9_OBJECTS_PATH)/%.d, $(C9_SOURCES))



C15_CC = $(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/bin/$(TOOLCHAIN_ARM_PREFIX)-g++

C15_OBJECTS_PATH = Objects/c15

C15_CFLAGS = $(CFLAGS) -O3 -D__OPENCL__ -D__ARM__ -march=armv7ve -mcpu=cortex-a15 -mfloat-abi=hard -mfpu=neon \
	-mvectorize-with-neon-quad -I$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/include \
	-I$(TOOLCHAIN_PATH)/libs/c15/$(GCC_VERSION)/include

C15_LDFLAGS = -L$(TOOLCHAIN_PATH)/$(TOOLCHAIN_TYPE)/$(GCC_VERSION)/$(TOOLCHAIN_ARM_PREFIX)/lib \
	-L$(TOOLCHAIN_PATH)/libs/c15/$(GCC_VERSION)/lib $(LDFLAGS) -lOpenCL -lusb-1.0

C15_FILES_DIR = $(addprefix $(C15_OBJECTS_PATH)/, $(sort $(dir $(ARM_FILES))))

C15_EXECUTABLE = $(EXECUTABLE)_c15

C15_SOURCES = $(addprefix $(SOURCE_PATH)/, $(ARM_FILES))

C15_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(C15_OBJECTS_PATH)/%.o, $(C15_SOURCES))

C15_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(C15_OBJECTS_PATH)/%.d, $(C15_SOURCES))



CMD_CC = g++

CMD_OBJECTS_PATH = Objects/cmd

CMD_CFLAGS = $(CFLAGS) -O3 -march=native -D__OPENCL__

CMD_LDFLAGS = -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu $(LDFLAGS) -lOpenCL -ludev

CMD_FILES_DIR = $(addprefix $(CMD_OBJECTS_PATH)/, $(sort $(dir $(CMD_FILES))))

CMD_EXECUTABLE = $(EXECUTABLE)_cmd

CMD_SOURCES = $(addprefix $(SOURCE_PATH)/, $(CMD_FILES))

CMD_OBJECTS = $(patsubst $(SOURCE_PATH)/%.cpp, $(CMD_OBJECTS_PATH)/%.o, $(CMD_SOURCES))

CMD_DEPENDENCIES = $(patsubst $(SOURCE_PATH)/%.cpp, $(CMD_OBJECTS_PATH)/%.d, $(CMD_SOURCES))



OSX_CC = clang-omp++

OSX_OBJECTS_PATH = Objects/osx

OSX_CFLAGS = $(CFLAGS) -O3 -march=native -D__OPENCL__

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
	
	
	
$(ARM_OBJECTS_PATH):
	@mkdir -p $(ARM_FILES_DIR)

$(ARM_EXECUTABLE): $(ARM_OBJECTS)
	$(ARM_CC) -o $(EXEC_PATH)/$@ $^ $(ARM_LDFLAGS) 

$(ARM_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(ARM_CC) $(ARM_CFLAGS) -MM -MT $(ARM_OBJECTS_PATH)/$*.o $< >> $@

$(ARM_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(ARM_CC) $(ARM_CFLAGS) -c -o $@ $<

arm: $(ARM_OBJECTS_PATH) $(EXEC_PATH) $(ARM_EXECUTABLE) $(ARM_DEPENDENCIES)


$(PI_OBJECTS_PATH):
	@mkdir -p $(PI_FILES_DIR)

$(PI_EXECUTABLE): $(PI_OBJECTS)
	$(PI_CC) -o $(EXEC_PATH)/$@ $^ $(PI_LDFLAGS) 

$(PI_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(PI_CC) $(PI_CFLAGS) -MM -MT $(PI_OBJECTS_PATH)/$*.o $< >> $@

$(PI_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(PI_CC) $(PI_CFLAGS) -c -o $@ $<

pi: $(PI_OBJECTS_PATH) $(EXEC_PATH) $(PI_EXECUTABLE) $(PI_DEPENDENCIES)


$(C7_OBJECTS_PATH):
	@mkdir -p $(C7_FILES_DIR)

$(C7_EXECUTABLE): $(C7_OBJECTS)
	$(C7_CC) -o $(EXEC_PATH)/$@ $^ $(C7_LDFLAGS) 

$(C7_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(C7_CC) $(C7_CFLAGS) -MM -MT $(C7_OBJECTS_PATH)/$*.o $< >> $@

$(C7_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(C7_CC) $(C7_CFLAGS) -c -o $@ $<

c7: $(C7_OBJECTS_PATH) $(EXEC_PATH) $(C7_EXECUTABLE) $(C7_DEPENDENCIES)


$(C8_OBJECTS_PATH):
	@mkdir -p $(C8_FILES_DIR)

$(C8_EXECUTABLE): $(C8_OBJECTS)
	$(C8_CC) -o $(EXEC_PATH)/$@ $^ $(C8_LDFLAGS) 

$(C8_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(C8_CC) $(C8_CFLAGS) -MM -MT $(C8_OBJECTS_PATH)/$*.o $< >> $@

$(C8_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(C8_CC) $(C8_CFLAGS) -c -o $@ $<

c8: $(C8_OBJECTS_PATH) $(EXEC_PATH) $(C8_EXECUTABLE) $(C8_DEPENDENCIES)


$(C9_OBJECTS_PATH):
	@mkdir -p $(C9_FILES_DIR)

$(C9_EXECUTABLE): $(C9_OBJECTS)
	$(C9_CC) -o $(EXEC_PATH)/$@ $^ $(C9_LDFLAGS) 

$(C9_OBJECTS_PATH)/%.d: $(SOURCE_PATH)/%.cpp
	$(C9_CC) $(C9_CFLAGS) -MM -MT $(C9_OBJECTS_PATH)/$*.o $< >> $@

$(C9_OBJECTS_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	$(C9_CC) $(C9_CFLAGS) -c -o $@ $<

c9: $(C9_OBJECTS_PATH) $(EXEC_PATH) $(C9_EXECUTABLE) $(C9_DEPENDENCIES)



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
