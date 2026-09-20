BUILD := build
ASM_FILES := $(wildcard *.asm)
GB_FILES := $(ASM_FILES:%.asm=$(BUILD)/%.gb)

$(BUILD)/%.o: %.asm
	mkdir -p $(BUILD)
	rgbasm -o $@ $<

$(BUILD)/%.gb: $(BUILD)/%.o
	rgblink -o $@ $<
# 	rgbfix -v -p 0 $@

build: $(GB_FILES)
