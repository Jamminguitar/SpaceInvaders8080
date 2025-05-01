CC=gcc

SRC_DIR=src
BUILD_DIR=build

.PHONY: all disassembler clean always

all: disassembler

disassembler: $(BUILD_DIR)/disassembler

$(BUILD_DIR)/disassembler: always
	mkdir -p $(BUILD_DIR)/disassembler
	$(CC) -g -o $(BUILD_DIR)/disassembler/disassembler $(SRC_DIR)/disassembler.c



always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*