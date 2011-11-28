stderred.so: write.c
	gcc write.c -D_GNU_SOURCE -Wall -ldl -fPIC -shared -o stderred.so
