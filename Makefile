all: test

build: clean
	mkdir build && cd build && cmake .. && make

32: clean32
	mkdir lib && cd lib && CFLAGS='-m32' cmake .. && make && make test

clean32:
	rm -rf lib32

64: clean64
	mkdir lib64 && cd lib64 && CFLAGS='-m64' cmake .. && make && make test

universal: clean
	mkdir build && cd build && cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64;i386" && make && make test

clean64:
	rm -rf lib64

test: build
	cd build && make test

clean:
	rm -rf build lib lib64
