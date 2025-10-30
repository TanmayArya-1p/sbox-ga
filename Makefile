build:
	cmake -S . -B build -DC‐MAKE_BUILD_TYPE=Release
	cd build && make

run : build
	./build/sbox-ga $(ARGS)

clean:
	rm -rf build/*

.PHONY: build clean run
