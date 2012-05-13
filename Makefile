pkg_name = stderred
version := $(shell git tag | grep 'v' | cut -d 'v' -f 2 | sort -nr | head -n 1)
description = 'stderr in red'
license = MIT
vendor = 'Marcin Kulik'
maintainer = 'Marcin Kulik <marcin.kulik+stderred@gmail.com>'
url = https://github.com/sickill/stderred

all: test

build: clean
	mkdir build && cd build && cmake ../src && make

32: clean32
	mkdir lib && cd lib && CFLAGS='-m32' cmake ../src && make && make test

clean32:
	rm -rf lib

64: clean64
	mkdir lib64 && cd lib64 && CFLAGS='-m64' cmake ../src && make && make test

universal: clean
	mkdir build && cd build && cmake ../src -DCMAKE_OSX_ARCHITECTURES="x86_64;i386" && make && make test

clean64:
	rm -rf lib64

test: build
	cd build && make test

clean:
	rm -rf build lib lib64

dist_clean:
	rm -rf dist

dist_prepare: dist_clean
	mkdir -p dist/usr/bin dist/usr/share/stderred dist/usr/share/doc/stderred-$(version)
	cp usr/bin/stderred dist/usr/bin/
	cp usr/share/stderred/stderred.sh dist/usr/share/stderred/
	cp README.md dist/usr/share/doc/stderred-$(version)/

package_deb_32: 32 dist_prepare
	rm -f *386.deb
	mkdir -p dist/usr/lib
	cp lib/libstderred.so dist/usr/lib/
	fpm -s dir -t deb -n $(pkg_name) -v $(version) -a i386 --license $(license) --vendor $(vendor) -m $(maintainer) --description $(description) --url $(url) -C dist usr

package_deb_64: 64 dist_prepare
	rm -f *amd64.deb
	mkdir -p dist/usr/lib
	cp lib64/libstderred.so dist/usr/lib/
	fpm -s dir -t deb -n $(pkg_name) -v $(version) -a amd64 --license $(license) --vendor $(vendor) -m $(maintainer) --description $(description) --url $(url) -C dist usr

package_rpm_32: 32 dist_prepare
	rm -f *i686.rpm
	mkdir -p dist/usr/lib
	cp lib/libstderred.so dist/usr/lib/
	fpm -s dir -t rpm -n $(pkg_name) -v $(version) -a i686 --license $(license) --vendor $(vendor) -m $(maintainer) --description $(description) --url $(url) -C dist usr

package_rpm_64: 32 64 dist_prepare
	rm -f *x86_64.rpm
	mkdir -p dist/usr/lib
	mkdir -p dist/usr/lib64
	cp lib/libstderred.so dist/usr/lib/
	cp lib64/libstderred.so dist/usr/lib64/
	fpm -s dir -t rpm -n $(pkg_name) -v $(version) -a x86_64 --license $(license) --vendor $(vendor) -m $(maintainer) --description $(description) --url $(url) -C dist usr
