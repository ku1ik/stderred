lib/stderred.so: stderred.c
	gcc stderred.c -D_GNU_SOURCE -Wall -ldl -fPIC -m32 -shared -o lib/stderred.so

lib64/stderred.so: stderred.c
	gcc stderred.c -D_GNU_SOURCE -Wall -ldl -fPIC -m64 -shared -o lib64/stderred.so

both: lib/stderred.so lib64/stderred.so

mac: both
	lipo -create lib/stderred.so lib64/stderred.so -output lib/libstderred.dylib

test: test.c
	gcc -o test test.c

clean:
	rm -fv lib*/*.so
