run : build
	./build/sbox-ga

build:
	cmake -S . -B build
	cd build && make

clean:
	rm -rf build/*

.PHONY: build clean run
