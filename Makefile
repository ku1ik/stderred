stderred.so: write.c
	gcc write.c -Wall -ldl -fPIC -shared -o stderred.so
