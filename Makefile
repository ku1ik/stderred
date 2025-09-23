ARCH := $(shell uname -m)

ifeq ($(ARCH), x86_64)
    CFLAGS := '-m64'
else ifeq ($(ARCH), i686)
    CFLAGS := '-m32'
else ifeq ($(ARCH), aarch64)
    CFLAGS := '-mabi=lp64'
else
    $(error Unsupported architecture: $(ARCH))
endif

all: build

build:
	mkdir build && cd build && cmake ../src && make

32: clean32
	mkdir lib && cd lib && CFLAGS=$(CFLAGS) cmake ../src && make && make test

clean32:
	rm -rf lib

64: clean64
	mkdir lib64 && cd lib64 && CFLAGS=$(CFLAGS) cmake ../src && make && make test

universal: clean
	mkdir build && cd build && cmake ../src -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64e" && make && make test

clean64:
	rm -rf lib64

test: build
	cd build && make test

install: build
	cd build && make install

clean: clean32 clean64
	rm -rf build

dist_prepare: test
	mkdir -p usr/share/doc/stderred && cp README.md usr/share/doc/stderred/

package_deb: dist_prepare
	rm -f *.deb
	mkdir -p usr/lib && cp build/libstderred.so usr/lib/
	fpm -s dir -t deb -n stderred -v `git tag | grep v | cut -d 'v' -f 2 | sort -nr | head -n 1` --license MIT --vendor 'Marcin Kulik' -m 'Marcin Kulik <marcin.kulik+stderred@gmail.com>' --description "stderr in red" --url https://github.com/sickill/stderred usr/bin/stderred usr/lib/libstderred.so usr/share/stderred/stderred.sh usr/share/doc/stderred/README.md

package_rpm_64: dist_prepare
	rm -f *.rpm
	mkdir -p usr/lib64 && cp build/libstderred.so usr/lib64/
	fpm -s dir -t rpm -n stderred -v `git tag | grep v | cut -d 'v' -f 2 | sort -nr | head -n 1` --license MIT --vendor 'Marcin Kulik' -m 'Marcin Kulik <marcin.kulik+stderred@gmail.com>' --description "stderr in red" --url https://github.com/sickill/stderred usr/bin/stderred usr/lib64/libstderred.so usr/share/stderred/stderred.sh usr/share/doc/stderred/README.md
