stderred.so: hack_write.c
	gcc hack_write.c -Wall -ldl -fPIC -shared -o stderred.so
