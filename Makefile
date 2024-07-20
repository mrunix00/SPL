BUILD_DIR = build
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1

all: fetch-submodules configure build test

fetch-submodules:
	@git submodule update --init --recursive

configure: fetch-submodules
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS)

build: configure
	@cmake --build $(BUILD_DIR) -- $(MAKEFLAGS)

test: build
	@$(BUILD_DIR)/tests

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all fetch-submodules configure build test clean
