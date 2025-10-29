run : build
	./build/sbox-ga

build:
	cmake -S . -B build -DC‐MAKE_BUILD_TYPE=Release
	cd build && make

clean:
	rm -rf build/*

.PHONY: build clean run
