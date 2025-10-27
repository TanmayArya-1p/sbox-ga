build:
	cmake -S . -B build
	cd build && make

clean:
	rm -rf build/*

.PHONY: build clean
