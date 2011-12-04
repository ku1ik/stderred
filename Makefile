all: lib/stderred.so lib64/stderred.so

lib/stderred.so: write.c
	gcc write.c -D_GNU_SOURCE -Wall -ldl -fPIC -m32 -shared -o lib/stderred.so

lib64/stderred.so: write.c
	gcc write.c -D_GNU_SOURCE -Wall -ldl -fPIC -m64 -shared -o lib64/stderred.so
