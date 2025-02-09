
HALIDE_ROOT ?= $(HOME)/install/Halide
HALIDE_BUILD_DIR ?= $(HALIDE_ROOT)/build
HALIDE_LIB_DIR ?= $(HALIDE_BUILD_DIR)/src
HALIDE_INC_DIR ?= $(HALIDE_BUILD_DIR)/include

# Compiler settings
CXX=g++
CXXFLAGS=-std=c++17

all:
	$(CXX) src/main.cpp src/automata.cpp -g -I $(HALIDE_INC_DIR) \
		-L $(HALIDE_LIB_DIR) \
		-lHalide -lpthread -ldl \
		-o gol $(CXXFLAGS)
print-vars:
	@echo "HALIDE_ROOT: $(HALIDE_ROOT)"
	@echo "HALIDE_BUILD_DIR: $(HALIDE_BUILD_DIR)"
	@echo "HALIDE_LIB_DIR: $(HALIDE_LIB_DIR)"
	@echo "HALIDE_INC_DIR: $(HALIDE_INC_DIR)"

.PHONY: all check_dirs print-vars clean

clean:
	rm -f lesson_01
