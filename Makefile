lib/stderred.so: stderred.c
	gcc stderred.c -D_GNU_SOURCE -Wall -fPIC -m32 -shared -o lib/stderred.so

lib64/stderred.so: stderred.c
	gcc stderred.c -D_GNU_SOURCE -Wall -fPIC -m64 -shared -o lib64/stderred.so

both: lib/stderred.so lib64/stderred.so

test: test.c
	gcc -o test test.c

clean:
	rm -fv lib*/*.so
