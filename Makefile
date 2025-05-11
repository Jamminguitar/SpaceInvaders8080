CC=gcc

SRC_DIR=src
BUILD_DIR=build

.PHONY: all debug disassembler emulator clean always

all: disassembler emulator

disassembler: $(BUILD_DIR)/disassembler

$(BUILD_DIR)/disassembler: always
	mkdir -p $(BUILD_DIR)/disassembler
	$(CC) -g -o $(BUILD_DIR)/disassembler/disassembler $(SRC_DIR)/disassembler.c

emulator: $(BUILD_DIR)/emulator

$(BUILD_DIR)/emulator: always
	mkdir -p $(BUILD_DIR)/emulator
	$(CC) -g -o $(BUILD_DIR)/emulator/emulator $(SRC_DIR)/emulator.c

always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*
