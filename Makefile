# Define compiler
CC_x86 = gcc
CC_arm = arm-linux-gnueabihf-gcc

CFLAGS = -Wall -g

# Define output directory
BUILD_x86 = build/x86
BUILD_arm = build/arm

# Define source file and object file
SOURCE = testgcc.c
OBJECT = $(SOURCE:.C=.O)

# Dedine executable file
EXECUTABLE_x86 = myprogram_x86
EXECUTABLE_arm = myprogram_arm

# Target
.PHONY: all clean x86 arm 
all: x86 arm

# Rule:x86
x86: $(BUILD_x86)/$(EXECUTABLE_x86)

# Rule:arm
arm: $(BUILD_arm)/$(EXECUTABLE_arm)

# Rule: .c to x86
$(BUILD_x86)/$(EXECUTABLE_x86): $(BUILD_x86)/$(OBJECT)
	mkdir -p $(@D)
	$(CC_arm) $(CFLAGS) -o $@ $^

# Rule: .c to arm
$(BUILD_arm)/$(EXECUTABLE_arm): $(BUILD_arm)/$(OBJECT)
	mkdir -p $(@D)
	$(CC_arm) $(CFLAGS) -o $@ $^

# Rule: .c to .o
$(BUILD_x86)/%.o: %.c
	mkdir -p $(@D)
	$(CC_x86) $(CFLAGS) -c -o $@ $<

$(BUILD_arm)/%.o: %.c
	mkdir -p $(@D)
	$(CC_arm) $(CFLAGS) -c -o $@ $<

# Clean files
clean:
	rm -rf build/*.o build/x86 build/arm

# Help 
help:
	@echo "Makefile usage:"

