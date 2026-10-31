# Plain-make fallback for environments without CMake. Same flags, same defects.
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -g -w -fno-stack-protector -Iinclude -Isrc
LDFLAGS  ?= -no-pie -Wl,-z,norelro
SRC      := $(wildcard src/*.cpp src/*/*.cpp) $(wildcard third_party/*/*.c)
CORPUS   := tools/replay-corpus.sh
BIN      := build/vulnshop

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $@ $(LDFLAGS)

# The target nobody runs. CR4 puts it in the pipeline.
asan:
	@mkdir -p build
	$(CXX) -std=c++17 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
	       -fno-omit-frame-pointer -Iinclude -Isrc $(SRC) -o build/vulnshop-asan

run: $(BIN)
	./$(BIN) config/sandbox.ini

clean:
	rm -rf build
.PHONY: all asan run clean
