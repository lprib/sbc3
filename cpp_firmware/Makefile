main:
	cd build && ninja -j10

gen:
	mkdir -p build
	cd build && cmake -S.. -B. -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/gnu-arm-toolchain.cmake -DCMAKE_BUILD_TYPE=Release

gen_m:
	mkdir -p build
	cd build && cmake -S.. -B. -D "CMAKE_TOOLCHAIN_FILE=../cmake/gnu-arm-toolchain.cmake"

clean:
	rm -rf build

.PHONY: all clean main gen gen_m