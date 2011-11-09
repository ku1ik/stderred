stderred.so: hack_write.c
	gcc hack_write.c -ldl -fPIC -shared -o stderred.so
